#include "transmitter.h"
#include "rtp/rtpenc_h264.h"
#include "pdb.h"
#include "video_codec.h"
#include "debug.h"
#include "tv.h"
#include <stdlib.h>

void *transmitter_encoder_routine(void *arg);
void *transmitter_rtpenc_routine(void *arg);
int transmitter_init_threads(struct participant_data *participant);
void *transmitter_master_routine(void *arg);

pthread_t MASTER_THREAD;
int RUN = 1;

void *transmitter_encoder_routine(void *arg)
{
    debug_msg(" entering transmitter encoder routine");
    struct participant_data *participant = (struct participant_data *)arg;

    participant->encoder->sc = (struct compress_state *) calloc(
        2, sizeof(struct compress_state *));

    compress_init("libavcodec:codec=H.264", &participant->encoder->sc);


    while (1) {
        int i = participant->encoder->index;
        struct video_frame frame;
        frame.tiles[0].data = participant->frame;
        frame.tiles[0].data_len = participant->frame_length;

        struct video_frame *tx_frame;
        tx_frame = compress_frame(participant->encoder->sc, &frame, i);

        participant->encoder->frame = tx_frame;

        sem_post(&participant->encoder->output_sem);    
        
        i = (i + 1)%2;
        participant->encoder->index = i;
    }

    int ret = 0;
    pthread_exit((void *)&ret);
}

void *transmitter_rtpenc_routine(void *arg)
{
    struct participant_data *participant = (struct participant_data *)arg;
    struct rtp_session *session = participant->session;

    // TODO initialization
    struct pdb *participants = pdb_init();
    char *mcast_if = NULL;
    double rtcp_bw = 5 * 1024 * 1024; /* FIXME */
    int ttl = 255;
    int recv_port = 2004;
    void *rtp_recv_callback = NULL;

    struct rtp *rtp  = rtp_init_if(session->addr, mcast_if,
                                   recv_port, session->port, ttl,
                                   rtcp_bw, 0, rtp_recv_callback,
                                   (void *)participants, 0);

    while (1) {
        sem_wait(&participant->encoder->output_sem);
        tx_send_base_h264(vf_get_tile(participant->encoder->frame, 0),
                          rtp, get_local_mediatime(), 1, participant->codec,
                          participant->encoder->frame->fps,
                          participant->encoder->frame->interlacing, 0, 0);
    }   

    int ret = 0;
    pthread_exit((void *)&ret);
}

int transmitter_init_threads(struct participant_data *participant)
{
    struct encoder_th *encoder = participant->encoder;
    encoder = malloc(sizeof(struct encoder_th));
    if (encoder == NULL) {
        return -1;
    }
    struct rtpenc_th *rtpenc = encoder->rtpenc;
    rtpenc = malloc(sizeof(struct rtpenc_th));
    if (rtpenc == NULL) {
        return -1;
    }

    sem_init(&encoder->input_sem, 1, 0);
    sem_init(&encoder->output_sem, 1, 0);

    int ret = 0;
    ret = pthread_create(&encoder->thread, NULL,
                transmitter_encoder_routine, participant);
    if (ret < 0) {
        // TODO
    }
    ret = pthread_create(rtpenc->thread, NULL,
                transmitter_rtpenc_routine, participant);
    if (ret < 0) {
        // TODO
    }
    return 0;
}

void *transmitter_master_routine(void *arg)
{
    struct participant_list *list = (struct participant_list *)arg;

    struct participant_data *participant = list->first;
    while (participant != NULL) {
        transmitter_init_threads(participant);
        participant = participant->next;
    }

    while (RUN) {
        struct participant_data *ptc = list->first;
        while (ptc != NULL) {
            if (ptc->encoder != NULL) { // -> has a pair of threads
                if (ptc->next) { // -> has new data
                    // notify!
                    ptc->next = 0;
                    sem_post(&ptc->encoder->input_sem);
                }
            }
            ptc = ptc->next;
        }
    }

    int ret = 0;
    participant = list->first;
    while (participant != NULL) {
        ret |= pthread_join(participant->encoder->thread, NULL);
        participant = participant->next;
    }
    if (ret != 0) {
        ret = -1;
    }
    pthread_exit((void *)&ret);
}

int start_out_manager(struct participant_list *list, uint32_t port)
{
    UNUSED(port);
    int ret = pthread_create(&MASTER_THREAD, NULL, transmitter_master_routine, list);
    if (ret < 0) {
        debug_msg(" could not initiate the transmitter master thread");
    }
    return ret;
}

int stop_out_manager()
{
    RUN = 0;
    int ret = pthread_join(MASTER_THREAD, NULL);
    return ret;
}

int main(int argc, char **argv)
{
    UNUSED(argc);
    UNUSED(argv);

    while(1);

    return 0;
}