#ifndef BUFFER_H
#define BUFFER_H

#include <stdio.h>
#include <pthread.h>

#define VIDEO_BUFFER_SIZE 2000

enum buffer_status{
    BUFFER_FULL,
    BUFFER_EMPTY
};
typedef struct circular_buf_t circular_buf_t;
typedef circular_buf_t* cbuf_handle_t;

struct circular_buf_t {
	char buffer[VIDEO_BUFFER_SIZE];
	size_t head;
	size_t tail;
	size_t max; //of the buffer
	int full;
    pthread_mutex_t lock;
};
void read_data_buffer(char* data, int amount, cbuf_handle_t buf);
void send_data_buffer(char* data, int dataLen, cbuf_handle_t buf);
int get_space(cbuf_handle_t buf);
cbuf_handle_t init_buffer();
int buffer_is_empty(cbuf_handle_t buf);

#endif // BUFFER_H