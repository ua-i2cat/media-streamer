#ifdef HAVE_CONFIG_H
#include "config.h"
#include "config_win32.h"
#include "config_unix.h"
#endif

#include "audio.h"

struct resampler;

// resampler refactor due to privacity of the struct resample definition
// Used by UG_modules project
struct resampler *resampler_prepare(int dst_sample_rate);
void resampler_set_resampled(struct resampler *s, audio_frame2 *frame);

struct resampler *resampler_init(int dst_sample_rate);
void              resampler_done(struct resampler *);
audio_frame2     *resampler_resample(struct resampler *, audio_frame2 *);

