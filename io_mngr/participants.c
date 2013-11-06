#include "config_unix.h"
#include "participants.h"
#include "transmitter.h"
#include "video_decompress/libavcodec.h"
#include "video_decompress.h"
#include "debug.h"

void destroy_participant(participant_data_t *src);
int remove_participant(participant_list_t *list, uint32_t id);
void destroy_participant_list(participant_list_t *list);

participant_data_t *init_participant(uint32_t id, io_type_t type, char *addr, uint32_t port){
    participant_data_t *participant;
  
    participant = (participant_data_t *) malloc(sizeof(participant_data_t));
  
    pthread_mutex_init(&participant->lock, NULL);
    participant->id = id;
    participant->ssrc = 0;
    participant->next = participant->previous = NULL;
    participant->type = type;
    participant->has_stream = FALSE;
    participant->rtp.port = port;
    participant->rtp.addr = addr;
    
    return participant;
}

int add_participant(participant_list_t *list, int id, io_type_t part_type, char *addr, uint32_t port){
    struct participant_data *participant;
  
    participant = init_participant(id, part_type, addr, port);

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
        //TODO: error_msg
        pthread_rwlock_unlock(&list->lock);
        return FALSE;
    }
    pthread_rwlock_unlock(&list->lock);
  
    return TRUE;
}

void destroy_participant(participant_data_t *src){
    pthread_mutex_destroy(&src->lock);
  
    free(src);
}

participant_list_t *init_participant_list(void){
  participant_list_t  *list;
    
  list = (participant_list_t *) malloc(sizeof(participant_list_t));
  
  pthread_rwlock_init(&list->lock, NULL);
  
  list->count = 0;
  list->first = NULL;
  list->last = NULL;
  
  return list;
}


participant_data_t *get_participant_id(participant_list_t *list, uint32_t id){
  participant_data_t *participant;
  
  participant = list->first;
  while(participant != NULL){
    if(participant->id == id)
      return participant; 
    participant = participant->next;
  }

  return NULL;
}

participant_data_t *get_participant_ssrc(participant_list_t *list, uint32_t ssrc){
  participant_data_t *participant;
  
  participant = list->first;
  while(participant != NULL){
    if(participant->ssrc == ssrc)
      return participant;
    participant = participant->next;
  }

  return NULL;
}

participant_data_t *get_participant_non_init(participant_list_t *list){
    participant_data_t *participant;

    participant = list->first;
    while(participant != NULL){
        if (participant->ssrc == 0 && !participant->has_stream)
            return participant;
        participant = participant->next;
    }
    return NULL;
}

int set_participant(participant_data_t *participant, uint32_t ssrc){
    pthread_mutex_lock(&participant->lock);
    participant->ssrc = ssrc;
    pthread_mutex_unlock(&participant->lock);
    uint32_t id = rand(); 
    stream_data_t *stream = init_stream(VIDEO, INPUT, id, I_AWAIT, NULL);
    add_participant_stream(participant, stream);
    printf("Stream added to participant with ID: %u\n", id);
    return TRUE;
}

int remove_participant(participant_list_t *list, uint32_t id){
    participant_data_t *participant;

    pthread_rwlock_wrlock(&list->lock);
  
    if (list->count == 0) {
        return FALSE;
    }
  
    participant = get_participant_id(list, id);

    if (participant == NULL)
        return FALSE;

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
  
    destroy_participant(participant);

    pthread_rwlock_unlock(&list->lock);
  
    return TRUE;
}

void destroy_participant_list(participant_list_t *list){
  participant_data_t *participant;
  
  participant = list->first;

  while(participant != NULL){
    pthread_rwlock_wrlock(&list->lock);
    remove_participant(list, participant->id);
    pthread_rwlock_unlock(&list->lock);
    participant = participant->next;
  }
  
  assert(list->count == 0);
 
  pthread_rwlock_destroy(&list->lock);
  
  free(list);
}

int add_participant_stream(participant_data_t *participant, stream_data_t *stream)
{
    pthread_mutex_lock(&participant->lock);

    participant->stream = stream;
    participant->has_stream = TRUE;

    pthread_mutex_unlock(&participant->lock);
    return TRUE;
}

int remove_participant_stream(participant_data_t *participant)
{
    pthread_mutex_lock(&participant->lock);

    participant->stream = NULL;
    participant->has_stream = FALSE;
    
    pthread_mutex_unlock(&participant->lock);

    return TRUE;
}

int get_participant_from_stream_id(participant_list_t *list, uint32_t stream_id){
    participant_data_t *participant;
    pthread_rwlock_rdlock(&list->lock);

    participant = list->first;

    while(participant != NULL){
        if (participant->stream->id == stream_id){
            pthread_rwlock_unlock(&list->lock);
            return participant->id;
        }
        participant = participant->next;
    }
    pthread_rwlock_unlock(&list->lock);

    return -1;
}
