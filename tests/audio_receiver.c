/*
 * audio_receiver.c - Test program that starts io_mngr receiver, writes 10
 * seconds of audio from one source in a file and then 10 seconds of audio from
 * another source to another file.
 *
 * By Txor <jordi.casas@i2cat.net>
 */

#include <stdio.h>
#include <errno.h>
#include "config.h"
#include "io_mngr/participants.h"
#include "io_mngr/receiver.h"


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

    if (start_receiver(receiver)) {
        printf(" ·Receiver started!\n");


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

