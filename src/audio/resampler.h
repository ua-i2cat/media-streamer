
#ifdef HAVE_CONFIG_H
#include "config.h"
#include "config_win32.h"
#include "config_unix.h"
#endif

#include "audio.h"

struct resampler;

// resampler refactor due to privacity of the struct resample definition
// Used by media-streamer.
struct resampler *resampler_prepare(int dst_sample_rate);
void resampler_set_resampled(struct resampler *s, audio_frame2 *frame);
int resampler_compare_sample_rate(struct resampler *s, int rate);

struct resampler *resampler_init(int dst_sample_rate);
void              resampler_done(struct resampler *);
audio_frame2     *resampler_resample(struct resampler *, audio_frame2 *);

