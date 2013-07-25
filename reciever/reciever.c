#include "rtp/rtp.h"
#include "rtp/rtp_callback.h"
#include "rtp/rtpdec_h264.h"
#include "rtp/rtpenc.h"
#include "pdb.h"
#include "video.h"
#include "tv.h"
#include "debug.h"

#include "video_decompress.h"
#include "video_decompress/libavcodec.h"

#define INITIAL_VIDEO_RECV_BUFFER_SIZE  ((4*1920*1080)*110/100) //command line net.core setup: sysctl -w net.core.rmem_max=9123840

#include "participants.h"

void decoder_th(void* participant){
      participant_data_t *src = (participant_data_t *) participant;
      
      //TODO: this should be reconfigurable
      src->frame_length = vc_get_linesize(src->width, RGB)*src->height;
      
      pthread_mutex_lock(&src->proc.decoder->lock);
      while(src->proc.decoder->run){
	
	while(! src->proc.decoder->new_frame)
	  pthread_cond_wait(&src->proc.decoder->notify_frame, &src->proc.decoder->lock);
	
	src->proc.decoder->new_frame = FALSE;
	pthread_mutex_lock(&src->lock);
	//decompress_frame(src->proc.decoder->sd,(unsigned char *) src->frame,(unsigned char *)src->proc.decoder->data,src->proc.decoder->data_len,0);
	sleep(1);
	printf("I am so cool, I am God, I am %u, better than other participants\n", src->ssrc);
	src->new = TRUE;
	pthread_mutex_unlock(&src->lock);
      }
      
      pthread_mutex_unlock(&src->proc.decoder->lock);

      decompress_done(src->proc.decoder->sd);
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

int start_input(void *participants) {
    struct rtp *session = NULL;
    struct pdb *part_db;
    struct pdb_e *cp;
    participant_data_t *src;
    participant_list_t *list = (participant_list_t *) participants;

    double rtcp_bw = 5 * 1024 * 1024; /* FIXME */
    int ttl = 255;
    // char *saveptr = NULL;
    char *addr="127.0.0.1";
    char *mcast_if= NULL;
    struct timeval curr_time;
    struct timeval timeout;
    struct timeval prev_time;
    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    // uint32_t ts;
    int recv_port = 5004;
    int send_port = 7004;

    gettimeofday(&prev_time, NULL);

    timeout.tv_sec = 0;
    timeout.tv_usec = 10000;
    uint32_t timestamp; //TODO: why is this used
        
    session = (struct rtp *) malloc(sizeof(struct rtp *));
    part_db = pdb_init();

    
    session = rtp_init_if(addr, mcast_if, recv_port, send_port, ttl, rtcp_bw, 0, rtp_recv_callback, (void *)part_db, 0);
      
      
    if (session != NULL) {
      if (!rtp_set_option(session, RTP_OPT_WEAK_VALIDATION, 1)) {
	return -1;
      }
      
      if (!rtp_set_sdes(session, rtp_my_ssrc(session),
	RTCP_SDES_TOOL, PACKAGE_STRING, strlen(PACKAGE_STRING))) {
	return -1;
      }

      if (!rtp_set_recv_buf(session, INITIAL_VIDEO_RECV_BUFFER_SIZE)) {
	return -1;
      }

    }
    
    struct recieved_data *rx_data = calloc(1, sizeof(struct recieved_data));

    while(TRUE){
        gettimeofday(&curr_time, NULL);
        timestamp = tv_diff(curr_time, start_time) * 90000;
        rtp_update(session, curr_time);
        rtp_send_ctrl(session, timestamp, 0, curr_time);

        timeout.tv_sec = 0;
        timeout.tv_usec = 10000;

        if (!rtp_recv_r(session, &timeout, timestamp)){
	    pdb_iter_t it;
	    cp = pdb_iter_init(part_db, &it);
	    
     	    while (cp != NULL ) {
	      
	      pthread_rwlock_rdlock(&list->lock);
	      src = get_participant_ssrc(list, cp->ssrc);
	      pthread_rwlock_unlock(&list->lock);
	      
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

    
    
    pthread_rwlock_rdlock(&list->lock);
    src = list->first;
    pthread_rwlock_unlock(&list->lock);
    while(src != NULL){
      pthread_join(src->proc.decoder->th_id, NULL);
      pthread_mutex_lock(&src->lock);
      src = src->next;
      pthread_mutex_unlock(&src->lock);
    }
    
    destroy_list(list);
    rtp_done(session);
    
    return 0;
}
