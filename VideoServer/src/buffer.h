#ifndef BUFFER_H
#define BUFFER_H

typedef struct circular_buf_t circular_buf_t;
typedef circular_buf_t* cbuf_handle_t;

struct circular_buf_t {
	char* buffer;
	size_t head;
	size_t tail;
	size_t max; //of the buffer
	int full;
};
void read_data_buffer(char* data, cbuf_handle_t buf);
void send_data_buffer(char* data, int dataLen, cbuf_handle_t buf);
int get_space(cbuf_handle_t buf);
cbuf_handle_t init_buffer(size_t size);
void free_buffer(cbuf_handle_t buf);
int buffer_is_empty(cbuf_handle_t buf);
int fill_level(cbuf_handle_t buf);
void print_buffer(cbuf_handle_t buf);

#endif // BUFFER_H