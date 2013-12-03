/*
 *
 * Test program to retransmit audio using only RTP and u-law compression.
 * It can resample the stream.
 * Based on UltraGrid code.
 *
 *
 * By Txor <jordi.casas@i2cat.net>
 *
 */

// General includes
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <getopt.h>

// Includes for librtp
#include "rtp/rtp.h"
#include "rtp/pbuf.h"
#include "rtp/rtp_callback.h"
#include "rtp/audio_decoders.h"
#include "rtp/audio_frame2.h"
#include "transmit.h"
#include "module.h"
#include "perf.h"
#include "tv.h"
#include "pdb.h"

// Includes for libadecompress
#include "audio.h"
#include "codec.h"

// Includes for resamplming
#include "audio/resampler.h"
#include "resized_resample.h"

#define DEFAULT_AUDIO_FEC       "mult:3"

// For debug pourpouses
#define RECEIVER_ENABLE 1       // Receive code.
#define SENDER_ENABLE   1       // Send code.
#define FRAME_SIZE      42      // Macro for resize_resample
//#define WRITE_TO_DISK
#ifdef WRITE_TO_DISK
#include "audio_frame2_to_disk.h"
#include "audio_resample.h"
#endif //WRITE_TO_DISK

/**************************************
 *     Variables 
 */

static volatile bool stop = false;
static audio_frame2 *shared_frame = NULL;
static volatile bool consumed = true;
static struct audio_desc audio_configuration;
FILE *transmit_file;

// Like state_audio (audio/audio.c:105)
// Used to communicate data to threads.
struct thread_data {
    // RTP related stuff
    struct rtp *rtp_session;
    struct pdb *participants;
    struct timeval start_time;

    // Codec stuff
    struct audio_codec_state * audio_coder;

    // Resampling stuff
    int resample_to;
    int resample_frame_size;

    // Sender stuff
    struct tx *tx_session;

    // Thread related stuff
    pthread_t id;
    pthread_mutex_t *wait;            // Lock for wait.
    pthread_mutex_t *go;              // Lock to wake up our bro.
};


/**************************************
 *     Functions 
 */

// Prototypes
extern int audio_pbuf_decode(struct pbuf *playout_buf, struct timeval curr_time, decode_frame_t decode_func, void *data);
extern void list_audio_codecs(void);
extern struct audio_codec_state *audio_codec_init(audio_codec_t audio_codec, audio_codec_direction_t);
extern audio_codec_t get_audio_codec_to_name(const char *name);
extern const char *get_name_to_audio_codec(audio_codec_t codec);

static void usage(void)
{
    printf("Usage: audio_resample [-r <receive_port>] [-h <sendto_host>] [-s <sendto_port>] [-c <channels>] [-p <sample_rate>] [-t <resample_to>] [-f <resample_frame_size>]\n");
    printf(" By Txor >:D\n");
}

// Crtl-C handler
static void signal_handler(int signal)
{
    if (signal) { // Avoid annoying warnings.
        stop = true;
    }
    return;
}

// Function to prepare an RTP session.
// Based on initialize_audio_network (audio/audio.c:436) and initialize_network (main.c:362)
static struct rtp *init_network(char *addr, int recv_port,
        int send_port, struct pdb *participants, bool use_ipv6)
{
    struct rtp *r;
    double rtcp_bw = 1024 * 512;
    char *mcast_if = NULL;                // We aren't going to use multicast.

    r = rtp_init_if(addr, mcast_if, recv_port, send_port, 255, rtcp_bw,
            false, rtp_recv_callback, (void *)participants,
            use_ipv6);
    if (r != NULL) {
        pdb_add(participants, rtp_my_ssrc(r));
        rtp_set_option(r, RTP_OPT_WEAK_VALIDATION, TRUE);
        rtp_set_sdes(r, rtp_my_ssrc(r), RTCP_SDES_TOOL,
                PACKAGE_STRING, strlen(PACKAGE_VERSION));
    }

    return r;
}

// Function for receiver thread.
// Based on audio_receiver_thread (audio/audio.c:456)
// Basically explained:
//   It listens to UDP port and when it receivers a packet (cp != NULL)
//   Iterate by the participants,
//     trying to decode an audio frame from stored RTP packets. 
//     If it decodes an RTP packet,
//       decompress it using u-law codec
//       and flag up the mark (consumed).
//   Once it's done, update the shared audio_frame2 poniter where the frame is.
static void *receiver_thread(void *arg)
{
    struct thread_data *d = arg;
    struct timeval timeout, curr_time;
    uint32_t ts;
    struct pdb_e *cp;
    audio_frame2 *decompressed_frame;

    // audio_decoder will be used for decoding RTP incoming data,
    // containing an audio_frame2, a resampler config
    // and the static audio configuration.
    struct state_audio_decoder audio_decoder;
    audio_decoder.frame = rtp_audio_frame2_init();
    audio_decoder.resampler = resampler_init(d->resample_to);
    audio_decoder.desc = &audio_configuration;

    printf(" Receiver started.\n");
    while (!stop) {
#if RECEIVER_ENABLE
        // Preparate timeouts and perform RTP session maintenance.
        gettimeofday(&curr_time, NULL);
        ts = tv_diff(curr_time, d->start_time) * 90000;
        rtp_update(d->rtp_session, curr_time);
        rtp_send_ctrl(d->rtp_session, ts, 0, curr_time);
        timeout.tv_sec = 0;
        timeout.tv_usec = 999999 / 59.94;

        // Get packets from network.
        rtp_recv_r(d->rtp_session, &timeout, ts);

        // Iterate throught participants
        pdb_iter_t it;
        cp = pdb_iter_init(d->participants, &it);

        while (cp != NULL) {
            // Get the data on pbuf and decode it on the frame using the callback.
            if (rtp_audio_pbuf_decode(cp->playout_buffer, curr_time, decode_audio_frame_mulaw, (void *) &audio_decoder)) {
                // If decoded, mark it ready to consume.
                consumed = false;
            }
            pbuf_remove(cp->playout_buffer, curr_time);
            cp = pdb_iter_next(&it);
        }

        // Save on shared_frame the result of audio_codec_decompress 
        // using the audio_frame2 from pbuf_data.decoder (received_frame).
        // Then resample.
        if (!consumed) {
#ifdef WRITE_TO_DISK
            write_audio_frame2_channels("aoutput_1_mulaw", audio_decoder.frame, false);
#endif //WRITE_TO_DISK
            decompressed_frame = audio_codec_decompress(d->audio_coder, audio_decoder.frame);
#ifdef WRITE_TO_DISK
            write_audio_frame2_channels("aoutput_2_PCM", decompressed_frame, false);
#endif //WRITE_TO_DISK
            //shared_frame = resampler_resample(audio_decoder.resampler, decompressed_frame);
            shared_frame = resize_resample(audio_decoder.resampler, decompressed_frame, (void *)resampler_resample, d->resample_frame_size);
#ifdef WRITE_TO_DISK
            write_audio_frame2_channels("aoutput_3_resampled", shared_frame, false);
#endif //WRITE_TO_DISK
        }
        pdb_iter_done(&it);
#endif //RECEIVER_ENABLE

        // Wait for sender to be ready.
        pthread_mutex_unlock(d->go);
        pthread_mutex_lock(d->wait);
    }
    // Finish RTP session and exit.
    rtp_audio_frame2_free(audio_decoder.frame);
    rtp_done(d->rtp_session);
    printf(" Receiver stopped.\n");

    // Don't let the bro thread gets locked.
    pthread_mutex_unlock(d->go);
    pthread_exit((void *)NULL);
}

// Function for sender thread.
// Based on audio_sender_thread (audio/audio.c:610)
// Basically explained:
//   If there's a new audio_frame2 
//     Iteratively compress (see audio/codec.c:210)
//       and send it throught the RTP session.
static void *sender_thread(void *arg)
{
    struct thread_data *d = arg;

    printf(" Sender started.\n");
    while (!stop) {
        // Wait for receiver to be ready.
        pthread_mutex_unlock(d->go);
        pthread_mutex_lock(d->wait);
#if SENDER_ENABLE
        // Send the data away only if not consumed before.
        if (!consumed) {
            consumed = true;
            // Iteratively compress-and-send, see audio/codec.c:210.
            audio_frame2 *uncompressed = shared_frame;
            audio_frame2 *compressed = NULL;
#ifdef WRITE_TO_DISK
            char name[256];
            FILE *frame_write[8];
            transmit_file = fopen("aoutput_4_send","ab");
            for (int ch = 0; ch < shared_frame->ch_count ; ch++) {
                sprintf(name, "%s_%i_%i_%i_chan%i.raw", "aoutput_5_compressed", shared_frame->sample_rate, shared_frame->ch_count, 1, ch);
                frame_write[ch] = fopen(name,"ab");
            }
#endif //WRITE_TO_DISK
            while((compressed = audio_codec_compress(d->audio_coder, uncompressed))) {
                audio_tx_send_mulaw(d->tx_session, d->rtp_session, compressed);
                uncompressed = NULL;
#ifdef WRITE_TO_DISK
                for (int ch = 0; ch < compressed->ch_count ; ch++) {
                    fwrite(compressed->data[ch], compressed->data_len[ch], 1, frame_write[ch]);
                }
#endif //WRITE_TO_DISK
            }
#ifdef WRITE_TO_DISK
            fclose(transmit_file);
            for (int ch = 0; ch < shared_frame->ch_count ; ch++) {
                fclose(frame_write[ch]);
            }
#endif //WRITE_TO_DISK
        }
#endif //SENDER_ENABLE
    }
    // Finish RTP session and exit.
    rtp_done(d->rtp_session);
    printf(" Sender stopped.\n");
    // Don't let the bro thread gets locked.
    pthread_mutex_unlock(d->go);
    pthread_exit((void *)NULL);
}


/**************************************
 *     Main program
 */

int main(int argc, char *argv[])
{
    // Attach the signal handler
    signal(SIGINT, signal_handler);

    // Default network options
    char *sendto_host = "localhost";
    uint16_t receive_port = PORT_AUDIO;
    uint16_t sendto_port = PORT_AUDIO + 1000;

    // Audio configuration
    int channels = 1;
    int sample_rate = 8000;
    int bps = 1;
    int resample_to = 8000;
    int resample_frame_size = FRAME_SIZE;

    // u-law codec options
    audio_codec_t audio_codec = AC_MULAW;

    // Option processing
    static struct option getopt_options[] = {
        {"sendto_host", required_argument, 0, 'h'},
        {"receive_port", required_argument, 0, 'r'},
        {"sendto_port", required_argument, 0, 's'},
        {"channels", required_argument, 0, 'c'},
        {"sample_rate", required_argument, 0, 'p'},
        {"resample_to", required_argument, 0, 't'},
        {"resample_frame_size", required_argument, 0, 'f'},
        {0, 0, 0, 0}
    };
    int ch;
    int option_index = 0;

    while ((ch = getopt_long(argc, argv, "h:s:r:c:p:t:f:", getopt_options, &option_index)) != -1) {
        switch (ch) {
            case 'h':
                sendto_host = optarg;
                break;
            case 's':
                sendto_port = atoi(optarg);
                break;
            case 'r':
                receive_port = atoi(optarg);
                break;
            case 'c':
                channels = atoi(optarg);
                break;
            case 'p':
                // TODO: Check valid values.
                sample_rate = atoi(optarg);
                break;
            case 't':
                // TODO: Check valid values.
                resample_to = atoi(optarg);
                break;
            case 'f':
                // TODO: Check valid values.
                resample_frame_size = atoi(optarg);
                break;
            case '?':
                usage();
                exit(0);
                break; 
            default:
                break; 
        }
    }

    printf("Audio proxy over RTP and u-law compression with resampling test!\n");
    printf("Host to send: %s\n", sendto_host);
    printf("Port to send: %i\n", sendto_port);
    printf("Port to receive: %i\n", receive_port);
    printf("Audio configuration:\n");
    printf("  Channels: %i\n", channels);
    printf("  Sample rate: %i\n", sample_rate);
    printf("  bps: %i\n", bps);
    printf("  Codec: %s\n", get_name_to_audio_codec(audio_codec));
    printf("Resampling to: %i\n", resample_to);
    printf("Resample frame size: %i\n", resample_frame_size);

    // We will create two threads with one different RTP session each, similar than audio_cfg_init (audio/audio.c:180) does.

    // Global audio configuration
    audio_configuration.bps = bps;
    audio_configuration.sample_rate = sample_rate;
    audio_configuration.ch_count = channels;
    audio_configuration.codec = audio_codec;

    // Synchronization stuff
    pthread_mutex_t receiver_mutex;
    pthread_mutex_init(&receiver_mutex, NULL);
    pthread_mutex_t sender_mutex;
    pthread_mutex_init(&sender_mutex, NULL);

    // Receiver RTP session stuff
    struct rtp *receiver_session;
    struct pdb *receiver_participants;
    receiver_participants = pdb_init();
    receiver_session = init_network(NULL, receive_port, 0, receiver_participants, false);
    // Receiver thread config stuff
    struct thread_data *receiver_data = calloc(1, sizeof(struct thread_data));
    receiver_data->rtp_session = receiver_session;
    receiver_data->participants = receiver_participants;
    receiver_data->wait = &receiver_mutex;
    receiver_data->go = &sender_mutex;
    // tx_session not used.
    receiver_data->tx_session = NULL;
    // Receiver codec configuration
    receiver_data->audio_coder = audio_codec_init(audio_codec, AUDIO_DECODER);
    // Resampling configuration
    receiver_data->resample_to = resample_to;
    receiver_data->resample_frame_size = resample_frame_size;

    // Sender RTP session stuff
    struct rtp *sender_session;
    struct pdb *sender_participants;
    sender_participants = pdb_init();
    sender_session = init_network(sendto_host, 0, sendto_port, sender_participants, false);
    // Sender thread creation stuff
    struct thread_data *sender_data = calloc(1, sizeof(struct thread_data));
    sender_data->rtp_session = sender_session;
    sender_data->participants = sender_participants;
    sender_data->wait = &sender_mutex;
    sender_data->go = &receiver_mutex;
    // Configure the dummiest tx_session.
    struct module mod;
    module_init_default(&mod);
    char *audio_fec = strdup(DEFAULT_AUDIO_FEC);
    sender_data->tx_session = tx_init(&mod, 1500, TX_MEDIA_AUDIO, audio_fec, NULL);
    // Sender codec configuration
    sender_data->audio_coder = audio_codec_init(audio_codec, AUDIO_CODER);
    // Resampling configuration (doen't care, sender_thread just ignores it).
    sender_data->resample_to = resample_to;
    sender_data->resample_frame_size = resample_frame_size;

    // Launch them
    gettimeofday(&receiver_data->start_time, NULL);
    if (pthread_create(&receiver_data->id, NULL, receiver_thread, (void *)receiver_data) != 0) {
        printf("Error creating receiver thread.\n");
        exit(-1); // Ugly exit...
    }
    gettimeofday(&sender_data->start_time, NULL);
    if (pthread_create(&sender_data->id, NULL, sender_thread, (void *)sender_data) != 0) {
        printf("Error creating sender thread.\n");
        exit(-1); // Ugly exit...
    }

    // Wait for them
    pthread_join(receiver_data->id, NULL);
    pthread_join(sender_data->id, NULL);
    printf("Done!\n");

}

