#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include "buffer.h"
#include <stdlib.h>

void send_data_buffer(char* data, int dataLen, cbuf_handle_t buf){
    if(dataLen + buf->head > buf->max){
        int split = buf->max - buf->head;
        send_data_buffer(data, split, buf);
        send_data_buffer(data + split, dataLen - split, buf);
    }
    else{
        pthread_mutex_lock(&(buf->lock));
        memcpy(&(buf->buffer[buf->head]), data, dataLen);
        buf->head = (buf->head + dataLen) % buf->max;
        pthread_mutex_unlock(&(buf->lock));
    }
}

void read_data_buffer(char* data, int amount, cbuf_handle_t buf){
    if(buf->tail + amount > buf->max){
        int split = buf->max - buf->tail;
        read_data_buffer(data, split, buf);
        read_data_buffer(data + split, amount - split, buf);
    }
    else{
        memcpy(data, &(buf->buffer[buf->tail]), amount);
        buf->tail = (buf->tail + amount) % buf->max;
    }
}

int get_space(cbuf_handle_t buf){
    if(buf->head > buf->tail){
        return buf->max - ((buf->head + buf->max) - buf->tail);
    }
    else{
        return buf->max - (buf->head - buf->tail);
    }
}

void free_buffer(cbuf_handle_t buf){

}

int buffer_is_empty(cbuf_handle_t buf){
    return buf->head == buf->tail;
}

cbuf_handle_t init_buffer(){
    cbuf_handle_t cbuf = malloc(sizeof(circular_buf_t));
    cbuf->head = 0;
    cbuf->tail = 0;
    cbuf->full = 0;
    cbuf->max = VIDEO_BUFFER_SIZE;
    if (pthread_mutex_init(&(cbuf->lock), NULL) != 0)
    {
        printf("\n mutex init failed\n");
    }
    return cbuf;
}

