#include "config_unix.h"
#include "transmitter.h"
#include "rtp/rtp.h"
#include "pdb.h"
#include "video_codec.h"
#include "video_compress.h"
#include "module.h"
#include "debug.h"
#include "tv.h"

#include <stdlib.h>

#define DEFAULT_FPS 24
#define DEFAULT_RECV_PORT 12006 // just trying to not interfere with anything
#define DEFAULT_RTCP_BW 5 * 1024 * 1024 * 10
#define DEFAULT_TTL 255
#define DEFAULT_SEND_BUFFER_SIZE 1920 * 1080 * 4 * sizeof(char) * 10
#define PIXEL_FORMAT RGB
#define MTU 1300 // 1400

void *transmitter_rtp_routine(void *arg);
int transmitter_init_threads(struct participant_data *participant);
void *transmitter_master_routine(void *arg);

void transmitter_dummy_callback(struct rtp *session, rtp_event *e);
void transmitter_destroy_encoder_thread(encoder_thread_t **encoder);

int init_transmission_rtp(participant_data_t *participant);

pthread_t MASTER_THREAD;
int RUN = 1;
float WAIT_TIME;
float FRAMERATE;
sem_t FRAME_SEM;


int init_transmission_rtp(participant_data_t *participant)
{
    pthread_mutex_lock(&participant->lock);
    assert(participant->type == OUTPUT);
    assert(participant->protocol == RTP);

    int ret = pthread_create(&participant->rtp.thread, NULL,
                             transmitter_rtp_routine, participant);
    participant->rtp.run = TRUE;
    if (ret < 0) {
        error_msg("init_transmission_rtp: pthread_create error");
    }

    pthread_mutex_unlock(&participant->lock);
    return ret;
}

int init_transmission(participant_data_t *participant)
{
    pthread_mutex_lock(&participant->lock);
    
    if (participant->type != OUTPUT) {
        pthread_mutex_unlock(&participant->lock);
        return FALSE;
    }

    int ret = TRUE;

    if (participant->protocol == RTSP) {
        // TODO: implement RTSP transmission
        error_msg("init_transmission: transmission RTP support not ready yet");
        ret = FALSE;
    } else if (participant->protocol == RTP) {
        ret = init_transmission_rtp(participant);
    }

    pthread_mutex_unlock(&participant->lock);
    return ret;
}

int stop_transmission(participant_data_t *participant)
{
    // TODO
    pthread_mutex_lock(&participant->lock);

    pthread_mutex_unlock(&participant->lock);
}

void transmitter_dummy_callback(struct rtp *session, rtp_event *e)
{
    UNUSED(session);
    UNUSED(e);
}

void *transmitter_rtp_routine(void *arg)
{
    debug_msg(" transmitter rtpenc routine START\n");
    participant_data_t *participant = (struct participant_data *)arg;
    rtp_session_t *session = &participant->rtp;

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

    // Only one stream

    while (participant->rtp.run && participant->streams_count > 0) {

        stream_data_t *stream = participant->streams[0];
        assert(stream != NULL);
        encoder_thread_t *encoder = stream->encoder;
        assert(encoder != NULL);
        

        if (sem_wait(&encoder->output_sem) < 0) {
            break;
        }

        pthread_rwlock_rdlock(&stream->video.lock);

        /* implement something like this?
        pthread_mutex_lock(&encoder->lock);
        encoder->rtpenc->ready = 0;
        pthread_mutex_unlock(&encoder->lock);
        */

        gettimeofday(&curr_time, NULL);
        rtp_update(rtp, curr_time);
        timestamp = tv_diff(curr_time, start_time)*90000;
        rtp_send_ctrl(rtp, timestamp, 0, curr_time);

        tx_send_h264(tx_session, stream->encoder->frame, rtp, FRAMERATE);

        /*
        pthread_mutex_lock(&encoder->lock);
        encoder->rtpenc->ready = 1;
        pthread_mutex_unlock(&encoder->lock);
         */

        pthread_rwlock_unlock(&stream->video.lock);
    }   

    rtp_send_bye(rtp);
    rtp_done(rtp);
    module_done(CAST_MODULE(&tmod));
    
    pthread_exit(NULL);
}

void *transmitter_master_routine(void *arg)
{
    debug_msg("transmitter master routine START\n");
    struct participant_list *list = (struct participant_list *)arg;

    struct participant_data *participant = list->first;
    while (participant != NULL) {
        debug_msg("participant found, initializing its threads...\n");
        init_transmission(participant);
        debug_msg("participant threads initialized\n");
        participant = participant->next;
    }

    debug_msg("entering the master loop\n");
    while (RUN) {
        usleep(WAIT_TIME);
        
        pthread_rwlock_rdlock(&list->lock);
        participant_data_t *ptc = list->first;
        while (ptc != NULL) {
            int i = 0;
            while (i++ < ptc->streams_count) {
                stream_data_t *str = ptc->streams[i];
                if (str->encoder != NULL && str->encoder->run) {
                    sem_post(&str->encoder->input_sem);
                }
            }

            if (!RUN) {
                break;
            }
            ptc = ptc->next;
        }
        pthread_rwlock_unlock(&list->lock);
    }

    debug_msg(" terminating pairs of threads\n");
    pthread_rwlock_rdlock(&list->lock);
    participant = list->first;
    while (participant != NULL) {
        stop_transmission(participant);
        participant = participant->next;
    }
    pthread_rwlock_unlock(&list->lock);
    pthread_exit((void *)NULL);
}

int start_out_manager(participant_list_t *list, float framerate)
{
    RUN = 1;
    FRAMERATE = framerate;
    WAIT_TIME = (1.0/framerate) * 1000000;
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
    int ret = pthread_join(MASTER_THREAD, NULL);
    return ret;
}
