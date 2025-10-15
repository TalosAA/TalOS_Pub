#include <libk/queue.h>
#include <libk/string.h>

void queue_init(queue_t* queue, void* buf_address, size_t queue_length, size_t elem_size){
    queue->address = buf_address;
    queue->head = 0;
    queue->tail = 0;
    queue->length = queue_length;
    queue->elem_size = elem_size;
    queue->elem_num = 0;
}

bool queue_insert(queue_t* queue, void* in_elem) {
    if(queue->elem_num == queue->length){
        return false;
    } else {
        size_t head_next = _QUEUE_INC_MOD_(queue->head, queue->length);
        memcpy(queue->address + (queue->head * queue->elem_size), in_elem, queue->elem_size);
        queue->head = head_next;
        queue->elem_num++;
        return true;
    }
}

bool queue_remove(queue_t* queue, void* out_elem) {
    if(queue->elem_num > 0) {
        memcpy(out_elem, queue->address + (queue->tail * queue->elem_size), queue->elem_size);
        queue->tail = _QUEUE_INC_MOD_(queue->tail, queue->length);
        queue->elem_num--;
        return true;
    } else {
        return false;
    }
}

bool queue_get_head(queue_t* queue, void* out_head){
    if(!queue_is_empty(queue)) {
        memcpy(out_head, queue->address + (queue->head * queue->elem_size), queue->elem_size);
        return true;
    }
    return false;
}

bool queue_get_tail(queue_t* queue, void* out_tail){
    if(!queue_is_empty(queue)) {
        memcpy(out_tail, queue->address + (queue->tail * queue->elem_size), queue->elem_size);
        return true;
    }
    return false;
}