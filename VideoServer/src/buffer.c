#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "buffer.h"

// This is the code for the circular buffer used in the server to store the received data before it is sent to the fifo
// The "head" points to the next spot where data will be put in and "tail" points to the next spot where data will be read from

// put data into the buffer
void send_data_buffer(char* data, int dataLen, cbuf_handle_t buf){
    if(dataLen + buf->head > buf->max){
        // if the length of the data plus the head of the buffer we put in exceeds the length of the array, split the data into 2 parts
        // the first part fills the array to the end
        // the second part is put in at the start of the array
        int split = buf->max - buf->head;
        send_data_buffer(data, split, buf);
        send_data_buffer(data + split, dataLen - split, buf);
    }
    else{
        // copy the data to the array and increment the pointer to the next empty spot in the array
        memcpy(&(buf->buffer[buf->head]), data, dataLen);
        buf->head = (buf->head + dataLen) % buf->max;
        buf->full = buf->head == buf->tail;
    }
}

// read and remove data from the buffer
// this function works much like the send_data_buffer, except it reads instead of writes
void read_data_buffer(char* data, cbuf_handle_t buf){
    *data = (buf->buffer[buf->tail]);
    buf->tail = (buf->tail + 1) % buf->max;
    buf->full = 0;
}

/* old read_data_buffer where you can read multiple bytes at a time
   this is not needed is our case at the moment
void read_data_buffer(char* data, int amount, cbuf_handle_t buf){
    if(buf->tail + amount > buf->max){
        int split = buf->max - buf->tail;
        read_data_buffer(data, split, buf);
        read_data_buffer(data + split, amount - split, buf);
    }
    else{
        memcpy(data, &(buf->buffer[buf->tail]), amount);
        buf->tail = (buf->tail + amount) % buf->max;
        buf->full = 0;
    }
}
*/

// get the remaining empty space left in the buffer
int get_space(cbuf_handle_t buf){
    if(buf->full){
        return 0;
    }
    else{ // if the head has wrapped around at the end of the array, the calculation for the remaining space is different
        if(buf->head < buf->tail){
            return buf->max - ((buf->head + buf->max) - buf->tail);
        }
        else{
            return buf->max - (buf->head - buf->tail);
        }
    }
}

// get the amount of used space in the buffer
// this is the opposite of the get_space function
int fill_level(cbuf_handle_t buf){
    return buf->max - get_space(buf);
}

// free the memory allocated by the buffer
// used when destroying the buffer
void free_buffer(cbuf_handle_t buf){
    free(buf->buffer);
    free(buf);
}

// check if the buffer is empty
int buffer_is_empty(cbuf_handle_t buf){
    return get_space(buf) == buf->max;
}

// creates a buffer
// the cbuf_handle_t is a pointer to the buffer
cbuf_handle_t init_buffer(size_t size){
    cbuf_handle_t cbuf = malloc(sizeof(circular_buf_t));
    char* buf = malloc(size * sizeof(char));
    cbuf->buffer = buf;
    cbuf->head = 0;
    cbuf->tail = 0;
    cbuf->full = 0;
    cbuf->max = size;
    // also create a lock for multithreading
    // this lock is not used at the moment
    if (pthread_mutex_init(&(cbuf->lock), NULL) != 0)
    {
        printf("\n mutex init failed\n");
    }
    return cbuf;
}

// print the content of a buffer
// usefull for debugging
void print_buffer(cbuf_handle_t buf){
    printf("buffer: ");
    for(int i = 0; i < buf->max; i++){
        printf("%d ", buf->buffer[i]);
    }
    printf("\n");
}

