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
#define VIDEO_BUFFER_SIZE (MAX_PACKET_SIZE * 5)

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
    char videoBuffer[VIDEO_BUFFER_SIZE]; 
    int dataLen = 0;
    int fifoStatus = 0;

    while(1){
        dataLen = recv_data(&recvAddr, data);
        if(dataLen <= 0){
            continue;
        }

        if(addrMatch(recvAddr, clientAddr)){
            //fifoStatus = send_data_fifo(data, dataLen);
        }
        else{
            char response = TERMINATE_BYTE;
            send_data(recvAddr, &response, 1);
        }
    }
}



