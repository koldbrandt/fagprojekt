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
}

int recv_data(struct sockaddr_in* src, char* data){
    int clientLen, recv_len;
    
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
}

int addrMatch(struct sockaddr_in* addr1, struct sockaddr_in* addr2){
    return (addr1->sin_addr.s_addr == addr2->sin_addr.s_addr) && (addr1->sin_port == addr2->sin_port);
}

int close_connection(){
    close(sockfd);
}