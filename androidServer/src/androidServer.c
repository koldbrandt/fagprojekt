#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include "androidServer.h"

int sockfd, connfd, len, connfd2, connected;
struct sockaddr_in servaddr, cli;


void startServer(){
    // socket create and verification 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("socket creation failed. \n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully created. \n"); 
    bzero(&servaddr, sizeof(servaddr)); 
  
    // assign IP, PORT 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(PORT); 
  
    //Reuse address on reconnect.
    int reuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed. \n");

    //Reuse port on reconnect.
    #ifdef SO_REUSEPORT
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(reuse)) < 0) 
        perror("setsockopt(SO_REUSEPORT) failed. \n");
    #endif

    // Binding newly created socket to given IP and verification 
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
        printf("socket bind failed.\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully binded.\n"); 
  
    // Now server is ready to listen and verification 
    if ((listen(sockfd, 5)) != 0) { 
        printf("Listen failed.\n"); 
        exit(0); 
    } 
    else
        printf("Server listening.\n"); 
    len = sizeof(cli); 

    // Accept the data packet from client and verification
    // This is for the sending server thread.
    connfd = accept(sockfd, (SA*)&cli, &len); 
    if (connfd < 0) { 
        printf("Send server thread acccept failed.\n"); 
    } else
        printf("Send server thread acccept the client.\n");
    connected = 1;
}

void serverReadFunc(int connfd2){
    char buff[MAX];
    bzero(buff, MAX); 
    
    int valread;
    if(valread = read(connfd2, buff, sizeof(buff)) > 0){
        // Upon closing the app it sends "EXIT"
        // Match it, and close all connects and restart server if that is the case.
        if(strcmp("EXIT",buff) != 0){
            //If not, put something in buffer.
            //For now just printing a message set in android studio on the other side.
            printf("From APP: %s\n", buff);
        } else {
            //Close connections & restart
            printf("ANDROID APP CLOSED: %s\n", buff);
            close(sockfd);
            close(connfd);
            connected = 0;
            startServer();
        }
    } 
}

void serverRead(void *threadID){
    long tid;
    tid = (long)threadID;
    printf("Created Read Server Thread! \n");
    // Accept the data packet from client and verification
    while(1){
        while(connected){
        //Accept connect as it closes everytime for reading.
        connfd2 = accept(sockfd, (SA*)&cli, &len); 
        if (connfd2 < 0) { 
            printf("Read server thread acccept failed. \n"); 
        } else {
            printf("Read server thread acccept the client. \n");
        }
        serverReadFunc(connfd2);
        //close this connection again
        close(connfd2);
    }
}
    pthread_exit(NULL);
}

void serverSendFunc(int connfd){
    char buff[MAX];
    bzero(buff, MAX); 
    int n;
    bzero(buff, sizeof(buff)); 
    // For now enter a message to send manually, should be read from buffer.
    // 00000001[message] to send to first parameter
    // 00000010[message] to send to second parameter
    // 00000011 to confirm changed encryption key
    printf("Enter the string : "); 
    n = 0; 
    while ((buff[n++] = getchar()) != '\n');

    //Send precode and message
    write(connfd, buff, sizeof(buff));
}

void serverSend(void *threadID){
    long tid;
    tid = (long)threadID;
    printf("Created Send Server Thread! \n");
    while(1){
        while(connected){
        serverSendFunc(connfd);
        }
    }
    pthread_exit(NULL);
}

// Driver function 
int main() { 
pthread_t threads[2];

//Start server
startServer();

int t1;
t1 = pthread_create(&threads[0], NULL, serverRead, (void *)0);
if (t1){
    printf("Unable to create read thread\n");
}

int t2;
t2 = pthread_create(&threads[1], NULL, serverSend, (void *)1);
if (t2){
    printf("Unable to create send thread\n");
}
pthread_exit(NULL);

} 
