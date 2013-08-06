/**
 * @file participants.h
 * @brief Participants data structures and functions.
 *
 */

#ifndef _PARTICIPANTS_H_
#define _PARTICIPANTS_H_

#include "../src/video.h"
#include <semaphore.h>

typedef struct participant_data participant_data_t;

typedef enum {INPUT, OUTPUT} ptype_t;

struct participant_data {
	pthread_mutex_t    	lock;
	uint8_t 	   	new_frame;
	uint32_t 		ssrc;
	uint32_t		id;
	char			*frame;
	int 			frame_length;
	participant_data_t	*next;
	participant_data_t	*previous;
	uint32_t		width;
	uint32_t		height;
	codec_t			codec;
	ptype_t			type;
	struct rtp_session	*session;
	union {
	  struct decoder_thread	*decoder;
	  struct encoder_thread	*encoder;
	} proc;
};

typedef struct decoder_thread {
	pthread_t		th_id;
	uint8_t			run;
	pthread_mutex_t		lock;
	pthread_cond_t		notify_frame;
	uint8_t			new_frame;
	char			*data;
	uint32_t		data_len;
	struct state_decompress *sd;
} decoder_thread_t;

/**
 * @brief Wraps the data used by rtp transmitter threads
 *
 */
typedef struct rtpenc_thread {
    pthread_t thread;
} rtpenc_thread_t;

/**
 * @brief Wraps the data used by encoder threads (transmission)
 *
 */
typedef struct encoder_thread {
    pthread_t   thread;
    uint8_t     run;
    rtpenc_thread_t *rtpenc;
    struct compress_state *sc;
    int index;
    struct video_frame *frame;
    char    *input_frame;
    int     input_frame_length;
    sem_t input_sem;
    sem_t output_sem;
} encoder_thread_t;

typedef struct participant_list {
	pthread_rwlock_t   	lock;
	int 			count;
	participant_data_t	*first;
	participant_data_t	*last;
} participant_list_t;

typedef struct rtp_session {
	uint32_t 		port;
	char			*addr;
} rtp_session_t;


participant_list_t *init_participant_list(void);

decoder_thread_t *init_decoder_thread(participant_data_t *src);

// Returns true or false
int add_participant(participant_list_t *list, int id, int width, int height, codec_t codec, char *dst, uint32_t port, ptype_t type);

participant_data_t *get_participant_id(participant_list_t *list, uint32_t id);

participant_data_t *get_participant_ssrc(participant_list_t *list, uint32_t ssrc);

int remove_participant(participant_list_t *list, uint32_t id);

void destroy_participant_list(participant_list_t *list);

void destroy_participant(participant_data_t *src);

void destroy_decoder_thread(decoder_thread_t *dec_th);

participant_data_t *init_participant(int id, int width, int height, codec_t codec, char *dst, uint32_t port, ptype_t type);

#endif
