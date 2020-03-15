#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define MAX_PACKET_SIZE 1024
#define PORT 1337

struct sockaddr_in ownAddr;
int sockfd;

int init(){
    
    if((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
    	printf("sock\n");
        exit(1);
    }

    memset((char *) &ownAddr, 0, sizeof(ownAddr));
    ownAddr.sin_family = AF_INET;
    ownAddr.sin_addr.s_addr = INADDR_ANY;
    ownAddr.sin_port = htons(PORT);

    if(bind(sockfd, (struct sockaddr*) &ownAddr, sizeof(ownAddr)) < 0)
    {
    	printf("bind\n");
        exit(1);
    }
    return 0;
}

int recv_data(int* src, char* data){
    struct sockaddr_in clientAddr;
    int clientLen, recv_len;
    
    recv_len = -1;

    clientLen = sizeof(clientAddr);

    if ((recv_len = recvfrom(sockfd, data, MAX_PACKET_SIZE, 0, (struct sockaddr *) &clientAddr, &clientLen)) == -1)
    {
        exit(1);
    }

    *src = ntohs(clientAddr.sin_port);

    return recv_len;
}

int main(){
    init();
    int src;
    char data[MAX_PACKET_SIZE];
    while(1){
        recv_data(&src, data);
        printf("%c \n", data[0]);
    }
}

