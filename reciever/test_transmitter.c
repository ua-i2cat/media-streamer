#include "transmitter.h"
#include "video_compress.h"
#include "debug.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

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
    av_dict_set(&rawdict, "pixel_format", "rgb24", 0);

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

    start_out_manager(list);
    
    int counter = 0;
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
                if (pthread_mutex_trylock(&participant->lock) == 0) {
                    participant->frame_length = vc_get_linesize(width, RGB)*height;
                    memcpy(participant->frame, (char *)b1, participant->frame_length);
                    participant->new_frame = 1;
                    pthread_mutex_unlock(&participant->lock);
                }
                participant = participant->next;
            }
            pthread_rwlock_unlock(&list->lock);
            notify_out_manager();
            usleep(40000);
        } else {
            break;
        }
    }
    debug_msg(" deallocating resources and terminating threads\n");
    stop_out_manager();
    destroy_participant_list(list);
    av_free(pformat_ctx);
    av_free(b1);
    debug_msg(" done!\n");

    return 0;
}
