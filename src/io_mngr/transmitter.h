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
int start_out_manager(participant_list_t *list);

/**
 * @brief Orders the manager to wake, compress, and send participant's frames.
 *
 * This call instructs the output manager to check for new frames (i.e. to
 * inspect each participant's <em>new_frame</em> flag) and, if there is new
 * data, to compress it and send it through RTP.
 *
 */
void notify_out_manager(void);

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

#endif
