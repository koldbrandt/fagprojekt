#ifndef SERVER_H
#define SERVER_H

#include "buffer.h"
#include "connection.h"

enum server_options{
    NO_FIFO = 1,
    SERVER_RUN_IPERF = 2
};

void wait_init(struct sockaddr_in* addr);
void recv_video(cbuf_handle_t buf);
void run_server(int serverPort, int options);
void* fifo_write_thread(void* buffer);
void run_server_iperf(int serverPort);
void recv_video_iperf(cbuf_handle_t buf);

#endif // SERVER_H