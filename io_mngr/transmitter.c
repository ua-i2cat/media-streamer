/*
 *  transmitter.c
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
#include <stdlib.h>
#include "transmitter.h"
#include "rtp/rtp.h"
#include "pdb.h"
#include "video_codec.h"
#include "video_compress.h"
#include "debug.h"
#include "tv.h"

static int send_video_frame(stream_data_t *stream, video_data_frame_t *coded_frame, struct timeval start_time);
static int send_audio_frame(stream_data_t *stream, audio_frame2 *frame, struct timeval start_time);
static void *video_transmitter_thread(void *arg);
static void *audio_transmitter_thread(void *arg);

// TODO: Rename to send_video_frame
static int send_video_frame(stream_data_t *stream, video_data_frame_t *coded_frame, struct timeval start_time)
{
    participant_data_t *participant;
    struct timeval curr_time;
    double timestamp;
    int ret = FALSE;

    pthread_rwlock_rdlock(&stream->plist->lock);

    participant = stream->plist->first;
    while (participant != NULL && participant->rtp != NULL){

        gettimeofday(&curr_time, NULL);
        rtp_update(participant->rtp->rtp, curr_time);
        timestamp = tv_diff(curr_time, start_time)*90000;
        rtp_send_ctrl(participant->rtp->rtp, timestamp, 0, curr_time);            

        tx_send_h264(participant->rtp->tx_session, stream->video->encoder->frame, 
                participant->rtp->rtp, coded_frame->media_time);
        ret = TRUE;

        participant = participant->next;
    }

    pthread_rwlock_unlock(&stream->plist->lock);

    return ret;
}

static int send_audio_frame(stream_data_t *stream, audio_frame2 *frame, struct timeval start_time)
{
    participant_data_t *participant;
    struct timeval curr_time;
    double timestamp;
    int ret = FALSE;

    pthread_rwlock_rdlock(&stream->plist->lock);

    participant = stream->plist->first;
    while (participant != NULL && participant->rtp != NULL) {

        gettimeofday(&curr_time, NULL);
        rtp_update(participant->rtp->rtp, curr_time);
        timestamp = tv_diff(curr_time, start_time)*90000;
        rtp_send_ctrl(participant->rtp->rtp, timestamp, 0, curr_time);            

        // TODO: support encoder depending on configuration
        audio_tx_send_mulaw(participant->rtp->tx_session, participant->rtp->rtp, frame);
        ret = TRUE;

        participant = participant->next;
    }

    pthread_rwlock_unlock(&stream->plist->lock);

    return ret;
}

static void *video_transmitter_thread(void *arg)
{
    transmitter_t *transmitter = (transmitter_t *)arg;
    stream_data_t *stream;
    video_data_frame_t *coded_frame;


    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    while(transmitter->video_run){
        usleep(100);

        pthread_rwlock_rdlock(&transmitter->video_stream_list->lock);

        stream = transmitter->video_stream_list->first;
        while(stream != NULL && transmitter->video_run){
            coded_frame = curr_out_frame(stream->video->coded_frames);
            if (coded_frame == NULL){
                stream = stream->next;
                continue;
            }
            send_video_frame(stream, coded_frame, start_time);
            remove_frame(stream->video->coded_frames);
            stream = stream->next;
        }

        pthread_rwlock_unlock(&transmitter->video_stream_list->lock);
    }

    pthread_exit(NULL);
}

static void *audio_transmitter_thread(void *arg)
{
    transmitter_t *transmitter = (transmitter_t *)arg;
    stream_data_t *stream;
    audio_frame2 *frame;

    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    while(transmitter->audio_run) {
//        usleep(100);
        pthread_rwlock_rdlock(&transmitter->audio_stream_list->lock);

        stream = transmitter->audio_stream_list->first;
        while(stream != NULL && transmitter->audio_run) {
            if ((frame = cq_get_front(stream->audio->coded_cq)) != NULL) {
                send_audio_frame(stream, frame, start_time);
                cq_remove_bag(stream->audio->coded_cq);
            }            
            stream = stream->next;
        }

        pthread_rwlock_unlock(&transmitter->audio_stream_list->lock);
    }

    pthread_exit(NULL);
}

// TODO: Put fps param away!
transmitter_t *init_transmitter(stream_list_t *video_stream_list, stream_list_t *audio_stream_list, float fps)
{
    transmitter_t *transmitter = malloc(sizeof(transmitter_t));
    if (transmitter == NULL) {
        error_msg("init_transmitter: malloc error");
        return NULL;
    }

    transmitter->video_run = FALSE;
    transmitter->audio_run = FALSE;

    if (fps <= 0.0) {
        transmitter->fps = DEFAULT_FPS;
    } else {
        transmitter->fps = fps;
    }
    transmitter->wait_time = (1000000.0/transmitter->fps);

    transmitter->ttl = DEFAULT_TTL;
    transmitter->send_buffer_size = DEFAULT_SEND_BUFFER_SIZE;
    transmitter->mtu = MTU;
    transmitter->video_stream_list = video_stream_list;

    transmitter->audio_ttl = DEFAULT_TTL;
    transmitter->audio_send_buffer_size = DEFAULT_SEND_BUFFER_SIZE;
    transmitter->audio_mtu = MTU;
    transmitter->audio_stream_list = audio_stream_list;

    return transmitter;
}

int start_transmitter(transmitter_t *transmitter)
{  
    transmitter->video_run = TRUE;
    if (pthread_create(&transmitter->video_thread, NULL, video_transmitter_thread, transmitter) != 0) {
        transmitter->video_run = FALSE;
    }

    transmitter->audio_run = TRUE;
    if (pthread_create(&transmitter->audio_thread, NULL, audio_transmitter_thread, transmitter) != 0) {
        transmitter->audio_run = FALSE;
    }

    return transmitter->video_run && transmitter->audio_run;
}

void stop_transmitter(transmitter_t *transmitter)
{
    transmitter->video_run = FALSE;
    pthread_join(transmitter->video_thread, NULL); 
    transmitter->audio_run = FALSE;
    pthread_join(transmitter->audio_thread, NULL); 
}

int destroy_transmitter(transmitter_t *transmitter)
{
    if (transmitter->video_run || transmitter->audio_run) {
        return FALSE;
    }

    free(transmitter);
    return TRUE;
}
