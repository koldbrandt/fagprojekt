#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include "client.h"
#include "connection.h"
#include "fifo.h"


#define DUMMY_DATA_LEN 11
#define DEFAULT_SLEEP_TIME 10000
#define SLEEP_TIME_INCREMENT 10000


struct sockaddr_in serverAddr;
pthread_t listenThreadId;
int sleepTime; //used for adjusting send rate

void run_client(char* serverIP, int serverPort){
    init_client(serverIP, serverPort);

    send_packet_type(&serverAddr, INIT);
    printf("sent INIT\n");

    struct sockaddr_in srcAddr;
    char response[MAX_PACKET_SIZE];

    recv_data(&srcAddr, response);

    sleepTime = DEFAULT_SLEEP_TIME;

    if(addrMatch(&srcAddr, &serverAddr) && response[0] == INIT_ACK){
        //pthread_create(&listenThreadId, NULL, &client_listen_thread, NULL);
        video_send_loop();
    }
    
    close_client();
}

void video_send_loop(){
    printf("starting video loop\n");
    while(1){
        int currentSize = 0;
        char dataBuffer[MAX_PACKET_SIZE];
        int returnValue = 0;
        while (currentSize < MAX_PACKET_SIZE - VIDEO_HEADER_LENGTH){
            returnValue = read_data_fifo(&dataBuffer[currentSize]);
            if(returnValue == 0){
                currentSize += 1;
            }
        }
        send_video_packet(dataBuffer, currentSize);
        usleep(sleepTime);
    }
}

void run_test_client(char* serverIP, int serverPort){
    init_client(serverIP, serverPort);
    //pthread_create(&listenThreadId, NULL, &client_listen_thread, NULL);
    printf("Running in debug mode\n");
    printf("Enter \n"
           "1 to send INIT \n"
           "2 to send dummy VIDEO_DATA \n"
           "3 to read and send VIDEO_DATA from FIFO to server \n"
           "4 to send TERMINATE\n");

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
                break;

            case 3:
                printf("amount of bytes to read from fifo: ");
                int amount = 0;
                scanf("%d", &amount);
                char* fifoData = malloc(amount * sizeof(char));
                int returnValue = 0;
                for(int i = 0; i < amount; i++){
                    
                    returnValue = read_data_fifo(&fifoData[i]);
                    
                    if(returnValue == 1){
                        printf("tried to read from empty fifo\n");
                    }
                }
                
                send_video_packet(fifoData, amount);
                free(fifoData);
                break;

            case 4:
                send_packet_type(&serverAddr, TERMINATE);
                printf("sent TERMINATE, exiting..\n");
                close_connection();
                exit(0);
                break;
        }
    }
    close_client();
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

void* client_listen_thread(){
    printf("started listening thread\n");
    char buffer[MAX_PACKET_SIZE];
    struct sockaddr_in datasrc;
    while(1){
        recv_data(&datasrc, buffer);
        if(buffer[0] == SEND_SLOW){
            sleepTime += SLEEP_TIME_INCREMENT;
        }
        else if(buffer[0] == SEND_FAST){
            sleepTime -= SLEEP_TIME_INCREMENT;
        }
        if(sleepTime < 0){
            sleepTime = 1;
        }
        printf("current sleep: %d\n", sleepTime);
    }
}

void init_client(char* serverIP, int serverPort){
    init_client_socket(&serverAddr, serverIP, serverPort);
    open_physical_memory_device();
    mmap_fpga_peripherals();
    print_init(serverIP, serverPort);
}

void close_client(){
    close_connection();
    pthread_join(listenThreadId, NULL);
    munmap_fpga_peripherals();
    close_physical_memory_device();
}

void print_init(char* serverIP, int serverPort){
    printf("Starting in client mode\n");
    printf("Connecting to %s on port %d\n", serverIP, serverPort);
}