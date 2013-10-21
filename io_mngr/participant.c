#include "participant.h"
#include <stdlib.h>
#include <pthread.h>

participant_list_t *init_participant_list(void)
{
    participant_list_t *list;

    list = malloc(sizeof(participant_list_t));
    if (list == NULL) {
        return NULL;
    }

    pthread_rwlock_init(&list->lock);
    list->count = 0;
    list->first = NULL;
    list->last = NULL;
    
    return list;
}

decoder_t *init_decoder(stream_t *stream)
{
    decoder_t *decoder;
    struct video_desc des;

    initialize_video_decompress();

    decoder = malloc(sizeof(decoder_thread_t));
    if (decoder == NULL) {
        error_msg("decoder malloc error");
        return NULL;
    }

    decoder->new_frame = FALSE;
    decoder->run = FALSE;
    
    decoder->sd = calloc(2, sizeof(struct state_decompress*));
    if (decoder->sd == NULL) {
        free(decoder);
        error_msg("decoder state decompress malloc error");
        return NULL;
    }

    pthread_mutex_init(&decoder->lock, NULL);
    pthread_cont_init(&decoder->notify_frame, NULL);

    if (decompress_is_available(LIBAVCODEC_MAGIC)) {
        // TODO: add some magic to determine codec
        decoder->sd = decompress_init(LIBAVCODEC_MAGIC);

        des.width = stream->video.width;
        des.height = stream->video

}

encoder_t *init_encoder(participant_t *participant)
{

}

int add_participant(participant_list_t *list, uint32_t id,
                    participant_type_t type,
                    transport_protocol_t protocol, uint32_t ssrc)
{
    participant_t *participant = init_participant(id, type, protocol, ssrc);
    if (participant == NULL) {
        return FALSE;
    }

    if (list->count == 0) {
        assert(list->first == NULL && list->last == NULL);
        list->count++;
        list->first = list->last = participantl;
    } else if (list->count > 0) {
        assert(list->first != NULL && list->last != NULL);
        participant->prev = list->last;
        participant->next = NULL
        list->last->next = participant;
        list->last = participant;
    } else {
        error_msg("participant list count < 0");
        return FALSE;
    }
    return TRUE;
}

int add_input_participant(participant_list_t *list,
                          uint32_t id, transport_protocol_t protocol,
                          uint32_t ssrc)
{
    return add_participant(list, id, INPUT, protocol, ssrc);
}

int add_output_participant(participant_list_t *list,
                           uint32_t id, transport_protocol_t protocol)
{
    return add_participant(list, id, OUTPUT, protocol, (uint32_t)NULL);
}

int remove_participant(participant_list_t *list, uint32_t id)
{

}

participant_t *get_participant_id(participant_list_t *list, uint32_t id)
{

}

participant_t *get_participant_ssrc(participant_list_t *list, uint32_t ssrc)
{
}

participant_t *init_participant(uint32_t id, participant_type_t type,
                                transport_protocol_t protocol,
                                uint32_t ssrc)
{
    
}
