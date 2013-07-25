#include "participants.h"

participant_list_t *init_participant_list(){
  participant_list_t 	*list;
    
  list = (participant_list_t *) malloc(sizeof(participant_list_t));
  
  pthread_rwlock_init(&list->lock, NULL);
  
  list->count = 0;
  list->first = NULL;
  list->last = NULL;
  
  return list;
}

participant_data_t *init_participant(int width, int height, codec_t codec, char *dst, uint32_t port){
  participant_data_t *participant;
  rtp_session_t *rtp;
  
  
  participant = (participant_data_t *) malloc(sizeof(participant_data_t));
  
  pthread_mutex_init(&participant->lock, NULL);
  participant->new = FALSE;
  participant->ssrc = 0;
  participant->frame = NULL;
  participant->height = height;
  participant->width = width;
  participant->codec = codec;
  participant->proc.decoder = participant->proc.encoder = NULL;
  participant->next = participant->previous = NULL;
  
  if (dst == NULL) {
    participant->session = NULL;
  } else {
    rtp = (rtp_session_t *) malloc(sizeof(rtp_session_t));
    rtp->addr = dst;
    rtp->port = port;
    participant->session = rtp;
  }
  
  return participant;
}

int destroy_list(participant_list_t *list){
  pthread_rwlock_rdlock(&list->lock);
  if (list->count == 0){
    free(list);
     //TODO: is it needed to unlock, it doesn't exist anymore:P
  } else {
    struct participant_data *participant;
    struct participant_data *tmp;
    //TODO: debug_msg("Not an empty list, forcing remove participants");
    participant = list->first;
    
    while(participant != NULL){
      //TODO: destroy threads
      tmp = participant;
      participant = participant->next;
      pthread_mutex_lock(&tmp->lock);
      free(tmp);
      //TODO: is it needed to unlock, it doesn't exist anymore:P
    }
    free(list);
  }
  
  pthread_rwlock_unlock(&list->lock);
}

int add_participant(participant_list_t *list, int width, int height, codec_t codec, char *dst, uint32_t port){
  struct participant_data *participant;
  
  participant = init_participant(width, height, codec, dst, port);
  
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
    return -1;
  }
  
  pthread_rwlock_unlock(&list->lock);
  return 0;
}

participant_data_t *get_participant_id(participant_list_t *list, uint32_t id){
  participant_data_t *participant;
  
  pthread_rwlock_rdlock(&list->lock);
  
  participant = list->first;
  while(participant != NULL){
    if(participant->id == id){
      pthread_rwlock_unlock(&list->lock);
      return participant;
    }
    participant = participant->next;
  }
  
  pthread_rwlock_unlock(&list->lock);
  return NULL;
}

participant_data_t *get_participant_id_nolock(participant_list_t *list, uint32_t id){
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
  
  pthread_rwlock_rdlock(&list->lock);
  participant = list->first;
  while(participant != NULL){
    if(participant->ssrc == ssrc){
      pthread_rwlock_unlock(&list->lock);
      return participant;
    }
    if (participant->ssrc == 0){
      assert(participant->proc.decoder == NULL);
      pthread_rwlock_unlock(&list->lock);
      return participant;
    }
    participant = participant->next;
  }
  
  pthread_rwlock_unlock(&list->lock);
  return NULL;
}

//TODO: boolean returning type
int remove_participant(participant_list_t *list, uint32_t id){
  participant_data_t *participant;
  
  pthread_rwlock_wrlock(&list->lock);
  
  if (list->count == 0) {
    pthread_rwlock_unlock(&list->lock);
    return FALSE;
  }
  
  participant = get_participant_id_nolock(list, id);
  
  if (participant == NULL)
    pthread_rwlock_unlock(&list->lock);
    return FALSE;

  pthread_mutex_lock(&participant->lock);
  
  if (participant->next == NULL){
    assert(list->last == participant);
    list->last = participant->previous;
  } else if (participant->previous == NULL){
    assert(list->first == participant);
    list->first = participant->next;
  } else {
    assert(participant->next != NULL && participant->next != NULL);
    participant->previous->next = participant->next;
  }
  list->count--;
  
  pthread_mutex_unlock(&participant->lock);
  pthread_rwlock_unlock(&list->lock);
  
  free(participant);
  
  return TRUE;
}

