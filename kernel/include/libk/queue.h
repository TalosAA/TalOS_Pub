#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <libk/stddef.h>

typedef struct {
    void* address;
    size_t length;
    size_t head;
    size_t tail;
    size_t elem_size;
    size_t elem_num;
} queue_t;

void queue_init(queue_t* queue, void* buf_address, size_t queue_length, size_t elem_size);
bool queue_insert(queue_t* queue, void* in_elem);
bool queue_remove(queue_t* queue, void* out_elem);

static inline bool queue_is_full(queue_t* queue){
    return (queue->elem_num == queue->length);
}

static inline bool queue_is_empty(queue_t* queue){
    return (queue->elem_num == 0);
}

bool queue_get_head(queue_t* queue, void* out_head);

bool queue_get_tail(queue_t* queue, void* out_head);

#define _QUEUE_INC_MOD_(NUM, N) (((NUM + 1) >= (N)) ? 0 : (NUM + 1))

#define queue_byte_insert(queue, in_elem) \
{\
    if((queue)->elem_num != (queue)->length){ \
        ((unsigned char*)(queue)->address)[(queue)->head] = (in_elem); \
        (queue)->head = _QUEUE_INC_MOD_((queue)->head, (queue)->length); \
        (queue)->elem_num++; \
    } \
}

#define queue_byte_remove(queue, out_elem)\
{\
    if((queue)->elem_num > 0) { \
        (out_elem) = ((unsigned char*)(queue)->address)[(queue)->tail]; \
        (queue)->tail = _QUEUE_INC_MOD_((queue)->tail, (queue)->length); \
        (queue)->elem_num--; \
    } \
}



#endif