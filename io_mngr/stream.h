#ifndef _STREAM_H_
#define _STREAM_H_

#include "video_data.h"
#include <pthread.h>
#include <semaphore.h>
#include "participants.h"
#include "commons.h"

typedef enum stream_type {
    AUDIO,
    VIDEO
} stream_type_t;

typedef enum stream_state{
    ACTIVE,
    I_AWAIT,
    NON_ACTIVE
} stream_state_t;

typedef struct audio_data {
    // TODO
} audio_data_t;

typedef struct stream_data {
    pthread_rwlock_t lock;
    stream_type_t type;
    io_type_t io_type;
    stream_state_t state;
	char *stream_name;
    uint32_t id;
    participant_list_t *plist;
    struct stream_data *prev;
    struct stream_data *next;
    union {
        audio_data_t *audio;
        video_data_t *video;
    };
} stream_data_t;

typedef struct stream_list {
    pthread_rwlock_t lock;
    int count;
    stream_data_t *first;
    stream_data_t *last;
} stream_list_t;

stream_list_t *init_stream_list(void);
void destroy_stream_list(stream_list_t *list);

stream_data_t *init_stream(stream_type_t type, io_type_t io_type, uint32_t id, stream_state_t state, char* stream_name);
int add_stream(stream_list_t *list, stream_data_t *stream);
int remove_stream(stream_list_t *list, uint32_t id);
int destroy_stream(stream_data_t *stream);

// TODO set_stream_audio_data
stream_data_t *get_stream_id(stream_list_t *list, uint32_t id);
void set_stream_state(stream_data_t *stream, stream_state_t state);
int add_participant_stream(stream_data_t *stream, participant_data_t *participant);

//TODO: rethink these function names
participant_data_t *get_participant_stream_id(stream_list_t *list, uint32_t id);
participant_data_t *get_participant_stream_ssrc(stream_list_t *list, uint32_t ssrc);
participant_data_t *get_participant_stream_non_init(stream_list_t *list);

#endif
