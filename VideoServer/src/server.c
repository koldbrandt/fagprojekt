#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "buffer.h"
#include "server.h"
#include "connection.h"
#include "fifo.h"

#define VIDEO_BUFFER_SIZE 20000000
#define IPERF_PACKET_SIZE 8192

double fill_highest = 0;

struct sockaddr_in clientAddr;
int connectionStatus;
pthread_t fifoWriteThreadId;

enum connection_status{
    WAITING_INIT,
    RECV_VIDEO
};

void run_server(int serverPort, int options){
    init_server_socket(serverPort);
    cbuf_handle_t video_buffer = init_buffer(VIDEO_BUFFER_SIZE);
    
    //Open physical memory 
	open_physical_memory_device();
    mmap_fpga_peripherals();

    printf("Starting in server mode on port %d\n", serverPort);
    
    if(!is_option_set(options, NO_FIFO)){
        printf("creating fifo write thread\n");
        pthread_create(&fifoWriteThreadId, NULL, &fifo_write_thread, video_buffer);
    }
    if(is_option_set(options, SERVER_RUN_IPERF)){
        run_server_iperf(video_buffer, options);
        return;
    }
    
    connectionStatus = WAITING_INIT;
	
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

void wait_init(struct sockaddr_in* client){
    char data[MAX_PACKET_SIZE];
    
    while(connectionStatus == WAITING_INIT){
        recv_data(client, data);
        if(data[0] == INIT){
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
            int bufferSpace = get_space(video_buffer);
            
            if(bufferSpace >= dataLen){
                send_data_buffer(&data[3], dataLen, video_buffer);
            }
            else{
                send_packet_type(&clientAddr, SEND_SLOW);
            }
        }
        else{
            send_packet_type(&clientAddr, TERMINATE);
        }
    }
}

void* fifo_write_thread(void* buffer){
    char readData[1];
    cbuf_handle_t video_buffer = (cbuf_handle_t) buffer;
    int sendStatus = 0;
    while(1){
        if(!buffer_is_empty(video_buffer)){
            read_data_buffer(readData, 1, video_buffer);
            sendStatus = send_data_fifo(readData[0]);
            while(sendStatus == 1){
                usleep(1); //wait for fifo to not be full
                sendStatus = send_data_fifo(readData[0]);
            }
        }
        else{
            usleep(1); //wait for video buffer to fill up
        } 
    }
}

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

void recv_video_iperf(cbuf_handle_t video_buffer){
    struct sockaddr_in recvAddr;
    char data[IPERF_PACKET_SIZE];
    unsigned int dataLen = IPERF_PACKET_SIZE;

    while(1){
        recv_data(&recvAddr, data);
        
        int bufferSpace = get_space(video_buffer);
        double used = (double) fill_level(video_buffer);
        double percent = (used / VIDEO_BUFFER_SIZE) * 100;
        if(percent - fill_highest > 1){
            printf("highest buffer fill level: %d%%\n", (int) percent);
            fill_highest = percent;
        }
        if(bufferSpace >= dataLen){
            send_data_buffer(data, dataLen, video_buffer);
        }
        else{
            printf("buffer is full\n");
        }
    }
}