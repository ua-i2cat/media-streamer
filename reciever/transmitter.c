#include "transmitter.h"
#include "rtp/rtp.h"
#include "rtp/rtpenc_h264.h"
#include "pdb.h"
#include "video_codec.h"
#include "video_compress.h"
#include "debug.h"
#include "tv.h"

#define DEFAULT_FPS 24
#define DEFAULT_RECV_PORT 12006 // just trying to not interfere with anything
#define DEFAULT_RTCP_BW 5 * 1024 * 1024
#define DEFAULT_TTL 255
#define DEFAULT_SEND_BUFFER_SIZE 1024 * 56

void *transmitter_encoder_routine(void *arg);
void *transmitter_rtpenc_routine(void *arg);
int transmitter_init_threads(struct participant_data *participant);
void *transmitter_master_routine(void *arg);
void *transmitter_dummy_callback(void *arg);

pthread_t MASTER_THREAD;
int RUN = 1;

void *transmitter_encoder_routine(void *arg)
{
    struct participant_data *participant = (struct participant_data *)arg;

    encoder_thread_t *encoder = participant->proc.encoder;

    encoder->input_frame_length = vc_get_linesize(participant->width, UYVY)*participant->height;
    encoder->input_frame = malloc(encoder->input_frame_length); // TODO error handling

    compress_init("libavcodec:codec=H.264", &encoder->sc);

    struct video_frame *frame = vf_alloc(1);
    int width = participant->width;
    int height = participant->height;
    vf_get_tile(frame, 0)->width=width;
    vf_get_tile(frame, 0)->height=height;
    vf_get_tile(frame, 0)->linesize=vc_get_linesize(width, UYVY);
    frame->color_spec=UYVY;
    frame->fps = DEFAULT_FPS; // FIXME: if it's not set -> core dump.
    frame->interlacing=PROGRESSIVE;
    
    while (RUN) {
        sem_wait(&encoder->input_sem);
        if (!RUN) {
            break;
        }

        pthread_mutex_lock(&participant->lock);
        memcpy(encoder->input_frame, participant->frame, participant->frame_length);
        encoder->input_frame_length = participant->frame_length;
        pthread_mutex_unlock(&participant->lock);
        
        frame->tiles[0].data = encoder->input_frame;
        frame->tiles[0].data_len = encoder->input_frame_length;

        struct video_frame *tx_frame;
        int i = encoder->index;
        tx_frame = compress_frame(encoder->sc, frame, i);
        i = (i + 1)%2;
        encoder->index = i;
        encoder->frame = tx_frame;
        
        sem_post(&encoder->output_sem);    
    }

    debug_msg(" encoder routine END\n");
    int ret = 0;
    compress_done(encoder->sc);
    free(encoder->input_frame);
    pthread_exit((void *)&ret);
}

void *transmitter_dummy_callback(void *arg)
{
    UNUSED(arg);
    return (void *)NULL;
}

void *transmitter_rtpenc_routine(void *arg)
{
    debug_msg(" transmitter rtpenc routine START\n");
    struct participant_data *participant = (struct participant_data *)arg;
    struct rtp_session *session = participant->session;

    char *mcast_if = NULL;
    double rtcp_bw = DEFAULT_RTCP_BW;
    int ttl = DEFAULT_TTL;
    int recv_port = DEFAULT_RECV_PORT;

    struct rtp *rtp  = rtp_init_if(session->addr, mcast_if,
                                   recv_port, session->port, ttl,
                                   rtcp_bw, 0, transmitter_dummy_callback,
                                   (void *)NULL, 0);
    
    rtp_set_option(rtp, RTP_OPT_WEAK_VALIDATION, 1);
    rtp_set_sdes(rtp, rtp_my_ssrc(rtp), RTCP_SDES_TOOL, PACKAGE_STRING, strlen(PACKAGE_STRING));
    rtp_set_send_buf(rtp, DEFAULT_SEND_BUFFER_SIZE);

    tx_init();
    
    struct timeval curr_time;
    struct timeval start_time;
    double timestamp;
    gettimeofday(&start_time, NULL);

    while (RUN) {
        encoder_thread_t *encoder = participant->proc.encoder;
        sem_wait(&encoder->output_sem);
        if (!RUN) {
            break;
        }
        gettimeofday(&curr_time, NULL);
        rtp_update(rtp, curr_time);
        timestamp = tv_diff(curr_time, start_time)*90000;
        rtp_send_ctrl(rtp, timestamp, 0, curr_time);
        tx_send_base_h264(vf_get_tile(encoder->frame, 0),
                          rtp, get_local_mediatime(), 1, participant->codec,
                          encoder->frame->fps,
                          encoder->frame->interlacing, 0, 0);
    }   

    rtp_send_bye(rtp);
    rtp_done(rtp);
    pthread_exit(NULL);
}

int transmitter_init_threads(struct participant_data *participant)
{
    participant->proc.encoder = malloc(sizeof(encoder_thread_t));
    encoder_thread_t *encoder = participant->proc.encoder;
    if (encoder == NULL) {
        error_msg(" unsuccessful malloc\n");
        return -1;
    }
    encoder->rtpenc = malloc(sizeof(rtpenc_thread_t));
    rtpenc_thread_t *rtpenc = encoder->rtpenc;
    if (rtpenc == NULL) {
        error_msg(" unsuccessful malloc\n");
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
    ret = pthread_create(&rtpenc->thread, NULL,
                transmitter_rtpenc_routine, participant);
    if (ret < 0) {
        // TODO
    }

    return 0;
}

void *transmitter_master_routine(void *arg)
{
    debug_msg("transmitter master routine START\n");
    struct participant_list *list = (struct participant_list *)arg;

    struct participant_data *participant = list->first;
    while (participant != NULL) {
        debug_msg("participant found, initializing its threads...\n");
        transmitter_init_threads(participant);
        debug_msg("participant threads initialized\n");
        participant = participant->next;
    }

    debug_msg("entering the master loop\n");
    while (RUN) {
        struct participant_data *ptc = list->first;
        while (ptc != NULL) {
            if (ptc->proc.encoder != NULL) { // -> has a pair of threads
                if (ptc->new_frame) { // -> has new data
                    pthread_mutex_lock(&ptc->lock);
                    ptc->new_frame = 0;
                    sem_post(&ptc->proc.encoder->input_sem);
                    pthread_mutex_unlock(&ptc->lock);
                }
            }
            ptc = ptc->next;
        }
    }

    debug_msg(" terminating pairs of threads\n");
    int ret = 0;
    void *end;
    participant = list->first;
    while (participant != NULL) {
        sem_post(&participant->proc.encoder->output_sem);
        sem_post(&participant->proc.encoder->input_sem);
        
        ret += pthread_join(participant->proc.encoder->rtpenc->thread, &end);
        ret += pthread_join(participant->proc.encoder->thread, &end);

        sem_destroy(&participant->proc.encoder->input_sem);
        sem_destroy(&participant->proc.encoder->output_sem);

        free(participant->proc.encoder->rtpenc);
        free(participant->proc.encoder);

        participant = participant->next;
    }
    if (ret != 0) {
        ret = -1;
    }
    pthread_exit((void *)&ret);
}

int start_out_manager(participant_list_t *list)
{
    debug_msg("creating the master thread...\n");
    int ret = pthread_create(&MASTER_THREAD, NULL, transmitter_master_routine, list);
    if (ret < 0) {
        error_msg("could not initiate the transmitter master thread\n");
    }
    return ret;
}

int stop_out_manager()
{
    RUN = 0;
    int ret = pthread_join(MASTER_THREAD, NULL);
    return ret;
}
