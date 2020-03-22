#ifndef CONNECTION_H
#define CONNECTION_H

#define MAX_PACKET_SIZE 1024
#define SERVER_PORT 1337
#define SERVER_IP "127.0.0.1"

int init_server_socket(int port);
int init_client_socket(struct sockaddr_in* serverAddr, char* serverIP, int port);
int recv_data(struct sockaddr_in* src, char* data);
int send_data(struct sockaddr_in* dest, char* data, int len);
int addrMatch(struct sockaddr_in* addr1, struct sockaddr_in* addr2);
int close_connection();

#endif // CONNECTION_H