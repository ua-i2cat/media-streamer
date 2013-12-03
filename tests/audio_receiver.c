/*
 * audio_receiver.c - Test program that starts io_mngr receiver, writes 10
 * seconds of audio from one source in a file and then 10 seconds of audio from
 * another source to another file.
 *
 * By Txor <jordi.casas@i2cat.net>
 */

#include <stdio.h>
#include <errno.h>
#include <time.h>
#include "config.h"
#include "io_mngr/participants.h"
#include "io_mngr/receiver.h"

// Debug control
#define STREAM1
#define STREAM1_WRITE
#define STREAM2
#define STREAM2_WRITE

FILE *F_audio1 = NULL;
FILE *F_audio2 = NULL;
char *name_audio1 = "test_receiver_audio1.pcm";
char *name_audio2 = "test_receiver_audio2.pcm";

int main() {

    printf("Starting audio_receiver test...\n");

    // Open files to write audio
    if ((F_audio1 = fopen(name_audio1, "wb")) == NULL) {
        perror(name_audio1);
        exit(errno);
    }
    if ((F_audio2 = fopen(name_audio2, "wb")) == NULL) {
        perror(name_audio2);
        exit(errno);
    }

    // Timer settings
    time_t start, stop;

    // Receiver configuration
    stream_list_t *stream_list = init_stream_list();
    receiver_t *receiver = init_receiver(stream_list, 5004, 5006);

    // First stream and participant configuration
    participant_data_t *p1 = init_participant(1, INPUT, NULL, 0);
    stream_data_t *stream1 = init_stream(AUDIO, INPUT, rand(), I_AWAIT, "Stream1");
    add_participant_stream(stream1, p1);
    add_stream(receiver->stream_list, stream1);

    // Second stream and participant configuration
    participant_data_t *p2 = init_participant(2, INPUT, NULL, 0);
    stream_data_t *stream2 = init_stream(AUDIO, INPUT, rand(), I_AWAIT, "Stream2");
    add_participant_stream(stream2, p2);
    add_stream(receiver->stream_list, stream2);

    // Temp vars
    audio_frame2 *audio_frame;
    stream_data_t *stream;

    if (start_receiver(receiver)) {
        printf(" ·Receiver started!\n");

#ifdef STREAM1
        // Wait until the first stream appears
        bool appeared = false;
        while (!appeared) {
            pthread_rwlock_rdlock(&stream_list->lock);
            stream = stream_list->first; // First stream
            pthread_rwlock_unlock(&stream_list->lock);
            if (stream != NULL) appeared = true;
            else usleep(500);
        }
#ifdef STREAM1_WRITE
        // Wait for the first decoded_cq gets filled and go ahead
        while (stream1->audio->decoded_cq->level == CIRCULAR_QUEUE_EMPTY) {
            usleep(250); //Sleep 0'25 seconds
        }
        printf("  ·Copying first stream... ");
        start = time(NULL);
        stop = start + 10; // 10 seconds
        // Work on file 1
        while (time(NULL) < stop) {
            audio_frame = cq_get_front(stream1->audio->decoded_cq);
            if (audio_frame != NULL) {
                fwrite(audio_frame->data[0], audio_frame->data_len[0], 1, F_audio1);
                cq_remove_bag(stream1->audio->decoded_cq);
            }
        }
#endif //STREAM1_WRITE
        printf("Done!\n");
#endif //STREAM1

#ifdef STREAM2
        // Wait until the second stream appears
        appeared = false;
        while (!appeared) {
            pthread_rwlock_rdlock(&stream_list->lock);
            stream = stream_list->first->next; // Second stream
            pthread_rwlock_unlock(&stream_list->lock);
            if (stream != NULL) appeared = true;
            else usleep(500);
        }
#ifdef STREAM2_WRITE
        // Wait for the second decoded_cq gets filled and go ahead
        while (stream2->audio->decoded_cq->level == CIRCULAR_QUEUE_EMPTY) {
            usleep(250); //Sleep 0'25 seconds
        }
        printf("  ·Copying first stream... ");
        start = time(NULL);
        stop = start + 10; // 10 seconds
        // Work on file 1
        while (time(NULL) < stop) {
            audio_frame = cq_get_front(stream2->audio->decoded_cq);
            if (audio_frame != NULL) {
                fwrite(audio_frame->data[0], audio_frame->data_len[0], 1, F_audio2);
                cq_remove_bag(stream2->audio->decoded_cq);
            }
        }
#endif //STREAM2_WRITE
        printf("Done!\n");
#endif //STREAM2

        // Finish and destroy objects
        stop_receiver(receiver);
        destroy_receiver(receiver);
        printf(" ·Receiver stopped\n");
        destroy_stream_list(stream_list);
    }

    if (fclose(F_audio1) != 0) {
        perror(name_audio1);
        exit(-1);
    }
    if (fclose(F_audio2) != 0) {
        perror(name_audio2);
        exit(-1);
    }
    printf("Finished\n");
}

