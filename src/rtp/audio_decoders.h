
#ifndef _AUDIO_DECODERS_H_
#define _AUDIO_DECODERS_H_

struct state_audio_decoder {
    audio_frame2 *frame;

    struct resampler *resampler;

    struct audio_desc *desc;
};

struct coded_data;

int decode_audio_frame(struct coded_data *cdata, void *data);
int decode_audio_frame_mulaw(struct coded_data *cdata, void *data);

#endif //_AUDIO_DECODERS_H_
