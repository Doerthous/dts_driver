/**
  ******************************************************************************
  * \brief      
  * \file       sdxx_test.c
  * \author     doerthous
  * \date       2019-09-21
  * \details    
  ******************************************************************************
  */

#include "sdxx.h"
#include <stdlib.h>
#include <string.h>

#define USING_UART_PRINTF

#if defined(USING_UART_PRINTF)
  #include <lib/uart_printf.h>
  #define printf(...) uart_printf(&uart1, ##__VA_ARGS__)
#endif

int sdxx_test(sdxx_t *sdxx)
{
    static uint8_t buf1[1024];
    static uint8_t buf2[1024];    
    uint64_t ret, i, j;
    
    
    ret = sdxx_init(sdxx);
    printf("ret: %d\n", ret);    
    printf("bus: %d\n", sdxx->bus_width);


    for (i = 1; i < 20; i += 2) {
        for (j = 0; j < 1024; ++j) {
            buf1[j] = (uint8_t)rand();
        }
        ret = sdxx_write_block(sdxx, i, 2, buf1);
        printf("w(%d) ", ret);

        ret = sdxx_read_block(sdxx, i, 2, buf2);
        printf("r(%d) ", ret);

        if (memcmp(buf1, buf2, 1024) == 0) {
            printf("%d ok\n", i);
        }
        else {
            printf("%d error\n", i);
        }
    }

    return i == 512;
}

/****************************** Copy right 2019 *******************************/
