/*
 * mmforwarder.c - Test program to forward audio and video together.
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

#define INPUT_VIDEO_FORMAT_FPS 25.0

#define INPUT_AUDIO_FORMAT_BPS 1
#define INPUT_AUDIO_FORMAT_SAMPLE_RATE 16000
#define INPUT_AUDIO_FORMAT_CHANNELS 1
#define INPUT_AUDIO_FORMAT_CODEC AC_MULAW

#define OUTPUT_IP "127.0.0.1"
#define OUTPUT_VIDEO_PORT 6004
#define OUTPUT_AUDIO_PORT 6006

#define OUTPUT_VIDEO_FORMAT_FPS 25.0

#define OUTPUT_AUDIO_FORMAT_BPS 1
#define OUTPUT_AUDIO_FORMAT_SAMPLE_RATE 32000
#define OUTPUT_AUDIO_FORMAT_CHANNELS 1
#define OUTPUT_AUDIO_FORMAT_CODEC AC_MULAW

// Global variables
static volatile bool stop = false;

/* Receiver and transmitter global pointers, all internal data access
   must be done from here. */
receiver_t *receiver;
transmitter_t *transmitter;

// Function prototypes
static void audio_frame_forward(stream_data_t *src, stream_data_t *dst);
static void finish_handler(int signal);

// Copy one video_data_frame_t between the decoded queues of two streams.
static void video_frame_forward(stream_data_t *src, stream_data_t *dst)
{
    video_data_frame_t *in_frame, *out_frame;
    in_frame = curr_out_frame(src->video->decoded_frames);
    if (in_frame != NULL) {
        out_frame = curr_in_frame(dst->video->decoded_frames);
        if (out_frame != NULL) {
            out_frame->curr_seqno = in_frame->curr_seqno;
            out_frame->width = in_frame->width;
            out_frame->height = in_frame->height;
            out_frame->media_time = in_frame->media_time;
            out_frame->seqno = in_frame->seqno;
            out_frame->frame_type = in_frame->frame_type;
            out_frame->codec = in_frame->codec;
            out_frame->buffer_len = in_frame->buffer_len;
            memcpy(out_frame->buffer, in_frame->buffer, in_frame->buffer_len);
            put_frame(dst->video->decoded_frames);
            remove_frame(src->video->decoded_frames);
        }
    }
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

int main()
{
    fprintf(stderr, "Starting mmforwarder\n");

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
    // Video stream with a participant
    stream = init_stream(VIDEO, INPUT, rand(), I_AWAIT,
            INPUT_VIDEO_FORMAT_FPS, "Input video stream");
    add_participant_stream(stream,
            init_participant(1, INPUT, NULL, 0));
    set_video_frame_cq(stream->video->coded_frames, H264, 0, 0);
    add_stream(receiver->video_stream_list, stream);
    // Audio stream with a participant
    stream = init_stream(AUDIO, INPUT, rand(), I_AWAIT, 0, 
            "Input audio stream");
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
    // Video stream with a participant
    stream = init_stream(VIDEO, OUTPUT, 0, ACTIVE,
            OUTPUT_VIDEO_FORMAT_FPS, "Output video stream");
    add_participant_stream(stream,
            init_participant(0, OUTPUT, OUTPUT_IP, OUTPUT_VIDEO_PORT));
    set_video_frame_cq(stream->video->decoded_frames, RAW, 1280, 534);
    set_video_frame_cq(stream->video->coded_frames, H264, 1280, 534);
    add_stream(transmitter->video_stream_list, stream);
    init_encoder(stream->video);
    // Audio stream with a participant
    stream = init_stream(AUDIO, OUTPUT, 0, ACTIVE, 0, "Output audio stream");
    add_participant_stream(stream, 
            init_participant(0, OUTPUT, OUTPUT_IP, OUTPUT_AUDIO_PORT));
    ap_config(stream->audio, OUTPUT_AUDIO_FORMAT_BPS,
            OUTPUT_AUDIO_FORMAT_SAMPLE_RATE,
            OUTPUT_AUDIO_FORMAT_CHANNELS,
            OUTPUT_AUDIO_FORMAT_CODEC);
    add_stream(transmitter->audio_stream_list, stream);
    ap_worker_start(stream->audio);

    // Temporal variables and initializations
    stream_data_t *audio_in_stream,
                  *audio_out_stream,
                  *video_in_stream,
                  *video_out_stream;

    // Main loop
    fprintf(stderr, "  ·Forwarding multimedia! ");
    while(!stop) {

        // Audio forward
        audio_in_stream = receiver->audio_stream_list->first;
        audio_out_stream = transmitter->audio_stream_list->first;
        audio_frame_forward(audio_in_stream, audio_out_stream);

        // Video forward
        video_in_stream = receiver->video_stream_list->first;
        video_out_stream = transmitter->video_stream_list->first;
        video_frame_forward(video_in_stream, video_out_stream);

        //Try to not send all the data suddently.
        usleep(SEND_TIME);
    }
    fprintf(stderr, "Done!\n");

    // Finish and destroy receiver objects
    stop_receiver(receiver);
    destroy_stream_list(receiver->video_stream_list);
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

