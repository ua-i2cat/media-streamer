#include "config_unix.h"
#include "receiver.h"
#include "rtp/rtp_callback.h"
#include "rtp/rtp.h"
#include "pdb.h"
#include "tv.h"
#include "debug.h"

#define INITIAL_VIDEO_RECV_BUFFER_SIZE  ((4*1920*1080)*110/100) //command line net.core setup: sysctl -w net.core.rmem_max=9123840

void *receiver_thread(receiver_t *receiver);
void *audio_receiver_thread(receiver_t *receiver);

receiver_t *init_receiver(stream_list_t *stream_list, uint32_t video_port, uint32_t audio_port) {
    receiver_t *receiver;
    double rtcp_bw = 5 * 1024 * 1024; /* FIXME */
    int ttl = 255; //TODO: get rid of magic numbers!

    receiver = malloc(sizeof(receiver_t));

    // Video initialization
    receiver->session = (struct rtp *) malloc(sizeof(struct rtp *));
    receiver->part_db = pdb_init();
    receiver->run = FALSE;
    receiver->port = video_port;
    receiver->stream_list = stream_list;
    receiver->session = rtp_init_if(NULL, NULL, receiver->port, 0, ttl,
            rtcp_bw, 0, rtp_recv_callback, (void *)receiver->part_db, 0);

    if (receiver->session != NULL) {
        if (!rtp_set_option(receiver->session, RTP_OPT_WEAK_VALIDATION, 1)) {
            return NULL;
        }
        if (!rtp_set_sdes(receiver->session, rtp_my_ssrc(receiver->session),
                    RTCP_SDES_TOOL, PACKAGE_STRING, strlen(PACKAGE_STRING))) { //TODO: is this needed?			
            return NULL;
        }
        if (!rtp_set_recv_buf(receiver->session, INITIAL_VIDEO_RECV_BUFFER_SIZE)) {
            return NULL;
        }
    }

    // Audio initialization
    receiver->audio_session = (struct rtp *) malloc(sizeof(struct rtp *));
    receiver->audio_part_db = pdb_init();
    receiver->audio_run = FALSE;
    receiver->audio_port = audio_port;
    receiver->audio_stream_list = stream_list;
    receiver->audio_session = rtp_init_if(NULL, NULL, receiver->audio_port,
            0, ttl, rtcp_bw, 0, rtp_recv_callback, (void *)receiver->audio_part_db, 0);

    if (receiver->audio_session != NULL) {
        if (!rtp_set_option(receiver->audio_session, RTP_OPT_WEAK_VALIDATION, 1)) {
            return NULL;
        }
        if (!rtp_set_sdes(receiver->audio_session, rtp_my_ssrc(receiver->audio_session),
                    RTCP_SDES_TOOL, PACKAGE_STRING, strlen(PACKAGE_STRING))) { //TODO: is this needed?			
            return NULL;
        }
        if (!rtp_set_recv_buf(receiver->audio_session, INITIAL_VIDEO_RECV_BUFFER_SIZE)) {
            return NULL;
        }
    }

    return receiver;
}

//TODO: refactor de la funció per evitar tants IF anidats
void *receiver_thread(receiver_t *receiver) {
    struct pdb_e *cp;
    participant_data_t *participant;
    video_data_frame_t* coded_frame;

    struct timeval curr_time;
    struct timeval timeout;
    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    timeout.tv_sec = 0;
    timeout.tv_usec = 10000;
    uint32_t timestamp; //TODO: why is this used

    while(receiver->run){
        gettimeofday(&curr_time, NULL);
        timestamp = tv_diff(curr_time, start_time) * 90000;
        rtp_update(receiver->session, curr_time);
        //rtp_send_ctrl(receiver->session, timestamp, 0, curr_time); //TODO: why is this used?

        timeout.tv_sec = 0;
        timeout.tv_usec = 10000;

        //TODO: repàs dels locks en accedir a src
        if (!rtp_recv_r(receiver->session, &timeout, timestamp)){
            pdb_iter_t it;
            cp = pdb_iter_init(receiver->part_db, &it);

            while (cp != NULL) {

                participant = get_participant_stream_ssrc(receiver->stream_list, cp->ssrc);
                if (participant == NULL){
                    participant = get_participant_stream_non_init(receiver->stream_list);
                    if (participant != NULL){
                        set_participant_ssrc(participant, cp->ssrc);
                    }
                }

                if (participant == NULL){
                    cp = pdb_iter_next(&it);
                    continue;
                }

                coded_frame = curr_in_frame(participant->stream->video->coded_frames);
                if (coded_frame == NULL){
                    error_msg("Missing packets\n");
                    cp = pdb_iter_next(&it);
                    continue;
                }

                if (pbuf_decode(cp->playout_buffer, curr_time, decode_frame_h264, coded_frame)) {

                    if (participant->stream->state == I_AWAIT && 
                            coded_frame->frame_type == INTRA && 
                            coded_frame->width != 0 && 
                            coded_frame->height != 0){

                        if(participant->stream->video->decoder == NULL){
                            set_video_frame_cq(participant->stream->video->decoded_frames, 
                                    RGB, 
                                    coded_frame->width, 
                                    coded_frame->height);
                            printf("starting decoder\n");
                            start_decoder(participant->stream->video); 
                        }
                        participant->stream->state = ACTIVE;
                    }

                    if (participant->stream->state == ACTIVE && coded_frame->frame_type != BFRAME) {
                        put_frame(participant->stream->video->coded_frames);
                    } else {
                        debug_msg("No support for Bframes\n");
                    }
                    //TODO: should be at the beginning of the loop
                    pbuf_remove_first(cp->playout_buffer);

                }
                cp = pdb_iter_next(&it);
            } 
            pdb_iter_done(&it);
        }
    }

    pthread_exit((void *)NULL);   
}

void *audio_receiver_thread(receiver_t *receiver) {

    struct pdb_e *cp;
    participant_data_t *participant;
    struct state_audio_decoder *coded_frame;

    struct timeval curr_time;
    struct timeval timeout;
    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    timeout.tv_sec = 0;
    timeout.tv_usec = 999999 / 59.94;
    uint32_t timestamp;

    while(receiver->run){
        gettimeofday(&curr_time, NULL);
        timestamp = tv_diff(curr_time, start_time) * 90000;
        rtp_update(receiver->audio_sessionh_id, curr_time);
        //rtp_send_ctrl(receiver->audio_sessionh_id, timestamp, 0, curr_time); //TODO: use this.

        //TODO: repàs dels locks en accedir a src
        if (!rtp_recv_r(receiver->audio_sessionh_id, &timeout, timestamp)){
            pdb_iter_t it;
            cp = pdb_iter_init(receiver->audio_part_db, &it);

            while (cp != NULL) {

                //TODO: Possible optimization using participant hash
                if (participant = get_participant_stream_ssrc(receiver->audio_stream_list, cp->ssrc) == NULL) {
                    if (participant = get_participant_stream_non_init(receiver->audio_stream_list) == NULL) {
                        debug_msg("audio_receiver_thread: Can't find configured streams, dropping data");
                        cp = pdb_iter_next(&it);
                        continue;
                    }
                    else {
                        set_participant_ssrc(participant, cp->ssrc);
                    }
                }

                if (coded_frame = cq_get_rear(participant->stream->audio->coded_cq) == NULL) {
                    debug_msg("receiver thread: coded circular queue is full");
                    cp = pdb_iter_next(&it);
                    continue;
                }

                // TODO: Generate tools to get the correct decoder callback from audio_processor_t audio configuration.
                // Maybe a codec_t table and its decoding callbacks paired with a getter function.
                if (rtp_audio_pbuf_decode(cp->playout_buffer, curr_time, decode_audio_frame_mulaw, coded_frame)) {
                    cq_add_bag(participant->stream->audio->coded_cq);
                }
                cp = pdb_iter_next(&it);
            } 
            pdb_iter_done(&it);
        }
    }

    pthread_exit((void *)NULL);   
}

int start_receiver(receiver_t *receiver) {
    receiver->run = TRUE;
    receiver->audio_run = TRUE;

    if (pthread_create(&receiver->th_id, NULL, (void *)receiver_thread, receiver) != 0)
        receiver->run = FALSE;

    if (pthread_create(&receiver->audio_th_id, NULL, (void *)audio_receiver_thread, receiver) != 0)
        receiver->audio_run = FALSE;

    return receiver->run && receiver->audio_run;
}

int stop_receiver(receiver_t *receiver) {
    receiver->run = FALSE;
    receiver->audio_run = FALSE;

    pthread_join(receiver->th_id, NULL); 
    pthread_join(receiver->audio_th_id, NULL);

    return TRUE;
}

int destroy_receiver(receiver_t *receiver) {
    if (receiver->run || receiver->audio_run) {
        return FALSE;
    }

    rtp_done(receiver->session);
    rtp_done(receiver->audio_session);
    pdb_destroy(&receiver->part_db);
    pdb_destroy(&receiver->audio_part_db);
    free(receiver);

    return TRUE;
}

