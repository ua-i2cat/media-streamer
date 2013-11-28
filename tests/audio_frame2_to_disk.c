/*
 * To file audio_frame2 recorder.
 *
 * By Txor <jordi.casas@i2cat.net>
 */

#include <stdio.h>
#include "audio_frame2_to_disk.h"

/*
 * write_audio_frame2 - Appends an audio_frame2 raw data to a file interleaving
 * the channels, with multiple options.
 *
 * Uses a file named prefix + "RATE_CHANNELS_BPS", i.e. for preffix="audio", RATE=8000,
 * CHANNELS=1 and BPS=1 the generated file name will be: audio_8000_1_1.raw
 * 
 * BPS can be configured with bps parameter.
 *
 * Verbose output can be configured with verbose boolean.
 *
 * TODO: Write only once depending on a IN bool.
 *
 */
void write_audio_frame2(char *preffix, const audio_frame2 *frame, int bps, bool verbose) {

    FILE *file = NULL;
    char *err = NULL;
    int bytes = 0;
    
    char name[255];

    sprintf(name, "%s_%i_%i_%i.raw", preffix, frame->sample_rate, frame->ch_count, bps);

    file = fopen(name, "ab");
    if (file == NULL) {
        sprintf(err, "Error: %s\n", name);
        perror(err);
        return;
    }

    if (verbose) printf("Adding frame to %s ... ", name);
    switch (frame->ch_count) {
        case 1: // Write whole frame
            fwrite(frame->data[0], frame->data_len[0], 1, file);
            bytes = frame->data_len[0];
            if (verbose) printf("One-Shot ... ");
            break;
        default: // Interleave the channels
            for (int sample = 0; sample < (frame->data_len[0] / bps); sample++) {
                for (int ch = 0; ch < frame->ch_count ; ch++) {
                    fwrite((char *)(frame->data[ch] + sample), bps, 1, file);
                    bytes += bps;
                }
            }
            if (verbose) printf("Interleaved ... ");
    }

    fclose(file);
    if (verbose) printf("Done (%i bytes)!\n", bytes);
}

/*
 * write_audio_frame2_channels - Appends each channel of a stereo audio_frame2 raw data to 
 * a different file on the disk, with multiple options.
 *
 * Uses a file named prefix + "RATE_CHANNELS_BPS_chan" + NCHAN, i.e. for preffix="audio", RATE=8000,
 * CHANNELS=1 and BPS=1 the generated file name will be: audio_8000_1_1_chan1.raw
 * 
 * Verbose output can be configured with verbose boolean.
 *
 * TODO: Write only once depending on a IN bool.
 *
 */
void write_audio_frame2_channels(char *preffix, const audio_frame2 *frame, bool verbose) {

    FILE *file = NULL;
    char *err = NULL;

    for(int ch = 0; ch < frame->ch_count; ch++) {
        char name[255];

        sprintf(name, "%s_%i_%i_%i_chan%i.raw", preffix, frame->sample_rate, frame->ch_count, frame->bps, ch);
        file = fopen(name, "ab");
        if (file == NULL) {
            sprintf(err, "Error: %s\n", name);
            perror(err);
            return;
        }

        if (verbose) printf("Adding frame to %s ... ", name);
        fwrite(frame->data[ch], frame->data_len[ch], 1, file);

        fclose(file);
        if (verbose) printf("Done (%i bytes)!\n", frame->data_len[ch]);
    }
}
