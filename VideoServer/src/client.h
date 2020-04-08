#ifndef CLIENT_H
#define CLIENT_H

void run_client(char* serverIP, int serverPort);
void run_test_client(char* serverIP, int serverPort);
void print_init(char* serverIP, int serverPort);
void send_video_packet(char* data, short len);
void video_send_loop();

#endif // CLIENT_H