/*
 * A resampler_resample wrapper:
 * Chop a greater audio_frame2 into smaller ones,
 * call resampler_resample for each of them and
 * attach them on one resulting frame.
 *
 * By Txor <jordi.casas@i2cat.net>
 */

#include "resized_resample.h"
#include "utils.h"

#define MAX_FRAME_SIZE 48000 * 2

/*
 * private data
 */
audio_frame2 *chunk;
audio_frame2 *result;

/*
 * init - Initialize private structures only once.
 */
static void init() {

    static int init = 0;

    if (!init) {
        chunk = audio_frame2_init();
        result = audio_frame2_init();
        // Allocate enought memory (far enought!) only once.
        audio_frame2_allocate(chunk, MAX_AUDIO_CHANNELS, MAX_FRAME_SIZE);
        audio_frame2_allocate(result, MAX_AUDIO_CHANNELS, MAX_FRAME_SIZE);
    }
}

/*
 * resample_chop - Updates chunk with the umpteenth (numbered by p) audio_frame2 from data size s.
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
 * resample_glue - Appends the audio frame in on the audio_frame2 out.
 */
static void resample_glue(audio_frame2 *out, audio_frame2 *in) {

    if (out->bps != in->bps || 
            out->sample_rate != in->sample_rate ||
            out->ch_count != in->ch_count ||
            out->codec != in->codec) {
        out->bps = in->bps;
        out->sample_rate = in->sample_rate;
        out->ch_count = in->ch_count;
        out->codec = in->codec;
    }

    for (int ch = 0; ch < out->ch_count; ch++) {
        memcpy(out->data[ch] + out->data_len[ch], in->data[ch], in->data_len[ch]);
        out->data_len[ch] += in->data_len[ch];
    }
}

/*
 * resize_resample - Using its private functions resample_chop and resample_glue,
 *                      splits an audio_frame2 in lesser parts,
 *                      executes resampler_resample as callback on each of them
 *                      and joins them all together as a returning audio_frame2.
 */
audio_frame2 *resize_resample(struct resampler *s, audio_frame2 *frame, void *callback, int max_size) {

    audio_frame2 *(*resample_callback)(struct resampler *, audio_frame2 *) = callback;

    if (frame->data_len[0] <= max_size) {
        // Don't start to amputate and sew, just call resampler_resample.
        result = resample_callback(s, frame);
    }

    init();

    int nframes = frame->data_len[0]/max_size;
    if (frame->data_len[0] % max_size) nframes++;

    for (int f = 0; f < nframes; f++){
        // Call the callback for each chunk
        audio_frame2 *resample_result = resample_callback(s, resample_chop(frame, max_size, f));

        // Glue the current audio_frame2
        resample_glue(result, resample_result);
    }

    return result;
}
