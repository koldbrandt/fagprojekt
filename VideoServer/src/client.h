#ifndef CLIENT_H
#define CLIENT_H

enum client_options{
    RUN_DEBUG = 1,
    PROTOCOL_TCP = 2,
};

void run_client(char* serverIP, int serverPort, int options);
void run_test_client(char* serverIP, int serverPort);
void send_video_packet(char* data, short len);
void init_client(char* serverIP, int serverPort);
void close_client();
void video_send_loop();
void* client_listen_thread();

#endif // CLIENT_H