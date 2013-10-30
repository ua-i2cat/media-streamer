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
    participant->streams_count = 0;
    participant->active = I_AWAIT;
    participant->rtp.port = port;
    participant->rtp.addr = addr;
    
    return participant;
}

int add_participant(participant_list_t *list, int id, io_type_t part_type, char *addr, uint32_t port){
    struct participant_data *participant;
  
    participant = init_participant(id, part_type, addr, port);
  
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
      return FALSE;
    }
  
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
        if (participant->ssrc == 0 && participant->streams_count == 0)
            return participant;
        participant = participant->next;
    }
    return NULL;
}

int set_participant(participant_data_t *participant, uint32_t ssrc){
    pthread_mutex_lock(&participant->lock);
    participant->ssrc = ssrc;
    uint32_t id = rand(); //TODO: ID generation
    stream_data_t *stream = init_stream(VIDEO, INPUT, id, I_AWAIT);
    pthread_mutex_unlock(&participant->lock);
    add_participant_stream(participant, stream);
    printf("Stream added to participant with ID: %u\n", id);
    return TRUE;
}

int remove_participant(participant_list_t *list, uint32_t id){
    participant_data_t *participant;
  
    if (list->count == 0) {
        return FALSE;
    }
  
    participant = get_participant_id(list, id);

    if (participant == NULL)
        return FALSE;

    if (participant->type == INPUT && participant->streams_count > 0){
        remove_participant_stream(participant, participant->streams[0]);
        //TODO: where to execute remove stream???
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
  
    destroy_participant(participant);
  
    return TRUE;
}

void destroy_participant_list(participant_list_t *list){
  participant_data_t *participant;
  
  participant = list->first;

  while(participant != NULL){
    pthread_rwlock_wrlock(&list->lock);
    remove_participant(list, participant->id);
    printf("remove_participant(list, participant->id);\n");
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

    if (participant->streams_count == MAX_PARTICIPANT_STREAMS) {
      error_msg("Max number of streams per participant reached.\n");
      pthread_mutex_unlock(&participant->lock);
      return FALSE;
    }

    /* TODO: if remove_participant_stream moves streams to the left,
       there should be no need to seek a free position 
     
       => simplify insertion

     */
    
    int ret = FALSE;
    int i = 0;
    while (i < MAX_PARTICIPANT_STREAMS) {
        if (participant->streams[i] == NULL) {
            participant->streams[i] = stream;
            participant->streams_count++;
            assert(participant->streams_count <= MAX_PARTICIPANT_STREAMS);
            ret = TRUE;
            break;
        }
        i++;
    }

    pthread_mutex_unlock(&participant->lock);
    return ret;
}

int remove_participant_stream(participant_data_t *participant, stream_data_t *stream)
{
    pthread_mutex_lock(&participant->lock);
    int ret = FALSE;
    int i = 0;
    while (i++ < MAX_PARTICIPANT_STREAMS) {
        if (participant->streams[i] == stream) {
            participant->streams[i] = NULL;
            assert(participant->streams_count > 0);
            participant->streams_count--;
            ret = TRUE;
            // TODO: this chunk of code moves the streams to the left
            int j = 0;
            for (j = i; j < MAX_PARTICIPANT_STREAMS - 1;) {
                if (participant->streams[j + 1] != NULL) {
                    participant->streams[j] = participant->streams[j + 1];
                }
            }
            break;
        }
    }
    pthread_mutex_unlock(&participant->lock);
    return ret;
}
