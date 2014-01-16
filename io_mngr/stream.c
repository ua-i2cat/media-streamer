/*
 *  stream.c - Implementation of the audio or video stream entity.
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
 *  Authors:  Jordi "Txor" Casas Ríos <txorlings@gmail.com>,
 *            David Cassany <david.cassany@i2cat.net>,
 *            Ignacio Contreras <ignacio.contreras@i2cat.net>,
 *            Marc Palau <marc.palau@i2cat.net>
 */

#include "stream.h"
#include "debug.h"
#include "commons.h"

stream_t *init_stream(stream_type_t type, io_type_t io_type, unsigned int id, stream_state_t state, char *stream_name)
{
    stream_t *stream = malloc(sizeof(stream_t));
    if (stream == NULL) {
        error_msg("init_video_stream malloc error");
        return NULL;
    }

    if (stream_name == NULL){
        stream->stream_name = NULL;
    } else {
        stream->stream_name = strdup(stream_name);
    }

    stream->id = id;
    stream->type = type;
    stream->io_type = io_type;
    stream->state = state;
    stream->prev = NULL;
    stream->next = NULL;

    if (type == VIDEO) {
        if (io_type == INPUT){
            stream->video = vp_init(DECODER);
        } else if (io_type == OUTPUT){
            stream->video = vp_init(ENCODER);
        }
    }
    else if (type == AUDIO) {
        if (io_type == INPUT){
            stream->audio = ap_init(DECODER);
        } else if (io_type == OUTPUT){
            stream->audio = ap_init(ENCODER);
        }
    }
    else {
        error_msg("No VIDEO nor AUDIO type? WTF!");
        return NULL;
    }

    stream->plist = init_participant_list();

    return stream;
}

void destroy_stream(stream_t *stream)
{
    if (stream->type == VIDEO){
        vp_destroy(stream->video);
    } else if (stream->type == AUDIO){
        ap_destroy(stream->audio);
    }

    destroy_participant_list(stream->plist);

    free(stream->stream_name);
    free(stream);
}

void set_stream_state(stream_t *stream, stream_state_t state)
{
    if (state == NON_ACTIVE) {
        stream->state = state;
    } else if (stream->state == NON_ACTIVE) {
        stream->state = I_AWAIT;
    }
}

void add_participant_stream(stream_t *stream, participant_t *participant)
{
    add_participant(stream->plist, participant);
    participant->stream = stream;
}

bool remove_participant_from_stream(stream_t *stream, unsigned int id)
{
    return remove_participant(stream->plist, id);
}

