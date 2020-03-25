#ifndef FIFO_H
#define FIFO_H

#include "buffer.h"

int send_data_fifo(int data);
void mmap_fpga_peripherals();
void munmap_fpga_peripherals();
void open_physical_memory_device();
void close_physical_memory_device();
void* fifo_write_thread(void* buffer);

#endif // FIFO_H