#ifndef CLIENT_H
#define CLIENT_H



#define DUMMY_DATA_LEN 11
#define DEFAULT_SLEEP_TIME 1
#define SLEEP_TIME_INCREMENT 10000 // microseconds

enum client_options{
    RUN_DEBUG = 1
};

void init_client(char* serverIP, int serverPort);
void close_client();
void read_fifo_blocking(char* data);

void send_video_packet(char* data, short len);
void empty_fifo();

void run_client(char* serverIP, int serverPort, int options);

void run_test_client(char* serverIP, int serverPort);

void video_send_loop();

#endif // CLIENT_H