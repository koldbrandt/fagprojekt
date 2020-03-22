#ifndef VIDEO_H
#define VIDEO_H

void wait_init(struct sockaddr_in* addr);
void recv_video();
void send_packet_type(struct sockaddr_in* client, char type);
void run_server(int serverPort);
void run_client(char* serverIP, int serverPort);
void print_help();


#endif // VIDEO_H