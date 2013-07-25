#include "participants.h"
#include "reciever.h"
#include "rtp/rtp.h"
#include "rtp/rtp_callback.h"
#include "rtp/rtpdec_h264.h"
#include "pdb.h"
#include "tv.h"

#define INITIAL_VIDEO_RECV_BUFFER_SIZE  ((4*1920*1080)*110/100) //command line net.core setup: sysctl -w net.core.rmem_max=9123840

void decoder_th(void* participant){
      participant_data_t *src = (participant_data_t *) participant;
      
      //TODO: this should be reconfigurable
      src->frame_length = vc_get_linesize(src->width, RGB)*src->height;
      
      printf("NEW decoder %d %d\n", src->proc.decoder->th_id, src->id);
      
      pthread_mutex_lock(&src->proc.decoder->lock);
      while(src->proc.decoder->run){
	
	while(! src->proc.decoder->new_frame && src->proc.decoder->run)
	  pthread_cond_wait(&src->proc.decoder->notify_frame, &src->proc.decoder->lock); //TODO: some timeout

	if (src->proc.decoder->run){
	  src->proc.decoder->new_frame = FALSE;
	  pthread_mutex_lock(&src->lock);
	  decompress_frame(src->proc.decoder->sd,(unsigned char *) src->frame, (unsigned char *)src->proc.decoder->data, src->proc.decoder->data_len, 0);
	  src->new = TRUE;
	  pthread_mutex_unlock(&src->lock);
	} 
      }

      pthread_mutex_unlock(&src->proc.decoder->lock);
}

void init_decoder(participant_data_t *src){
      assert(src->proc.decoder == NULL);
  
      pthread_mutex_lock(&src->lock);
      src->proc.decoder = init_decoder_thread(src);
      src->proc.decoder->run = TRUE;
      pthread_mutex_unlock(&src->lock);
     
      if (pthread_create(&src->proc.decoder->th_id, NULL, &decoder_th, src) != 0)
	src->proc.decoder->run = FALSE;

}

reciever_t *init_reciever(participant_list_t *list, int port){
    reciever_t *reciever;
    double rtcp_bw = 5 * 1024 * 1024; /* FIXME */
    int ttl = 255; //TODO: get rid of magic numbers!
    
    reciever = malloc(sizeof(reciever_t));
    
    reciever->session = (struct rtp *) malloc(sizeof(struct rtp *));
    reciever->part_db = pdb_init();
    reciever->run = FALSE;
    reciever->port = port;
    reciever->list = list;
    
    //TODO: this shouldn't open this ports and addr should be NULL
    reciever->session = rtp_init_if("127.0.0.1", NULL, reciever->port, 2*reciever->port, ttl, rtcp_bw, 0, rtp_recv_callback, (void *)reciever->part_db, 0);
      
    if (reciever->session != NULL) {
      if (!rtp_set_option(reciever->session, RTP_OPT_WEAK_VALIDATION, 1)) {
	return -1;
      }
      
      if (!rtp_set_sdes(reciever->session, rtp_my_ssrc(reciever->session),
	RTCP_SDES_TOOL, PACKAGE_STRING, strlen(PACKAGE_STRING))) { //TODO: is this needed?
	return -1;
      }

      if (!rtp_set_recv_buf(reciever->session, INITIAL_VIDEO_RECV_BUFFER_SIZE)) {
	return -1;
      }

    }
    
    return reciever;
}

int reciever_thread(reciever_t *reciever) {
    struct pdb_e *cp;
    participant_data_t *src;

    struct timeval curr_time;
    struct timeval timeout;
    struct timeval start_time;
    gettimeofday(&start_time, NULL);
    
    timeout.tv_sec = 0;
    timeout.tv_usec = 10000;
    uint32_t timestamp; //TODO: why is this used
    
    struct recieved_data *rx_data = calloc(1, sizeof(struct recieved_data));

    while(reciever->run){
        gettimeofday(&curr_time, NULL);
        timestamp = tv_diff(curr_time, start_time) * 90000;
        rtp_update(reciever->session, curr_time);
        rtp_send_ctrl(reciever->session, timestamp, 0, curr_time);

        timeout.tv_sec = 0;
        timeout.tv_usec = 10000;
	
        if (!rtp_recv_r(reciever->session, &timeout, timestamp)){
	    pdb_iter_t it;
	    cp = pdb_iter_init(reciever->part_db, &it);
	    
     	    while (cp != NULL ) {
	      
	      pthread_rwlock_rdlock(&reciever->list->lock);
	      src = get_participant_ssrc(reciever->list, cp->ssrc);
	      pthread_rwlock_unlock(&reciever->list->lock);
    
	      if (src != NULL && src->proc.decoder == NULL) {
		
		pthread_mutex_lock(&src->lock);
		src->ssrc = cp->ssrc;
		pthread_mutex_unlock(&src->lock);
		init_decoder(src);
		
	      } else if (src != NULL) {
		if (pbuf_decode(cp->playout_buffer, curr_time, decode_frame_h264, rx_data)) {	  
		  gettimeofday(&curr_time, NULL);
		  
		  pthread_mutex_lock(&src->proc.decoder->lock); 
		  
		  memcpy(src->proc.decoder->data, rx_data->frame_buffer[0], rx_data->buffer_len[0]); //TODO: get rid of this magic number
		  src->proc.decoder->data_len = rx_data->buffer_len[0];
		  src->proc.decoder->new_frame = TRUE;
		  pthread_cond_signal(&src->proc.decoder->notify_frame);
		  
		  pthread_mutex_unlock(&src->proc.decoder->lock);
		}
	      } else {
		//TODO: delete cp form pdb or ignore it
	      }
	      pbuf_remove(cp->playout_buffer, curr_time);
	      cp = pdb_iter_next(&it);
	    }
	    pdb_iter_done(&it);
        }
    }
  
    destroy_participant_list(reciever->list);
    rtp_done(reciever->session);
    free(reciever);
    
    return 0;
}

int start_reciever(reciever_t *reciever){
  reciever->run = TRUE;
  
  if (pthread_create(&reciever->th_id, NULL, &reciever_thread, (void *) reciever) != 0)
    reciever->run = FALSE;
  
  return reciever->run;
}

int stop_reciever(reciever_t *reciever){
  reciever->run = FALSE;
  
  pthread_join(reciever->th_id, NULL); //TODO: add some timeout
  
  return TRUE;
}
