#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include "client.h"
#include "connection.h"
#include "fifo.h"

#define INIT_WAIT_TIMEOUT 3000 // milliseconds

struct sockaddr_in serverAddr;

void run_client(char* serverIP, int serverPort, int options){
    // map memory so we can read/write from/to the fifo
    open_physical_memory_device();
    mmap_fpga_peripherals();
    
    printf("Starting in client mode\n");
    printf("Connecting to %s on port %d\n", serverIP, serverPort);

    init_client_socket_udp(&serverAddr, serverIP, serverPort);
    
    if(is_option_set(options, RUN_DEBUG)){
        // if we are running in debug mode, simply run the test client and return
        run_test_client(serverIP, serverPort);
        return;
    }

    char response[MAX_PACKET_SIZE];
    struct sockaddr_in srcAddr;

    empty_fifo();
    
    do {
        send_packet_type(&serverAddr, INIT); // send the INIT packet to the server
        printf("sent INIT\n");

        int status = recv_data_timeout(&srcAddr, response, INIT_WAIT_TIMEOUT); // wait for server response. This should be the INIT_ACK packet
        if(status == -1){
            printf("Timed out while waiting for INIT_ACK, retrying...\n");
        }
    }
    while(!addrMatch(&srcAddr, &serverAddr) || response[0] != INIT_ACK);
    
    video_send_loop();

    close_client();
}

void video_send_loop(){
    printf("starting video loop\n");
    unsigned short packet_len = 0;
    int currentSize = 0;
    char dataBuffer[MAX_PACKET_SIZE];
    char pack_len[2];

    while(1){
        currentSize = 0;
        
        read_fifo_blocking(&pack_len[0]);
        read_fifo_blocking(&pack_len[1]);
        
        memcpy(&packet_len, &pack_len[0], 2);
        if(packet_len > MAX_PACKET_SIZE){
            printf("invalid packet len %d\n", packet_len);
            continue;
        }
        // read data from the fifo and put it into the buffer until the buffer contains the maximum allowed data in a VIDEO_DATA packet
        while (currentSize < packet_len){
            read_fifo_blocking(&dataBuffer[currentSize]);
            currentSize += 1;
        }
        // when the buffer is full, send the VIDEO_DATA packet
        send_video_packet(dataBuffer, currentSize);
    }
}

// the test client is used for checking that the protocol works correctly
void run_test_client(char* serverIP, int serverPort){
    printf("Running in debug mode\n");
    char* test_options = 
           "Enter \n"
           "1 to send INIT \n"
           "2 to send dummy VIDEO_DATA \n"
           "3 to read and send VIDEO_DATA from FIFO to server \n"
           "4 to run the video send loop \n"
           "5 to send TERMINATE\n"
           "6 to empty FIFO\n";

    printf("%s", test_options);

    int choice = 0;
    while(1){
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch(choice){
            case 1:
                send_packet_type(&serverAddr, INIT);
                printf("sent INIT, waiting for INIT_ACK\n");
                struct sockaddr_in srcAddr;
                char response[MAX_PACKET_SIZE];
                int status = recv_data_timeout(&srcAddr, response, INIT_WAIT_TIMEOUT); // wait for server response. This should be the INIT_ACK packet
                if(status == -1){
                    printf("Timed out while waiting for INIT_ACK\n");
                }
                else if (addrMatch(&srcAddr, &serverAddr) && response[0] == INIT_ACK){
                    printf("Successfully received INIT_ACK from the server\n");
                }

                break;
            
            case 2:;
                // send the string "0123456789" in a VIDEO_DATA packet
                char packet[DUMMY_DATA_LEN];
                sprintf(&packet[0], "0123456789");
                printf("sent the string \"%s\" to the server as video data\n", "0123456789");
                send_video_packet(packet, DUMMY_DATA_LEN);
                break;

            case 3:
                // read a given amount of bytes from the fifo and send those bytes to the server as VIDEO_DATA
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
                // simply run the loop that sends VIDEO_DATA to the server.
                // this does not do the initial handshake before sending video data
                video_send_loop();
                break;

            case 5:
                // close the connections to the server and stop the client program
                send_packet_type(&serverAddr, TERMINATE);
                printf("sent TERMINATE, exiting..\n");
                close_connection();
                exit(0);
                break;

            case 6:
                empty_fifo(); 
                break;
        }
    }
    close_client();
}

void empty_fifo(){
    char temp = 0;
    char dataHolder = 0;
    int counter = 0;
    printf("emptying fifo\n");
    temp = read_data_fifo(&dataHolder);
    while(temp == 0){
        temp = read_data_fifo(&dataHolder);
        counter += 1;
    }
    printf("read %d bytes from fifo\n", counter);
}

void send_video_packet(char* data, short len){
    send_data(&serverAddr, data, len);
}

void close_client(){
    close_connection();
    munmap_fpga_peripherals();
    close_physical_memory_device();
}