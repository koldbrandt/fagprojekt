#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "connection.h"
#include "video.h"
#include "fifo.h"

#define VIDEO_BUFFER_SIZE (MAX_PACKET_SIZE * 5)

int connectionStatus;
struct sockaddr_in clientAddr;

enum connection_status{
    WAITING_INIT,
    RECV_VIDEO
};

enum packet_types{
    INIT = 1,
    ACK_INIT = 2,
    TERMINATE = 3,
    VIDEO_DATA = 4,
    SEND_SLOW = 5,
    SEND_FAST = 6
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
        if(data[0] == INIT){
            send_packet_type(client, ACK_INIT);
            connectionStatus = RECV_VIDEO;
            printf("init done\n");
        }
    }
}

void recv_video(){
    struct sockaddr_in recvAddr;
    char data[MAX_PACKET_SIZE];
    int dataLen = 0;
    int recvLen = 0;
    int fifoStatus = 0;

    while(1){
        recvLen = recv_data(&recvAddr, data);
        if(recvLen < 3){
            continue;
        }

        int type = data[0];
        short len = 0;
        memcpy(&data[1], &dataLen, 2);
        int dataLen = (int) len;

        if(dataLen <= 0 || (type != VIDEO_DATA && type != TERMINATE)){
            continue;
        }
        
        if(addrMatch(&recvAddr, &clientAddr)){
            fifoStatus = send_data_fifo(&data[2], dataLen);
            if(fifoStatus == FIFO_FULL){
                send_packet_type(&clientAddr, SEND_SLOW);
            }
            else if(fifoStatus == FIFO_EMPTY){
                send_packet_type(&clientAddr, SEND_FAST);
            }
        }
        else{
            send_packet_type(&clientAddr, TERMINATE);
        }
    }
}

void send_packet_type(struct sockaddr_in* client, char type){
    char response = type;
    send_data(client, &response, 1);
}



