#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "connection.h"




struct sockaddr_in ownAddr;
int sockfd;

int init_socket(){
    
    if((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
    	printf("sock\n");
        exit(1);
    }

    memset((char *) &ownAddr, 0, sizeof(ownAddr));
    ownAddr.sin_family = AF_INET;
    ownAddr.sin_addr.s_addr = INADDR_ANY;
    ownAddr.sin_port = htons(SERVER_PORT);

    if(bind(sockfd, (struct sockaddr*) &ownAddr, sizeof(ownAddr)) < 0)
    {
    	printf("bind\n");
        exit(1);
    }
    return 0;
}

int recv_data(struct sockaddr_in* src, char* data){
    int clientLen, recv_len;
    struct sockaddr_in cAddr;
    
    recv_len = -1;

    clientLen = sizeof(cAddr);

    if ((recv_len = recvfrom(sockfd, data, MAX_PACKET_SIZE, 0, (struct sockaddr *) &cAddr, &clientLen)) == -1)
    {
        exit(1);
    }

    *src = cAddr;

    return recv_len;
}

int send_data(struct sockaddr_in dest, char* data, int len){
    sendto(sockfd, data, len, 0, (struct sockaddr *) &dest, sizeof(dest));
}