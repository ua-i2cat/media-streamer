#include "config.h"
#include "participants.h"
#include "receiver.h"
#include "rtp/rtp.h"
#include "rtp/rtp_callback.h"
#include "rtp/rtpdec.h"
#include "pdb.h"
#include "video_decompress.h"
#include "tv.h"
#include "debug.h"

#define INITIAL_VIDEO_RECV_BUFFER_SIZE  ((4*1920*1080)*110/100) //command line net.core setup: sysctl -w net.core.rmem_max=9123840

void *decoder_th(void* participant){
      participant_data_t *src = (participant_data_t *) participant;
      
      //TODO: this should be reconfigurable
      src->frame_length = vc_get_linesize(src->width, RGB)*src->height;
      
      pthread_mutex_lock(&src->proc.decoder->lock);
      while(src->proc.decoder->run){
	
	while(! src->proc.decoder->new_frame && src->proc.decoder->run)
	  pthread_cond_wait(&src->proc.decoder->notify_frame, &src->proc.decoder->lock); //TODO: some timeout

	if (src->proc.decoder->run){
	  src->proc.decoder->new_frame = FALSE;
	  pthread_mutex_lock(&src->lock);
	  decompress_frame(src->proc.decoder->sd,(unsigned char *) src->frame, (unsigned char *)src->proc.decoder->data, src->proc.decoder->data_len, 0);
	  src->new_frame = TRUE;
	  pthread_mutex_unlock(&src->lock);
	} 
      }

      pthread_mutex_unlock(&src->proc.decoder->lock);
      pthread_exit((void *)NULL);    
}

void init_decoder(participant_data_t *src){
      assert(src->proc.decoder == NULL);
  
      pthread_mutex_lock(&src->lock);
      src->proc.decoder = init_decoder_thread(src);
      src->proc.decoder->run = TRUE;
      pthread_mutex_unlock(&src->lock);
     
      if (pthread_create(&src->proc.decoder->th_id, NULL, (void *) decoder_th, src) != 0)
	src->proc.decoder->run = FALSE;

}

receiver_t *init_receiver(participant_list_t *list, int port){
    receiver_t *receiver;
    double rtcp_bw = 5 * 1024 * 1024; /* FIXME */
    int ttl = 255; //TODO: get rid of magic numbers!
    
    receiver = malloc(sizeof(receiver_t));
    
    receiver->session = (struct rtp *) malloc(sizeof(struct rtp *));
    receiver->part_db = pdb_init();
    receiver->run = FALSE;
    receiver->port = port;
    receiver->list = list;
    
    //TODO: this shouldn't open this ports and addr should be NULL
    receiver->session = rtp_init_if(NULL, NULL, receiver->port, 0, ttl, rtcp_bw, 0, rtp_recv_callback, (void *)receiver->part_db, 0);
      
    if (receiver->session != NULL) {
      if (!rtp_set_option(receiver->session, RTP_OPT_WEAK_VALIDATION, 1)) {
	return NULL;
      }
      
      if (!rtp_set_sdes(receiver->session, rtp_my_ssrc(receiver->session),
	RTCP_SDES_TOOL, PACKAGE_STRING, strlen(PACKAGE_STRING))) { //TODO: is this needed?
	return NULL;
      }

      if (!rtp_set_recv_buf(receiver->session, INITIAL_VIDEO_RECV_BUFFER_SIZE)) {
	return NULL;
      }

    }
    
    return receiver;
}

void *receiver_thread(receiver_t *receiver) {
	struct pdb_e *cp;
	participant_data_t *src;

	struct timeval curr_time;
	struct timeval timeout;
	struct timeval start_time;
	gettimeofday(&start_time, NULL);
    
	timeout.tv_sec = 0;
    timeout.tv_usec = 10000;
    uint32_t timestamp; //TODO: why is this used
    uint8_t srclck;
    
    struct recieved_data *rx_data = calloc(1, sizeof(struct recieved_data));
    rx_data->frame_buffer[0] = malloc(1920*1080*4*sizeof(char));

    while(receiver->run){
        gettimeofday(&curr_time, NULL);
		timestamp = tv_diff(curr_time, start_time) * 90000;
		rtp_update(receiver->session, curr_time);
        //rtp_send_ctrl(receiver->session, timestamp, 0, curr_time); //TODO: why is this used?

		timeout.tv_sec = 0;
		timeout.tv_usec = 10000;
	
		if (!rtp_recv_r(receiver->session, &timeout, timestamp)){
			pdb_iter_t it;
			cp = pdb_iter_init(receiver->part_db, &it);
	    
			while (cp != NULL) {
	      
				pthread_rwlock_rdlock(&receiver->list->lock);
				src = get_participant_ssrc(receiver->list, cp->ssrc);
				pthread_rwlock_unlock(&receiver->list->lock);
				
				if (src != NULL && src->proc.decoder == NULL && (src->active > 0)) {
		
					pthread_mutex_lock(&src->lock);
					src->ssrc = cp->ssrc;
					pthread_mutex_unlock(&src->lock);
					init_decoder(src);
				} else if (src != NULL && src->active > 0) {
					if (pbuf_decode(cp->playout_buffer, curr_time, decode_frame_h264, rx_data)) {	  
						gettimeofday(&curr_time, NULL);
						
						if (src->active == I_AWAIT && rx_data->iframe){
							src->active = TRUE;
						}
						if (rx_data->width != NULL && rx_data->height != NULL){
							printf("Width = %lu Height = %lu\n", rx_data->width, rx_data->height);
						}
						
						srclck = pthread_mutex_trylock(&src->lock);
						
						if (src->active == TRUE && (srclck == 0 || !rx_data->bframe)) {
							
							if (!rx_data->bframe && srclck != 0){
								pthread_mutex_lock(&src->lock);
							}
														
							pthread_mutex_lock(&src->proc.decoder->lock); 
 		  
							memcpy(src->proc.decoder->data, rx_data->frame_buffer[0], rx_data->buffer_len[0]); //TODO: get rid of this magic number
							src->proc.decoder->data_len = rx_data->buffer_len[0];
							src->proc.decoder->new_frame = TRUE;
							pthread_cond_signal(&src->proc.decoder->notify_frame);
							
							pthread_mutex_unlock(&src->proc.decoder->lock);
							pthread_mutex_unlock(&src->lock);
		    
						} else {
							if (srclck == 0){
								pthread_mutex_unlock(&src->lock);
							}
							debug_msg("Warning: Frame missed!\n"); //TODO: test it properly, it should not cause decoding damage
						}
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
  
    destroy_participant_list(receiver->list);
    rtp_done(receiver->session);
    free(receiver);
    pthread_exit((void *)NULL);   
}

int start_receiver(receiver_t *receiver){
  receiver->run = TRUE;
  
  if (pthread_create(&receiver->th_id, NULL, (void *) receiver_thread, receiver) != 0)
    receiver->run = FALSE;
  
  return receiver->run;
}

int stop_receiver(receiver_t *receiver){
  receiver->run = FALSE;
  
  pthread_join(receiver->th_id, NULL); //TODO: add some timeout
  
  return TRUE;
}
