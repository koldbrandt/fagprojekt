#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "connection.h"
#include "video.h"

#define INIT_BYTE 48
#define TERMINATE_BYTE 0

int connectionStatus;
struct sockaddr_in clientAddr;

enum connection_status{
    WAITING_INIT,
    RECV_VIDEO
};

int main(){
    
    init_socket();
    connectionStatus = WAITING_INIT;

    while(1){
        switch(connectionStatus){
            case WAITING_INIT:
                wait_init(&clientAddr);
                break;
            
            case RECV_VIDEO:
                recv_video(&clientAddr);
                break;
        }
    }
}

void wait_init(struct sockaddr_in* client){
    char data[MAX_PACKET_SIZE];
    
    while(connectionStatus == WAITING_INIT){
        recv_data(client, data);
        if(data[0] == INIT_BYTE){
            char response = INIT_BYTE;
            send_data(clientAddr, &response, 1);
            connectionStatus = RECV_VIDEO;
            printf("init done\n");
        }
    }
}

void recv_video(){
    struct sockaddr_in recvAddr;
    char data[MAX_PACKET_SIZE];

    while(1){
        recv_data(&recvAddr, data);
        if((recvAddr.sin_addr.s_addr == clientAddr.sin_addr.s_addr) && (recvAddr.sin_port == clientAddr.sin_port)){
            
        }
    }
}

