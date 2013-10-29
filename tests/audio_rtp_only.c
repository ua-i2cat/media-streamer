/*
 *
 * Test program to retransmit audio using only RTP to RTP.
 * Based on UltraGrid code.
 *
 * It creates two different RTP sessions managed by two different threads.
 * Each packet processed on the receiver thread RTP session is pointed by a global shared object.
 * Using this the sender thread sends it throught its RTP session.
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
#include "rtp/audio_decoders.h"
#include "rtp/audio_frame2.h"
#include "module.h"
#include "tv.h"
#include "pdb.h"

#define DEFAULT_AUDIO_FEC       "mult:3"


/**************************************
 *     Variables 
 */

static volatile bool stop = false;
static audio_frame2 *shared_frame = NULL;
static volatile bool consumed = true;

// Like state_audio (audio/audio.c:105)
// Used to communicate data to threads.
struct thread_data {
    // RTP related stuff
    struct rtp *rtp_session;
    struct pdb *participants;
    struct timeval start_time;

    // Sender stuff
    struct tx *tx_session;

    // Thread related stuff
    pthread_t id;
    pthread_mutex_t *wait;            // Lock for wait.
    pthread_mutex_t *go;              // Lock to wake up our bro.
};


/**************************************
 *     Functions 
 */

// Prototypes
extern int audio_pbuf_decode(struct pbuf *playout_buf, struct timeval curr_time, decode_frame_t decode_func, void *data);

static void usage(void)
{
    printf("Usage: audio_rtp_only [-r <receive_port>] [-h <sendto_host>] [-s <sendto_port>]\n");
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
// Basically explained:
//   It listens to UDP port and when it receivers a packet (cp != NULL)
//   Iterate by the participants,
//     trying to decode an audio frame from stored RTP packets. 
//   Once it's done, update the shared audio_frame2 poniter where the frame is.
static void *receiver_thread(void *arg)
{
    struct thread_data *d = arg;
    struct timeval timeout, curr_time;
    uint32_t ts;
    struct pdb_e *cp;
    audio_frame2 *frame;

    frame = rtp_audio_frame2_init();

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
        rtp_recv_r(d->rtp_session, &timeout, ts);

        // Iterate throught participants
        pdb_iter_t it;
        cp = pdb_iter_init(d->participants, &it);

        while (cp != NULL) {
            // Get the data on pbuf and decode it on the frame using the callback.
            if (audio_pbuf_decode(cp->playout_buffer, curr_time, decode_audio_frame, frame)) {
                // If decoded, mark it ready to consume.
                consumed = false;
            }
            pbuf_remove(cp->playout_buffer, curr_time);
            cp = pdb_iter_next(&it);
        }
        // Point shared_frame to the received frame.
        shared_frame = frame;
        pdb_iter_done(&it);
        // Wait for sender to be ready.
        pthread_mutex_unlock(d->go);
        pthread_mutex_lock(d->wait);
    }
    // Finish RTP session and exit.
    rtp_done(d->rtp_session);
    printf(" Receiver stopped.\n");

    // Don't let the bro thread gets locked.
    pthread_mutex_unlock(d->go);
    pthread_exit((void *)NULL);
}

// Function for sender thread.
// Based on audio_sender_thread (audio/audio.c:610)
// Basically explained:
//   If there's a new audio_frame2, send it throught the RTP session.
static void *sender_thread(void *arg)
{
    struct thread_data *d = arg;

    printf(" Sender started.\n");
    while (!stop) {
        // Wait for receiver to be ready.
        pthread_mutex_unlock(d->go);
        pthread_mutex_lock(d->wait);
        // Send the data away only if not consumed before.
        if (!consumed) {
            consumed = true;
            audio_tx_send(d->tx_session, d->rtp_session, shared_frame);
        }
    }
    // Finish RTP session and exit.
    rtp_done(d->rtp_session);
    printf(" Sender stopped.\n");
    // Don't let the bro thread gets locked.
    pthread_mutex_unlock(d->go);
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
    char *sendto_host = "localhost";
    uint16_t sendto_port = PORT_AUDIO;
    uint16_t receive_port = PORT_AUDIO + 1000;

    // Option processing
    static struct option getopt_options[] = {
        {"sendto_host", required_argument, 0, 'h'},
        {"receive_port", required_argument, 0, 'r'},
        {"sendto_port", required_argument, 0, 's'},
        {0, 0, 0, 0}
    };
    int ch;
    int option_index = 0;

    while ((ch = getopt_long(argc, argv, "h:s:r:", getopt_options, &option_index)) != -1) {
        switch (ch) {
            case 'h':
                sendto_host = optarg;
                break;
            case 's':
                sendto_port = atoi(optarg);
                break;
            case 'r':
                receive_port = atoi(optarg);
                break;
            case '?':
                usage();
                exit(0);
                break; 
            default:
                break; 
        }
    }

    printf("Host to send: %s\n", sendto_host);
    printf("Port to send: %i\n", sendto_port);
    printf("Port to receive: %i\n", receive_port);

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
    receiver_session = init_network(NULL, receive_port, 0, receiver_participants, false);
    // Receiver thread config stuff
    struct thread_data *receiver_data = calloc(1, sizeof(struct thread_data));
    receiver_data->rtp_session = receiver_session;
    receiver_data->participants = receiver_participants;
    receiver_data->wait = &receiver_mutex;
    receiver_data->go = &sender_mutex;
    // tx_session not used.
    receiver_data->tx_session = NULL;

    // Sender RTP session stuff
    struct rtp *sender_session;
    struct pdb *sender_participants;
    sender_participants = pdb_init();
    sender_session = init_network(sendto_host, 0, sendto_port, sender_participants, false);
    // Sender thread creation stuff
    struct thread_data *sender_data = calloc(1, sizeof(struct thread_data));
    sender_data->rtp_session = sender_session;
    sender_data->participants = sender_participants;
    sender_data->wait = &sender_mutex;
    sender_data->go = &receiver_mutex;
    // Configure the dummiest tx_session.
    struct module mod;
    module_init_default(&mod);
    char *audio_fec = strdup(DEFAULT_AUDIO_FEC);
    sender_data->tx_session = tx_init(&mod, 1500, TX_MEDIA_AUDIO, audio_fec, NULL);

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

