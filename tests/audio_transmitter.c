/*
 * audio_transmitter.c - Test program that starts io_mngr transmitter,
 * reads PLAY_TIME seconds from a file and sends the audio to two
 * different participants.
 *
 * By Txor <jordi.casas@i2cat.net>
 */

#include <stdio.h>
#include <errno.h>
#include <time.h>
//#include "config.h"
#include "transmitter.h"
#include "audio_config.h"

#define PLAY_TIME 60

FILE *F_audio = NULL;
char *name_audio = "~/testing/atrans_test_internal_format.pcm";

int main() {

    fprintf(stderr, "Starting audio_transmitter test...\n");

    // Open files to write audio
    if ((F_audio = fopen(name_audio, "rb")) == NULL) {
        perror(name_audio);
        exit(errno);
    }

    // Transmitter and stream configuration
    stream_list_t *video_stream_list = init_stream_list(); // Not used
    stream_list_t *audio_stream_list = init_stream_list();
    stream_data_t *stream = init_stream(AUDIO, OUTPUT, 0, ACTIVE, "Stream");
    add_stream(audio_stream_list, stream);
    transmitter_t *transmitter = init_transmitter(streams, 25.0);
    start_transmitter(transmitter);
    fprintf(stderr, " ·Transmitter started!\n");

    // Configure two participants
    participant_data_t *p1 = init_participant(0, OUTPUT, "127.0.0.1", 5006);
    participant_data_t *p2 = init_participant(0, OUTPUT, "127.0.0.1", 6006);
    add_participant_stream(stream, p1);    
    add_participant_stream(stream, p2);

    // configure output stream audio format
    fprintf(stderr, " ·Stream configuration: 1 bps, 8000Hz, 1 channel, mulaw\n");
    ap_config(stream->audio, 1, 8000, 1, AC_MULAW);
    ap_worker_start(stream->audio);

    fprintf(stderr, "  ·Reading file... ");
    start = time(NULL);
    stop = start + RECORD_TIME;
    while (time(NULL) < stop) { // RECORD_TIME seconds loop
        audio_frame = cq_get_rear(stream->audio->decoded_cq);
        if (audio_frame != NULL) {
            audio_frame->data_len[0] = AUDIO_INTERNAL_SIZE;
            if (fread(audio_frame->data[0], audio_frame->data_len[0], 1, F_audio) < 1) {
                perror(name_audio);
            }
            cq_add_bag(stream->audio->decoded_cq);
        }
    }
    fprintf(stderr, "Done!\n");

    // Finish and destroy objects
    stop_transmitter(transmitter);
    //destroy_receiver(receiver);
    fprintf(stderr, " ·Transmitter stopped\n");
    destroy_stream_list(audio_stream_list);
    if (fclose(F_audio) != 0) {
        perror(name_audio);
        exit(-1);
    }
    fprintf(stderr, "Finished\n");
}

