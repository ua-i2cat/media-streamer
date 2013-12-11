/*
 * audio_rec_trans.c - Test program that starts one io_mngr receiver, one
 * io_mngr transmitter and, initially, adds one stream with one participant to
 * the receiver and to the transmitter.
 * It continously forwards the audio on the receiver streams (one or two) to
 * the transmitter streams (one or two) for LIVE_TIME seconds or until user
 * cancels.
 * On receiving SIGUSR1 signal (i.e. issued by kill), again adds one stream 
 * with one participant to the receiver and to the transmitter.
 * The receiver streams are configured (throught constants) with 1 bps,
 * 16000Hz, 1 channel and the uLaw codec.
 * The transmitter streams are configured (throught constants) with 1 bps,
 * 32000Hz, 1 channel and the uLaw codec.
 * Every SWITCH_TIME seconds the audio forwarding is swapped creating a
 * confusing effect!
 *
 * By Txor <jordi.casas@i2cat.net>
 */

#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include "io_mngr/participants.h"
#include "io_mngr/receiver.h"
#include "io_mngr/transmitter.h"
#include "utils.h"

// Configuration constants
#define SWITCH_TIME 5
#define LIVE_TIME 620
#define SEND_TIME 125

#define RECEIVER_VIDEO_PORT 5004
#define RECEIVER_AUDIO_PORT 5006

#define RECEIVER_AUDIO_FORMAT_BPS 1
#define RECEIVER_AUDIO_FORMAT_SAMPLE_RATE 16000
#define RECEIVER_AUDIO_FORMAT_CHANNELS 1
#define RECEIVER_AUDIO_FORMAT_CODEC AC_MULAW

#define TRANSMITTER_IP_1 "127.0.0.1"
#define TRANSMITTER_PORT_1 6006
#define TRANSMITTER_IP_2 "127.0.0.1"
#define TRANSMITTER_PORT_2 7006

#define TRANSMITTER_AUDIO_FORMAT_BPS 1
#define TRANSMITTER_AUDIO_FORMAT_SAMPLE_RATE 32000
#define TRANSMITTER_AUDIO_FORMAT_CHANNELS 1
#define TRANSMITTER_AUDIO_FORMAT_CODEC AC_MULAW

// Global variables
static volatile bool stop = false;
static char msg[1024];

/* Receiver and transmitter global pointers, all internal data access
   must be done from here. */
receiver_t *receiver;
transmitter_t *transmitter;

// Function prototypes
static void add_receiver_entity();
static void add_transmitter_entity(char *ip, int port);
static void finish_handler(int signal);
static void action_handler(int signal);

/* Add and configure a stream with a participant and an audio configuration
   into the receiver. */
static void add_receiver_entity() {
    static int count = 1;
    char name[256];
    sprintf(name, "IN_%i", count);
    participant_data_t *p = init_participant(1, INPUT, NULL, 0);
    stream_data_t *s = init_stream(AUDIO, INPUT, rand(), I_AWAIT, 0, name);
    add_participant_stream(s, p);
    ap_config(s->audio, RECEIVER_AUDIO_FORMAT_BPS,
            RECEIVER_AUDIO_FORMAT_SAMPLE_RATE,
            RECEIVER_AUDIO_FORMAT_CHANNELS,
            RECEIVER_AUDIO_FORMAT_CODEC);
    add_stream(receiver->audio_stream_list, s);
    ap_worker_start(s->audio);
    fprintf(stderr, "  ·Adding %s: %i bps, %iHz, %i channel, %s\n",
            name,
            RECEIVER_AUDIO_FORMAT_BPS,
            RECEIVER_AUDIO_FORMAT_SAMPLE_RATE,
            RECEIVER_AUDIO_FORMAT_CHANNELS,
            get_name_to_audio_codec(RECEIVER_AUDIO_FORMAT_CODEC));
    count++;
}

/* Add and configure a stream with a participant and an audio configuration
   into the transmitter. */
static void add_transmitter_entity(char *ip, int port) {
    static int count = 1;
    char name[256];
    sprintf(name, "OUT_%i", count);
    participant_data_t *p = init_participant(0, OUTPUT, ip, port);
    stream_data_t *s = init_stream(AUDIO, OUTPUT, 0, ACTIVE, 0, name);
    add_participant_stream(s, p);
    ap_config(s->audio, TRANSMITTER_AUDIO_FORMAT_BPS,
            TRANSMITTER_AUDIO_FORMAT_SAMPLE_RATE,
            TRANSMITTER_AUDIO_FORMAT_CHANNELS,
            TRANSMITTER_AUDIO_FORMAT_CODEC);
    add_stream(transmitter->audio_stream_list, s);
    ap_worker_start(s->audio);
    fprintf(stderr,
            "  ·Adding %s: %i bps, %iHz, %i channel, %s, to addr %s:%i\n",
            name,
            TRANSMITTER_AUDIO_FORMAT_BPS,
            TRANSMITTER_AUDIO_FORMAT_SAMPLE_RATE,
            TRANSMITTER_AUDIO_FORMAT_CHANNELS,
            get_name_to_audio_codec(TRANSMITTER_AUDIO_FORMAT_CODEC),
            ip,
            port);
    count++;
}

// Copy one audio_frame2 between the decoded queues of two streams.
static void audio_frame_forward(stream_data_t *src, stream_data_t *dst)
{
    audio_frame2 *in_frame, *out_frame;
    in_frame = cq_get_front(src->audio->decoded_cq);
    if (in_frame != NULL) {
        out_frame = cq_get_rear(dst->audio->decoded_cq);
        if (out_frame != NULL) {
            out_frame->bps = in_frame->bps;
            out_frame->sample_rate = in_frame->sample_rate;
            out_frame->ch_count = in_frame->ch_count;
            out_frame->codec = in_frame->codec;
            for (int i = 0; i < in_frame->ch_count; i++) {
                memcpy(out_frame->data[i],
                        in_frame->data[i],
                        in_frame->data_len[i]);
                out_frame->data_len[i] = in_frame->data_len[i];
            }
            cq_add_bag(dst->audio->decoded_cq);
            cq_remove_bag(src->audio->decoded_cq);
        }
    }
}

// Triggers the stops condition of the program.
static void finish_handler(int sig)
{
    switch (sig) {
        case SIGINT:
        case SIGALRM:
            stop = true;
            break;
        default:
            break;
    }
}

// Adds a couple of receiver and transmitter streams on user action.
static void action_handler(int sig)
{
    switch (sig) {
        case SIGUSR1:
            add_receiver_entity();
            add_transmitter_entity(TRANSMITTER_IP_2, TRANSMITTER_PORT_2);
            signal(SIGUSR1, SIG_DFL); // Disable the signal
            break;
        default:
            break;
    }
}

int main()
{
    fprintf(stderr,
            "Starting audio_rec_trans test (max %i seconds)\n",
            LIVE_TIME);
    fprintf(stderr,
            "Issue kill -10 %i to dinamically add a couple of streams.\n",
            getpid()); 

    // Attach the handlers to the signals and prepare block stuff.
    signal(SIGINT, finish_handler);
    signal(SIGALRM, finish_handler);
    signal(SIGUSR1, action_handler);
    //    sigset_t signal_set;
    //    sigemptyset(&signal_set);
    //    sigaddset(&signal_set, SIGUSR1);

    // Start live time alarm
    alarm(LIVE_TIME);

    // Receiver startup
    fprintf(stderr,
            " ·Configuring receiver (listen at %i)\n",
            RECEIVER_AUDIO_PORT);
    receiver = init_receiver(init_stream_list(),
            init_stream_list(),
            RECEIVER_VIDEO_PORT,
            RECEIVER_AUDIO_PORT);
    start_receiver(receiver);
    add_receiver_entity();

    // Transmitter startup
    fprintf(stderr, " ·Configuring transmitter\n");
    transmitter = init_transmitter(init_stream_list(),
            init_stream_list(),
            25.0);
    start_transmitter(transmitter);
    add_transmitter_entity(TRANSMITTER_IP_1, TRANSMITTER_PORT_1);

    // Temporal variables and initializations
    stream_data_t *in_stream_couple1,
                  *out_stream_couple1,
                  *in_stream_couple2,
                  *out_stream_couple2;
    bool cross = false;
    bool verbose = true;
    time_t lap_time = time(NULL) + SWITCH_TIME;

    // Main loop
    fprintf(stderr,
            " ·Sending each input stream to the output every %i seconds...\n",
            SWITCH_TIME);
    while(!stop) {

        // Lap time control
        if (time(NULL) > lap_time) {
            lap_time = time(NULL) + SWITCH_TIME;
//            cross = !cross;
            verbose = true;
            fprintf(stderr, "Done!\n");
            //            pthread_sigmask(SIG_UNBLOCK, &signal_set, NULL); // Unblock the SIGUSR1
//            add_receiver_entity();
//            add_transmitter_entity(TRANSMITTER_IP_2, TRANSMITTER_PORT_2);
        }

        // Cross sending control
        if (!cross) {
            in_stream_couple1 = receiver->audio_stream_list->first;
            out_stream_couple1 = transmitter->audio_stream_list->first;
            in_stream_couple2 = in_stream_couple1->next;
            out_stream_couple2 = out_stream_couple1->next;
        }
        else {
            in_stream_couple1 = receiver->audio_stream_list->first;
            out_stream_couple1 = transmitter->audio_stream_list->last;
            in_stream_couple2 = in_stream_couple1->next;
            out_stream_couple2 = out_stream_couple1->prev;
        }

        // Forward audio from receiver to transmitter
        if (in_stream_couple2 == NULL || out_stream_couple2 == NULL) {
            // One couple case (first to first).
            if (verbose) {
                sprintf(msg, "  ·Sending %s -> %s ",
                        in_stream_couple1->stream_name,
                        out_stream_couple1->stream_name);
            }
            audio_frame_forward(in_stream_couple1, out_stream_couple1);
        }
        else {
            // Two couples case (first to first, next to next).
            if (verbose) {
                sprintf(msg, "  ·Sending %s -> %s and %s -> %s ",
                        in_stream_couple1->stream_name,
                        out_stream_couple1->stream_name,
                        in_stream_couple2->stream_name,
                        out_stream_couple2->stream_name);
            }
            audio_frame_forward(in_stream_couple1, out_stream_couple1);
            audio_frame_forward(in_stream_couple2, out_stream_couple2);
        }

        // Print information message
        if (verbose) {
            fprintf(stderr, "%s", msg);
            verbose = false;
            //            pthread_sigmask(SIG_BLOCK, &signal_set, NULL); // Block the SIGUSR1
        }

        //Try to not send all the audio suddently.
        usleep(SEND_TIME);
    }
    fprintf(stderr, "Done!\n");

    // Finish and destroy receiver objects
    stop_receiver(receiver);
    destroy_stream_list(receiver->stream_list);
    destroy_stream_list(receiver->audio_stream_list);
    destroy_receiver(receiver);
    fprintf(stderr, " ·Receiver stopped\n");

    // Finish and destroy transmitter objects
    stop_transmitter(transmitter);
    destroy_stream_list(transmitter->video_stream_list);
    destroy_stream_list(transmitter->audio_stream_list);
    destroy_transmitter(transmitter);
    fprintf(stderr, " ·Transmitter stopped\n");

    fprintf(stderr, "Finished\n");
}

