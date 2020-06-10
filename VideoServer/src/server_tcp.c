#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "buffer.h"
#include "server.h"
#include "connection.h"
#include "fifo.h"

void run_server_tcp(cbuf_handle_t video_buffer, int options){
    if(is_option_set(options, SERVER_RUN_IPERF)){
        // run the server that expects an iperf client, and exit afterwards
        // this is the path for the -iperf option
        // this path ignores the video data protocol/headers and treats everything as data
        run_server_iperf_tcp(video_buffer, options);
        return;
    }
    recv_video_tcp(video_buffer);
}

void recv_video_tcp(cbuf_handle_t video_buffer){
    char data[MAX_PACKET_SIZE];

    // video receive loop
    while(1){
        
        char packetType = 0;
        char dataLenHolder[2];

        recv_data_tcp(&packetType, 1);
    
        if(packetType != VIDEO_DATA){
            continue;
            
        }
        
        recv_data_tcp(&dataLenHolder[0], 2);
        unsigned short len = 0;
        memcpy(&len, &dataLenHolder[0], 2);
        unsigned int dataLen = (unsigned int) len;

        if(dataLen <= 0){
            continue;
        }

        recv_data_tcp(data, dataLen);
        send_packet_buffer(data, dataLen, video_buffer);
    }
}

// function for running the performance test server that expects an iperf client to connect
// the performance test server ignores the protocol as iperf of course cannot follow it when sending data
void run_server_iperf_tcp(cbuf_handle_t video_buffer, int options){
    printf("Running in iperf test mode\n");
    recv_video_iperf_tcp(video_buffer);
}

// the video receive function for the performance test server
// this receive function treats all data in the packet as video data and writes it all to the video data buffer
void recv_video_iperf_tcp(cbuf_handle_t video_buffer){
    char data[MAX_PACKET_SIZE];
    unsigned int dataLen = 0;
    double fill_highest = 0;

    while(1){
        dataLen = recv_data_tcp(data, MAX_PACKET_SIZE); // receive the next packet from the iperf client

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