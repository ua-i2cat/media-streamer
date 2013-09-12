#include "config.h"
#include "transmitter.h"
#include "rtp/rtp.h"
#include "pdb.h"
#include "video_codec.h"
#include "video_compress.h"
#include "module.h"
#include "debug.h"
#include "tv.h"

#define DEFAULT_FPS 24
#define DEFAULT_RECV_PORT 12006 // just trying to not interfere with anything
#define DEFAULT_RTCP_BW 5 * 1024 * 1024 * 10
#define DEFAULT_TTL 255
#define DEFAULT_SEND_BUFFER_SIZE 1920 * 1080 * 4 * sizeof(char) * 10
#define PIXEL_FORMAT RGB
#define MTU 1300 // 1400

void *transmitter_encoder_routine(void *arg);
void *transmitter_rtpenc_routine(void *arg);
int transmitter_init_threads(struct participant_data *participant);
void *transmitter_master_routine(void *arg);

void transmitter_dummy_callback(struct rtp *session, rtp_event *e);
void transmitter_destroy_encoder_thread(encoder_thread_t **encoder);

pthread_t MASTER_THREAD;
int RUN = 1;
sem_t FRAME_SEM;

void notify_out_manager()
{
    sem_post(&FRAME_SEM);
}

void *transmitter_encoder_routine(void *arg)
{
    struct participant_data *participant = (struct participant_data *)arg;
    struct module cmod;

    encoder_thread_t *encoder = participant->proc.encoder;

    encoder->input_frame_length = vc_get_linesize(participant->width, PIXEL_FORMAT)*participant->height;
    encoder->input_frame = malloc(encoder->input_frame_length); // TODO error handling

    module_init_default(&cmod);

    compress_init(&cmod, "libavcodec:codec=H.264", &encoder->sc);

    struct video_frame *frame = vf_alloc(1);
    int width = participant->width;
    int height = participant->height;
    vf_get_tile(frame, 0)->width=width;
    vf_get_tile(frame, 0)->height=height;
    frame->color_spec = PIXEL_FORMAT;
    frame->fps = DEFAULT_FPS; // FIXME: if it's not set -> core dump.
    frame->interlacing=PROGRESSIVE;
    
    while (RUN) {
        sem_wait(&encoder->input_sem);
        if (!RUN) {
            break;
        }

        // Lock while compressing ( = reading the data buffer)
        pthread_mutex_lock(&participant->lock);
        
        frame->tiles[0].data = participant->frame;
        frame->tiles[0].data_len = participant->frame_length;
        struct video_frame *tx_frame;
        int i = encoder->index;

        tx_frame = compress_frame(encoder->sc, frame, i);

        pthread_mutex_unlock(&participant->lock);
        
        i = (i + 1)%2;
        encoder->index = i;
        
        while(1) {
            pthread_mutex_lock(&encoder->lock);     
            if (encoder->rtpenc->ready) {
                pthread_mutex_unlock(&encoder->lock);
                break;
            }
            pthread_mutex_unlock(&encoder->lock);
        }

        pthread_mutex_lock(&encoder->lock);     
        encoder->frame = tx_frame;
        pthread_mutex_unlock(&encoder->lock);
        
        sem_post(&encoder->output_sem);    
    }

    debug_msg(" encoder routine END\n");
    int ret = 0;
    module_done(CAST_MODULE(&cmod));
    free(encoder->input_frame);
    pthread_exit((void *)&ret);
}

void transmitter_dummy_callback(struct rtp *session, rtp_event *e)
{
    UNUSED(session);
    UNUSED(e);
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
    
    struct module tmod;
    struct tx *tx_session;
    
    module_init_default(&tmod);

    struct rtp *rtp  = rtp_init_if(session->addr, mcast_if,
                                   recv_port, session->port, ttl,
                                   rtcp_bw, 0, transmitter_dummy_callback,
                                   (void *)NULL, 0);
    
    rtp_set_option(rtp, RTP_OPT_WEAK_VALIDATION, 1);
    rtp_set_sdes(rtp, rtp_my_ssrc(rtp), RTCP_SDES_TOOL, PACKAGE_STRING, strlen(PACKAGE_STRING));
    rtp_set_send_buf(rtp, DEFAULT_SEND_BUFFER_SIZE);

    tx_session = tx_init_h264(&tmod, MTU, TX_MEDIA_VIDEO, NULL, NULL);
    
    struct timeval curr_time;
    struct timeval start_time;
    double timestamp;
    gettimeofday(&start_time, NULL);

    participant->proc.encoder->run = TRUE;

    while (RUN) {
        encoder_thread_t *encoder = participant->proc.encoder;
        sem_wait(&encoder->output_sem);
        if (!RUN) {
            break;
        }

        pthread_mutex_lock(&encoder->lock);
        encoder->rtpenc->ready = 0;
        pthread_mutex_unlock(&encoder->lock);

        pthread_mutex_lock(&encoder->lock);
        gettimeofday(&curr_time, NULL);
        rtp_update(rtp, curr_time);
        timestamp = tv_diff(curr_time, start_time)*90000;
        rtp_send_ctrl(rtp, timestamp, 0, curr_time);
        tx_send_h264(tx_session, encoder->frame, rtp);
        pthread_mutex_unlock(&encoder->lock);

        pthread_mutex_lock(&encoder->lock);
        encoder->rtpenc->ready = 1;
        pthread_mutex_unlock(&encoder->lock);
    }   

    rtp_send_bye(rtp);
    rtp_done(rtp);
    module_done(CAST_MODULE(&tmod));
    
    pthread_exit(NULL);
}


void transmitter_destroy_encoder_thread(encoder_thread_t **encoder)
{
    if (*encoder == NULL) {
        return;
    }

    if (encoder[0]->run != TRUE) {
        return;
    }

    sem_post(&encoder[0]->output_sem);
    sem_post(&encoder[0]->input_sem);
    
    // TODO: error control? reporting?
    pthread_join(encoder[0]->rtpenc->thread, NULL);
    pthread_join(encoder[0]->thread, NULL);

    sem_destroy(&encoder[0]->input_sem);
    sem_destroy(&encoder[0]->output_sem);

    pthread_mutex_destroy(&encoder[0]->lock);

    free(encoder[0]->rtpenc);
    free(encoder[0]);

    encoder[0] = NULL;

    //encoder->run = FALSE;
}

int transmitter_init_threads(struct participant_data *participant)
{
    participant->proc.encoder = malloc(sizeof(encoder_thread_t));
    encoder_thread_t *encoder = participant->proc.encoder;
    if (encoder == NULL) {
        error_msg(" unsuccessful malloc\n");
        return -1;
    }
    encoder->run = FALSE;
    encoder->rtpenc = malloc(sizeof(rtpenc_thread_t));
    rtpenc_thread_t *rtpenc = encoder->rtpenc;
    if (rtpenc == NULL) {
        error_msg(" unsuccessful malloc\n");
        return -1;
    }

    pthread_mutex_init(&encoder->lock, NULL);
    sem_init(&encoder->input_sem, 1, 0);
    sem_init(&encoder->output_sem, 1, 0);

    encoder->rtpenc->ready = 1;

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

    encoder->run = TRUE;

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
        sem_wait(&FRAME_SEM);
        if (!RUN) {
            break;
        }
        while (ptc != NULL) {
            if (ptc->proc.encoder != NULL) { // -> has a pair of threads
                if (ptc->new_frame) { // -> has new data
                    pthread_mutex_lock(&ptc->lock);
                    ptc->new_frame = 0;
                    sem_post(&ptc->proc.encoder->input_sem);
                    pthread_mutex_unlock(&ptc->lock);
                }
            } else {
                transmitter_init_threads(ptc);
            }
            ptc = ptc->next;
        }
    }

    debug_msg(" terminating pairs of threads\n");
    pthread_rwlock_rdlock(&list->lock);
    participant = list->first;
    while (participant != NULL) {
        transmitter_destroy_encoder_thread(&participant->proc.encoder);
        participant = participant->next;
    }
    pthread_rwlock_unlock(&list->lock);
    pthread_exit((void *)NULL);
}

int start_out_manager(participant_list_t *list)
{
    debug_msg("creating the master thread...\n");
    sem_init(&FRAME_SEM, 1, 0);
    int ret = pthread_create(&MASTER_THREAD, NULL, transmitter_master_routine, list);
    if (ret < 0) {
        error_msg("could not initiate the transmitter master thread\n");
    }
    return ret;
}

int stop_out_manager()
{
    RUN = 0;
    notify_out_manager();
    int ret = pthread_join(MASTER_THREAD, NULL);
    return ret;
}
