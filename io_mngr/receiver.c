#include "config_unix.h"
#include "receiver.h"
#include "rtp/rtp_callback.h"
#include "rtp/rtp.h"
#include "pdb.h"
#include "video_decompress.h"
#include "tv.h"
#include "debug.h"

#define INITIAL_VIDEO_RECV_BUFFER_SIZE  ((4*1920*1080)*110/100) //command line net.core setup: sysctl -w net.core.rmem_max=9123840

void *receiver_thread(receiver_t *receiver);

receiver_t *init_receiver(participant_list_t *participant_list, stream_list_t *stream_list, uint32_t port){
    receiver_t *receiver;
    double rtcp_bw = 5 * 1024 * 1024; /* FIXME */
    int ttl = 255; //TODO: get rid of magic numbers!
    
    receiver = malloc(sizeof(receiver_t));
    
    receiver->session = (struct rtp *) malloc(sizeof(struct rtp *));
    receiver->part_db = pdb_init();
    receiver->run = FALSE;
    receiver->port = port;
    receiver->participant_list = participant_list;
    receiver->stream_list = stream_list;
    
 
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
	participant_data_t *participant;
	received_data_t *rx_data;

	struct timeval curr_time;
	struct timeval timeout;
	struct timeval start_time;
	gettimeofday(&start_time, NULL);
    
	timeout.tv_sec = 0;
    timeout.tv_usec = 10000;
    uint32_t timestamp; //TODO: why is this used

    rx_data = init_rx_data();

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
				pthread_rwlock_rdlock(&receiver->participant_list->lock);
				participant = get_participant_ssrc(receiver->participant_list, cp->ssrc);
				pthread_rwlock_unlock(&receiver->participant_list->lock);

				if (participant != NULL) {
					if (pbuf_decode(cp->playout_buffer, curr_time, decode_frame_h264, rx_data)) {	

						gettimeofday(&curr_time, NULL);

						if (participant->streams_count == 0){ //In reception we only contemplate one stream per participant
							int id = 0; //TODO: ID generation
							stream_data_t *stream = init_stream(VIDEO, INPUT, id, I_AWAIT);
							add_stream(receiver->stream_list, stream);
							add_participant_stream(participant, stream);
							participant->streams_count ++;
						}

						if (rx_data->info.width != 0 && rx_data->info.height != 0){
							if (participant->streams[0]->decoder == NULL){
								set_stream_video_data(participant->streams[0], H264, rx_data->info.width, rx_data->info.height);
							}

							if (rx_data->info.width != participant->streams[0]->video.width || rx_data->info.height != participant->streams[0]->video.height){
								//TODO: reconfigure decoder
							}
						}

						if (participant->streams[0]->state == I_AWAIT && rx_data->frame_type == INTRA){
							participant->streams[0]->state = ACTIVE;
						}

						if (participant->streams[0]->state == ACTIVE && rx_data->frame_type != BFRAME) {
							
							pthread_mutex_lock(&participant->lock);

							pthread_rwlock_wrlock(&participant->streams[0]->video.lock); 
							memcpy(participant->streams[0]->video.coded_frame, rx_data->frame_buffer[0], rx_data->buffer_len[0]); //TODO: get rid of this magic number
							participant->streams[0]->video.coded_frame_len = rx_data->buffer_len[0];
							participant->streams[0]->video.coded_frame_seqno ++;
							pthread_rwlock_unlock(&participant->streams[0]->video.lock); 


							pthread_mutex_lock(&participant->streams[0]->video.new_coded_frame_lock);
							participant->streams[0]->video.new_coded_frame = TRUE;
							pthread_cond_signal(&participant->streams[0]->decoder->notify_frame);
							pthread_mutex_unlock(&participant->streams[0]->video.new_coded_frame_lock);
							
							pthread_mutex_unlock(&participant->lock);
		    
						} else {
							debug_msg("No support for Bframes\n"); //TODO: test it properly, it should not cause decoding damage
						}
						
						pbuf_remove_first(cp->playout_buffer);
					}
				} else {
				//TODO: delete cp form pdb or ignore it
				}
				cp = pdb_iter_next(&it);
				reset_rx_data(rx_data);
			}
			pdb_iter_done(&it);
		}
	}
  	
  	destroy_rx_data(rx_data);
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
