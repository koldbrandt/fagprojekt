#ifndef SERVER_H
#define SERVER_H

#include "buffer.h"
#include "connection.h"

#define VIDEO_BUFFER_SIZE 20000000

enum server_options{
    NO_FIFO = 1,
    SERVER_RUN_IPERF = 2,
    SERVER_TCP = 4
};

void wait_init(struct sockaddr_in* addr);
void* fifo_write_thread(void* buffer);
void send_packet_buffer(char* data, unsigned short dataLen, cbuf_handle_t video_buffer);
void close_server();

void recv_video(cbuf_handle_t buf);
void recv_video_tcp(cbuf_handle_t video_buffer);

void run_server(int serverPort, int options);
void run_server_tcp(cbuf_handle_t video_buffer, int options);

void run_server_iperf(cbuf_handle_t video_buffer, int options);
void run_server_iperf_tcp(cbuf_handle_t video_buffer, int options);

void recv_video_iperf(cbuf_handle_t buf);
void recv_video_iperf_tcp(cbuf_handle_t video_buffer);

#endif // SERVER_H