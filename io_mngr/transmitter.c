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

void *transmitter_rtp_routine(void *arg);
int transmitter_init_threads(struct participant_data *participant);

void transmitter_dummy_callback(struct rtp *session, rtp_event *e);
void transmitter_destroy_encoder_thread(encoder_thread_t **encoder);

typedef struct {
    participant_data_t *participant;
    transmitter_t *transmitter;
} rtp_routine_params_t;

transmitter_t *init_transmitter(stream_list_t *stream_list, float fps)
{
    transmitter_t *transmitter = malloc(sizeof(transmitter_t));
    if (transmitter == NULL) {
        error_msg("init_transmitter: malloc error");
        return NULL;
    }

    transmitter->run = FALSE;
    
    if (fps <= 0.0) {
        transmitter->fps = DEFAULT_FPS;
    } else {
        transmitter->fps = fps;
    }
    transmitter->wait_time = (1000000.0/transmitter->fps);

    transmitter->recv_port = DEFAULT_RECV_PORT;
    transmitter->ttl = DEFAULT_TTL;
    transmitter->send_buffer_size = DEFAULT_SEND_BUFFER_SIZE;
    transmitter->mtu = MTU;

    transmitter->participants = init_participant_list();
    transmitter->stream_list = stream_list;

    return transmitter;
}

int init_transmission(participant_data_t *participant, transmitter_t *transmitter)
{
    pthread_mutex_lock(&participant->lock);
    
    if (participant->type != OUTPUT) {
        pthread_mutex_unlock(&participant->lock);
        return FALSE;
    }

    if (participant->rtp.run == TRUE) {
        pthread_mutex_unlock(&participant->lock);
        // If it's already initialized, do nothing
        return TRUE;
    }

    rtp_routine_params_t *params = malloc(sizeof(rtp_routine_params_t));
    if (params == NULL) {
        error_msg("init_transmission: malloc error");
        pthread_mutex_unlock(&participant->lock);
        return FALSE;
    }
    params->participant = participant;
    params->transmitter = transmitter;

    int ret = pthread_create(&participant->rtp.thread, NULL,
                             transmitter_rtp_routine, params);
    if (ret < 0) {
        error_msg("init_transmission: pthread_create error");
        pthread_mutex_unlock(&participant->lock);
    }
    participant->rtp.run = TRUE;

    pthread_mutex_unlock(&participant->lock);
    return TRUE;
}

int stop_transmission(participant_data_t *participant)
{
    pthread_mutex_lock(&participant->lock);

    participant->rtp.run = FALSE;
    pthread_join(participant->rtp.thread, NULL);

    // TODO: signaling may be required?

    pthread_mutex_unlock(&participant->lock);

    return TRUE;
}

void transmitter_dummy_callback(struct rtp *session, rtp_event *e)
{
    UNUSED(session);
    UNUSED(e);
}

void *transmitter_rtp_routine(void *arg)
{
    debug_msg(" transmitter rtpenc routine START\n");
    rtp_routine_params_t *params = (rtp_routine_params_t *)arg;
    transmitter_t *transmitter = params->transmitter;
    participant_data_t *participant = params->participant;
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
    uint32_t last_seqno = 0;

    while (participant->rtp.run && participant->has_stream) {

        stream_data_t *stream = participant->stream;
        assert(stream != NULL);
        encoder_thread_t *encoder = stream->video->encoder;
        assert(encoder != NULL);

        // TODO: add protection against spurious wakes. They should not be harmful in this scenario though.
        pthread_cond_wait(&participant->stream->video->encoder->output_cond, &participant->stream->video->encoder->output_lock);

        pthread_rwlock_rdlock(&stream->video->coded_frame_lock);

        // TODO: just protecting the initialization! should be outside maybe?
        if (stream->video->encoder->frame != NULL) {
            if (stream->video->coded_frame_seqno != last_seqno) {
                
                gettimeofday(&curr_time, NULL);
                rtp_update(rtp, curr_time);
                timestamp = tv_diff(curr_time, start_time)*90000;
                rtp_send_ctrl(rtp, timestamp, 0, curr_time);

                tx_send_h264(tx_session, stream->video->encoder->frame, rtp, transmitter->fps);
                last_seqno = stream->video->coded_frame_seqno;
            }
        }

        pthread_rwlock_unlock(&stream->video->coded_frame_lock);
    }   

    rtp_send_bye(rtp);
    rtp_done(rtp);
    module_done(CAST_MODULE(&tmod));
    
    pthread_exit(NULL);
}

int start_transmitter(transmitter_t *transmitter)
{
    transmitter->run = TRUE;
    
    participant_list_t *list = transmitter->participants;
    struct participant_data *participant = list->first;
    while (participant != NULL) {
        printf("[trans] [master] initializing encoder for stream %d\n", participant->stream->id);
        init_encoder(participant->stream->video);
    
        printf("[trans] [master] initializing transmission for participant %d\n", participant->id);
        init_transmission(participant, transmitter);
        participant = participant->next;
    }

    
    return TRUE;
}

int stop_transmitter(transmitter_t *transmitter)
{
    transmitter->run = FALSE;
    
    pthread_rwlock_rdlock(&transmitter->participants->lock);
    participant_data_t *participant = transmitter->participants->first;
    while (participant != NULL) {
        stop_transmission(participant);
        participant = participant->next;
    }
    pthread_rwlock_unlock(&transmitter->participants->lock);
    free(transmitter);
    return TRUE;
}

int add_transmitter_participant(transmitter_t *transmitter, participant_data_t *participant)
{
    if (add_participant(transmitter->participants, participant) < 0) {
        return FALSE;
    }
    return init_transmission(participant, transmitter);
}

int destroy_transmitter_participant(transmitter_t *transmitter, uint32_t id)
{
    participant_data_t *participant = get_participant_id(transmitter->participants, id);
    if (participant == NULL) {
        return FALSE;
    }
    if (stop_transmission(participant) < 0) {
        return FALSE;   
    }
    if (remove_participant(transmitter->participants, participant->id) < 0) {
        return FALSE;
    }
    return TRUE;
}
