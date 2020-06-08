#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/poll.h>
#include "connection.h"

struct sockaddr_in ownAddr;
int sockfd;

int init_server_socket(int port){
    
    if((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
    	printf("sock\n");
        exit(1);
    }

    // clear the data in the sockaddr_in struct
    memset((char *) &ownAddr, 0, sizeof(ownAddr));

    // set the server to use IPv4 and listen to any incoming connections on the specified port
    ownAddr.sin_family = AF_INET;
    ownAddr.sin_addr.s_addr = INADDR_ANY;
    ownAddr.sin_port = htons(port);

    // start listening
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

    // clear the data in the sockaddr_in struct
    memset(serverAddr, 0, sizeof(*serverAddr)); 

    // set the client to use IPv4 and set the server ip and port
    serverAddr->sin_family = AF_INET; 
    serverAddr->sin_addr.s_addr = inet_addr(serverIP); 
    serverAddr->sin_port = htons(port);
    return 0;
}

// received data from the socket initialized from either init_client_socket or init_server_socket
int recv_data(struct sockaddr_in* src, char* data){
    socklen_t clientLen;
    int recv_len = -1;

    clientLen = sizeof(*src);

    if ((recv_len = recvfrom(sockfd, data, MAX_PACKET_SIZE, 0, (struct sockaddr *) src, &clientLen)) == -1)
    {
        exit(1);
    }

    return recv_len;
}

// the recv_data function but with a timeout as to not block execution indefinitely
int recv_data_timeout(struct sockaddr_in* src, char* data, int timeout_ms){
    socklen_t clientLen;
    int recv_len = -1;

    clientLen = sizeof(*src);

    struct pollfd fds[1];
    fds[0].fd = sockfd;
    fds[0].events = POLLIN;

    int pollResult = poll(fds, 1, timeout_ms);
    // pollresult
    // > 0 something was received
    // == 0 timed out
    // < 0 something went wrong

    if(pollResult > 0){
        if ((recv_len = recvfrom(sockfd, data, MAX_PACKET_SIZE, 0, (struct sockaddr *) src, &clientLen)) == -1){
            printf("recvfrom failed\n");
        }
    }
    else if(pollResult < 0){
    	printf("%s\n", "receive failed");
    }

    // recv_len
    // -1 is timeout
    // > 0 is length of received packet
    return recv_len;
}

// sends data to the socket initialized from either init_client_socket or init_server_socket
int send_data(struct sockaddr_in* dest, char* data, int len){
    sendto(sockfd, data, len, 0, (struct sockaddr *) dest, sizeof(*dest));
    return 0;
}

// check if 2 addresses are the same
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