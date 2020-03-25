#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include "connection.h"
#include "video.h"
#include "fifo.h"
#include "buffer.h"

#define DUMMY_DATA_LEN 14

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


pthread_t fifoWriteThreadId;

int main(int argc, char *argv[]){
    if(argc == 1 || strcmp(argv[1],"-h") == 0){
        print_help();
        exit(0);
    }

    int serverPort = SERVER_PORT;
    char* serverIP = SERVER_IP;
    for(int i = 0; i < argc; i++){
        if(strcmp(argv[i],"-p") == 0){
            serverPort = atoi(argv[i + 1]);
        }
        if(strcmp(argv[i],"-ip") == 0){
            serverIP = argv[i + 1];
        }
    }
    if(strcmp(argv[1],"-s") == 0){
        cbuf_handle_t video_buffer = init_buffer();
        pthread_create(&fifoWriteThreadId, NULL, &fifo_write_thread, video_buffer);
        run_server(serverPort, video_buffer);
    }
    else if(strcmp(argv[1],"-c") == 0){
        run_client(serverIP, serverPort);
    }
    pthread_join(fifoWriteThreadId, NULL);
}

void run_server(int serverPort, cbuf_handle_t video_buf){
    init_server_socket(serverPort);
    printf("Starting in server mode on port %d\n", serverPort);
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
                recv_video(video_buf);
                break;
        }
    }
	
	//close physical memory
	munmap_fpga_peripherals();
    close_physical_memory_device();
	
}

void run_client(char* serverIP, int serverPort){

    struct sockaddr_in serverAddr;
    init_client_socket(&serverAddr, serverIP, serverPort);
    int choice = 0;
    printf("Starting in client mode\n");
    printf("Connecting to %s on port %d\n", serverIP, serverPort);
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
                short len = DUMMY_DATA_LEN - 3;
                memcpy(&packet[1], &len, 2);
                sprintf(&packet[3], "0123456789");
                printf("sent the string \"%s\" to the server as video data\n", "0123456789");
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

void recv_video(cbuf_handle_t video_buffer){
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
            send_data_buffer(&data[3], dataLen, video_buffer);
            /*if(fifoStatus == BUFFER_FULL){
                send_packet_type(&clientAddr, SEND_SLOW);
            }
            else if(fifoStatus == BUFFER_EMPTY){
                send_packet_type(&clientAddr, SEND_FAST);
            }*/
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

void print_help(){
    printf("Server syntax\n");
    printf("./main.out -s -p PORT\n");
    printf("Example server:\n");
    printf("./main.out -s -p 1234\n");
    printf("\n");
    printf("Client syntax\n");
    printf("./main.out -c -ip IP -p PORT\n");
    printf("Example client:\n");
    printf("./main.out -c -ip 127.0.0.1 -p 1234\n");
}