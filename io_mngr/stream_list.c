/*
 *  stream_list.h - Stream list implementation.
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

#include "stream_list.h"
#include "debug.h"

stream_list_t *init_stream_list(void)
{
    stream_list_t *list;
    if ((list = malloc(sizeof(stream_list_t))) == NULL) {
        error_msg("init_stream_list malloc out of memory!");
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
    stream_t *current = list->first;
    while (current != NULL) {
        stream_t *next = current->next;
        destroy_stream(current);
        current = next;
    }
    pthread_rwlock_unlock(&list->lock);
    pthread_rwlock_destroy(&list->lock);
    free(list);
}

void add_stream(stream_list_t *list, stream_t *stream) 
{
    pthread_rwlock_wrlock(&list->lock);
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
    }
    pthread_rwlock_unlock(&list->lock);
}

bool remove_stream(stream_list_t *list, unsigned int id)
{
    pthread_rwlock_wrlock(&list->lock);
    if (list->count == 0) {
        pthread_rwlock_unlock(&list->lock);
        return false;
    }
    pthread_rwlock_unlock(&list->lock);

    stream_t *stream = get_stream_id(list, id);

    pthread_rwlock_wrlock(&list->lock);
    if (stream == NULL) {
        pthread_rwlock_unlock(&list->lock);
        return false;
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

    return true;
}

stream_t *get_stream_id(stream_list_t *list, unsigned int id)
{
    pthread_rwlock_rdlock(&list->lock);
    stream_t *stream = list->first;
    while (stream != NULL) {
        if (stream->id == id) {
            break;
        }
        stream = stream->next;
    }
    pthread_rwlock_unlock(&list->lock);

    return stream;
}

participant_data_t *get_participant_stream_id(stream_list_t *list, unsigned int id)
{
    stream_t *stream;
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

participant_data_t *get_participant_stream_ssrc(stream_list_t *list, uint32_t ssrc)
{
    stream_t *stream;
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

participant_data_t *get_participant_stream_non_init(stream_list_t *list)
{
    stream_t *stream;
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

