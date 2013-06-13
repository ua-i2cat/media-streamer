/*
 * FILE:   	video_capture/net.c
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
 * $Revision: 1.3.2.1 $
 * $Date: 2010/01/30 20:07:35 $
 *
 */

#include "config.h"
#include "config_unix.h"
#include "config_win32.h"
#include "debug.h"
#include "video_codec.h"
#include "video_capture.h"
#include "video_capture/net.h"

#define MAX_AUDIO_SIZE 48*1024

extern bool net_frame_decoded;

struct net_state_capture{
	struct timeval	last_frame_time;
	struct video_frame 	*net_video_frame;
	char* 	net_audio_frame;
	int 	width;
	int 	height;
	int 	video_size;
};

struct net_state_capture *s_net_capture;

void *vidcap_net_init(char *fmt, unsigned int flags)
{
	printf("		vidcap_net_init\n");
    	s_net_capture = malloc(sizeof(struct net_state_capture));
    	char *tmp, *token, *saveptr = NULL;
		int grid_w, grid_h;

		if(fmt == NULL){
			grid_w = 1920;
			grid_h = 1080;
		}else{
			tmp = strdup(&fmt[2]);
			token = strtok_r(tmp, "x", &saveptr);
			grid_w = atoi(token);
			token = strtok_r(NULL, "x", &saveptr);
			grid_h = atoi(token);
			free(tmp);
		}

    	if (s_net_capture != NULL) {
    		gettimeofday(&(s_net_capture->last_frame_time), NULL);
    		s_net_capture->width = grid_w;
			s_net_capture->height = grid_h;
			s_net_capture->video_size = grid_w * grid_h * 2;
			s_net_capture->net_video_frame = get_net_video_frame();
//			s_net_capture->net_video_frame = (char*) malloc(s_net_capture->video_size);
//			s_net_capture->net_audio_frame = net_audio_frame;
//			s_net_capture->net_audio_frame = (char*) malloc(MAX_AUDIO_SIZE);
    	}

    	return s_net_capture;
}

void vidcap_net_finish(void *state)
{
//        assert(state == &s_net_capture);
        UNUSED(state);
}

void vidcap_net_done(void *state)
{
	//        assert(state == &s_net_capture);
	        UNUSED(state);
}

struct video_frame *vidcap_net_grab(void *state, struct audio_frame **audio)
{
	//printf("		vidcap_net_grab\n");
		s_net_capture = (struct net_state_capture *) state;
        *audio = NULL;

    	struct video_frame	*vf;
    	struct tile	*vf_tile;



    	if(net_frame_decoded){
    		s_net_capture->net_video_frame = get_net_video_frame();
    		net_frame_decoded = false;
    		/*vf = (struct video_frame *) malloc(sizeof(struct video_frame));
    		vf_tile = (struct tile *) malloc(sizeof(struct tile));
    		vf->tiles = vf_tile;
    		if (vf != NULL) {
    			vf->tile_count = 1;
    			vf->tiles->width = 1920;//s_net_capture->width;
    			vf->tiles->height = 1080;//s_net_capture->height;
    			vf->tiles->data = s_net_capture->net_video_frame;

    			int i;
    				for(i=1920*2*0;i<1920*2*200;i++)
    					s_net_capture->net_video_frame[i] = 200;

    			vf->tiles->data_len = s_net_capture->video_size; // ToDo AUDIO
    			vf->tiles->linesize = s_net_capture->width;
    			vf->color_spec=UYVY;
    			vf->fps=29.97;
    			vf->interlacing=PROGRESSIVE;
    		}*/
    		return s_net_capture->net_video_frame;
    	}
    	return NULL;
}

struct vidcap_type *vidcap_net_probe(void)
{
        struct vidcap_type *vt;

        vt = (struct vidcap_type *)malloc(sizeof(struct vidcap_type));
        if (vt != NULL) {
                vt->id = VIDCAP_NET_ID;
                vt->name = "net";
                vt->description = "Capture network device";
        }
        return vt;
}
