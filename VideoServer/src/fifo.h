#ifndef FIFO_H
#define FIFO_H

enum fifo_status{
    FIFO_FULL,
    FIFO_EMPTY
};

int send_data_fifo(char* data, int dataLen);
void mmap_fpga_peripherals();
void munmap_fpga_peripherals();
void open_physical_memory_device();
void close_physical_memory_device();

#endif // FIFO_H