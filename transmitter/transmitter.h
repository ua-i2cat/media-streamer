#ifndef _MANAGER_H_
#define _MANAGER_H_

#include "rtp/rtp.h"
#include "video.h"
#include <pthread.h>
#include <semaphore.h>
#include <sys/sem.h>

typedef struct participant_data participant_data_t;

typedef enum ptype {INPUT, OUTPUT} ptype_t;

struct participant_data {
	pthread_mutex_t    	lock;
	uint8_t 	   	new;
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
	};
};

typedef struct rtp_session {
	uint32_t 		port;
	char			*addr;
} rtp_session_t;

typedef struct encoder_thread {
    pthread_t               thread;
    struct rtpenc_thread    *rtpenc;
    struct compress_state   *sc;
    int                     index;
    struct video_frame      *frame;
    char                    *input_frame;
    int                     input_frame_length;
    sem_t                   input_sem;
    sem_t                   output_sem;
} encoder_thread_t;

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

struct rtpenc_thread {
    pthread_t   thread;
};

typedef struct participant_list {
	pthread_rwlock_t   	lock;
	int 			count;
	participant_data_t	*first;
	participant_data_t	*last;
} participant_list_t;

int start_out_manager(struct participant_list *list, uint32_t port);
int stop_out_manager(void);

#endif
