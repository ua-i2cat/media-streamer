/*
 * main.c

 *
 *  Created on: Jun 17, 2013
 *      Author: gerardcl
 */

#include "rtp/rtp.h"
#define INITIAL_VIDEO_RECV_BUFFER_SIZE  ((4*1920*1080)*110/100)
#define PACKAGE_STRING "rtp-module"


int main(){
	struct rtp **devices = NULL;
	double rtcp_bw = 5 * 1024 * 1024; /* FIXME */
	int ttl = 255;
	char *saveptr = NULL;
	char *addr="127.0.0.1";
	char *mcast_if= NULL;

	int required_connections;
	int recv_port = 5004;
	int send_port = 5004;
	int index=0;

	required_connections = 1;

	devices = (struct rtp **) malloc((required_connections + 1) * sizeof(struct rtp *));
	printf("RTP INIT IFACE\n");
	devices[index] = rtp_init_if(addr, mcast_if, recv_port, send_port, ttl,
				rtcp_bw, 0, NULL, NULL,
				0);
	if (devices[index] != NULL) {
		printf("RTP INIT OPTIONS\n");
		if (!rtp_set_option(devices[index], RTP_OPT_WEAK_VALIDATION, 1)) {
			printf("RTP INIT OPTIONS FAIL 1\n");
			return -1;
		}
		if (!rtp_set_sdes(devices[index], rtp_my_ssrc(devices[index]),
				RTCP_SDES_TOOL, PACKAGE_STRING, strlen(PACKAGE_STRING))) {
			printf("RTP INIT OPTIONS FAIL 2\n");
			return -1;
		}


		int size = INITIAL_VIDEO_RECV_BUFFER_SIZE;
		int ret = rtp_set_recv_buf(devices[index],
				INITIAL_VIDEO_RECV_BUFFER_SIZE);
		if (!ret) {
			printf("RTP INIT OPTIONS FAIL 3\n");
			return -1;
		}
		printf("RTP DONE\n");

		rtp_set_send_buf(devices[index], 1024 * 56);
	}


	rtp_done(devices[index]);


}
