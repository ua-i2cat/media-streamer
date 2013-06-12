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

#include "config.h"
#include "config_unix.h"
#include "config_win32.h"
#include "debug.h"
#include "video.h"
#include "video_display.h"
#include "video_display/net.h"

#define MAGIC_NET	0x12acb060

extern bool net_frame_decoded;

struct net_state_display{
        uint32_t magic;
    	struct video_frame 	*net_video_frame;
        struct tile            *tile;
    	char* 	net_audio_frame;
        struct video_frame *vf2;
    	struct tile *vf_tile2;
};

struct net_state_display *s_net_display;

void *display_net_init(char *fmt, unsigned int flags)
{
	//printf("		display_net_init\n");
		s_net_display = (struct net_state_display *)calloc(1, sizeof(struct net_state_display));






//		s_net_display->vf2 = vf_alloc(1);
//
//		s_net_display->vf_tile2 = vf_get_tile(s_net_display->vf2, 0);


//		s_net_display->vf2 = (struct video_frame *) malloc(sizeof(video_frame));
//		s_net_display->vf_tile2 = (struct tile *) malloc(sizeof(struct tile));
        if (s_net_display != NULL) {
        	s_net_display->magic = MAGIC_NET;
			net_frame_decoded=0;



			s_net_display->net_video_frame = get_net_video_frame();
	       //		if (vf != NULL) {
			s_net_display->net_video_frame->tiles->width = 1920;//s_net_capture->width;
			s_net_display->net_video_frame->tiles->height = 1080;//s_net_capture->height;
	//        s->net_video_frame->tiles->data = s->net_video_frame;


			s_net_display->net_video_frame->tiles->data = malloc (1920*1080*2);










//			s_net_display->net_audio_frame = net_audio_frame;
        }

        put_net_video_frame(s_net_display->net_video_frame);

        return s_net_display;
}

void display_net_run(void *arg)
{
        UNUSED(arg);
}

void display_net_finish(void *state)
{
        UNUSED(state);
}

void display_net_done(void *state)
{
        struct net_state_display *s = (struct net_state_display *)state;
        assert(s->magic == MAGIC_NET);
        free(s);
}

struct video_frame *display_net_getf(void *state)
{
	//printf("		display_net_getf\n");
        struct net_state_display *s = (struct net_state_display *)state;
        assert(s->magic == MAGIC_NET);





//        s->net_video_frame->tiles->data = (char *) s->sdl_screen->pixels +
//                                    s->sdl_screen->pitch * s->dst_rect.y +
//                                    s->dst_rect.x *
//                                    s->sdl_screen->format->BytesPerPixel;
//
//
//        s->frame->tiles[1].data = (char*)s->state[0].delegate->pixelFrameRight;







        s->net_video_frame->tiles->data_len = 1920*1080*2; // ToDo AUDIO
       //        s_net_display->vf2->tiles->linesize = s_net_display->width;
        s->net_video_frame->color_spec=UYVY;
        s->net_video_frame->fps=29.97;
        s->net_video_frame->interlacing=PROGRESSIVE;


        /*s->net_video_frame = get_net_video_frame();
//        return s->net_video_frame;
        s_net_display->vf2->tiles = s_net_display->vf_tile2;
//		if (vf != NULL) {
        s_net_display->vf2->tile_count = 1;
        s_net_display->vf2->tiles->width = 1920;//s_net_capture->width;
        s_net_display->vf2->tiles->height = 1080;//s_net_capture->height;
        s_net_display->vf2->tiles->data = s_net_display->net_video_frame;
//        s_net_display->vf2->tiles->data_len = s_net_display->video_size; // ToDo AUDIO
//        s_net_display->vf2->tiles->linesize = s_net_display->width;
        s_net_display->vf2->color_spec=UYVY;
        s_net_display->vf2->fps=29.97;
        s_net_display->vf2->interlacing=PROGRESSIVE;
//		}
//		return vf;

    	int i;
        for(i=1920*2*400;i<1920*2*600;i++)
        	s_net_display->net_video_frame[i] = 200;

    	return (struct video_frame*) s_net_display->vf2;*/

        /*int i;
        for(i=1920*2*400;i<1920*2*600;i++)
        	s_net_display->net_video_frame[i] = 200;*/

//    	return s_net_display->net_video_frame;

        //printf("GETF tiles count: %d\n",s->net_video_frame->tile_count);

        return s->net_video_frame;
}


/*media_display_format *
display_net_getf(void *state)
{

	struct state_net *s = (struct state_net *) state;
	assert(s->magic == MAGIC_NET);
	mf2->video_data = s->net_video;
	mf2->audio_data = s->net_audio;
	return (media_display_format*) mf2;

}*/







int display_net_putf(void *state, struct video_frame *frame, int nonblock)
{
	//printf("		display_net_putf\n");
        struct net_state_display *s = (struct net_state_display *)state;

        //printf("PUTF tiles count: %d\n",frame->tile_count);

		put_net_video_frame(frame);
        assert(s->magic == MAGIC_NET);
        net_frame_decoded = true;
//    	int i;


//UNUSED(nonblock);

//    	for(i=1920*2*0;i<1920*2*1;i++)
////    		printf("*(frame->tiles->data+i) = %d\n",*(frame->tiles->data+i));
//    		printf("*(frame->fps) = %d\n",frame->fps);

    	/*for(i=1920*2*0;i<1920*2*1080;i++)
    		*(s_net_display->net_video_frame+i) = *(frame->tiles->data+i);*/

//    	s_net_display->net_video_frame = frame->tiles->data;


//        				for(i=1920*2*200;i<1920*2*400;i++)
//        					s_net_display->net_video_frame[i] = 125;
        return 0;
}

display_type_t *display_net_probe(void)
{
        display_type_t *dt;

        dt = malloc(sizeof(display_type_t));
        if (dt != NULL) {
                dt->id = DISPLAY_NET_ID;
                dt->name = "net";
                dt->description = "Display network device";
        }
        return dt;
}

int display_net_get_property(void *state, int property, void *val, size_t *len)
{
	//printf("		display_net_get_property\n");
    struct net_state_display *s = (struct net_state_display *) state;

    codec_t codecs[] = {UYVY, RGBA, RGB};

    switch (property) {
            case DISPLAY_PROPERTY_CODECS:
                    if(sizeof(codecs) <= *len) {
                            memcpy(val, codecs, sizeof(codecs));
                    } else {
                            return FALSE;
                    }

                    *len = sizeof(codecs);
                    break;

            default:
                    return FALSE;
    }
    return TRUE;
}

int display_net_reconfigure(void *state, struct video_desc desc)
{
        UNUSED(desc);
        struct net_state_display *s = (struct net_state_display *)state;
        assert(s->magic == MAGIC_NET);

        return TRUE;
}

void display_net_put_audio_frame(void *state, struct audio_frame *frame)
{
        UNUSED(state);
        UNUSED(frame);
}

int display_net_reconfigure_audio(void *state, int quant_samples, int channels,
                int sample_rate)
{
        UNUSED(state);
        UNUSED(quant_samples);
        UNUSED(channels);
        UNUSED(sample_rate);

        return FALSE;
}

