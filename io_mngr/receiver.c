/*
 *  receiver.c
 *  Copyright (C) 2013  Fundació i2CAT, Internet i Innovació digital a Catalunya
 *
 *  This file is part of io_mngr.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Authors:  Jordi "Txor" Casas Ríos <jordi.casas@i2cat.net>,
 *            David Cassany <david.cassany@i2cat.net>,
 *            Ignacio Contreras <ignacio.contreras@i2cat.net>,
 *            Marc Palau <marc.palau@i2cat.net>
 */

//#include "config_unix.h"
#include "receiver.h"
#include "rtp/rtp_callback.h"
#include "rtp/rtp.h"
#include "rtp/audio_decoders.h"
#include "pdb.h"
#include "tv.h"
#include "debug.h"
#include "video_config.h"

static void *video_receiver_thread(receiver_t *receiver);
static void *audio_receiver_thread(receiver_t *receiver);

void *video_receiver_thread(receiver_t *receiver)
{
    struct pdb_e *cp;
    participant_data_t *participant;
    video_frame2* coded_frame;

    struct timeval curr_time;
    struct timeval timeout;
    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    timeout.tv_sec = 0;
    timeout.tv_usec = 10000;
    uint32_t timestamp;

    while(receiver->video_run) {

        gettimeofday(&curr_time, NULL);
        timestamp = tv_diff(curr_time, start_time) * 90000;
        rtp_update(receiver->video_session, curr_time);
        rtp_send_ctrl(receiver->session, timestamp, 0, curr_time);

        timeout.tv_sec = 0;
        timeout.tv_usec = 10000;

        //TODO: repàs dels locks en accedir a src
        if (!rtp_recv_r(receiver->video_session, &timeout, timestamp)) {
            pdb_iter_t it;
            cp = pdb_iter_init(receiver->video_part_db, &it);

            while (cp != NULL) {

                //TODO: Possible optimization using participant hash
                if ((participant = get_participant_stream_ssrc(receiver->video_stream_list, cp->ssrc)) == NULL) {
                    if ((participant = get_participant_stream_non_init(receiver->video_stream_list)) == NULL) {
                        debug_msg("audio_receiver_thread: Can't find configured streams, dropping data");
                        cp = pdb_iter_next(&it);
                        continue;
                    }
                    else {
                        set_participant_ssrc(participant, cp->ssrc);
                    }
                }

                if ((coded_frame = cq_get_rear(participant->stream->video->coded_cq)) == NULL) {
                    if(pbuf_check_if_complete_frame(cp->playout_buffer, curr_time)) {
                        pbuf_remove_first(cp->playout_buffer);
                        error_msg("Warning! Coded frame discarded in reception\n");
                        participant->stream->video->lost_coded_frames++;
                    }
                }
                else {

                    if (pbuf_decode(cp->playout_buffer, curr_time, decode_frame_h264, coded_frame)) {

                        if (participant->stream->state == I_AWAIT && 
                                coded_frame->type == INTRA && 
                                coded_frame->width != 0 && 
                                coded_frame->height != 0) {

                            vp_reconfig_external(participant->stream->video, 
                                    coded_frame->codec,
                                    coded_frame->width, 
                                    coded_frame->height);
                            vp_reconfig_internal(participant->stream->video, 
                                    participant->stream->video->internal_config->codec,
                                    coded_frame->width, 
                                    coded_frame->height);
                            participant->stream->state = ACTIVE;
                        }

                        if (participant->stream->state == ACTIVE && coded_frame->type != BFRAME) {
                            participant->stream->video->seqno++;
                            coded_frame->seqno = participant->stream->video->seqno;
                            coded_frame->media_time = get_local_mediatime_us();
                            cq_add_bag(participant->stream->video->coded_cq);
                        }
                        else {
                            debug_msg("No support for Bframes\n");
                        }
                        //TODO: should be at the beginning of the loop
                        pbuf_remove_first(cp->playout_buffer);
                    }
                }
                cp = pdb_iter_next(&it);
            } 
            pdb_iter_done(&it);
        }
    }

    pthread_exit((void *)NULL);   
}

static void *audio_receiver_thread(receiver_t *receiver)
{
    struct pdb_e *cp;
    participant_data_t *participant;
    struct state_audio_decoder decode_object;

    struct timeval curr_time;
    struct timeval timeout;
    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    timeout.tv_sec = 0;
    timeout.tv_usec = 999999 / 59.94;
    uint32_t timestamp;

    decode_object.resampler = NULL;

    while(receiver->video_run) {
        gettimeofday(&curr_time, NULL);
        timestamp = tv_diff(curr_time, start_time) * 90000;
        rtp_update(receiver->audio_session, curr_time);
        rtp_send_ctrl(receiver->audio_session, timestamp, 0, curr_time);

        //TODO: repàs dels locks en accedir a src
        if (!rtp_recv_r(receiver->audio_session, &timeout, timestamp)){
            pdb_iter_t it;
            cp = pdb_iter_init(receiver->audio_part_db, &it);

            while (cp != NULL) {

                //TODO: Possible optimization using participant hash
                if ((participant = get_participant_stream_ssrc(receiver->audio_stream_list, cp->ssrc)) == NULL) {
                    if ((participant = get_participant_stream_non_init(receiver->audio_stream_list)) == NULL) {
                        debug_msg("audio_receiver_thread: Can't find configured streams, dropping data");
                        cp = pdb_iter_next(&it);
                        continue;
                    }
                    else {
                        set_participant_ssrc(participant, cp->ssrc);
                    }
                }

                if ((decode_object.frame = cq_get_rear(participant->stream->audio->coded_cq)) == NULL) {
                    debug_msg("receiver thread: coded circular queue is full");
                    cp = pdb_iter_next(&it);
                    continue;
                }

                decode_object.desc = ap_get_config(participant->stream->audio);

                // TODO: Generate tools to get the correct decoder callback from audio_processor_t audio configuration.
                // Maybe a codec_t table and its decoding callbacks paired with a getter function.
                if (rtp_audio_pbuf_decode(cp->playout_buffer, curr_time, decode_audio_frame_mulaw, &decode_object)) {
                    cq_add_bag(participant->stream->audio->coded_cq);
                }
                cp = pdb_iter_next(&it);
            } 
            pdb_iter_done(&it);
        }
    }

    pthread_exit((void *)NULL);   
}

receiver_t *init_receiver(stream_list_t *video_stream_list, stream_list_t *audio_stream_list, uint32_t video_port, uint32_t audio_port)
{
    receiver_t *receiver;
    double rtcp_bw = 5 * 1024 * 1024; /* FIXME */
    int ttl = 255; //TODO: get rid of magic numbers!

    receiver = malloc(sizeof(receiver_t));

    // Video initialization
    receiver->video_session = (struct rtp *) malloc(sizeof(struct rtp *));
    receiver->video_part_db = pdb_init();
    receiver->video_run = FALSE;
    receiver->video_port = video_port;
    receiver->video_stream_list = video_stream_list;
    receiver->video_session = rtp_init_if(NULL, NULL, receiver->video_port, 0, ttl,
            rtcp_bw, 0, rtp_recv_callback, (void *)receiver->video_part_db, 0);

    if (receiver->video_session != NULL) {
        if (!rtp_set_option(receiver->video_session, RTP_OPT_WEAK_VALIDATION, 1)) {
            return NULL;
        }
        if (!rtp_set_sdes(receiver->video_session, rtp_my_ssrc(receiver->video_session),
                    RTCP_SDES_TOOL, PACKAGE_STRING, strlen(PACKAGE_STRING))) { //TODO: is this needed?			
            return NULL;
        }
        if (!rtp_set_recv_buf(receiver->video_session, INITIAL_VIDEO_RECV_BUFFER_SIZE)) {
            return NULL;
        }
    }

    // Audio initialization
    receiver->audio_session = (struct rtp *) malloc(sizeof(struct rtp *));
    receiver->audio_part_db = pdb_init();
    receiver->audio_run = FALSE;
    receiver->audio_port = audio_port;
    receiver->audio_stream_list = audio_stream_list;
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

int start_receiver(receiver_t *receiver)
{
    receiver->video_run = TRUE;
    receiver->audio_run = TRUE;

    if (pthread_create(&receiver->video_th_id, NULL, (void *)video_receiver_thread, receiver) != 0)
        receiver->video_run = FALSE;

    if (pthread_create(&receiver->audio_th_id, NULL, (void *)audio_receiver_thread, receiver) != 0)
        receiver->audio_run = FALSE;

    return receiver->video_run && receiver->audio_run;
}

void stop_receiver(receiver_t *receiver)
{
    receiver->video_run = FALSE;
    pthread_join(receiver->video_th_id, NULL); 

    receiver->audio_run = FALSE;
    pthread_join(receiver->audio_th_id, NULL);
}

int destroy_receiver(receiver_t *receiver)
{
    if (receiver->video_run || receiver->audio_run) {
        return FALSE;
    }

    rtp_done(receiver->video_session);
    rtp_done(receiver->audio_session);
    pdb_destroy(&receiver->video_part_db);
    pdb_destroy(&receiver->audio_part_db);
    free(receiver);

    return TRUE;
}
