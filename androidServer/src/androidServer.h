#ifndef ANDROIDSERVER_H
#define ANDROIDSERVER_H
#define MAX 64 
#define PORT 8080
#define SA struct sockaddr

void startServer();

void serverReadFunc(int connfd2);

void serverRead(void *threadID);

void serverSendFunc(int connfd);

void serverSend(void *threadID);
#endif
