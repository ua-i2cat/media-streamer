typedef struct decoder {
    pthread_t thread;
    uint8_t run;

    pthread_mutex_t lock;
    pthread_cond_t notify_frame;
    uint8_t new_frame;

    uint8_t *data;
    uint32_t data_len;
    struct state_decompress *sd;
} decoder_t;

typedef struct encoder {
    pthread_t thread;
    uint8_t run;

    pthread_mutex_t lock;
    
    /* TODO:
       thread management:
       - like the encoder?
    */

    uint8_t *data;
    uint32_t data_len;
    struct state_compress *sc;
} encoder_t;

typedef enum stream_type {
    AUDIO;
    VIDEO;
} stream_type_t;

typedef struct audio_stream {
    // TODO
} audio_stream_t;


typedef struct video_stream {
    codec_t codec;
    uint32_t width;
    uint32_t height;
    uint8_t *frame;  // TODO: char *?
    uint32_t frame_len;
} video_stream_t;

typedef struct stream {
    stream_type_t type;
    uint8_t active;
    union {
        audio_stream_t *audio;
        video_stream_t *video;
    };
    union {
        encoder_t *encoder;
        decoder_t *decoder;
    };
} stream_t;

typedef enum transport_protocol {
    RTP;
    RTSP;
} transport_protocol_t;

typedef enum participant_type {
    INPUT;
    OUTPUT;
} participant_type_t;

typedef enum rtp_session {
    uint32_t port;
    char *addr;
} rtp_session_t;

typedef enum rtsp_session {
    uint32_t port;
    char *addr;
    // TODO: RTSP fields
} rtsp_session_t;

typedef struct participant_struct {
    participant_type_t type;
    transport_protocol_t protocol;
    uint32_t ssrc;
    union {
        rtp_session_t *rtp;
        rtsp_session_t *rtsp;
    };
    stream_t **streams;
    participant_t *prev;
    participant_t *next;
} participant_t;

typedef struct participant_list {
    int count;
    participant_t *first;
    participant_t *last;
} participant_list_t;
