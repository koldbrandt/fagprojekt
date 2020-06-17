#ifndef CONNECTION_H
#define CONNECTION_H

#define MAX_PACKET_SIZE 2000

#define SERVER_PORT 1337
#define SERVER_IP "127.0.0.1"
#define VIDEO_HEADER_LENGTH 3

#include <arpa/inet.h>

enum packet_types{
    INIT = 49,
    INIT_ACK = 50,
    TERMINATE = 51,
    VIDEO_DATA = 52,
    SEND_SLOW = 53,
    SEND_FAST = 54
};

int addrMatch(struct sockaddr_in* addr1, struct sockaddr_in* addr2);
void print_data(char* data, int len);
int is_option_set(int value, int option);
void close_connection();
int recv_data_timeout(struct sockaddr_in* src, char* data, int timeout_ms);

int init_server_socket_udp(int port);

int init_client_socket_udp(struct sockaddr_in* serverAddr, char* serverIP, int port);

int recv_data(struct sockaddr_in* src, char* data);

int send_data(struct sockaddr_in* dest, char* data, int len);

void send_packet_type(struct sockaddr_in* dest, char type);

#endif // CONNECTION_H