/*
 *  audio_processor.c
 *  Copyright (C) 2013  Fundació i2CAT, Internet i Innovació digital a Catalunya
 *
 *  This file is part of io_mngr.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Authors:  Jordi "Txor" Casas Ríos <jordi.casas@i2cat.net>,
 *            David Cassany <david.cassany@i2cat.net>,
 *            Ignacio Contreras <ignacio.contreras@i2cat.net>,
 *            Marc Palau <marc.palau@i2cat.net>
 */

//#include <stdlib.h>
//#include "video_data.h"
//#include "video_decompress.h"
//#include "video_decompress/libavcodec.h"
//#include "video_codec.h"
//#include "video_compress.h"
//#include "video_frame.h"
//#include "module.h"
#include "debug.h"
#include "audio_processor.h"

// private functions
static void *decoder_thread(void *arg);
static void *encoder_thread(void *arg);

static void *decoder_thread(void* arg) {

    audio_processor_t *ap = (audio_processor_t *) arg;

    audio_frame2 *frame;

    while(ap->decoder->run){
        //        usleep(100);

        // Get normalized audio_frame2 size
//        if (frame = (audio_frame2 *)cq_get_front(ap->coded_cq) == NULL) continue ;

        // Decompress audio_frame2

        // Resample audio_frame2

//        decoded_frame = curr_in_frame(ap->decoded_frames);
//        remove_frame(ap->coded_frames);
//        put_frame(ap->decoded_frames);
    }

    pthread_exit((void *)NULL);    
}

static void *encoder_thread(void *arg) {

    audio_processor_t *ap = (audio_processor_t *)arg;

//    ap->encoder->run = TRUE; 

    while (ap->run) {

        //TODO: Encoder code
    }

    pthread_exit((void *)NULL);
}

audio_processor_t *ap_init(role_t role) {

    if (audio_processor_t *ap = malloc(sizeof(audio_processor_t) == NULL)) {
        error_msg("ap_init: malloc: out of memory!");
        return NULL;
    }
    ap->role = role;
    // TODO: configure the correct audio data unit on the bags callbacks: init and destroy.
    ap->decoded = cq_init(10, init, destroy);
    ap->coded = cq_init(10, init, destroy);
    //  TODO: More fields
    //  

    data->decoder = NULL; // As decoder and encoder share its space inside a union, both becomes initialized.

    return ap;
}

void ap_destroy(audio_processor_t *ap) {

    if (ap->type == DECODER && ap->decoder != NULL){
        stop_decoder(ap);
    } else if (ap->type == ENCODER && ap->encoder != NULL){
        ap_encoder_stop(ap);
    }
    cq_destroy(ap->decoded_cq);
    cq_destroy(ap->coded_cq);
    free(ap);
}

decoder_thread_t *ap_decoder_init(audio_processor_t *ap) {

    decoder_thread_t *decoder;
    if (decoder = malloc(sizeof(decoder_thread_t) == NULL)) {
        error_msg("ap_decoder_init: malloc: out of memory!");
        return NULL;
    }
    decoder->run = FALSE;

    // Audio decoder thread initialization
    // TODO
    if (1 /* config sucsessful */) {
    } else {
        error_msg("decompress not available");
        free(decoder);
        return NULL;
    }

    return decoder;
}

void ap_decoder_destroy(decoder_thread_t *decoder) {

    if (decoder != NULL) {
        // TODO: End operations
        free(decoder);
    }
}

void ap_decoder_start(audio_processor_t *ap) {

    assert(ap->decoder == NULL);

    ap->decoder = ap_decoder_init(ap);
    ap->decoder->run = TRUE;

    if (pthread_create(&ap->decoder->thread, NULL, (void *) decoder_th, ap) != 0)
        ap->decoder->run = FALSE;
}

void ap_decoder_stop(audio_processor_t *ap) {

    if (ap->decoder != NULL) {
        ap->decoder->run = FALSE;
        pthread_join(ap->decoder->thread, NULL);
        ap_decoder_destroy(ap->decoder);
    }
}

encoder_thread_t *ap_encoder_init(audio_processor_t *ap) {

    if (ap->encoder != NULL) {
        debug_msg("ap_encoder_init: encoder already initialized");
        return NULL;
    }
    if (ap->encoder = malloc(sizeof(encoder_thread_t)) == NULL) {
        error_msg("ap_encoder_init: malloc: out of memory!");
        return NULL;
    }
    ap->encoder->run = FALSE;
    if (pthread_create(&ap->encoder->thread, NULL, encoder_routine, ap) < 0) {
        error_msg("ap_encoder_init: pthread_create error");
        free(encoder);
        return NULL;
    }

    return ap->encoder;
}

void ap_encoder_destroy(audio_processor_t *ap)
{
    pthread_join(ap->encoder->thread, NULL);
    free(ap->encoder);
}

void ap_encoder_stop(audio_processor_t *ap) {
    ap->encoder->run = FALSE;
    ap_encoder_destroy(ap);
}

