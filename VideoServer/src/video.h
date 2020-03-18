#ifndef VIDEO_H
#define VIDEO_H

void wait_init(struct sockaddr_in* addr);
void recv_video();
void send_packet_type(struct sockaddr_in* client, char type);
void run_server();
void run_client();


#endif // VIDEO_H