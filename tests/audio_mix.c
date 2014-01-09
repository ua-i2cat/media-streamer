/*
 * audio_mix.c - Test program to mix audio using io_mngr. 
 *               2 -> 1 mix is preformed.
 *
 * By Txor <jordi.casas@i2cat.net>
 */

#include <stdio.h>
#include <signal.h>
#include "io_mngr/participants.h"
#include "io_mngr/receiver.h"
#include "io_mngr/transmitter.h"
#include "utils.h"

#define SEND_TIME 125

#define INPUT_VIDEO_PORT 5004
#define INPUT_AUDIO_PORT 5006

#define INPUT_VIDEO_FORMAT_FPS 5.0

#define INPUT_AUDIO_FORMAT_BPS 1
#define INPUT_AUDIO_FORMAT_SAMPLE_RATE 48000
#define INPUT_AUDIO_FORMAT_CHANNELS 1
#define INPUT_AUDIO_FORMAT_CODEC AC_MULAW

#define OUTPUT_IP "127.0.0.1"
#define OUTPUT_VIDEO_PORT 6004
#define OUTPUT_AUDIO_PORT 6006

#define OUTPUT_VIDEO_FORMAT_FPS 5.0

#define OUTPUT_AUDIO_FORMAT_BPS 1
#define OUTPUT_AUDIO_FORMAT_SAMPLE_RATE 48000
#define OUTPUT_AUDIO_FORMAT_CHANNELS 1
#define OUTPUT_AUDIO_FORMAT_CODEC AC_MULAW

// Global variables
static volatile bool stop = false;
static stream_data_t *streams[8];


/* Receiver and transmitter global pointers, all internal data access
   must be done from here. */
receiver_t *receiver;
transmitter_t *transmitter;

// Function prototypes
static void audio_mix(stream_data_t *dst);
static void finish_handler(int signal);

// Mix each input audio_frame2 into the output audio_frame2 (if possible),
// from each stream_data_t decoded queue (input ones from streams global var).
static void audio_mix(stream_data_t *dst)
{
    audio_frame2 *in_frame[8];
    circular_queue_t *used_queues[8];
    int in_count = 0;
    audio_frame2 *reference_frame;
    audio_frame2 *out_frame;
    bool something_to_mix = false;

    for (int i = 0; i < 8; i++) {
        if (streams[i] != NULL) {
            if ((in_frame[in_count] = cq_get_front(streams[i]->audio->decoded_cq)) != NULL) {
                if (something_to_mix == false) {
                    something_to_mix = true;
                    reference_frame->bps = in_frame[in_count]->bps;
                    reference_frame->sample_rate = in_frame[in_count]->sample_rate;
                    reference_frame->ch_count = in_frame[in_count]->ch_count;
                    reference_frame->codec = in_frame[in_count]->codec;
                }
                in_count++;
                // save used queues to allow removing of bags
                used_queues[i] = streams[i]->audio->decoded_cq;
            }
        }
        else {
            in_frame[i] = NULL;
        }
    }
    if (something_to_mix) {
        if ((out_frame = cq_get_rear(dst->audio->decoded_cq)) != NULL) {
            out_frame->bps = reference_frame->bps;
            out_frame->sample_rate = reference_frame->sample_rate;
            out_frame->ch_count = reference_frame->ch_count;
            out_frame->codec = reference_frame->codec;
            out_frame->data_len[ch] = 0;
            // Volume balance
            //for (i = 0; i < input_count; ++i) {
            //    z->ilen[i] = sox_read_wide(files[i]->ft, z->ibuf[i], *osamp);
            //    balance_input(z->ibuf[i], z->ilen[i], files[i]);
            //    olen = max(olen, z->ilen[i]);
            //}
            // Wave sum
            for (int sample = 0; sample < olen; ++sample) { /* for each samples */
                for (int ch = 0; ch < reference_frame->ch_count; ++ch) { /* for each channel */
                    for (i = 0; i < in_count; ++i) /* for each audio_frame2 */
                        if (sample < in_frame[i]->data_len[0] && ch < in_frame[i]->ch_count) {
                            // Sum with overflow in mind
                            if (out_frame->data[ch][sample] + in_frame[in_count]->data[ch][sample] > 127) out_frame->data[ch][sample] = 127;
                            else if (out_frame->data[ch][sample] + in_frame[in_count]->data[ch][sample] < -127) out_frame->data[ch][sample] = -127;
                            else out_frame->data[ch][sample] += in_frame[in_count]->data[ch][sample];
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

int main()
{
    fprintf(stderr, "Starting audio_mix\n");

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
    // First audio stream with a participant
    stream = init_stream(AUDIO, INPUT, rand(), I_AWAIT, "Input audio stream 1");
    add_participant_stream(stream,
            init_participant(1, INPUT, NULL, 0));
    ap_config(stream->audio, INPUT_AUDIO_FORMAT_BPS,
            INPUT_AUDIO_FORMAT_SAMPLE_RATE,
            INPUT_AUDIO_FORMAT_CHANNELS,
            INPUT_AUDIO_FORMAT_CODEC);
    add_stream(receiver->audio_stream_list, stream);
    ap_worker_start(stream->audio);
    // Second audio stream with a participant
    stream = init_stream(AUDIO, INPUT, rand(), I_AWAIT, "Input audio stream 2");
    add_participant_stream(stream,
            init_participant(1, INPUT, NULL, 0));
    ap_config(stream->audio, INPUT_AUDIO_FORMAT_BPS,
            INPUT_AUDIO_FORMAT_SAMPLE_RATE,
            INPUT_AUDIO_FORMAT_CHANNELS,
            INPUT_AUDIO_FORMAT_CODEC);
    add_stream(receiver->audio_stream_list, stream);
    ap_worker_start(stream->audio);

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
    stream_data_t *iterator_audio_in_stream,
                  *audio_out_stream;
    int index_audio_in_stream;

    // Main loop
    fprintf(stderr, "  ·Mixing audio! ");
    while(!stop) {

        // Audio selection
        iterator_audio_in_stream = receiver->audio_stream_list->first;
        audio_out_stream = transmitter->audio_stream_list->first;
        index_audio_in_stream = 0;
        // SEGV if we add more than 8 input streams...
        while (iterator_audio_in_stream != NULL) {
            streams[index_audio_in_stream] = iterator_audio_in_stream;
            iterator_audio_in_stream = iterator_audio_in_stream->next;
            index_audio_in_stream++;
        }

        // mix
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

