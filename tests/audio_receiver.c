/*
 * audio_receiver.c - Test program that starts io_mngr receiver, writes
 * RECORD_TIME seconds from one audio stream in to a file and then 
 * RECORD_TIME seconds more of another audio stream to another file.
 *
 * By Txor <jordi.casas@i2cat.net>
 */

#include <stdio.h>
#include <errno.h>
#include <time.h>
#include "config.h"
#include "io_mngr/participants.h"
#include "io_mngr/receiver.h"

#define RECORD_TIME 5

// Debug control
//#define QUEUE_PRINT
#define STREAM1
#define STREAM2

FILE *F_audio1 = NULL;
FILE *F_audio2 = NULL;
char *name_audio1 = "atest_receiver_audio1.pcm";
char *name_audio2 = "atest_receiver_audio2.pcm";

#ifdef QUEUE_PRINT
// Private circular_queue debug function
static void print_cq_status(circular_queue_t *cq, char *msg);

static void print_cq_status(circular_queue_t *cq, char *msg) {
    static char level;
    static int front;
    static int rear;
    static int count = 0;
    static char *m = NULL;
    if (m != msg) {
        level = '0';
        front = -1;
        rear = -1;
        m = msg;
    }
    char l;
    switch(cq->level) {
        case CIRCULAR_QUEUE_MID:
            l = 'M';
            break;
        case CIRCULAR_QUEUE_EMPTY:
            l = 'E';
            break;
        case CIRCULAR_QUEUE_FULL:
            l = 'F';
            break;
    }
    if (level != l || front != cq->front || rear != cq->rear) {
        level = l;
        front = cq->front;
        rear = cq->rear;
        fprintf(stderr, "[circular_queue %i %s] Level(%c), Front(%i), Rear(%i).\n", count, m, level, front, rear);
    }
    count++;
}
#endif //QUEUE_PRINT

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

    if (start_receiver(receiver)) {
        fprintf(stderr, " ·Receiver started!\n");

#ifdef STREAM1
        fprintf(stderr, "  ·Waiting for audio_frame2 data\n");
        // Wait for the first coded_cq gets filled and go ahead
#ifdef QUEUE_PRINT
        print_cq_status(stream1->audio->coded_cq, "first stream1");
#endif
        while (stream1->audio->coded_cq->level == CIRCULAR_QUEUE_EMPTY) {
#ifdef QUEUE_PRINT
            print_cq_status(stream1->audio->coded_cq, "wait stream1");
#endif
        }
#ifdef QUEUE_PRINT
        print_cq_status(stream1->audio->coded_cq, "continue stream1");
#endif
        if (!stream1->audio->run) ap_worker_start(stream1->audio);
        fprintf(stderr, "   ·Copying to file... ");
        start = time(NULL);
        stop = start + RECORD_TIME;
        // Work on file 1
        while (time(NULL) < stop) {
            audio_frame = cq_get_front(stream1->audio->decoded_cq);
            if (audio_frame != NULL) {
                fwrite(audio_frame->data[0], audio_frame->data_len[0], 1, F_audio1);
                cq_remove_bag(stream1->audio->decoded_cq);
            }
        }
#endif //STREAM1
        fprintf(stderr, "Done!\n");

#ifdef STREAM2
        fprintf(stderr, "  ·Waiting for audio_frame2 data\n");
        // Wait for the second coded_cq gets filled and go ahead
#ifdef QUEUE_PRINT
        print_cq_status(stream2->audio->coded_cq, "first stream2");
#endif
        while (stream2->audio->coded_cq->level == CIRCULAR_QUEUE_EMPTY) {
#ifdef QUEUE_PRINT
            print_cq_status(stream2->audio->coded_cq, "wait stream2");
#endif
        }
#ifdef QUEUE_PRINT
        print_cq_status(stream2->audio->decoded_cq, "continue stream2");
#endif
        if (!stream2->audio->run) ap_worker_start(stream2->audio);
        fprintf(stderr, "   ·Copying to file... ");
        start = time(NULL);
        stop = start + RECORD_TIME;
        // Work on file 1
        while (time(NULL) < stop) {
            audio_frame = cq_get_front(stream2->audio->decoded_cq);
            if (audio_frame != NULL) {
                fwrite(audio_frame->data[0], audio_frame->data_len[0], 1, F_audio2);
                cq_remove_bag(stream2->audio->decoded_cq);
            }
        }
#endif //STREAM2
        fprintf(stderr, "Done!\n");

        // Finish and destroy objects
        stop_receiver(receiver);
        destroy_receiver(receiver);
        fprintf(stderr, " ·Receiver stopped\n");
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
    fprintf(stderr, "Finished\n");
}

