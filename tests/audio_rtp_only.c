/*
 *
 * Main program to retransmit audio using only RTP to RTP.
 * Based on UltraGrid code.
 *
 * By Txor <jordi.casas@i2cat.net>
 *
 */

#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "rtp/rtp.h"
#include "rtp/rtp_callback.h"
#include "pdb.h"
#include "tv.h"
#include "perf.h"

#define PORT_AUDIO              5006
#define PT_AUDIO        21


/**************************************
 *     Variables 
 */

static volatile bool stop = false;

// Like state_audio (audio/audio.c:105)
struct thread_data {
	struct rtp *rtp_session;
	struct pdb *participants;

	struct timeval start_time;

	pthread_t audio_sender_thread_id,
		  thread_id;
};

/**************************************
 *     Functions 
 */
static void signal_handler(int signal)
{
//	debug_msg("Caught signal %d\n", signal); 
	stop = true;
	return;
}


// Function to prepare an RTP session.
// Based on initialize_audio_network (audio/audio.c:436) and initialize_network (main.c:362)
static struct rtp *init_network(char *addr, int recv_port,
		int send_port, struct pdb *participants, bool use_ipv6)
{
	struct rtp *r;
	double rtcp_bw = 1024 * 512;
	char *mcast_if = NULL;                // We aren't going to use multicast.

	r = rtp_init_if(addr, mcast_if, recv_port, send_port, 255, rtcp_bw,
			false, rtp_recv_callback, (void *)participants,
			use_ipv6);
	if (r != NULL) {
		pdb_add(participants, rtp_my_ssrc(r));
		rtp_set_option(r, RTP_OPT_WEAK_VALIDATION, TRUE);
		rtp_set_sdes(r, rtp_my_ssrc(r), RTCP_SDES_TOOL,
				PACKAGE_STRING, strlen(PACKAGE_VERSION));
	}

	return r;
}

// Function to prepare an RTP session.
// Based on thread (audio/audio.c:439)
static void *receiver_thread(void *arg)
{
	struct thread_data *d = arg;

	struct timeval timeout, curr_time;
	uint32_t ts;

	printf("Audio receiving started.\n");
	while (!stop) {
		gettimeofday(&curr_time, NULL);
		ts = tv_diff(curr_time, d->start_time) * 90000;
		rtp_update(d->rtp_session, curr_time);
		rtp_send_ctrl(d->rtp_session, ts, 0, curr_time);
		timeout.tv_sec = 0;
		timeout.tv_usec = 999999 / 59.94; /* audio goes almost always at the same rate
						     as video frames */
		rtp_recv_r(d->rtp_session, &timeout, ts);
	}
	printf("Audio receiving stoped.\n");
	return NULL;
}

// Function to prepare an RTP session.
// Based on thread (audio/audio.c:593)
static void *sender_thread(void *arg)
{
	struct thread_data *d = arg;

	uint32_t timestamp;

	unsigned int m = 0u;

	int data_len;
	char *data;
	// see definition in rtp_callback.h
	uint32_t hdr_data[100];
	uint32_t *audio_hdr = hdr_data;

	int rtp_hdr_len = sizeof(audio_payload_hdr_t);
	int pt = PT_AUDIO;

	timestamp = get_local_mediatime();
	perf_record(UVP_SEND, timestamp);

	printf("Audio sending started.\n");
	while (!stop) {
		rtp_send_data_hdr(d->rtp_session, timestamp, pt, m, 0,        /* contributing sources */
				0,        /* contributing sources length */
				(char *) audio_hdr, rtp_hdr_len,
				data, data_len,
				0, 0, 0);
	}
	printf("Audio sending stoped.\n");
	return NULL;
}

int main(int argc, char *argv[])
{

	signal(SIGINT, signal_handler);

	// Network options
	char *host = "localhost";
	uint16_t recv_port = PORT_AUDIO;
	uint16_t send_port = PORT_AUDIO + 2;

	// Option processing
	int tmp;
	printf("Example program for RTP-RTP audio transmission.\n");
	switch (argc) {
		case 1: 
			break;
		case 2:	
			tmp = atoi(argv[1]);
			if (1024 <= tmp && tmp <= 65535 && tmp % 2 == 0) {
				recv_port = tmp;
			}
			else {
				printf("I don't like %s as receive_port! >:(\n", argv[1]);
			}
			break;
		case 3:	
			tmp = atoi(argv[1]);
			if (1024 <= tmp && tmp <= 65535 && tmp % 2 == 0) {
				recv_port = tmp;
			}
			else {
				printf("I don't like %s as receive_port! >:(\n", argv[1]);
			}
			tmp = atoi(argv[2]);
			if (1024 <= tmp && tmp <= 65535 && tmp % 2 == 0) {
				send_port = tmp;
			}
			else {
				printf("I don't like %s as receive_port! >:(\n", argv[2]);
			}
			break;
		default:
			printf("Usage: %s [receive_port [send_port]]\n", argv[0]);
			exit(0);
			break;
	}
	printf("receive_port: %i\n", recv_port);
	printf("send_port: %i\n", send_port);

	// Setup two RTP sessions.
	struct rtp *receiver_session;
	struct pdb *receiver_participants;

	receiver_participants = pdb_init();
	receiver_session = init_network(host, recv_port, recv_port, receiver_participants, false);

	struct rtp *sender_session;
	struct pdb *sender_participants;

	sender_participants = pdb_init();
	sender_session = init_network(host, send_port, send_port, receiver_participants, false);


	// Creation of two threads like audio_cfg_init (audio/audio.c:180) does.
	struct thread_data *receiver_data = calloc(1, sizeof(struct thread_data));
	receiver_data->rtp_session = receiver_session;
	receiver_data->participants = receiver_participants;
	gettimeofday(&receiver_data->start_time, NULL);

	if (pthread_create(&receiver_data->thread_id, NULL, receiver_thread, (void *)receiver_data) != 0) {
		fprintf(stderr, "Error creating receiver thread.\n");
		exit(-1); // Ugly exit...
	}

	struct thread_data *sender_data = calloc(1, sizeof(struct thread_data));
	sender_data->rtp_session = sender_session;
	sender_data->participants = sender_participants;
	gettimeofday(&sender_data->start_time, NULL);

	if (pthread_create(&sender_data->thread_id, NULL, sender_thread, (void *)sender_data) != 0) {
		fprintf(stderr, "Error creating sender thread.\n");
		exit(-1); // Ugly exit...
	}

	pthread_join(receiver_data->thread_id, NULL);
	pthread_join(sender_data->thread_id, NULL);

	// Job finished!
	printf("Done!\n");

}

