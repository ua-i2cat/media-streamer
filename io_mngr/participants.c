#include "config_unix.h"
#include "participants.h"
#include "transmitter.h"
#include "video_decompress/libavcodec.h"
#include "video_decompress.h"


participant_data_t *init_participant(int id, int width, int height, codec_t codec, char *addr, uint32_t port, ptype_t type);
void destroy_decoder_thread(decoder_thread_t *dec_th);
void destroy_participant(participant_data_t *src);
int remove_participant(participant_list_t *list, uint32_t id);
void destroy_participant_list(participant_list_t *list);

participant_data_t *init_participant(int id, participant_type_t type, participant_protocol_t protocol, char *addr, uint32_t port){
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

int add_participant(participant_list_t *list, int id, participant_type_t part_type, participant_protocol_t prot_type, char *addr, uint32_t port){
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
      assert(participant->proc.decoder == NULL);
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
  
  if (participant->type == INPUT && participant->proc.decoder != NULL
    && participant->proc.decoder->run == TRUE){

    participant->proc.decoder->run = FALSE;
    pthread_cond_signal(&participant->proc.decoder->notify_frame);
    pthread_mutex_unlock(&participant->lock);
    
    pthread_join(participant->proc.decoder->th_id, NULL); //TODO: timeout to force thread kill //TODO unlock 
    
    pthread_mutex_lock(&participant->lock);
 
  } else if (participant->type == OUTPUT /*&& participant->proc.encoder->run == TRUE*/){
    destroy_encoder_thread(participant);
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

void set_active_participant(participant_data_t *participant, uint8_t active) {
	
	pthread_mutex_lock(&participant->lock);
	assert(active == TRUE || active == FALSE);

	if (active == FALSE){
		participant->active = active;
	} else if (participant->active == FALSE) {
		participant->active = I_AWAIT;
	}
	
	pthread_mutex_unlock(&participant->lock);
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

int add_participant_stream(participant_data_t *participant, stream_data_t *stream){
    pthread_mutex_lock(&participant->lock);

    if (participant->streams_count == MAX_PARTICIPANT_STREAMS){
      debug_msg("Max number of streams per participant reached.\n");
      pthread_mutex_unlock(&participant->lock);
      return -1;
    }

    //TODO: manage participant stream list pointer

    pthread_mutex_unlock(&participant->lock);
}

int remove_participant_stream(participant_data_t *participant, stream_data_t *stream){

}

