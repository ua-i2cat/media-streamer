/*
 * audio_receiver.c - Test program that starts media-streamer receiver, writes
 * RECORD_TIME seconds from one audio stream in to a file and then 
 * RECORD_TIME seconds more of another audio stream to another file.
 *
 * By Txor <jordi.casas@i2cat.net>
 */

#include <stdio.h>
#include <errno.h>
#include <time.h>
//#include "config.h"
#include "io_mngr/participants.h"
#include "io_mngr/receiver.h"

#define RECORD_TIME 5

FILE *F_audio1 = NULL;
FILE *F_audio2 = NULL;
char *name_audio1 = "atest_audio_receiver_stream1.pcm";
char *name_audio2 = "atest_audio_receiver_stream2.pcm";

int main() {

    fprintf(stderr, "Starting audio_receiver test...\n");

    // Open files to write audio
    if ((F_audio1 = fopen(name_audio1, "wb")) == NULL) {
        perror(name_audio1);
        exit(errno);
    }
    if ((F_audio2 = fopen(name_audio2, "wb")) == NULL) {
        perror(name_audio2);
        exit(errno);
    }

    // General pourpouse variables.
    time_t start, stop;
    audio_frame2 *audio_frame;

    // Receiver configuration
    stream_list_t *video_stream_list = init_stream_list(); // Not used
    stream_list_t *audio_stream_list = init_stream_list();
    receiver_t *receiver = init_receiver(video_stream_list, audio_stream_list, 5004, 5006);

    // First stream and participant configuration
    participant_t *p1 = init_participant(1, INPUT, NULL, 0);
    stream_t *stream1 = init_stream(AUDIO, INPUT, rand(), I_AWAIT, "Stream1");
    add_participant_stream(stream1, p1);
    add_stream(receiver->audio_stream_list, stream1);
    fprintf(stderr, " ·Stream1 configuration: 1 bps, 32000Hz, 1 channel, mulaw\n");
    ap_config(stream1->audio, 1, 32000, 1, AC_MULAW);
    ap_worker_start(stream1->audio);

    // Second stream and participant configuration
    participant_t *p2 = init_participant(2, INPUT, NULL, 0);
    stream_t *stream2 = init_stream(AUDIO, INPUT, rand(), I_AWAIT, "Stream2");
    add_participant_stream(stream2, p2);
    add_stream(receiver->audio_stream_list, stream2);
    fprintf(stderr, " ·Stream2 configuration: 1 bps, 8000Hz, 1 channel, mulaw\n");
    ap_config(stream2->audio, 1, 8000, 1, AC_MULAW);
    ap_worker_start(stream2->audio);

    if (start_receiver(receiver)) {
        fprintf(stderr, " ·Receiver started!\n");

        // STREAM1 recording block
        fprintf(stderr, "  ·Waiting for audio_frame2 data\n");
        while (stream1->audio->decoded_cq->level == CIRCULAR_QUEUE_EMPTY) {
            usleep(50);
        }

        fprintf(stderr, "   ·Copying to file... ");
        start = time(NULL);
        stop = start + RECORD_TIME;
        while (time(NULL) < stop) { // RECORD_TIME seconds loop
            audio_frame = cq_get_front(stream1->audio->decoded_cq);
            if (audio_frame != NULL) {
                fwrite(audio_frame->data[0], audio_frame->data_len[0], 1, F_audio1);
                cq_remove_bag(stream1->audio->decoded_cq);
            }
        }
        fprintf(stderr, "Done!\n");

        // STREAM2 recording block
        fprintf(stderr, "  ·Waiting for audio_frame2 data\n");
        while (stream2->audio->decoded_cq->level == CIRCULAR_QUEUE_EMPTY) {
            usleep(50);
        }

        fprintf(stderr, "   ·Copying to file... ");
        start = time(NULL);
        stop = start + RECORD_TIME;
        while (time(NULL) < stop) { // RECORD_TIME seconds loop
            audio_frame = cq_get_front(stream2->audio->decoded_cq);
            if (audio_frame != NULL) {
                fwrite(audio_frame->data[0], audio_frame->data_len[0], 1, F_audio2);
                cq_remove_bag(stream2->audio->decoded_cq);
            }
        }
        fprintf(stderr, "Done!\n");

        // Finish and destroy objects
        stop_receiver(receiver);
        destroy_receiver(receiver);
        fprintf(stderr, " ·Receiver stopped\n");
        destroy_stream_list(video_stream_list);
        destroy_stream_list(audio_stream_list);
    }

    if (fclose(F_audio1) != 0) {
        perror(name_audio1);
        exit(-1);
    }
    if (fclose(F_audio2) != 0) {
        perror(name_audio2);
        exit(-1);
    }
    fprintf(stderr, "Finished\n");
}

