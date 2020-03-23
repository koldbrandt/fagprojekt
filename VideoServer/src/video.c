#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "connection.h"
#include "video.h"
#include "fifo.h"



#define DUMMY_DATA_LEN 1000

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

int main(int argc, char *argv[]){
    if(strcmp(argv[1],"-s") == 0){
        run_server();
    }
    else if(strcmp(argv[1],"-c") == 0){
        run_client();
    }
}

void run_server(){
    init_server_socket();
    printf("Starting in server mode\n");
    connectionStatus = WAITING_INIT;
	
	//Open physical memory 
	open_physical_memory_device();
    mmap_fpga_peripherals();
	
	
    while(1){
        switch(connectionStatus){
            case WAITING_INIT:
                wait_init(&clientAddr);
                break;
            
            case RECV_VIDEO:
                recv_video();
                break;
        }
    }
	
	//close physical memory
	munmap_fpga_peripherals();
    close_physical_memory_device();
	
}

void run_client(){

    struct sockaddr_in serverAddr;
    init_client_socket(&serverAddr);
    int choice = 0;
    printf("Starting in client mode\n");
    printf("enter \n1 to send INIT \n2 to send VIDEO_DATA \n3 to send TERMINATE\n");
    while(1){
        scanf("%d", &choice);

        switch(choice){
            case 1:
                send_packet_type(&serverAddr, INIT);
                printf("sent INIT\n");
                break;
            
            case 2:;
                char packet[DUMMY_DATA_LEN];
                packet[0] = VIDEO_DATA;
                short len = 500;
                memcpy(&packet[1], &len, 2);
                send_data(&serverAddr, packet, DUMMY_DATA_LEN);
                printf("sent VIDEO_DATA\n");
                break;

            case 3:
                send_packet_type(&serverAddr, TERMINATE);
                printf("sent TERMINATE, exiting..\n");
                close_connection();
                exit(0);
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
            printf("INIT received\n");
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
        
        int type = data[0];

        if(type == TERMINATE){
            printf("received TERMINATE, waiting for new client\n");
            connectionStatus = WAITING_INIT;
            break;
        }

        if(recvLen < 3){
            continue;
        }

        unsigned short len = 0;
        memcpy(&len, &data[1], 2);
        unsigned int dataLen = (unsigned int) len;

        if(dataLen <= 0 || type != VIDEO_DATA){
            continue;
        }

        if(addrMatch(&recvAddr, &clientAddr)){
            printf("received VIDEO_DATAA\n");
            fifoStatus = send_data_fifo(&data[2], dataLen);
			printf("kusse");
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

void send_packet_type(struct sockaddr_in* dest, char type){
    char response = type;
    send_data(dest, &response, 1);
}