/*
 * FILE:     from tv.c
 */

#include "config.h"
#include "config_unix.h"
#include "config_win32.h"
#include "debug.h"
#include "crypto/random.h"
#include "tv_std.h"

/*
 * Calculate initial time on first execution, add per sample time otherwise.
 */
uint32_t get_std_audio_local_mediatime(int samples, int rate)
{
        static uint32_t saved_timestamp;
        static int first = 0;

        uint32_t curr_timestamp;
        struct timeval start_time;

        if (first == 0) {
                gettimeofday(&start_time, NULL);
                curr_timestamp = start_time.tv_sec +
                                    (start_time.tv_usec / 1000000.0) + 
                                    lbl_random();
                first = 1;
        }
        else {
            curr_timestamp = saved_timestamp;
        }
//TODO: SOLVE THIS FUCK MESS WITH MILISECONDS!!!!
        saved_timestamp = curr_timestamp + (samples * (double)(1/rate));

        return curr_timestamp;
}

