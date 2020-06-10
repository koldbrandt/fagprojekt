#ifndef CLIENT_H
#define CLIENT_H

#include <pthread.h>

#define DUMMY_DATA_LEN 11
#define DEFAULT_SLEEP_TIME 1
#define SLEEP_TIME_INCREMENT 10000 // microseconds

enum client_options{
    RUN_DEBUG = 1,
    CLIENT_TCP = 2,
};

void init_client(char* serverIP, int serverPort);
void close_client(pthread_t listenThreadID);

void send_video_packet(char* data, short len);
void send_video_packet_tcp(char* data, short len);

void* client_listen_thread();
void* client_listen_thread_tcp();

void run_client(char* serverIP, int serverPort, int options);
void run_client_tcp(char* serverIP, int serverPort, int options);

void run_test_client(char* serverIP, int serverPort);
void run_test_client_tcp(char* serverIP, int serverPort);

void video_send_loop();
void video_send_loop_tcp();

#endif // CLIENT_H