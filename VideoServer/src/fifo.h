#ifndef FIFO_H
#define FIFO_H

enum fifo_status{
    FIFO_FULL,
    FIFO_EMPTY
};

int send_data_fifo(char* data, int dataLen);


#endif // FIFO_H