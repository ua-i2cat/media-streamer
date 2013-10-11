/*
 *
 * Main program to retransmit audio using only RTP to RTP.
 * Based on UltraGrid code.
 *
 * It creates two different RTP sessions managed by two different threads.
 * Each packet that arrives at the receiver RTP session is copied to the
 * sender RTP session, it is send away. 
 *
 * By Txor <jordi.casas@i2cat.net>
 *
 */

#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <getopt.h>
#include "rtp/rtp.h"
#include "rtp/pbuf.h"
#include "rtp/rtp_callback.h"
#include "pdb.h"
#include "tv.h"
#include "perf.h"

#define PORT_AUDIO	5006
//#define UNUSED(x)       (x=x)


/**************************************
 *     Variables 
 */

static volatile bool stop = false;
char data_placeholder[256];

// Like state_audio (audio/audio.c:105)
struct thread_data {
	// RTP related stuff
	struct rtp *rtp_session;
	struct pdb *participants;
	struct timeval start_time;

	// Thread related stuff
	pthread_t id;
	pthread_mutex_t *wait;            // Lock for wait.
	pthread_mutex_t *go;              // Lock to wake up our bro.
};

/**************************************
 *     Functions 
 */

// Prototypes
int copy_raw_frame(struct coded_data *cdata, void *data);
extern int audio_pbuf_decode(struct pbuf *playout_buf, struct timeval curr_time,
		decode_frame_t decode_func, void *data);

static void usage(void)
{
	printf("Usage: audio_rtp_only -h <remote-host> [-p <receive_port>] [-l <send_port>]\n");
	printf(" By Txor >:D\n");
}

// Crtl-C handler
static void signal_handler(int signal)
{
	if (signal) { // Avoid annoying warnings.
		stop = true;
	}
	return;
}

// Dummy callback to avoid decode anything.
int copy_raw_frame(struct coded_data *cdata, void *data) {
	memcpy(&data, cdata, sizeof(struct coded_data));
	return true;
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

// Function for receiver thread.
// Based on audio_receiver_thread (audio/audio.c:456)
static void *receiver_thread(void *arg)
{
	struct thread_data *d = arg;
	struct timeval timeout, curr_time;
	uint32_t ts;
	struct pdb_e *cp;
	void *data = NULL;

	printf(" Receiver started.\n");
	while (!stop) {
		// Preparate timeouts and perform RTP session maintenance.
		gettimeofday(&curr_time, NULL);
		ts = tv_diff(curr_time, d->start_time) * 90000;
		rtp_update(d->rtp_session, curr_time);
		rtp_send_ctrl(d->rtp_session, ts, 0, curr_time);
		timeout.tv_sec = 0;
		timeout.tv_usec = 999999 / 59.94;
		// Get packets from network.
		if (!rtp_recv_r(d->rtp_session, &timeout, ts)) {
			// Iterate throught participants
			pdb_iter_t it;
			cp = pdb_iter_init(d->participants, &it);
			while (cp != NULL) {
				// Get the data.
				if (audio_pbuf_decode(cp->playout_buffer, curr_time, copy_raw_frame, data)) {
					// Copy data on to the thread shared placeholder.
					memcpy(&data_placeholder, data, sizeof(char)*256);
				}
				cp = pdb_iter_next(&it);
			}
			pdb_iter_done(&it);
		}
		pthread_mutex_unlock(d->go);
		pthread_mutex_lock(d->wait);
	}
	// Finish RTP session and exit.
	rtp_done(d->rtp_session);
	printf(" Receiver stopped.\n");
	pthread_exit((void *)NULL);
}

// Function for sender thread.
// Based on audio_sender_thread (audio/audio.c:610)
static void *sender_thread(void *arg)
{
	struct thread_data *d = arg;
	uint32_t timestamp;
	unsigned int m = 0u;
	int pt = PT_AUDIO;

	timestamp = get_local_mediatime();
	perf_record(UVP_SEND, timestamp);

	printf(" Sender started.\n");
	while (!stop) {
		// Send the data away.
		rtp_send_data(d->rtp_session, timestamp, pt, m,
				0, 0,
				data_placeholder, sizeof(char)*256,
				0, 0, 0);
		pthread_mutex_unlock(d->go);
		pthread_mutex_lock(d->wait);
	}
	// Finish RTP session and exit.
	rtp_done(d->rtp_session);
	printf(" Sender stopped.\n");
	pthread_exit((void *)NULL);
}

/**************************************
 *     Main program
 */

int main(int argc, char *argv[])
{
	// Attach the signal handler
	signal(SIGINT, signal_handler);

	// Default network options
	char *remote_host = "localhost";
	char *local_host = "localhost";
	uint16_t remote_port = PORT_AUDIO;
	uint16_t local_port = PORT_AUDIO + 2;

	// Option processing
	if (argc == 1) {
		usage();
		exit(0);
	}

	static struct option getopt_options[] = {
		{"remote-host", required_argument, 0, 'h'},
		{"remote-port", optional_argument, 0, 'p'},
		{"local-port", optional_argument, 0, 'l'},
		{0, 0, 0, 0}
	};
	int ch;
	int option_index = 0;
	int tmp;

	while ((ch = getopt_long(argc, argv, "h:p:l:", getopt_options, &option_index)) != -1) {
		switch (ch) {
			case 'h':
				remote_host = optarg;
				break;
			case 'p':
				remote_port = atoi(optarg);
				break;
			case 'l':
				tmp = atoi(optarg);
				if (1024 <= tmp && tmp <= 65535 && tmp % 2 == 0) {
					local_port = tmp;
				}
				else {
					printf("I don't like %s as send_port! >:(\n", argv[2]);
				}
				break;
			case '?':
			default:
				usage();
				exit(0);
				break; 
		}
	}

	printf("Remote host: %s\n", remote_host);
	printf("Remote port: %i\n", remote_port);
	printf("Local port: %i\n", local_port);

	// We will create two threads with one different RTP session each, similar than audio_cfg_init (audio/audio.c:180) does.

	// Synchronization stuff
	pthread_mutex_t receiver_mutex;
	pthread_mutex_init(&receiver_mutex, NULL);
	pthread_mutex_t sender_mutex;
	pthread_mutex_init(&sender_mutex, NULL);

	// Receiver RTP session stuff
	struct rtp *receiver_session;
	struct pdb *receiver_participants;
	receiver_participants = pdb_init();
	//receiver_session = init_network(remote_host, remote_port, remote_port, receiver_participants, false);
	receiver_session = init_network(remote_host, remote_port, remote_port, receiver_participants, false);
	// Receiver thread config stuff
	struct thread_data *receiver_data = calloc(1, sizeof(struct thread_data));
	receiver_data->rtp_session = receiver_session;
	receiver_data->participants = receiver_participants;
	receiver_data->wait = &receiver_mutex;
	receiver_data->go = &sender_mutex;

	// Sender RTP session stuff
	struct rtp *sender_session;
	struct pdb *sender_participants;
	sender_participants = pdb_init();
	sender_session = init_network(local_host, local_port, local_port, sender_participants, false);
	// Sender thread creation stuff
	struct thread_data *sender_data = calloc(1, sizeof(struct thread_data));
	sender_data->rtp_session = sender_session;
	sender_data->participants = sender_participants;
	sender_data->wait = &sender_mutex;
	sender_data->go = &receiver_mutex;

	// Launch them
	gettimeofday(&receiver_data->start_time, NULL);
	if (pthread_create(&receiver_data->id, NULL, receiver_thread, (void *)receiver_data) != 0) {
		printf("Error creating receiver thread.\n");
		exit(-1); // Ugly exit...
	}
	gettimeofday(&sender_data->start_time, NULL);
	if (pthread_create(&sender_data->id, NULL, sender_thread, (void *)sender_data) != 0) {
		printf("Error creating sender thread.\n");
		exit(-1); // Ugly exit...
	}

	// Wait for them
	pthread_join(receiver_data->id, NULL);
	pthread_join(sender_data->id, NULL);
	printf("Done!\n");
}

