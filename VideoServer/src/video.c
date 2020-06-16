#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "connection.h"
#include "video.h"
#include "server.h"
#include "client.h"

int main(int argc, char *argv[]){
    if(argc == 1 || strcmp(argv[1],"-h") == 0){
        print_help();
        exit(0);
    }
    
    // set default options
    int serverPort = SERVER_PORT;
    char* serverIP = SERVER_IP;
    int clientOptions = 0;
    int serverOptions = 0;
    
    //check if any options are set
    for(int i = 0; i < argc; i++){
        //shared options
        if(strcmp(argv[i],"-p") == 0){
            serverPort = atoi(argv[i + 1]);
        }
        
        //client options
        if(strcmp(argv[i],"-ip") == 0){
            serverIP = argv[i + 1];
        }
        if(strcmp(argv[i], "-d") == 0){
            clientOptions = clientOptions | RUN_DEBUG;
        }

        //server options
        if(strcmp(argv[i], "-nf") == 0){
            serverOptions = serverOptions | NO_FIFO;
        }
        if(strcmp(argv[i], "-iperf") == 0){
            serverOptions = serverOptions | SERVER_RUN_IPERF;
        }
    }
    if(strcmp(argv[1],"-s") == 0){
        run_server(serverPort, serverOptions);
    }
    else if(strcmp(argv[1],"-c") == 0){
        run_client(serverIP, serverPort, clientOptions);
    }
}

void print_help(){
    printf("Usage:\n");
    printf("video {-s|-c} [options]\n");
    printf("video -h\n");
    printf("    -s: Run as server\n");
    printf("    -c: Run as client\n");
    printf("    -h: Print this help message\n");
    printf("\n");

    printf("Server options\n");
    printf("    -p <port>: Set listen port [default: %d]\n", SERVER_PORT);
    printf("    -nf:       Don't write to fifo\n");
    printf("    -iperf:    Ignore protocol and put all data received from the socket\n");
    printf("               into the FIFO. Used for testing with iperf client\n");
    
    printf("\n");

    printf("Client options\n");
    printf("    -p <port>: Set destination port [default: %d]\n", SERVER_PORT);
    printf("    -ip <ip>:  Set destination ip   [default: %s]\n", SERVER_IP);
    printf("    -d:        Run in debug mode\n");
    printf("    -iperf:    Ignore protocol and send data as fast as it is available in the FIFO.\n");
    printf("               Used when connecting to iperf server\n");
    printf("\n");

    printf("Example server:\n");
    printf("video -s -p 1234\n");
    printf("Example client:\n");
    printf("video -c -ip 192.168.1.102 -p 1234\n");
}