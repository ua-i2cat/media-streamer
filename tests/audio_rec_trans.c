/*
 * audio_rec_trans.c - Test program that starts one io_mngr receiver, 
 * starts one io_mngr transmitter and configures the following:
 *  ·Receiver:
 *      2 streams to get 2 different audios (different source and format).
 *  ·Transmitter:
 *      1 stream with 2 participants to send the audio twice.
 * It sends alternatively SWITCH_TIME seconds of audio from one receiver
 * stream and SWITCH_TIME seconds of audio from the other receiver stream.
 * The program will finish on CTRL+C or after LIVE_TIME seconds.
 *
 * This example is a mix of audio_receiver and audio_transmitter
 * functionalities.
 *
 * By Txor <jordi.casas@i2cat.net>
 */

#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include "io_mngr/participants.h"
#include "io_mngr/receiver.h"
#include "io_mngr/transmitter.h"

#define SWITCH_TIME 5
#define LIVE_TIME 120

#define SLOW_SEND 125

#define IP_FIRST "127.0.0.1"
#define IP_SECOND "127.0.0.1"
#define PORT_FIRST 5006
#define PORT_SECOND 6006

static volatile bool stop = false;

static void signal_handler(int signal)
{
    switch (signal) {
        case SIGINT:
        case SIGALRM:
            stop = true;
            break;
        default:
            break;
    }
}

int main()
{
    fprintf(stderr, "Starting audio_rec_trans test (max %i seconds)...\n", LIVE_TIME);

    // Attach the handler to the signals
    signal(SIGINT, signal_handler);
    signal(SIGALRM, signal_handler);

    // Start livetime alarm
    alarm(LIVE_TIME);

    /*
     * Receiver startup
     */
    fprintf(stderr, " ·Configuraing receiver\n");
    stream_list_t *receive_video_stream_list = init_stream_list(); // Not used
    stream_list_t *receive_audio_stream_list = init_stream_list();
    receiver_t *receiver = init_receiver(receive_video_stream_list, receive_audio_stream_list, 5004, 5006);
    start_receiver(receiver);
    // First stream and participant
    participant_data_t *receive_p1 = init_participant(1, INPUT, NULL, 0);
    stream_data_t *receive_stream1 = init_stream(AUDIO, INPUT, rand(), I_AWAIT, 0, "input stream 1");
    add_participant_stream(receive_stream1, receive_p1);
    add_stream(receiver->audio_stream_list, receive_stream1);
    fprintf(stderr, "  ·Input stream 1: 1 bps, 32000Hz, 1 channel, mulaw\n");
    ap_config(receive_stream1->audio, 1, 32000, 1, AC_MULAW);
    ap_worker_start(receive_stream1->audio);
    // Second stream and participant
    participant_data_t *receive_p2 = init_participant(2, INPUT, NULL, 0);
    stream_data_t *receive_stream2 = init_stream(AUDIO, INPUT, rand(), I_AWAIT, 0, "input stream 2");
    add_participant_stream(receive_stream2, receive_p2);
    add_stream(receiver->audio_stream_list, receive_stream2);
    fprintf(stderr, "  ·Input stream 2: 1 bps, 8000Hz, 1 channel, mulaw\n");
    ap_config(receive_stream2->audio, 1, 8000, 1, AC_MULAW);
    ap_worker_start(receive_stream2->audio);

    /*
     * Transmitter startup
     */
    fprintf(stderr, " ·Configuraing transmitter\n");
    stream_list_t *transmit_video_stream_list = init_stream_list(); // Not used
    stream_list_t *transmit_audio_stream_list = init_stream_list();
    stream_data_t *transmit_stream = init_stream(AUDIO, OUTPUT, 0, ACTIVE, 25.0, "Output stream");
    add_stream(transmit_audio_stream_list, transmit_stream);
    transmitter_t *transmitter = init_transmitter(transmit_video_stream_list, transmit_audio_stream_list, 25.0);
    start_transmitter(transmitter);
    // Configure two participants
    participant_data_t *transmit_p1 = init_participant(0, OUTPUT, IP_FIRST, PORT_FIRST);
    add_participant_stream(transmit_stream, transmit_p1);    
    fprintf(stderr, "  ·Output participant configuration: %s:%i\n", IP_FIRST, PORT_FIRST);
    participant_data_t *transmit_p2 = init_participant(0, OUTPUT, IP_SECOND, PORT_SECOND);
    add_participant_stream(transmit_stream, transmit_p2);
    fprintf(stderr, "  ·Output participant configuration: %s:%i\n", IP_SECOND, PORT_SECOND);
    // configure output stream audio format
    //ap_config(stream->audio, 1, 8000, 1, AC_MULAW);
    ap_config(transmit_stream->audio, 1, 32000, 1, AC_MULAW);
    ap_worker_start(transmit_stream->audio);
    //fprintf(stderr, " ·Output stream configuration: 1 bps, 8000Hz, 1 channel, mulaw\n");
    fprintf(stderr, "  ·Output stream configuration: 1 bps, 32000Hz, 1 channel, mulaw\n");

    time_t lap_time = time(NULL) + SWITCH_TIME;
    bool first_stream = true;
    bool verbose = true;
    fprintf(stderr, " ·Sending each input stream to the output every %i seconds...\n", SWITCH_TIME);
    while(!stop) {
        if (time(NULL) > lap_time) {
            lap_time = time(NULL) + SWITCH_TIME;
            first_stream = !first_stream;
            verbose = true;
            fprintf(stderr, "Done!\n");
        }
        if (first_stream) {
            if (verbose) {
                fprintf(stderr, "  ·Sending %s... ", receive_stream1->stream_name);
                verbose = false;
            }
            // TODOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO
        }
        else {
            if (verbose) {
                fprintf(stderr, "  ·Sending %s... ", receive_stream2->stream_name);
                verbose = false;
            }
            // TODOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO
        }
        usleep(SLOW_SEND); //Try to not send all the audio suddently.
    }
    fprintf(stderr, "Done!\n");

    // Finish and destroy receiver objects
    stop_receiver(receiver);
    destroy_receiver(receiver);
    fprintf(stderr, " ·Receiver stopped\n");
    destroy_stream_list(receive_video_stream_list);
    destroy_stream_list(receive_audio_stream_list);

    // Finish and destroy transmitter objects
    stop_transmitter(transmitter);
    destroy_transmitter(transmitter);
    fprintf(stderr, " ·Transmitter stopped\n");
    destroy_stream_list(transmit_video_stream_list);
    destroy_stream_list(transmit_audio_stream_list);

    fprintf(stderr, "Finished\n");
}
