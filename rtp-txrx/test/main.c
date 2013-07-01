#include "rtp/rtp.h"
#include "rtp/rtp_callback.h"
#include "rtp/rtpdec.h"
#include "pdb.h"
#define INITIAL_VIDEO_RECV_BUFFER_SIZE  262142//((4*1920*1080)*110/100)



int main(){
	struct rtp **devices = NULL;
	struct pdb *participants;
	struct pdb_e *cp;
	    
	

	double rtcp_bw = 5 * 1024 * 1024; /* FIXME */
	int ttl = 255;
	char *saveptr = NULL;
	char *addr="127.0.0.1";
	char *mcast_if= NULL;
	struct timeval curr_time;
	struct timeval timeout;
	struct timeval prev_time;

	int required_connections;
	uint32_t ts;
	int recv_port = 5004;
	int send_port = 6004;
	int index=0;
	int exit = 1;

	gettimeofday(&prev_time, NULL);

	timeout.tv_sec = 0;
	timeout.tv_usec = 10000;
	
		
	required_connections = 1;

	devices = (struct rtp **) malloc((required_connections + 1) * sizeof(struct rtp *));
	participants = pdb_init();

	printf("RTP INIT IFACE\n");
	devices[index] = rtp_init_if(addr, mcast_if, recv_port, send_port, ttl, rtcp_bw, 0, rtp_recv_callback, participants, 0);
	
	if (devices[index] != NULL) {
		printf("RTP INIT OPTIONS\n");
		if (!rtp_set_option(devices[index], RTP_OPT_WEAK_VALIDATION, 1)) {
			printf("RTP INIT OPTIONS FAIL 1: set option\n");
			return -1;
		}
		if (!rtp_set_sdes(devices[index], rtp_my_ssrc(devices[index]),
				RTCP_SDES_TOOL, PACKAGE_STRING, strlen(PACKAGE_STRING))) {
			printf("RTP INIT OPTIONS FAIL 2: set sdes\n");
			return -1;
		}


		INITIAL_VIDEO_RECV_BUFFER_SIZE;
		int ret = rtp_set_recv_buf(devices[index],
				INITIAL_VIDEO_RECV_BUFFER_SIZE);
		if (!ret) {
			printf("RTP INIT OPTIONS FAIL 3: set recv buf\n");
			return -1;
		}

		if (!rtp_set_send_buf(devices[index], 1024 * 56)) {
			printf("RTP INIT OPTIONS FAIL 4: set send buf\n");
			return -1;
		}
	}

	gettimeofday(&curr_time, NULL);
	while(exit){
		rtp_update(devices[index], curr_time);
		rtp_send_ctrl(devices[index], 0, 0, curr_time);

		if (!rtp_recv_poll_r(devices, &timeout, 0)){
			printf("PACKET NOT RECIEVED\n");
			sleep(1);
		} else {
		  uint32_t xec = tv_diff_usec(curr_time, prev_time);
		  printf("interlooping time: %d\n", xec);
		  if (xec > 900){
			  gettimeofday(&prev_time, NULL);
			  pdb_iter_t it;
			  cp = pdb_iter_init(participants, &it);
			  int ret;
			  printf("PACKET RECIEVED, building FRAME\n");
			  while(cp != NULL){
			    ret = pbuf_decode(cp->playout_buffer, curr_time, decode_frame, NULL);
			    printf("DECODE return value: %d\n", ret);
			    if (ret){
			      printf("FRAME RECIEVED\n");
			      exit = 0;
			    } else {
			      printf("FRAME not ready yet\n");
			    }
			    pbuf_remove(cp->playout_buffer, curr_time);
			    cp = pdb_iter_next(&it);
			  }
		  } else {
		    printf("PACKET RECIEVED, not yet for FRAME\n");
		  }
		}

//FRAME REFLECTOR
/*		pdb_iter_t it;
		cp = pdb_iter_init(participants, &it);
		while (cp != NULL) {
			if (tfrc_feedback_is_due(cp->tfrc_state, curr_time)) {
				printf("tfrc rate %f\n",
						tfrc_feedback_txrate(cp->tfrc_state, curr_time));
			}
			 if (pbuf_decode(cp->playout_buffer, curr_time, decode_frame, cp->video_decoder_state)){
				 gettimeofday(&curr_time, NULL);

			 }





			pbuf_remove(cp->playout_buffer, curr_time);
			cp = pdb_iter_next(&it);
		}
		pdb_iter_done(&it);*/
//END FRAME REFLECTOR
	  gettimeofday(&curr_time, NULL);
	}

	rtp_done(devices[index]);
	printf("RTP DONE\n");


}
