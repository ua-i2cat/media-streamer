#include "config_unix.h"
#include "participants.h"
#include "transmitter.h"
#include "video_decompress/libavcodec.h"
#include "video_decompress.h"

void destroy_decoder_thread(decoder_thread_t *dec_th);
void destroy_participant(participant_data_t *src);
int remove_participant(participant_list_t *list, uint32_t id);
void destroy_participant_list(participant_list_t *list);

participant_data_t *init_participant(uint32_t id, io_type_t type, participant_protocol_t protocol, char *addr, uint32_t port){
    participant_data_t *participant;
  
    participant = (participant_data_t *) malloc(sizeof(participant_data_t));
  
    pthread_mutex_init(&participant->lock, NULL);
    participant->id = id;
    participant->ssrc = 0;
    participant->next = participant->previous = NULL;
    participant->type = type;
    participant->protocol = protocol;
    participant->streams_count = 0;

    if (protocol == RTP){
        //TODO: init RTP struct
        participant->rtp.port = port;
        participant->rtp.addr = addr;

    } else if (protocol == RTSP){
        //TODO: init RTSP struct
        participant->rtsp.port = port;
        participant->rtsp.addr = addr;

    } else {
        //Error when introducing protocol
    }
    
    return participant;
}

int add_participant(participant_list_t *list, int id, io_type_t part_type, participant_protocol_t prot_type, char *addr, uint32_t port){
    struct participant_data *participant;
  
    participant = init_participant(id, part_type, prot_type, addr, port);
  
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
    if(participant->ssrc == ssrc){
      return participant;
    }
    if (participant->ssrc == 0){
      return participant;
    }
    participant = participant->next;
  }

  return NULL;
}

int remove_participant(participant_list_t *list, uint32_t id){
    participant_data_t *participant;
  
    if (list->count == 0) {
        return FALSE;
    }
  
    participant = get_participant_id(list, id);

    if (participant == NULL)
        return FALSE;

    pthread_mutex_lock(&participant->lock);
  
    if (participant->type == INPUT && participant->streams_count > 0){
        remove_participant_stream(participant, participant->streams[0]);
        //TODO: where to execute remove stream???
    }

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

// void set_active_participant(participant_data_t *participant, uint8_t active) {
	
//     pthread_mutex_lock(&participant->lock);
//   	assert(active == TRUE || active == FALSE);

// 	  if (active == FALSE){
// 		    participant->active = active;
// 	  } else if (participant->active == FALSE) {
// 		    participant->active = I_AWAIT;
// 	  }
	
// 	  pthread_mutex_unlock(&participant->lock);
// }

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

    if (participant->streams_count == MAX_PARTICIPANT_STREAMS) {
      error_msg("Max number of streams per participant reached.\n");
      pthread_mutex_unlock(&participant->lock);
      return FALSE;
    }

    //TODO: manage participant stream list pointer
    int ret = FALSE;
    int i = 0;
    while (i++ < MAX_PARTICIPANT_STREAMS) {
        if (participant->streams[i] == NULL) {
            participant->streams[i] = stream;
            participant->streams_count++;
            assert(participant->streams_count <= MAX_PARTICIPANT_STREAMS);
            ret = TRUE;
            break;
        }
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
            participant->streams_count--;
            assert(participant->streams_count >= 0);
            ret = TRUE;
            break;
        }
    }
    pthread_mutex_unlock(&participant->lock);
    return ret;
}
