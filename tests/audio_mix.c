/*
 * audio_mix.c - Test program to mix audio using io_mngr. 
 *               N -> 1 mix is preformed (max N = 8).
 *
 * By Txor <jordi.casas@i2cat.net>
 */

#include <stdio.h>
#include <signal.h>
#include <getopt.h>
#include "io_mngr/participants.h"
#include "io_mngr/receiver.h"
#include "io_mngr/transmitter.h"
#include "utils.h"

#define SEND_TIME 300

#define N_PARTICIPANTS 2

#define INPUT_VIDEO_PORT 5004
#define INPUT_AUDIO_PORT 5006

#define INPUT_VIDEO_FORMAT_FPS 5.0

#define INPUT_AUDIO_FORMAT_BPS 1
#define INPUT_AUDIO_FORMAT_SAMPLE_RATE 16000
#define INPUT_AUDIO_FORMAT_CHANNELS 1
#define INPUT_AUDIO_FORMAT_CODEC AC_MULAW

#define OUTPUT_IP "127.0.0.1"
#define OUTPUT_VIDEO_PORT 6004
#define OUTPUT_AUDIO_PORT 6006

#define OUTPUT_VIDEO_FORMAT_FPS 5.0

#define OUTPUT_AUDIO_FORMAT_BPS 1
#define OUTPUT_AUDIO_FORMAT_SAMPLE_RATE 32000
#define OUTPUT_AUDIO_FORMAT_CHANNELS 1
#define OUTPUT_AUDIO_FORMAT_CODEC AC_MULAW

// Global variables
static volatile bool stop = false;
static stream_data_t *streams[8];
static int nstreams = 0;
static int nparticipants = N_PARTICIPANTS;

/* Receiver and transmitter global pointers, all internal data access
   must be done from here. */
receiver_t *receiver;
transmitter_t *transmitter;

/**
  Client API:
  Returns the smallest (negative) value storable in a twos-complement signed
  integer with the specified number of bits, cast to an unsigned integer;
  for example, SOX_INT_MIN(8) = 0x80, SOX_INT_MIN(16) = 0x8000, etc.
  @param bits Size of value for which to calculate minimum.
  @returns the smallest (negative) value storable in a twos-complement signed
  integer with the specified number of bits, cast to an unsigned integer.
 */
#define SOX_INT_MIN(bits) (1 <<((bits)-1))

/**         
  Client API:
  Returns the largest (positive) value storable in a twos-complement signed
  integer with the specified number of bits, cast to an unsigned integer;
  for example, SOX_INT_MAX(8) = 0x7F, SOX_INT_MAX(16) = 0x7FFF, etc.
  @param bits Size of value for which to calculate maximum.
  @returns the largest (positive) value storable in a twos-complement signed
  integer with the specified number of bits, cast to an unsigned integer.
 */
#define SOX_INT_MAX(bits) (((unsigned)-1)>>(33-(bits)))

/**
  Client API:
  Min value for sox_sample_t = 0x80000000.
 */
#define SOX_SAMPLE_MIN (char)SOX_INT_MIN(8)

/**
  Client API:
  Max value for sox_sample_t = 0x7FFFFFFF.
 */
#define SOX_SAMPLE_MAX (char)SOX_INT_MAX(8)

/**
  SoX code, Client API:
  Clips a value of a type that is larger then sox_sample_t (for example, int64)
  to sox_sample_t's limits and increment a counter if clipping occurs.
  @param d Value (rvalue) to be clipped.
  @param clips Value (lvalue) that is incremented if clipping is needed.
  @returns Clipped value.
 */
#define SOX_ROUND_CLIP_COUNT(d) \
    ((d) < 0? (d) <= SOX_SAMPLE_MIN - 0.5? SOX_SAMPLE_MIN: (d) - 0.5 \
     : (d) >= SOX_SAMPLE_MAX + 0.5? SOX_SAMPLE_MAX: (d) + 0.5)

// Function prototypes
static void audio_mix(stream_data_t *dst);
static void change_volume(audio_frame2 *frame, double volume);
static void finish_handler(int signal);

// Mix each input audio_frame2 into the output audio_frame2 (if possible),
// from each stream_data_t decoded queue (input ones from streams global var).
static void audio_mix(stream_data_t *dst)
{
    audio_frame2 *in_frame[8];
    circular_queue_t *used_queues[8];

    audio_frame2 *out_frame;

    bool something_to_mix = false;
    int in_count = 0;

    audio_frame2 reference_frame;
    // Preliminar initialization
    reference_frame.bps = 1;
    reference_frame.sample_rate = 8000;
    reference_frame.ch_count = 8;
    reference_frame.codec = AC_MULAW;
    for (int j = 0; j < reference_frame.ch_count; j++)
        reference_frame.data_len[j] = 0;

    for (int i = 0; i < nstreams; i++) {
        if ((in_frame[in_count] = cq_get_front(streams[i]->audio->decoded_cq)) != NULL) {
            if (something_to_mix == false) {
                something_to_mix = true;
                reference_frame.bps = in_frame[in_count]->bps;
                reference_frame.sample_rate = in_frame[in_count]->sample_rate;
                reference_frame.ch_count = in_frame[in_count]->ch_count;
                reference_frame.codec = in_frame[in_count]->codec;
                for (int j = 0; j < reference_frame.ch_count; j++)
                    reference_frame.data_len[j] = in_frame[in_count]->data_len[j];
            }
            in_count++;
            // save used queues to allow removing of bags
            used_queues[i] = streams[i]->audio->decoded_cq;
        }
    }

    if (in_count == nparticipants) {
        if ((out_frame = cq_get_rear(dst->audio->decoded_cq)) != NULL) {
            out_frame->bps = reference_frame.bps;
            out_frame->sample_rate = reference_frame.sample_rate;
            out_frame->ch_count = reference_frame.ch_count;
            out_frame->codec = reference_frame.codec;
            // Volume balance
            //for (i = 0; i < input_count; ++i) {
            //    z->ilen[i] = sox_read_wide(files[i]->ft, z->ibuf[i], *osamp);
            //    balance_input(z->ibuf[i], z->ilen[i], files[i]);
            //    olen = max(olen, z->ilen[i]);
            //}
            // Wave sum
            for (int i = 0; i < in_count; ++i) {
                change_volume(in_frame[i], 1.0 / (double)nparticipants);
            }
            for (int ch = 0; ch < reference_frame.ch_count; ++ch) { /* for each channel */
                out_frame->data_len[ch] = 0;
                for (int sample = 0; sample < reference_frame.data_len[ch]; ++sample) { /* for each samples */
                    out_frame->data[ch][sample] = 0;
                    for (int i = 0; i < in_count; ++i) {/* for each audio_frame2 */
                        // Sum with overflow in mind
                        //if (out_frame->data[ch][sample] + in_frame[i]->data[ch][sample] > 127) out_frame->data[ch][sample] = 127;
                        //else if (out_frame->data[ch][sample] + in_frame[i]->data[ch][sample] < -127) out_frame->data[ch][sample] = -127;
                        //else out_frame->data[ch][sample] += in_frame[i]->data[ch][sample];
                        double s = out_frame->data[ch][sample] + in_frame[i]->data[ch][sample];
                        out_frame->data[ch][sample] = SOX_ROUND_CLIP_COUNT(s);
                        //out_frame->data[ch][sample] += in_frame[i]->data[ch][sample];
                    }
                    out_frame->data_len[ch]++;
                }
            }
            // Add and remove bags
            cq_add_bag(dst->audio->decoded_cq);
            for (int i = 0; i < in_count; ++i) cq_remove_bag(used_queues[i]);
        }
    }
}

// Change the volume of an audio_frame2.
static void change_volume(audio_frame2 *frame, double volume)
{
    if (volume != 1) {
        for (int ch = 0; ch < frame->ch_count; ch++) {
            for (int sample = 0; sample < frame->data_len[ch]; sample++) {
                double s = volume * frame->data[ch][sample];
                frame->data[ch][sample] = SOX_ROUND_CLIP_COUNT(s);
            }
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

int main(int argc, char *argv[])
{
    fprintf(stderr, "Starting audio_mix\n");

    // Option processing (only number of streams)
    if (argc > 1) {
        int arg = atoi(argv[1]);
        if (arg >= 1 && arg < 9) {
            nparticipants = arg;
        }
    }
    fprintf(stderr, " ·%i streams configured\n", nparticipants);

    // Attach the handler to CTRL+C.
    signal(SIGINT, finish_handler);

    stream_data_t *stream;

    // Receiver startup
    fprintf(stderr, " ·Configuring receiver\n");
    receiver = init_receiver(init_stream_list(),
            init_stream_list(),
            INPUT_VIDEO_PORT,
            INPUT_AUDIO_PORT);
    start_receiver(receiver);
    char msg[32];
    for (int i = 0; i < nparticipants; i++) {
        // audio stream with a participant
        sprintf(msg, "Input audio stream %d", i + 1);
        stream = init_stream(AUDIO, INPUT, rand(), I_AWAIT, msg);
        add_participant_stream(stream,
                init_participant(1, INPUT, NULL, 0));
        ap_config(stream->audio, INPUT_AUDIO_FORMAT_BPS,
                INPUT_AUDIO_FORMAT_SAMPLE_RATE,
                INPUT_AUDIO_FORMAT_CHANNELS,
                INPUT_AUDIO_FORMAT_CODEC);
        add_stream(receiver->audio_stream_list, stream);
        ap_worker_start(stream->audio);
        streams[i] = stream;
        nstreams++;
    }

    // Transmitter startup
    fprintf(stderr, " ·Configuring transmitter\n");
    transmitter = init_transmitter(init_stream_list(),
            init_stream_list(),
            OUTPUT_VIDEO_FORMAT_FPS);
    start_transmitter(transmitter);
    // Audio stream with a participant
    stream = init_stream(AUDIO, OUTPUT, 0, ACTIVE, "Output audio stream");
    add_participant_stream(stream, 
            init_participant(0, OUTPUT, OUTPUT_IP, OUTPUT_AUDIO_PORT));
    ap_config(stream->audio, OUTPUT_AUDIO_FORMAT_BPS,
            OUTPUT_AUDIO_FORMAT_SAMPLE_RATE,
            OUTPUT_AUDIO_FORMAT_CHANNELS,
            OUTPUT_AUDIO_FORMAT_CODEC);
    add_stream(transmitter->audio_stream_list, stream);
    ap_worker_start(stream->audio);

    // Temporal variables and initializations
    stream_data_t *audio_out_stream = transmitter->audio_stream_list->first;

    // Main loop
    fprintf(stderr, "  ·Mixing audio! ");
    while(!stop) {
        // mix using global variable streams (stream container).
        audio_mix(audio_out_stream);

        //Try to not send all the data suddently.
        usleep(SEND_TIME);
    }
    fprintf(stderr, "Done!\n");

    // Finish and destroy receiver objects
    stop_receiver(receiver);
    destroy_stream_list(receiver->audio_stream_list);
    destroy_receiver(receiver);
    fprintf(stderr, " ·Receiver stopped\n");

    // Finish and destroy transmitter objects
    stop_transmitter(transmitter);
    destroy_stream_list(transmitter->audio_stream_list);
    destroy_transmitter(transmitter);
    fprintf(stderr, " ·Transmitter stopped\n");

    fprintf(stderr, "Finished\n");
}

