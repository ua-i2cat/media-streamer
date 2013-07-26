#include "transmitter.h"
#include "rtp/rtp.h"
#include "rtp/rtpenc_h264.h"
#include "pdb.h"
#include "video_codec.h"
#include "video_compress.h"
#include "debug.h"
#include "tv.h"
#include <stdlib.h>
#include <unistd.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

void *transmitter_encoder_routine(void *arg);
void *transmitter_rtpenc_routine(void *arg);
int transmitter_init_threads(struct participant_data *participant);
void *transmitter_master_routine(void *arg);

pthread_t MASTER_THREAD;
int RUN = 1;

void *transmitter_encoder_routine(void *arg)
{
    struct participant_data *participant = (struct participant_data *)arg;

    encoder_thread_t *encoder = participant->proc.encoder;

    encoder->input_frame_length = vc_get_linesize(participant->width, UYVY)*participant->height;
    encoder->input_frame = malloc(encoder->input_frame_length); // TODO error handling

    compress_init("libavcodec:codec=H.264", &encoder->sc);

    struct video_frame *frame = vf_alloc(1);
    int width = participant->width;
    int height = participant->height;
    vf_get_tile(frame, 0)->width=width;
    vf_get_tile(frame, 0)->height=height;
    vf_get_tile(frame, 0)->linesize=vc_get_linesize(width, UYVY);
    frame->fps=5; // TODO!
    frame->color_spec=UYVY;
    frame->interlacing=PROGRESSIVE;
    
    while (RUN) {
        sem_wait(&encoder->input_sem);
        if (!RUN) {
            break;
        }

        pthread_mutex_lock(&participant->lock);
        memcpy(encoder->input_frame, participant->frame, encoder->input_frame_length);
        encoder->input_frame_length = participant->frame_length;
        pthread_mutex_unlock(&participant->lock);

        
        frame->tiles[0].data = encoder->input_frame;
        frame->tiles[0].data_len = encoder->input_frame_length;

        struct video_frame *tx_frame;
        int i = encoder->index;
        tx_frame = compress_frame(encoder->sc, frame, i);
        i = (i + 1)%2;
        encoder->index = i;
        encoder->frame = tx_frame;
        
        sem_post(&encoder->output_sem);    
    }

    debug_msg(" encoder routine END\n");
    int ret = 0;
    compress_done(encoder->sc);
    free(encoder->input_frame);
    pthread_exit((void *)&ret);
}

void *transmitter_rtpenc_routine(void *arg)
{
    debug_msg(" transmitter rtpenc routine START\n");
    struct participant_data *participant = (struct participant_data *)arg;
    struct rtp_session *session = participant->session;

    // TODO initialization
    char *mcast_if = NULL;
    double rtcp_bw = 5 * 1024 * 1024; /* FIXME */
    int ttl = 255;
    int recv_port = 2004;

    struct rtp *rtp  = rtp_init_if(session->addr, mcast_if,
                                   recv_port, session->port, ttl,
                                   rtcp_bw, 0, (void *)NULL,
                                   (void *)NULL, 0);
    
    rtp_set_option(rtp, RTP_OPT_WEAK_VALIDATION, 1);
    rtp_set_sdes(rtp, rtp_my_ssrc(rtp), RTCP_SDES_TOOL, PACKAGE_STRING, strlen(PACKAGE_STRING));

    while (RUN) {
        encoder_thread_t *encoder = participant->proc.encoder;
        sem_wait(&encoder->output_sem);
        if (!RUN) {
            break;
        }
        tx_send_base_h264(vf_get_tile(encoder->frame, 0),
                          rtp, get_local_mediatime(), 1, participant->codec,
                          encoder->frame->fps,
                          encoder->frame->interlacing, 0, 0);
    }   

    // TODO
    //rtp_done(rtp);
    pthread_exit(NULL);
}

int transmitter_init_threads(struct participant_data *participant)
{
    participant->proc.encoder = malloc(sizeof(encoder_thread_t));
    encoder_thread_t *encoder = participant->proc.encoder;
    if (encoder == NULL) {
        error_msg(" unsuccessful malloc\n");
        return -1;
    }
    encoder->rtpenc = malloc(sizeof(rtpenc_thread_t));
    rtpenc_thread_t *rtpenc = encoder->rtpenc;
    if (rtpenc == NULL) {
        error_msg(" unsuccessful malloc\n");
        return -1;
    }

    sem_init(&encoder->input_sem, 1, 0);
    sem_init(&encoder->output_sem, 1, 0);

    int ret = 0;
    ret = pthread_create(&encoder->thread, NULL,
                transmitter_encoder_routine, participant);
    if (ret < 0) {
        // TODO
    }
    ret = pthread_create(&rtpenc->thread, NULL,
                transmitter_rtpenc_routine, participant);
    if (ret < 0) {
        // TODO
    }

    return 0;
}

void *transmitter_master_routine(void *arg)
{
    debug_msg("transmitter master routine START\n");
    struct participant_list *list = (struct participant_list *)arg;

    struct participant_data *participant = list->first;
    while (participant != NULL) {
        debug_msg("participant found, initializing its threads...\n");
        transmitter_init_threads(participant);
        debug_msg("participant threads initialized\n");
        participant = participant->next;
    }

    debug_msg("entering the master loop\n");
    while (RUN) {
        struct participant_data *ptc = list->first;
        while (ptc != NULL) {
            if (ptc->proc.encoder != NULL) { // -> has a pair of threads
                if (ptc->new) { // -> has new data
                    pthread_mutex_lock(&ptc->lock);
                    ptc->new = 0;
                    sem_post(&ptc->proc.encoder->input_sem);
                    pthread_mutex_unlock(&ptc->lock);
                }
            }
            ptc = ptc->next;
        }
    }

    debug_msg(" terminating pairs of threads\n");
    int ret = 0;
    void *end;
    participant = list->first;
    while (participant != NULL) {
        sem_post(&participant->proc.encoder->input_sem);
        sem_post(&participant->proc.encoder->output_sem);
        
        ret += pthread_join(participant->proc.encoder->rtpenc->thread, &end);
        ret += pthread_join(participant->proc.encoder->thread, &end);

        free(participant->proc.encoder->rtpenc);
        free(participant->proc.encoder);

        participant = participant->next;

        // TODO semaphore destruction
    }
    if (ret != 0) {
        ret = -1;
    }
    pthread_exit((void *)&ret);
}

int start_out_manager(participant_list_t *list, uint32_t port)
{
    UNUSED(port);
    debug_msg("creating the master thread...\n");
    int ret = pthread_create(&MASTER_THREAD, NULL, transmitter_master_routine, list);
    if (ret < 0) {
        error_msg("could not initiate the transmitter master thread\n");
    }
    return ret;
}

int stop_out_manager()
{
    RUN = 0;
    int ret = pthread_join(MASTER_THREAD, NULL);
    return ret;
}





int load_video(const char* path, AVFormatContext *pFormatCtx, AVCodecContext *pCodecCtx, int *videostream);
int read_frame(AVFormatContext *pFormatCtx, int videostream, AVCodecContext *pCodecCtx, uint8_t *buff);

int load_video(const char* path, AVFormatContext *pFormatCtx, AVCodecContext *pCodecCtx, int *videostream)
{

    AVDictionary *rawdict = NULL, *optionsDict = NULL;
    AVCodec *pCodec = NULL;
    AVCodecContext *aux_codec_ctx = NULL;

    // Define YUV input video features
    pFormatCtx->iformat = av_find_input_format("rawvideo");
    unsigned int i;

    av_dict_set(&rawdict, "video_size", "854x480", 0);
    av_dict_set(&rawdict, "pixel_format", "uyvy422", 0);

    // Open video file
    if(avformat_open_input(&pFormatCtx, path, pFormatCtx->iformat, &rawdict)!=0)
        return -1; // Couldn't open file

    av_dict_free(&rawdict);

    // Retrieve stream information
    if(avformat_find_stream_info(pFormatCtx, NULL)<0)
        return -1; // Couldn't find stream information

    // Find the first video stream
    *videostream=-1;
    for(i=0; i<pFormatCtx->nb_streams; i++){
        if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
            *videostream=i;
            break;
        }
    }
    if(*videostream==-1)
        return -1; // Didn't find a video stream

    // Get a pointer to the codec context for the video stream
    aux_codec_ctx=pFormatCtx->streams[*videostream]->codec;

    // Find the decoder for the video stream
    pCodec=avcodec_find_decoder(aux_codec_ctx->codec_id);
    if(pCodec==NULL) {
        fprintf(stderr, "Unsupported codec!\n");
        return -1; // Codec not found
    }

    // Open codec
    if(avcodec_open2(aux_codec_ctx, pCodec, &optionsDict)<0)
        return -1; // Could not open codec

    *pCodecCtx = *aux_codec_ctx;

    return 0;

}

int read_frame(AVFormatContext *pFormatCtx, int videostream, AVCodecContext *pCodecCtx, uint8_t *buff)
{
    AVPacket packet;
    AVFrame* pFrame;
    int frameFinished, ret;

    pFrame = avcodec_alloc_frame();
    ret = av_read_frame(pFormatCtx, &packet);

    if(packet.stream_index==videostream) {
        // Decode video frame
        avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
        // Did we get a video frame?
        if(frameFinished) {
            avpicture_layout((AVPicture *)pFrame, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, buff,
                    avpicture_get_size(pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height)*sizeof(uint8_t));

            // Free the packet that was allocated by av_read_frame
            av_free_packet(&packet);
        }
    }

    av_free(pFrame);

    return ret;
}


int main(int argc, char **argv)
{
    int stop_at = 0;
    char *yuv_path;
    if (argc == 2) {
        yuv_path = argv[1];
    } else if (argc == 3) {
        yuv_path = argv[1];
        stop_at = atoi(argv[2]);
    } else {
        printf("usage: %s input [max frames]\n", argv[0]);
        return -1;
    }

    participant_list_t *list = init_participant_list();
    if (list == NULL) {
        error_msg(" could not initialize a participant list\n");
    }
    int ret = 0;
    ret = add_participant(list, 0, 854, 480, H264, "127.0.0.1", 5004, OUTPUT);
    if (ret < 0) {
        error_msg(" could not add a new participant\n");
        destroy_participant_list(list);
        return -1;
    }
    ret = add_participant(list, 0, 854, 480, H264, "127.0.0.1", 6004, OUTPUT);
    if (ret < 0) {
        error_msg(" could not add a new participant\n");
        destroy_participant_list(list);
        return -1;
    }


    AVFormatContext *pformat_ctx = avformat_alloc_context();
    AVCodecContext codec_ctx;
    int video_stream = -1;
    av_register_all();

    int width = 854;
    int height = 480;

    load_video(yuv_path, pformat_ctx, &codec_ctx, &video_stream);

    uint8_t *b1 = (uint8_t *)av_malloc(avpicture_get_size(codec_ctx.pix_fmt,
                        codec_ctx.width, codec_ctx.height)*sizeof(uint8_t));

    int counter = 0;
    
    start_out_manager(list, 8000);
    
    while(1) {
        int ret = read_frame(pformat_ctx, video_stream, &codec_ctx, b1);

        if (stop_at > 0 && counter == stop_at) {
            break;
        }

        if (ret == 0) {
            counter++;
            debug_msg(" new frame read!\n");
            pthread_rwlock_wrlock(&list->lock);
            struct participant_data *participant = list->first;
            while (participant != NULL) {
                pthread_mutex_lock(&participant->lock);
                participant->frame = (char *)b1;
                participant->frame_length = vc_get_linesize(width, UYVY)*height;
                participant->new = 1;
                pthread_mutex_unlock(&participant->lock);
                participant = participant->next;
            }
            pthread_rwlock_unlock(&list->lock);
            usleep(40000);
        } else {
            break;
        }
    }
    debug_msg(" deallocating resources and terminating threads\n");
    stop_out_manager();
    av_free(pformat_ctx);
    av_free(b1);
    destroy_participant_list(list);
    debug_msg(" done!\n");

    return 0;
}
