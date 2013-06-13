/*
 * FILE:	video_display/net.h
 * AUTHORS: David Cuenca 	<david.cuenca@i2cat.net>
 * 			Gerard Castillo <gerard.castillo@i2cat.net>
 *
 * Copyright (c) 2005-2013 Fundació i2CAT, Internet I Innovació Digital a Catalunya
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, is permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 * 
 *      This product includes software developed by the University of Southern
 *      California Information Sciences Institute.
 * 
 * 4. Neither the name of the University nor of the Institute may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $Revision: 1.1 $
 * $Date: 2007/11/08 09:48:59 $
 *
 */

#define DISPLAY_NET_ID	0x12aaa060
#include "video.h"

struct audio_frame;

display_type_t		*display_net_probe(void);
void 				*display_net_init(char *fmt, unsigned int flags);
void 			 	display_net_run(void *state);
void 			 	display_net_finish(void *state);
void 			 	display_net_done(void *state);
struct video_frame	*display_net_getf(void *state);
int 			 	display_net_putf(void *state, struct video_frame *frame, int nonblock);
int                 display_net_reconfigure(void *state, struct video_desc desc);
int                 display_net_get_property(void *state, int property, void *val, size_t *len);
void                display_net_put_audio_frame(void *state, struct audio_frame *frame);
int                 display_net_reconfigure_audio(void *state, int quant_samples, int channels, int sample_rate);

