#include "transmitter.h"
#include "rtp/rtpenc_h264.h"
#include "pdb.h"
#include "video_codec.h"
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
    //while(1); // TODO
    debug_msg(" transmitter encoder routine START\n");
    struct participant_data *participant = (struct participant_data *)arg;

    participant->encoder->sc = (struct compress_state *) calloc(2, sizeof(struct compress_state *));
    participant->encoder->input_frame_length = vc_get_linesize(participant->width, UYVY)*participant->height;
    participant->encoder->input_frame = malloc(participant->encoder->input_frame_length);

    compress_init("libavcodec:codec=H.264", &participant->encoder->sc);

    debug_msg(" transmitter encoder routine: entering loop\n");
    while (RUN) {
        sem_wait(&participant->encoder->input_sem);
        if (!RUN) {
            break;
        }
        pthread_mutex_lock(&participant->lock);
        memcpy(participant->encoder->input_frame, participant->frame, participant->encoder->input_frame_length);
        participant->encoder->input_frame_length = participant->frame_length;
        pthread_mutex_unlock(&participant->lock);

        int i = participant->encoder->index;
        struct video_frame *frame = vf_alloc(1);
        int width = participant->width;
        int height = participant->height;
        vf_get_tile(frame, 0)->width=width;
        vf_get_tile(frame, 0)->height=height;
        vf_get_tile(frame, 0)->linesize=vc_get_linesize(width, UYVY);
        frame->fps=5; // TODO!
        frame->color_spec=UYVY;
        frame->interlacing=PROGRESSIVE;
        
        frame->tiles[0].data = participant->encoder->input_frame;
        frame->tiles[0].data_len = participant->encoder->input_frame_length;

        struct video_frame *tx_frame;
        debug_msg("compressing new frame...\n");
        tx_frame = compress_frame(participant->encoder->sc, frame, i);
        debug_msg("new frame compressed!\n");
        
        participant->encoder->frame = tx_frame;

        i = (i + 1)%2;
        participant->encoder->index = i;
        
        sem_post(&participant->encoder->output_sem);    
    }

    debug_msg(" encoder routine END\n");
    int ret = 0;
    //pthread_join(participant->encoder->rtpenc->thread, NULL);
    compress_done(participant->encoder->sc);
    free(participant->encoder->sc);
    free(participant->encoder->input_frame);
    pthread_exit((void *)&ret);
}

void *transmitter_rtpenc_routine(void *arg)
{
    //while(1); // TODO
    debug_msg(" transmitter rtpenc routine START\n");
    struct participant_data *participant = (struct participant_data *)arg;
    struct rtp_session *session = participant->session;

    // TODO initialization
    struct pdb *participants = pdb_init();
    char *mcast_if = NULL;
    double rtcp_bw = 5 * 1024 * 1024; /* FIXME */
    int ttl = 255;
    int recv_port = 2004;
    void *rtp_recv_callback = NULL;

    struct rtp *rtp  = rtp_init_if(session->addr, mcast_if,
                                   recv_port, session->port, ttl,
                                   rtcp_bw, 0, rtp_recv_callback,
                                   (void *)participants, 0);

    while (RUN) {
        sem_wait(&participant->encoder->output_sem);
        if (!RUN) {
            debug_msg(" rtpenc_routine break detected after sem_wat!\n");
            break;
        }
        tx_send_base_h264(vf_get_tile(participant->encoder->frame, 0),
                          rtp, get_local_mediatime(), 1, participant->codec,
                          participant->encoder->frame->fps,
                          participant->encoder->frame->interlacing, 0, 0);
        debug_msg(" new frame sent!\n");
    }   

    debug_msg(" rtpenc routine END\n");
    int ret = 0;
    pthread_exit(NULL);
}

int transmitter_init_threads(struct participant_data *participant)
{
    debug_msg(" initializing the pair of threads for a participant\n");
    participant->encoder = malloc(sizeof(struct encoder_th));
    struct encoder_th *encoder = participant->encoder;
    if (encoder == NULL) {
        error_msg(" unsuccessful malloc\n");
        return -1;
    }
    encoder->rtpenc = malloc(sizeof(struct rtpenc_th));
    struct rtpenc_th *rtpenc = encoder->rtpenc;
    if (rtpenc == NULL) {
        error_msg(" unsuccessful malloc\n");
        return -1;
    }

    debug_msg("initializing semaphores\n");
    sem_init(&encoder->input_sem, 1, 0);
    sem_init(&encoder->output_sem, 1, 0);

    debug_msg("initializing the pair of threads\n");
    int ret = 0;
    ret = pthread_create(&encoder->thread, NULL,
                transmitter_encoder_routine, participant);
    if (ret < 0) {
        // TODO
    }
    debug_msg("pthread_create encoder OK!\n");
    ret = pthread_create(&rtpenc->thread, NULL,
                transmitter_rtpenc_routine, participant);
    if (ret < 0) {
        // TODO
    }
    debug_msg("pthread_create rtpenc OK!\n");

    debug_msg(" threads (for a participant) initialized\n");
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
        //debug_msg("master loop - A\n");
        struct participant_data *ptc = list->first;
        //debug_msg("master loop - B\n");
        while (ptc != NULL) {
            if (ptc->encoder != NULL) { // -> has a pair of threads
                if (*(ptc->new)) { // -> has new data
                    // notify!
                    pthread_mutex_lock(&ptc->lock);
                    *(ptc->new) = 0;
                    sem_post(&ptc->encoder->input_sem);
                    pthread_mutex_unlock(&ptc->lock);
                }
            }
            ptc = ptc->next;
        }
        //debug_msg("master loop - C\n");
    }

    debug_msg(" terminating pairs of threads\n");
    int ret = 0;
    void *end;
    participant = list->first;
    while (participant != NULL) {
        sem_post(&participant->encoder->input_sem);
        sem_post(&participant->encoder->output_sem);
        if (participant->encoder->rtpenc->thread == NULL) {
            printf("AAAAAAAAH!!!\n");
        }
        ret += pthread_join(participant->encoder->rtpenc->thread, &end);
        ret += pthread_join(participant->encoder->thread, &end);

        free(participant->encoder->rtpenc);
        free(participant->encoder);

        participant = participant->next;
    }
    if (ret != 0) {
        ret = -1;
    }
    pthread_exit((void *)&ret);
}

int start_out_manager(struct participant_list *list, uint32_t port)
{
    UNUSED(port);
    debug_msg("creating the master thread...\n");
    int ret = pthread_create(&MASTER_THREAD, NULL, transmitter_master_routine, list);
    if (ret < 0) {
        debug_msg("could not initiate the transmitter master thread\n");
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

    return ret;
}


int main(int argc, char **argv)
{
    int stop_at = 0;
    if (argc == 2) {
        stop_at = atoi(argv[1]);
    }

    int new_first = 0;
    int new_last = 0;

    struct rtp_session session_first = {
        .addr = "127.0.0.1",
        .port = 5004
    };

    struct rtp_session session_last = {
        .addr = "127.0.0.1",
        .port = 6004
    };

    struct participant_data first = {
        .new = &new_first,
        .ssrc = 0,
        .frame = NULL,
        .frame_length = 0,
        .next = NULL,
        .previous = NULL,
        .width = 854,
        .height = 480,
        .codec = H264,
        .session = &session_first,
        .encoder = NULL
    };

    struct participant_data last = {
        .new = &new_last,
        .ssrc = 0,
        .frame = (char *)NULL,
        .frame_length = 0,
        .next = NULL,
        .previous = NULL,
        .width = 854,
        .height = 480,
        .codec = H264,
        .session = &session_last,
        .encoder = NULL  
    };

    first.next = &last;
    last.previous = &first;

    struct participant_list list = {
        .count = 2,
        .first = &first,
        .last = &last
    };


    start_out_manager(&list, 8000);

    AVFormatContext *pformat_ctx = avformat_alloc_context();
    AVCodecContext codec_ctx;
    int video_stream = -1;
    av_register_all();

    char *yuv_path = "/home/ignacio/Development/samples/sintel.yuv";

    int width = 854;
    int height = 480;

    load_video(yuv_path, pformat_ctx, &codec_ctx, &video_stream);

    uint8_t *b1 = (uint8_t *)av_malloc(avpicture_get_size(codec_ctx.pix_fmt,
                        codec_ctx.width, codec_ctx.height)*sizeof(uint8_t));

    int counter = 0;
    while(1) {
        int ret = read_frame(pformat_ctx, video_stream, &codec_ctx, b1);

        if (stop_at > 0 && counter == stop_at) {
            break;
        }

        if (ret == 0) {
            counter++;
            debug_msg(" new frame read!\n");
            pthread_mutex_lock(&list.lock);
            struct participant_data *participant = list.first;
            while (participant != NULL) {
                pthread_mutex_lock(&participant->lock);
                participant->frame = (char *)b1;
                participant->frame_length = vc_get_linesize(width, UYVY)*height;
                *(participant->new) = 1;
                pthread_mutex_unlock(&participant->lock);
                participant = participant->next;
            }
            pthread_mutex_unlock(&list.lock);
            usleep(40000);
        } else {
            break;
        }
    }
    debug_msg(" deallocating resources and terminating threads\n");
    stop_out_manager();
    debug_msg(" done!\n");

    return 0;
}
