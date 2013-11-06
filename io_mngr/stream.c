#include "stream.h"
#include "debug.h"

#define DEFAULT_FPS 24
#define PIXEL_FORMAT RGB

stream_list_t *init_stream_list(void)
{
    stream_list_t *list = malloc(sizeof(stream_list_t));
    if (list == NULL) {
        error_msg("init_stream_list malloc error");
        return NULL;
    }
    pthread_rwlock_init(&list->lock, NULL);
    list->count = 0;
    list->first = NULL;
    list->last = NULL;
    return list;
}

void destroy_stream_list(stream_list_t *list)
{
    pthread_rwlock_wrlock(&list->lock);
    stream_data_t *current = list->first;
    while (current != NULL) {
        stream_data_t *next = current->next;
        destroy_stream(current);
        current = next;
    }
    pthread_rwlock_unlock(&list->lock);
    pthread_rwlock_destroy(&list->lock);
    free(list);
}

stream_data_t *init_stream(stream_type_t type, io_type_t io_type, uint32_t id, stream_state_t state, char *stream_name)
{
    stream_data_t *stream = malloc(sizeof(stream_data_t));
    if (stream == NULL) {
        error_msg("init_video_stream malloc error");
        return NULL;
    }

    if (pthread_rwlock_init(&stream->lock, NULL) < 0) {
        error_msg("init_stream: pthread_rwlock_init error");
        free(stream);
        return NULL;
    }
    stream->id = id;
    stream->type = type;
    stream->io_type = io_type;
    stream->state = state;
	stream->stream_name = strdup(stream_name);
    stream->prev = NULL;
    stream->next = NULL;

    if (io_type == INPUT){
        stream->video = init_video_data(DECODER);
    } else if (io_type == OUTPUT){
        stream->video = init_video_data(ENCODER);
    }

    return stream;
}

int destroy_stream(stream_data_t *stream)
{
    pthread_rwlock_wrlock(&stream->lock);

    if (stream->type == VIDEO){
        destroy_video_data(stream->video);
    } else if (stream->type == AUDIO){
        //Free audio structures
    }

    pthread_rwlock_destroy(&stream->lock);
	free(stream->stream_name);
    free(stream);
    return TRUE;
}

int add_stream(stream_list_t *list, stream_data_t *stream)
{
    pthread_rwlock_wrlock(&list->lock);
    pthread_rwlock_wrlock(&stream->lock);
    int ret = TRUE;

    if (list->count == 0) {
        assert(list->first == NULL && list->last == NULL);
        list->count++;
        list->first = list->last = stream;
    } else if (list->count > 0) {
        assert(list->first != NULL && list->last != NULL);
        stream->next = NULL;
        stream->prev = list->last;
        list->last->next = stream;
        list->last = stream;
        list->count++;
    } else {
        error_msg("add_stream list->count < 0");
        ret = FALSE;
    }
    pthread_rwlock_unlock(&stream->lock);
    pthread_rwlock_unlock(&list->lock);

    return ret;
}

stream_data_t *get_stream_id(stream_list_t *list, uint32_t id)
{
    pthread_rwlock_rdlock(&list->lock);

    stream_data_t *stream = list->first;
    while (stream != NULL) {
        if (stream->id == id) {
            break;
        }
        stream = stream->next;
    }
    pthread_rwlock_unlock(&list->lock);
    return stream;
}

int remove_stream(stream_list_t *list, uint32_t id)
{
    pthread_rwlock_wrlock(&list->lock);
    if (list->count == 0) {
        pthread_rwlock_unlock(&list->lock);
        return FALSE;
    }
    pthread_rwlock_unlock(&list->lock);

    stream_data_t *stream = get_stream_id(list, id);

    pthread_rwlock_wrlock(&list->lock);
    if (stream == NULL) {
        pthread_rwlock_unlock(&list->lock);
        return FALSE;
    }

    pthread_rwlock_wrlock(&stream->lock);
    if (stream->prev == NULL) {
        assert(list->first == stream);
        list->first = stream->next;
    } else {
        stream->prev->next = stream->next;
    }

    if (stream->next == NULL) {
        assert(list->last == stream);
        list->last = stream->prev;
    } else {
        stream->next->prev = stream->prev;
    }

    list->count--;
    pthread_rwlock_unlock(&list->lock);
    
    pthread_rwlock_unlock(&stream->lock);
    destroy_stream(stream);
    
    return TRUE;
}

void set_stream_state(stream_data_t *stream, stream_state_t state) {
    
    pthread_rwlock_wrlock(&stream->lock); 

    if (state == NON_ACTIVE){
        stream->state = state;
    } else if (stream->state == NON_ACTIVE) {
        stream->state = I_AWAIT;
    }
    
    pthread_rwlock_unlock(&stream->lock);
}
