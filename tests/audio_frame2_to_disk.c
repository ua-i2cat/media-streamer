/*
 * To file audio_frame2 recorder.
 *
 * By Txor <jordi.casas@i2cat.net>
 */

#include <stdio.h>
#include "audio_frame2_to_disk.h"

// Write interleaved audio samples from an audio_frame2 to a file, truncating it.
void write_audio_frame2_to_file (char *path, audio_frame2 *frame) {

    FILE *file = NULL;
    char *err = NULL;
    int bytes = 0;

    file = fopen(path, "wb");
    if (file == NULL) {
        sprintf(err, "Error: %s\n", path);
        perror(err);
        return;
    }

    printf("Writing frame to %s ... ", path);
    // Interleave the channels
    for (int sample = 0; sample < (frame->data_len[0] / frame->bps); sample++) {
        for (int ch = 0; ch < frame->ch_count ; ch++) {
            fwrite((char *)(frame->data[ch] + sample), frame->bps, 1, file);
            bytes += frame->bps;
        }
    }

    fclose(file);
    printf("Done (%i bytes)!\n", bytes);
}

// Write interleaved audio samples from an audio_frame2 to a file, appending it.
void add_audio_frame2_to_file (char *path, audio_frame2 *frame) {

    FILE *file = NULL;
    char *err = NULL;
    int bytes = 0;

    file = fopen(path, "ab");
    if (file == NULL) {
        sprintf(err, "Error: %s\n", path);
        perror(err);
        return;
    }

    printf("Adding frame to %s ... ", path);
    switch (frame->ch_count) {
        case 1: // Write whole frame
            fwrite(frame->data[0], frame->data_len[0], 1, file);
            bytes = frame->data_len[0];
            printf("One-Shot ... ");
            break;
        default: // Interleave the channels
            for (int sample = 0; sample < (frame->data_len[0] / frame->bps); sample++) {
                for (int ch = 0; ch < frame->ch_count ; ch++) {
                    fwrite((char *)(frame->data[ch] + sample), frame->bps, 1, file);
                    bytes += frame->bps;
                }
            }
    }

    fclose(file);
    printf("Done (%i bytes)!\n", bytes);
}

