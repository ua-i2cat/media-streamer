/*
 *  stream.c
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

//TODO: Decouple stream and stream_list.

#include "stream.h"
#include "debug.h"

#define DEFAULT_FPS 24
#define PIXEL_FORMAT RGB

stream_list_t *init_stream_list(void)
{
    stream_list_t *list = malloc(sizeof(stream_list_t));
    if (list == NULL) {
        error_msg("init_stream_list malloc error");
        return NULL;
    }
    pthread_rwlock_init(&list->lock, NULL);
    list->count = 0;
    list->first = NULL;
    list->last = NULL;
    return list;
}

void destroy_stream_list(stream_list_t *list)
{
    pthread_rwlock_wrlock(&list->lock);
    stream_data_t *current = list->first;
    while (current != NULL) {
        stream_data_t *next = current->next;
        destroy_stream(current);
        current = next;
    }
    pthread_rwlock_unlock(&list->lock);
    pthread_rwlock_destroy(&list->lock);
    free(list);
}

stream_data_t *init_stream(stream_type_t type, io_type_t io_type, uint32_t id, stream_state_t state, char *stream_name)
{
    stream_data_t *stream = malloc(sizeof(stream_data_t));
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

void destroy_stream(stream_data_t *stream)
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

int add_stream(stream_list_t *list, stream_data_t *stream) 
{
    pthread_rwlock_wrlock(&list->lock);

    int ret = TRUE;

    if (list->count == 0) {
        assert(list->first == NULL && list->last == NULL);
        list->count++;
        list->first = list->last = stream;
    } else if (list->count > 0) {
        assert(list->first != NULL && list->last != NULL);
        stream->next = NULL;
        stream->prev = list->last;
        list->last->next = stream;
        list->last = stream;
        list->count++;
    } else {
        error_msg("add_stream list->count < 0");
        pthread_rwlock_unlock(&list->lock);
        ret = FALSE;
    }

    pthread_rwlock_unlock(&list->lock);
    return ret;
}

int remove_stream(stream_list_t *list, uint32_t id)
{
    pthread_rwlock_wrlock(&list->lock);
    if (list->count == 0) {
        pthread_rwlock_unlock(&list->lock);
        return FALSE;
    }
    pthread_rwlock_unlock(&list->lock);

    stream_data_t *stream = get_stream_id(list, id);

    pthread_rwlock_wrlock(&list->lock);
    if (stream == NULL) {
        pthread_rwlock_unlock(&list->lock);
        return FALSE;
    }

    if (stream->prev == NULL) {
        assert(list->first == stream);
        list->first = stream->next;
    } else {
        stream->prev->next = stream->next;
    }

    if (stream->next == NULL) {
        assert(list->last == stream);
        list->last = stream->prev;
    } else {
        stream->next->prev = stream->prev;
    }

    list->count--;

    destroy_stream(stream);

    pthread_rwlock_unlock(&list->lock);

    return TRUE;
}

stream_data_t *get_stream_id(stream_list_t *list, uint32_t id)
{
    pthread_rwlock_rdlock(&list->lock);

    stream_data_t *stream = list->first;
    while (stream != NULL) {
        if (stream->id == id) {
            break;
        }
        stream = stream->next;
    }
    pthread_rwlock_unlock(&list->lock);
    return stream;
}

void set_stream_state(stream_data_t *stream, stream_state_t state)
{
    if (state == NON_ACTIVE) {
        stream->state = state;
    } else if (stream->state == NON_ACTIVE) {
        stream->state = I_AWAIT;
    }
}

void add_participant_stream(stream_data_t *stream, participant_data_t *participant)
{
    add_participant(stream->plist, participant);
    participant->stream = stream;
}

int remove_participant_from_stream(stream_data_t *stream, uint32_t id)
{
    return remove_participant(stream->plist, id);
}

participant_data_t *get_participant_stream_id(stream_list_t *list, uint32_t id)
{
    stream_data_t *stream;
    participant_data_t *part = NULL;

    pthread_rwlock_rdlock(&list->lock);

    stream = list->first;
    while(stream != NULL && part == NULL){
        part = get_participant_id(stream->plist, id);
        stream = stream->next;
    }

    pthread_rwlock_unlock(&list->lock);
    return part;
}

participant_data_t *get_participant_stream_ssrc(stream_list_t *list, uint32_t ssrc){
    stream_data_t *stream;
    participant_data_t *part = NULL;

    pthread_rwlock_rdlock(&list->lock);

    stream = list->first;
    while(stream != NULL && part == NULL){
        part = get_participant_ssrc(stream->plist, ssrc);
        stream = stream->next;
    }

    pthread_rwlock_unlock(&list->lock);
    return part;
}

participant_data_t *get_participant_stream_non_init(stream_list_t *list){
    stream_data_t *stream;
    participant_data_t *part = NULL;

    pthread_rwlock_rdlock(&list->lock);

    stream = list->first;
    while(stream != NULL && part == NULL){
        part = get_participant_non_init(stream->plist);
        stream = stream->next;
    }

    pthread_rwlock_unlock(&list->lock);
    return part;
}

