/*
 * A resampler_resample wrapper:
 * Chop a greater audio_frame2 into smaller ones,
 * call resampler_resample for each of them and
 * attach them on one resulting frame.
 *
 * By Txor <jordi.casas@i2cat.net>
 */

#include <stdio.h>
#include "resized_resample.h"
#include "audio_frame2_to_disk.h"
#include "utils.h"

// private audio_frame2
audio_frame2 *chunk;
audio_frame2 *result;

/*
 * init - Initialize private strauctures only once.
 */
static void init() {

    static int init = 0;

    if (!init) {
        chunk = audio_frame2_init();
        result = audio_frame2_init();
    }
}

/*
 * resample_chop - Updates chunk with the umpteenth (numbered by p) audio_frame2 of data size s.
 */
static audio_frame2 *resample_chop(audio_frame2 *in, int s, int p) {

    chunk->bps = in->bps;
    chunk->sample_rate = in->sample_rate;
    chunk->ch_count = in->ch_count;
    chunk->codec = in->codec;
    chunk->max_size = s;

    int size;
    if (s * (p + 1) > in->data_len[0]) {
        size = in->data_len[0] % s;
    }
    else {
        size = s;
    }

    for (int ch = 0; ch < in->ch_count; ch++) {
        memcpy(chunk->data[ch], in->data[ch] + (s * p), size);
        chunk->data_len[ch] = size;
    }

    return chunk;
}

/*
 * resample_glue - Append the audio frame in on the audio_frame2 out.
 */
static void resample_glue(audio_frame2 *out, audio_frame *in) {

    for (int ch = 0; ch < out->ch_count; ch++) {
        memcpy(out->data[ch] + out->data_len[ch], in->data[ch], in->data_len[ch]);
        out->data_len[ch] += in->data_len[ch];
    }
}

/*
 * resize_resample - TODO
 */
audio_frame2 *resize_resample(struct resampler *s, audio_frame2 *frame, void *callback, int max_size) {

    if (frame->data_len[0] <= max_size) {
        // Don't start to amputate and sew, just call resampler_resample.
        result = resample_callback(s, frame);
    }

    audio_frame2 *(*resample_callback)(struct resampler *, audio_frame2 *) = callback;

    init();
    // Allocate space for the chunk audio_frame2
    audio_frame2_allocate(chunk, frame->ch_count, max_size);

    int nframes = frame->data_len[0]/max_size + (frame->data_len[0]%max_size)?0:1;

    for (int f = 0; f < nframes; f++){
        // Call the callback for each chunk
        audio_frame2 *resample_result = resample_callback(s, resample_chop(frame, max_size, f));

        // Resize result audio_frame2 if needed or only once.
        if (result->sample_rate != resample_result->sample_rate) {
            audio_frame2_allocate(result, resample_result->ch_count, resample_result->data_len[0] * nframes);
        }

        // Glue the current audio_frame2
        resample_glue(result, resample_result);
    }
    
    return result;
}


/*
 * resize_resample - Split frame in lesser audio_frame2s of size max_size or less,
 * call resample_callback for each of them,
 * and join them on to another audio_frame2.
 *
audio_frame2 *resize_resample_old(struct resampler *s, audio_frame2 *frame, void *callback, int max_size) {

    static audio_frame2 *result = NULL;
    audio_frame2 *(*resample_callback)(struct resampler *, audio_frame2 *) = callback;

    if (frame->data_len[0] <= max_size) {
        // Don't act as Jack The Ripper, just call resampler_resample.
        result = resample_callback(s, frame);
    }
    else {
        int nframe;
        audio_frame2 *callback_result;

        // Calculate how many frames we'll need.
        int frames = frame->data_len[0] / max_size;

        // Init and allocate result equal than frame 
        if (result == NULL) result = audio_frame2_init();
        audio_frame2_allocate(result, frame->ch_count, frame->max_size);
        for (int ch = 0; ch < frame->ch_count; ch++) {
            result->data_len[ch] = 0;
        }
        // and an array of pointers to his data.
        char *result_ptr[MAX_AUDIO_CHANNELS];
        for (int ch = 0; ch < frame->ch_count; ch++) {
            result_ptr[ch] = result->data[ch];
        }


        // Prepare a chunk audio_frame2
        audio_frame2 *chunk = audio_frame2_init(); // audio_frame2 space only
        chunk->bps = frame->bps;
        chunk->sample_rate = frame->sample_rate;
        chunk->ch_count = frame->ch_count;
        chunk->max_size = frame->max_size;
        chunk->codec = frame->codec;
        for (int ch = 0; ch < frame->ch_count; ch++) {
            chunk->data[ch] = frame->data[ch];
            chunk->data_len[ch] = max_size;
        }

        for (nframe = 0; nframe < frames; nframe++) {
            // Resample.
            callback_result = resample_callback(s, chunk);
            // If resampler_resample is returning the same audio_frame2 pointer,
            // it didn't do nor going to do something, then return the original frame.
            if (chunk == callback_result) {
                return frame;
            }
            write_audio_frame2_channels("aoutput_2a_resized_before", chunk, false);
            write_audio_frame2_channels("aoutput_2b_resized_after", callback_result, false);

            // Advance chunk data pointer.
            for (int ch = 0; ch < frame->ch_count; ch++) {
                chunk->data[ch] += max_size;
            }

            // Init some result fields only once
            if (nframe == 0) {
                result->bps = callback_result->bps;
                result->sample_rate = callback_result->sample_rate;
                result->codec = callback_result->codec;
            }

            // Add the resampled chunk to the resulting frame.
            for (int ch = 0; ch < frame->ch_count; ch++) {
                memcpy(result_ptr[ch], callback_result->data[ch], callback_result->data_len[ch]);
                result->data_len[ch] += callback_result->data_len[ch];
                result_ptr[ch] += callback_result->data_len[ch];
            }
        }

        // We need to process the remainder data.
        if (frame->data_len[0] % max_size != 0) {
            // Point the chunk to the remainder data with remainder size.
            for (int ch = 0; ch < frame->ch_count; ch++) {
                chunk->data_len[ch] = frame->data_len[ch] - (nframe * max_size);
            }

            // Resample.
            callback_result = resample_callback(s, chunk);

            // Add to resulting frame.
            for (int ch = 0; ch < frame->ch_count; ch++) {
                memcpy(result_ptr[ch], callback_result->data[ch], callback_result->data_len[ch]);
                result->data_len[ch] += callback_result->data_len[ch];
            }
        }

        free(chunk); // We can't use audio_frame2_free coz nobody alloc'd data buffers.
    }

    return result;
}
*/
