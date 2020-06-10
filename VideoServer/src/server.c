#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "buffer.h"
#include "server.h"
#include "connection.h"
#include "fifo.h"

#define VIDEO_BUFFER_SIZE 20000000

struct sockaddr_in clientAddr;
int connectionStatus;
pthread_t fifoWriteThreadId;

enum connection_status{
    WAITING_INIT,
    RECV_VIDEO
};

void run_server(int serverPort, int options){
    init_server_socket_udp(serverPort);
    // create buffer for received video data
    cbuf_handle_t video_buffer = init_buffer(VIDEO_BUFFER_SIZE);
    
    //Open physical memory to allow for read/write to/from the fifo
	open_physical_memory_device();
    mmap_fpga_peripherals();

    printf("Starting in server mode on port %d\n", serverPort);
    
    if(!is_option_set(options, NO_FIFO)){
        // start the thread that reads from the video data buffer and writes to the fifo, unless the -nf option is specified
        printf("creating fifo write thread\n");
        pthread_create(&fifoWriteThreadId, NULL, &fifo_write_thread, video_buffer);
    }
    if(is_option_set(options, SERVER_RUN_IPERF)){
        // run the server that expects an iperf client, and exit afterwards
        // this is the path for the -iperf option
        // this path ignores the video data protocol/headers and treats everything as data
        run_server_iperf(video_buffer, options);
        return;
    }
    
    connectionStatus = WAITING_INIT;
	
    // simple state machine for whether we are waiting for a client connection or currently receiving data from a client
    while(1){
        switch(connectionStatus){
            case WAITING_INIT:
                wait_init(&clientAddr);
                break;
            
            case RECV_VIDEO:
                recv_video(video_buffer);
                break;
        }
    }
    close_connection();
	pthread_join(fifoWriteThreadId, NULL);
    free_buffer(video_buffer);
	//close physical memory
	munmap_fpga_peripherals();
    close_physical_memory_device();
}

// function for the state where we are waiting for an INIT packet from a client
void wait_init(struct sockaddr_in* client){
    char data[MAX_PACKET_SIZE];
    // wait until we receive an INIT packet from a client
    while(connectionStatus == WAITING_INIT){
        recv_data(client, data); // receive the next packet
        if(data[0] == INIT){
            // if the packet is an INIT packet, respond with INIT_ACK and go to the receiving video state
            send_packet_type(client, INIT_ACK);
            connectionStatus = RECV_VIDEO;
            printf("INIT received\n");
        }
    }
}

void recv_video(cbuf_handle_t video_buffer){
    struct sockaddr_in recvAddr;
    char data[MAX_PACKET_SIZE];
    int recvLen = 0;

    // video receive loop
    while(1){
        recvLen = recv_data(&recvAddr, data);
        
        int type = data[0];

        if(type == TERMINATE){
            printf("received TERMINATE, waiting for new client\n");
            connectionStatus = WAITING_INIT;
            break;
        }

        if(recvLen < 3){
            // if the length of the packet is less than 3, it cannot be a VIDEO_DATA packet
            // in that case simply skip to the next packet
            continue;
        }
        
        // put the length of the from the VIDEO_DATA header into the dataLen variable
        unsigned short len = 0;
        memcpy(&len, &data[1], 2);
        unsigned int dataLen = (unsigned int) len;

        if(dataLen <= 0 || type != VIDEO_DATA){
            // again if the packet is the wrong type we skip it
            // this is for the case where the length is larger or equal to 3, but still has the wrong packet type in the header
            // or an invalid length in the VIDEO_DATA header
            continue;
        }
        
        if(addrMatch(&recvAddr, &clientAddr)){ // make sure the packet is from the client we are currently connected to
            // this check is only needed because we use UDP, as this allows for any client to send any kind of data at any time
            // we probably don't need this check when the program is running in the full system as there should only ever be one client sending any data to the server
            
            int bufferSpace = get_space(video_buffer); // get the remaining space left in the video data buffer
            
            if(bufferSpace >= dataLen){ // if we have enough room in the buffer, put the received data in the buffer
                // start at data[3] as we do not want the header information to be written to the video data buffer
                send_data_buffer(&data[3], dataLen, video_buffer);
            }
            else{
                send_packet_type(&clientAddr, SEND_SLOW); // otherwise skip the data and send a SEND_SLOW packet to the client
            }
        }
        else { // if the packet is from a new client, try to terminate the connection to the new client
            send_packet_type(&clientAddr, TERMINATE);
        }
    }
}

// this is the function that runs in the second thread and reads the data from the video data buffer and writes it to the fifo
// the buffer is passed as a void pointer as that seemed to be the only way to pass an argument to another thread
void* fifo_write_thread(void* buffer){
    char readData;
    cbuf_handle_t video_buffer = (cbuf_handle_t) buffer; // cast the void pointer to a pointer to the buffer
    int sendStatus = 0;

    // fifo write loop
    while(1){
        if(!buffer_is_empty(video_buffer)){
            // if the buffer is not empty, read one byte from it
            read_data_buffer(&readData, video_buffer);
            // send status indicates whether the data was written to the fifo successfully
            // this can fail if the fifo is full 
            printf("123 %c\n", readData);
            sendStatus = send_data_fifo(readData); 
            // keep trying to write the data to the fifo until it succeeds
            while(sendStatus == 1){
                usleep(1); //wait for fifo to not be full
                sendStatus = send_data_fifo(readData);
            }
        }
        else{
            usleep(1); //wait for video buffer to fill up
        } 
    }
}

// function for running the performance test server that expects an iperf client to connect
// the performance test server ignores the protocol as iperf of course cannot follow it when sending data
void run_server_iperf(cbuf_handle_t video_buffer, int options){
    printf("Running in iperf test mode\n");
    connectionStatus = RECV_VIDEO;
	
    recv_video_iperf(video_buffer);

    close_connection();
	pthread_join(fifoWriteThreadId, NULL);
    free_buffer(video_buffer);
	//close physical memory
	munmap_fpga_peripherals();
    close_physical_memory_device();
}

// the video receive function for the performance test server
// this receive function treats all data in the packet as video data and writes it all to the video data buffer
void recv_video_iperf(cbuf_handle_t video_buffer){
    struct sockaddr_in recvAddr;
    char data[MAX_PACKET_SIZE];
    unsigned int dataLen = 0;
    double fill_highest = 0;

    while(1){
        dataLen = recv_data(&recvAddr, data); // receive the next packet from the iperf client

        if(dataLen <= 0){ // if we get an error when receiving or an invalid packet, skip to the next packet
            continue;
        }
        
        int bufferSpace = get_space(video_buffer); // get the remaining space in the video data buffer

        // calculate how much of the video data buffer is used in percent
        double used = (double) fill_level(video_buffer);
        double percent = (used / VIDEO_BUFFER_SIZE) * 100;

        if(percent - fill_highest > 1){
            // if the percentage used is the highest it have been since the start of the program, print the percentage used and update the highest percentage
            printf("highest buffer fill level: %d%%\n", (int) percent);
            fill_highest = percent;
        }
        if(bufferSpace >= dataLen){ // if we have enough room in the buffer, put the received data in the buffer
            send_data_buffer(data, dataLen, video_buffer); // send the entire packet to the video data buffer
        }
        else{
            printf("buffer is full\n");
        }
    }
}