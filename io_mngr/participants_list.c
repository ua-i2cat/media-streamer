/*
 *  participants_list.c - Implementation of the participant list.
 *  Copyright (C) 2013  Fundació i2CAT, Internet i Innovació digital a Catalunya
 *
 *  This file is part of media-streamer.
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

#include "participants_list.h"
#include "debug.h"

participant_list_t *init_participant_list(void)
{
    participant_list_t  *list;

    if ((list = malloc(sizeof(participant_list_t))) == NULL) {
        error_msg("init_participant_list malloc out of memory!");
        return NULL;
    }
    pthread_rwlock_init(&list->lock, NULL);
    list->count = 0;
    list->first = NULL;
    list->last = NULL;

    return list;
}

void destroy_participant_list(participant_list_t *list)
{
    participant_t *participant;

    participant = list->first;
    while(participant != NULL){
        remove_participant(list, participant->id);
        participant = participant->next;
    }
    assert(list->count == 0);
    pthread_rwlock_destroy(&list->lock);

    free(list);
}

void add_participant(participant_list_t *list, participant_t *participant)
{
    pthread_rwlock_wrlock(&list->lock);
    if (list->count == 0) {
        assert(list->first == NULL && list->last == NULL);
        list->count++;
        list->first = list->last = participant;
    } else if (list->count > 0){
        assert(list->first != NULL && list->last != NULL);
        participant->previous = list->last;
        list->count++;
        list->last->next = participant;
        list->last = participant;
    } else{
        error_msg("add_participant error");
    }
    pthread_rwlock_unlock(&list->lock);
}

bool remove_participant(participant_list_t *list, unsigned int id)
{
    participant_t *participant;

    pthread_rwlock_wrlock(&list->lock);

    if (list->count == 0) {
        pthread_rwlock_unlock(&list->lock);
        return false;
    }

    participant = get_participant_id(list, id);

    if (participant == NULL) {
        pthread_rwlock_unlock(&list->lock);
        return false;
    }

    pthread_mutex_lock(&participant->lock);

    if (participant->next == NULL && participant->previous == NULL) {
        assert(list->last == participant && list->first == participant);
        list->first = NULL;
        list->last = NULL;
    } else if (participant->next == NULL) {
        assert(list->last == participant);
        list->last = participant->previous;
        participant->previous->next = NULL;
    } else if (participant->previous == NULL) {
        assert(list->first == participant);
        list->first = participant->next;
        participant->next->previous = NULL;
    } else {
        assert(participant->next != NULL && participant->previous != NULL);
        participant->previous->next = participant->next;
        participant->next->previous = participant->previous;
    }

    list->count--;

    pthread_mutex_unlock(&participant->lock);
    pthread_rwlock_unlock(&list->lock);

    destroy_participant(participant);

    return true;
}

participant_t *get_participant_id(participant_list_t *list, unsigned int id)
{
    participant_t *participant;

    participant = list->first;
    while(participant != NULL) {

        if(participant->id == id) {
            return participant; 
        }
        participant = participant->next;

    }

    return NULL;
}

participant_t *get_participant_ssrc(participant_list_t *list, uint32_t ssrc)
{
    participant_t *participant;

    participant = list->first;
    while(participant != NULL){
        if(participant->ssrc == ssrc)
            return participant;
        participant = participant->next;
    }

    return NULL;
}

participant_t *get_participant_non_init(participant_list_t *list)
{
    participant_t *participant;

    participant = list->first;
    while(participant != NULL) {
        if (participant->ssrc == 0)
            return participant;
        participant = participant->next;
    }

    return NULL;
}

