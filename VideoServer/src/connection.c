#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "connection.h"

struct sockaddr_in ownAddr;
int sockfd;

int init_server_socket(int port){
    
    if((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
    	printf("sock\n");
        exit(1);
    }

    memset((char *) &ownAddr, 0, sizeof(ownAddr));
    ownAddr.sin_family = AF_INET;
    ownAddr.sin_addr.s_addr = INADDR_ANY;
    ownAddr.sin_port = htons(port);

    if(bind(sockfd, (struct sockaddr*) &ownAddr, sizeof(ownAddr)) < 0)
    {
    	printf("bind\n");
        exit(1);
    }
    return 0;
}


int init_client_socket(struct sockaddr_in* serverAddr, char* serverIP, int port){
    if((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
    	printf("sock\n");
        exit(1);
    }

    memset(serverAddr, 0, sizeof(*serverAddr)); 
    serverAddr->sin_family = AF_INET; 
    serverAddr->sin_addr.s_addr = inet_addr(serverIP); 
    serverAddr->sin_port = htons(port);
    return 0;
}

int recv_data(struct sockaddr_in* src, char* data){
    int recv_len;
    
    socklen_t clientLen;
    recv_len = -1;

    clientLen = sizeof(*src);

    if ((recv_len = recvfrom(sockfd, data, MAX_PACKET_SIZE, 0, (struct sockaddr *) src, &clientLen)) == -1)
    {
        exit(1);
    }

    return recv_len;
}

int send_data(struct sockaddr_in* dest, char* data, int len){
    sendto(sockfd, data, len, 0, (struct sockaddr *) dest, sizeof(*dest));
    return 0;
}

int addrMatch(struct sockaddr_in* addr1, struct sockaddr_in* addr2){
    return (addr1->sin_addr.s_addr == addr2->sin_addr.s_addr) && (addr1->sin_port == addr2->sin_port);
}

void close_connection(){
    close(sockfd);
}

// send a packet type that is specified in the protocol
// This function only supports packet types that do not have any additional information in the header (like the VIDEO_DATA packet)
void send_packet_type(struct sockaddr_in* dest, char type){
    char response = type;
    send_data(dest, &response, 1);
}

// print an array
// used for debugging
void print_data(char* data, int len){
    for(int i = 0; i < len; i++){
        printf("%c", data[i]);
    }
    printf("\n");
}

// check whether a specific bit flag is set in an integer
int is_option_set(int value, int option){
    return (value & option) == option;
}