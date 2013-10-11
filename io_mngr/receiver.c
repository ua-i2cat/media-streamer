#include "config_unix.h"
#include "receiver.h"
#include "rtp/rtp_callback.h"
#include "rtp/rtp.h"
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

//TODO: refactor de la funció per evitar tants IF anidats
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

    while(receiver->run){
        gettimeofday(&curr_time, NULL);
		timestamp = tv_diff(curr_time, start_time) * 90000;
		rtp_update(receiver->session, curr_time);
        //rtp_send_ctrl(receiver->session, timestamp, 0, curr_time); //TODO: why is this used?

		timeout.tv_sec = 0;
		timeout.tv_usec = 10000;
	
		//TODO: repàs dels locks en accedir a src
		if (!rtp_recv_r(receiver->session, &timeout, timestamp)){
			pdb_iter_t it;
			cp = pdb_iter_init(receiver->part_db, &it);
	    
			while (cp != NULL) {
	      
				pthread_rwlock_rdlock(&receiver->list->lock);
				src = get_participant_ssrc(receiver->list, cp->ssrc);
				pthread_rwlock_unlock(&receiver->list->lock);

				if (src != NULL && src->active > 0) {

					if (pbuf_decode(cp->playout_buffer, curr_time, decode_frame_h264, src->rx_data)) {	 

						gettimeofday(&curr_time, NULL);

						if (src->active == I_AWAIT && src->rx_data->frame_type == INTRA){
							if (src->proc.decoder == NULL && src->rx_data->info.width != 0 && src->rx_data->info.height != 0){
								pthread_mutex_lock(&src->lock);
								src->ssrc = cp->ssrc;
								src->width = src->rx_data->info.width;
								src->height = src->rx_data->info.height;
								src->frame = malloc(vc_get_linesize(src->rx_data->info.width, RGB)*src->rx_data->info.height);
								pthread_mutex_unlock(&src->lock);
								init_decoder(src);
								src->active = TRUE;
							} else if (src->proc.decoder != NULL) {
								src->active = TRUE;
							}
						}

						

						if (src->active == TRUE && src->rx_data->frame_type != BFRAME) {
							
							pthread_mutex_lock(&src->lock);
							pthread_mutex_lock(&src->proc.decoder->lock); 
 		  
							memcpy(src->proc.decoder->data, src->rx_data->frame_buffer[0], src->rx_data->buffer_len[0]); //TODO: get rid of this magic number
							src->proc.decoder->data_len = src->rx_data->buffer_len[0];
							src->proc.decoder->new_frame = TRUE;
							pthread_cond_signal(&src->proc.decoder->notify_frame);
							
							pthread_mutex_unlock(&src->proc.decoder->lock);
							pthread_mutex_unlock(&src->lock);
		    
						} else {
							debug_msg("No support for Bframes\n"); //TODO: test it properly, it should not cause decoding damage
						}
						
						pbuf_remove_first(cp->playout_buffer);
					}
				} else {
				//TODO: delete cp form pdb or ignore it
				}
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
