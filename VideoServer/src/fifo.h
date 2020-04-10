#ifndef FIFO_H
#define FIFO_H

void mmap_fpga_peripherals();
void munmap_fpga_peripherals();
void open_physical_memory_device();
void close_physical_memory_device();
int send_data_fifo(char data);
int read_data_fifo(char* dataPtr);

#endif // FIFO_H