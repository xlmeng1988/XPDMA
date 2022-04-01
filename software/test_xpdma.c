#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stddef.h>
#include "xpdma.h"
#include <sys/time.h>
#include <stdlib.h> // for rand()

#define TEST_SIZE   1024*1024*1024 // 1GB test data
// #define TEST_SIZE   (1024*1024*8) // 1MB test data
// #define TEST_SIZE   (16) // 16B test data
#define TEST_ADDR   0 // offset of DDR start address
#define BOARD_ID    0 // board number (for multiple boards)

int main(int argc, char *argv[]) {
    xpdma_t * fpga;
    uint32_t buf_size = TEST_SIZE;
    uint32_t addr_in = TEST_ADDR;
    uint32_t addr_out = TEST_ADDR;
    uint32_t c = 0;
    uint32_t err_count = 0;
    int mode = 1; // 0: simple dma; 1: sg

    char *data_in;
    char *data_out;

    unsigned int len = 0;
    struct timeval _timers[4];
    double time_ms[4];

    printf("Open FPGA: ");
    fpga = xpdma_open(BOARD_ID);
    if (NULL == fpga) {
        printf ("Failed to open XPDMA device\n");
        return 1;
    }
    printf("Successfull\n");

    data_in = (char *)malloc(buf_size);
    if (NULL == data_in) {
        printf ("Failed to allocate input buffer memory (size: %u bytes)\n", buf_size);
        xpdma_close(fpga);
        return 1;
    }

    data_out = (char *)malloc(buf_size);
    if (NULL == data_out) {
        printf ("Failed to allocate output buffer memory (size: %u bytes)\n", buf_size);
        xpdma_close(fpga);
        return 1;
    }

    printf("Fill input data: ");
    for (c = 0; c < buf_size; ++c) {
        // data_in[c] = (unsigned)(c % 128);
        // data_in[c] = (unsigned)(rand() % 128);
        data_in[c] = 65;
        // printf("%u ", data_in[c]);
        // if ( (c+1)%16 == 0) printf("\n");
    }
    // printf("Fill input data: %s ", data_in);
    printf("Ok\n");
    memset(data_out, 0, buf_size);

    if (argc > 1) {
        printf("Simple DMA mode!\n");
        mode = 0;
    } else {
        printf("SG DMA mode!\n");
    }

    printf("Send Data: ");
    gettimeofday(&_timers[0], NULL);
    if (mode)
        xpdma_send(fpga, data_in, buf_size, addr_in);
    else
        xpdma_write(fpga, data_in, buf_size);
    gettimeofday(&_timers[1], NULL);
    printf("Ok\n");

    printf("Receive Data: ");
    gettimeofday(&_timers[2], NULL);
    if (mode)
        xpdma_recv(fpga, data_out, buf_size, addr_out);
    else
        xpdma_read(fpga, data_out, buf_size);
    gettimeofday(&_timers[3], NULL);
    printf("Ok\n");

    printf("Close FPGA\n");
    xpdma_close(fpga);

    printf("Check Data: ");
    for (c = 0; c < buf_size; ++c)
        err_count += (data_in[c] != data_out[c]);

    if (err_count) {
        printf("%u errors\n", err_count);
        // for (c = 0; c < buf_size; c++)
        // {
        //     printf("%u ", data_out[c]);
        //     if ( (c+1)%16 == 0) printf("\n");
        // }
    } else
        printf("Ok\n");

    free(data_in);
    free(data_out);

    for (c = 0; c < 4; ++c)
        time_ms[c] =
                ((double)_timers[c].tv_sec*1000.0) +
                ((double)_timers[c].tv_usec/1000.0);

    printf("Send speed: %f MB/s (%f ms)\n", buf_size/(1024*1024)/((time_ms[1] - time_ms[0])/1000.0), (time_ms[1] - time_ms[0]));
    printf("Recv speed: %f MB/s (%f ms)\n", buf_size/1024/1024/((time_ms[3] - time_ms[2])/1000.0), (time_ms[3] - time_ms[2]));
    return 0;
}
