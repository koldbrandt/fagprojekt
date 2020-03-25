#ifndef VIDEO_H
#define VIDEO_H

#include "buffer.h"

void wait_init(struct sockaddr_in* addr);
void recv_video(cbuf_handle_t buf);
void send_packet_type(struct sockaddr_in* client, char type);
void run_server(int serverPort, cbuf_handle_t buf);
void run_client(char* serverIP, int serverPort);
void print_help();


#endif // VIDEO_H