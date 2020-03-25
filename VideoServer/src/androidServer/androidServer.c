#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#define PORT 8080


void clientConnected(int sockfd){
  char buffer[256];
  int n;

  while (;;) {
    bzero(buffer,256);
    read(sockfd, buff, sizeof(buff));
    if (n < 0) error("ERROR reading from socket");
    printf("Here is the message: %s\n",buffer);

    bzero(buff, 256);
    n = 0;

    buff = "some message";

    // and send that buffer to client
    write(sockfd, buff, sizeof(buff));

    // if msg contains "Exit" then server exit and chat ended.
    if (strncmp("exit", buff, 4) == 0) {
        printf("Server Exit...\n");
        break;
    }

  }





}




int main() {

  int sockfd, connfd, len;
  struct sockaddr_in servaddr, client;


  //create TCP Socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if(sockfd == -1){
    printf("Failes to create socket\n" );
    exit(0);
  }
  printf("Socket was created succesfully ");

  //sets server buffer to zeros
  bzero(&servaddr, sizeof(servaddr));
  //setup server Socket
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(PORT);
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

  //binding socket to IP
  if ((bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))) < 0) {
      printf("Failed to bind socket");
      exit(0);
  }


  //listen for connection to socket
  if ((listen(sockfd, 5)) != 0) {
      printf("Listen failed");
      exit(0);
  }
  len = length(client);

  // Accept the data packet from client and verification
  connfd = accept(sockfd, (struct sockaddr *) &cli, &len);

  if (connfd <0) {
    printf("Error on accept");
    exit(0);
  }

  //function for connection
  clientConnected(connfd);

  //closing socket
  close(sockfd);


}
