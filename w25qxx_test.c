#include <stdlib.h>
#include <string.h>

#include "w25qxx.h"

int w25qxx_test(w25qxx_t *w25qxx)
{
    static uint8_t buff1[4096];
    static uint8_t buff2[4096];
    uint8_t tc = 10;
    uint32_t addr, i;
    
    
    if (w25qxx_init(w25qxx)) {
        while (tc) {
            addr = rand() % w25qxx->capacity;
            if (w25qxx_erase_sector(w25qxx, addr) 
                && w25qxx_erase_sector(w25qxx, addr + w25qxx->sector_size)) {
                for (i = 0; i < 4096; ++i) {
                    buff1[i] = rand();
                }
                memset(buff2, 0, 4096);
                
                w25qxx_write(w25qxx, addr, buff1, 4096);
                w25qxx_read(w25qxx, addr, buff2, 4096);
                
                if (memcmp(buff1, buff2, 4096) != 0) {
                    break;
                }
            }
            --tc;
        }
    }
    
    return tc == 0;
}
