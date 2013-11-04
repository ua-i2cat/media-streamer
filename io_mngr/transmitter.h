/**
 * @file transmitter.h
 * @brief Output manager functions.
 *
 * Allows the compression and transmition over RTP of uncompressed video
 * (RGB interleaved 4:4:4, according to <em>ffmpeg</em>: <em>rgb24</em>),
 * using the participants.h API.
 *
 */

#ifndef _MANAGER_H_
#define _MANAGER_H_

#include "participants.h"

#define DEFAULT_FPS 24
#define DEFAULT_RECV_PORT 12006 // just trying to not interfere with anything
#define DEFAULT_RTCP_BW 5 * 1024 * 1024 * 10
#define DEFAULT_TTL 255
#define DEFAULT_SEND_BUFFER_SIZE 1920 * 1080 * 4 * sizeof(char) * 10
#define PIXEL_FORMAT RGB
#define MTU 1300 // 1400

typedef struct transmitter {
    uint32_t run;
    participant_list_t *participants;
    stream_list_t *stream_list;
    float fps;
    float wait_time;
    uint32_t recv_port;
    uint32_t ttl;
    uint64_t send_buffer_size;
    uint32_t mtu;
} transmitter_t;


transmitter_t *init_transmitter(stream_list_t *list, float fps);
int start_transmitter(transmitter_t *transmitter);
int stop_transmitter(transmitter_t *transmitter);

/**
 * @brief Initializes the output manager.
 *
 * It starts the compressor and rtp threads for each of the members of
 * <em>list</em> (if any). If the list is modified at runtime, the necessary
 * threads will be destroyed or created.
 *
 * After initialization, the output manager threads will remain asleep waiting
 * until a notify_out_manager() call is done.
 *
 * @param list initial list of output participants
 *
 */
int start_out_manager(participant_list_t *list, float framerate);

/**
 * @brief Stops the out manager, freeing its allocated resources.
 *
 * Destroys any remaining thread and frees any allocated resource for the
 * remaining participants in the list originally specified through
 * start_out_manager().
 * 
 */
int stop_out_manager(void);

/**
 * @brief Frees the resources allocated by a certain encoder thread and ends
 *        the thread itself.
 *
 * @param encoder points to the encoder to be freed.
 *
 * @note exposed as an API function to allow its use from, for example,
 *       <em>remove_participant</em>.
 * @deprecated
 */
void transmitter_destroy_encoder_thread(encoder_thread_t **encoder);

int init_transmission(participant_data_t *participant, transmitter_t *transmitter);
int stop_transmission(participant_data_t *participant);

#endif
