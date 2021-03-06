#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#include "buffer.h"
#include "server.h"
#include "connection.h"
#include "fifo.h"

struct sockaddr_in clientAddr;
int connectionStatus;
pthread_t fifoWriteThreadId;
cbuf_handle_t video_buffer;

#define BUFFER_PRINT_INTERVAL 250 //print buffer status every 250 packets

enum connection_status{
    WAITING_INIT,
    RECV_VIDEO
};

void run_server(int serverPort, int options){

    // create buffer for received video data
    video_buffer = init_buffer(VIDEO_BUFFER_SIZE);

    //Open physical memory to allow for read/write to/from the fifo
	open_physical_memory_device();
    mmap_fpga_peripherals();

    if(!is_option_set(options, NO_FIFO)){
        // start the thread that reads from the video data buffer and writes to the fifo, unless the -nf option is specified
        printf("creating fifo write thread\n");
        pthread_create(&fifoWriteThreadId, NULL, &fifo_write_thread, video_buffer);
    }

    printf("Starting in server mode on port %d\n", serverPort);

    init_server_socket_udp(serverPort);

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
    close_server();
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
    unsigned short recvLen = 0;
    int pack_num = 0;
    // video receive loop
    while(1){
        recvLen = recv_data(&recvAddr, data);

        if(recvLen <= 0){
            continue;
        }

        send_packet_buffer(&data[0], recvLen, video_buffer);
        pack_num += 1;

        if(pack_num >= BUFFER_PRINT_INTERVAL){
            print_buffer_fill_level(video_buffer);
            pack_num = 0;
        }
    }
}

void send_packet_buffer(char* data, unsigned short dataLen, cbuf_handle_t video_buffer){
    int bufferSpace = get_space(video_buffer);
    if (bufferSpace > dataLen + 2){ // if we have enough room in the buffer, put the received data in the buffer
        char len_header[2];
        memcpy(&len_header[0], &dataLen, 2);
        send_data_buffer(&len_header[0], 2, video_buffer);
        send_data_buffer(data, dataLen, video_buffer);
    }
}

// this is the function that runs in the second thread and reads the data from the video data buffer and writes it to the fifo
// the buffer is passed as a void pointer as that seemed to be the only way to pass an argument to another thread
void* fifo_write_thread(void* buffer){
    char readData;
    cbuf_handle_t video_buffer = (cbuf_handle_t) buffer; // cast the void pointer to a pointer to the buffer

    // fifo write loop
    while(1){
        if(!buffer_is_empty(video_buffer)){
            // if the buffer is not empty, read one byte from it
            read_data_buffer(&readData, video_buffer);
            // send status indicates whether the data was written to the fifo successfully
            // this can fail if the fifo is full
            write_fifo_blocking(readData);
        }
        else{
            usleep(10000); //wait for video buffer to fill up
        }
    }
}

// function for running the performance test server that expects an iperf client to connect
// the performance test server ignores the protocol as iperf of course cannot follow it when sending data
void run_server_iperf(cbuf_handle_t video_buffer, int options){
    printf("Running in iperf test mode\n");
    connectionStatus = RECV_VIDEO;

    recv_video(video_buffer);
    close_server();
}

void print_buffer_fill_level(cbuf_handle_t video_buffer){
    double used = (double) fill_level(video_buffer);
    double percent = (used / VIDEO_BUFFER_SIZE) * 100;
    int floorPercent = (int) percent;
    /*
    if(floorPercent < 20){
        printf("Send fast packet sent\n");
        send_packet_type(&clientAddr, SEND_FAST);
    }
    if(floorPercent > 80){
        printf("send slow packet sent\n");
        send_packet_type(&clientAddr, SEND_SLOW);
    }
    */
    printf("Buffer is at %d%%\n", floorPercent);
}

void close_server(){
    close_connection();
	pthread_join(fifoWriteThreadId, NULL);
    free_buffer(video_buffer);
	//close physical memory
	munmap_fpga_peripherals();
    close_physical_memory_device();
}
