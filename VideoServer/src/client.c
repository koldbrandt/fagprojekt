#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "client.h"
#include "connection.h"
#include "fifo.h"


#define DUMMY_DATA_LEN 11

struct sockaddr_in serverAddr;

void run_client(char* serverIP, int serverPort){
    init_client_socket(&serverAddr, serverIP, serverPort);
    print_init(serverIP, serverPort);

    send_packet_type(&serverAddr, INIT);
    printf("sent INIT\n");

    struct sockaddr_in srcAddr;
    char response[MAX_PACKET_SIZE];

    recv_data(&srcAddr, response);

    if(addrMatch(&srcAddr, &serverAddr) && response[0] == INIT_ACK){
        video_send_loop();
    }
    
    close_connection();
}

void video_send_loop(){
    printf("starting video loop\n");
    while(1){
        int currentSize = 0;
        char dataBuffer[MAX_PACKET_SIZE];
        while (currentSize < MAX_PACKET_SIZE - VIDEO_HEADER_LENGTH){
            //dataBuffer[currentSize] = read_data_fifo() 
            currentSize += 1;
            usleep(200000 / MAX_PACKET_SIZE);
        }
        send_video_packet(dataBuffer, currentSize);
    }
}

void run_test_client(char* serverIP, int serverPort){
    init_client_socket(&serverAddr, serverIP, serverPort);
    print_init(serverIP, serverPort);
    printf("Running in debug mode\n");
    printf("Enter \n1 to send INIT \n2 to send VIDEO_DATA \n3 to send TERMINATE\n");
    int choice = 0;
    while(1){
        scanf("%d", &choice);

        switch(choice){
            case 1:
                send_packet_type(&serverAddr, INIT);
                printf("sent INIT\n");
                break;
            
            case 2:;
                char packet[DUMMY_DATA_LEN];
                sprintf(&packet[0], "0123456789");
                printf("sent the string \"%s\" to the server as video data\n", "0123456789");
                send_video_packet(packet, DUMMY_DATA_LEN);
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

void send_video_packet(char* data, short len){
    int length = VIDEO_HEADER_LENGTH + len;
    char* packet = malloc(length * sizeof(char));
    packet[0] = VIDEO_DATA;
    memcpy(&packet[1], &len, 2);
    memcpy(&packet[3], data, len);
    send_data(&serverAddr, packet, length);
    free(packet);
}

void print_init(char* serverIP, int serverPort){
    printf("Starting in client mode\n");
    printf("Connecting to %s on port %d\n", serverIP, serverPort);
}