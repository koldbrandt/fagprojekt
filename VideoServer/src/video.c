#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#include "connection.h"
#include "video.h"
#include "server.h"
#include "client.h"
#include "buffer.h"

int main(int argc, char *argv[]){
    if(argc == 1 || strcmp(argv[1],"-h") == 0){
        print_help();
        exit(0);
    }
    int serverPort = SERVER_PORT;
    char* serverIP = SERVER_IP;
    char debug = 0;
    for(int i = 0; i < argc; i++){
        if(strcmp(argv[i],"-p") == 0){
            serverPort = atoi(argv[i + 1]);
        }
        if(strcmp(argv[i],"-ip") == 0){
            serverIP = argv[i + 1];
        }
        if(strcmp(argv[i], "-d") == 0){
            debug = 1;
        }
    }
    if(strcmp(argv[1],"-s") == 0){
        cbuf_handle_t video_buffer = init_buffer();
        run_server(serverPort, video_buffer);
    }
    else if(strcmp(argv[1],"-c") == 0){
        if(debug == 1){
            run_test_client(serverIP, serverPort);
        }
        else{
            run_client(serverIP, serverPort);
        }
    }
}

void print_help(){
    printf("Server syntax\n");
    printf("./main.out -s -p PORT\n");
    printf("Example server:\n");
    printf("./main.out -s -p 1234\n");
    printf("Default server uses:\n");
    printf("Port: 1337\n");

    printf("\n");

    printf("Client syntax\n");
    printf("./main.out -c -ip IP -p PORT\n");
    printf("Additionally -d can be used for debug mode\n");
    printf("Example client:\n");
    printf("./main.out -c -ip 127.0.0.1 -p 1234\n");
    printf("Default client uses:\n");
    printf("IP: 127.0.0.1\n");
    printf("Port: 1337\n");
}