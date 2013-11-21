#include "config_unix.h"
#include "participants.h"
#include "transmitter.h"
#include "video_decompress/libavcodec.h"
#include "video_decompress.h"
#include "debug.h"

void destroy_participant(participant_data_t *src);
int remove_participant(participant_list_t *list, uint32_t id);
void destroy_participant_list(participant_list_t *list);

void dummy_callback(struct rtp *session, rtp_event *e)
{
    UNUSED(session);
    UNUSED(e);
}

rtp_session_t * init_rtp_session(uint32_t port, char *addr){
    rtp_session_t *rtp;
    char *mcast_if = NULL;
    double rtcp_bw = DEFAULT_RTCP_BW;
    int ttl = DEFAULT_TTL;
    struct module tmod;
    struct tx *tx_session;
    
    rtp = (rtp_session_t *) malloc(sizeof(rtp_session_t));
    if (rtp == NULL){
        error_msg("rtp_session: malloc error");
        return NULL;
    }

    rtp->port = port;
    rtp->addr = addr;
    
    module_init_default(&tmod);

    struct rtp *rtp_conn  = rtp_init_if(addr, mcast_if,
                                   0, port, ttl,
                                   rtcp_bw, 0, dummy_callback,
                                   (void *)NULL, 0);
     if (rtp_conn == NULL){
        error_msg("rtp_session: rtp_init error");
        return NULL;
    }
    
    rtp_set_option(rtp_conn, RTP_OPT_WEAK_VALIDATION, 1);
    rtp_set_sdes(rtp_conn, rtp_my_ssrc(rtp_conn), RTCP_SDES_TOOL, PACKAGE_STRING, strlen(PACKAGE_STRING));
    rtp_set_send_buf(rtp_conn, DEFAULT_SEND_BUFFER_SIZE);

    tx_session = tx_init_h264(&tmod, MTU, TX_MEDIA_VIDEO, NULL, NULL);
    if (tx_session == NULL){
        error_msg("rtp_session: tx_init error");
        return NULL;
    }
    
    rtp->rtp = rtp_conn;
    rtp->tx_session = tx_session;
    
    return rtp;
}

int destroy_rtp_session(rtp_session_t *rtp){
    if (rtp == NULL){
        return TRUE;
    }
    if (rtp->rtp != NULL){
        rtp_send_bye(rtp->rtp);
        rtp_done(rtp->rtp);
    }
    //TODO: deleting module not working...
//     if (rtp->tx_session != NULL){
//         module_done(CAST_MODULE(rtp->tx_session));
//     }

    free(rtp->addr);
    free(rtp);
    
    return TRUE;
}

participant_data_t *init_participant(uint32_t id, io_type_t type, char *addr, uint32_t port){
    participant_data_t *participant;
  
    participant = (participant_data_t *) malloc(sizeof(participant_data_t));
  
    pthread_mutex_init(&participant->lock, NULL);
    participant->id = id;
    participant->ssrc = 0;
    participant->next = participant->previous = NULL;
    participant->type = type;
    
    if (type == OUTPUT){
        participant->rtp = init_rtp_session(port, addr);
        if (participant->rtp == NULL){
            return NULL;
        }
    } else {
        participant->rtp = NULL;
    }
    
    return participant;
}

int add_participant(participant_list_t *list, participant_data_t *participant)
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
        pthread_rwlock_unlock(&list->lock);
        return FALSE;
    }
    pthread_rwlock_unlock(&list->lock);
  
    return TRUE;
}

void destroy_participant(participant_data_t *src){
    pthread_mutex_destroy(&src->lock);
    destroy_rtp_session(src->rtp);
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
  while(participant != NULL) {

    if(participant->id == id) {
      return participant; 
    }
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
        if (participant->ssrc == 0)
            return participant;
        participant = participant->next;
    }
    return NULL;
}

int set_participant_ssrc(participant_data_t *participant, uint32_t ssrc){
    pthread_mutex_lock(&participant->lock);
    participant->ssrc = ssrc;
    pthread_mutex_unlock(&participant->lock);
    return TRUE;
}

int remove_participant(participant_list_t *list, uint32_t id){
    participant_data_t *participant;

    pthread_rwlock_wrlock(&list->lock);
  
    if (list->count == 0) {
        pthread_rwlock_unlock(&list->lock);
        return FALSE;
    }
  
    participant = get_participant_id(list, id);

    if (participant == NULL) {
        pthread_rwlock_unlock(&list->lock);
        return FALSE;
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



