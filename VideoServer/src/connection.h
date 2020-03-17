#ifndef CONNECTION_H
#define CONNECTION_H

#define MAX_PACKET_SIZE 1024
#define SERVER_PORT 1337

int init_socket();
int recv_data(struct sockaddr_in* src, char* data);
int send_data(struct sockaddr_in dest, char* data, int len);

#endif // CONNECTION_H