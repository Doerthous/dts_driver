/**
  ******************************************************************************
  * \brief      at24cxx
  * \details    at24c01/02/04/08/16
  *             capacity: 128x8(1K)/256x8(2K)/512x8(4K)/1024x8(8K)/2048x8(16K)
  * \file       at24cxx.c
  * \author     doerthous
  * \date       2019-08-26
  * \details    
  ******************************************************************************
  */

#include "at24cxx.h"
#include <lib/delay.h>


uint32_t at24cxx_read(at24cxx_t *at24cxx,
    uint32_t addr, uint8_t *buff, uint32_t size)
{
    int i = 0;
    
    
//	while (i2c_busy(at24cxx->i2c));
	
    if (size > 0) {
        i2c_start(at24cxx->i2c);
        i2c_7b_addr(at24cxx->i2c, at24cxx->address, 0);
        i2c_write(at24cxx->i2c, addr);
        i2c_start(at24cxx->i2c);
        i2c_7b_addr(at24cxx->i2c, at24cxx->address, 1);
        
        for (i = 0; i < size-1; ++i) {
            buff[i] = i2c_read(at24cxx->i2c, 1);
        }
        buff[i] = i2c_read(at24cxx->i2c, 0);
        
        i2c_stop(at24cxx->i2c); // stop condition
    }
    
    return size;
}

uint32_t at24cxx_write(at24cxx_t *at24cxx,
    uint32_t addr, uint8_t *data, uint32_t size)
{
    int poffset = 0;
    int twc = 0;
    int wc = 0;
    
    
//	while (i2c_busy(at24cxx->i2c));
	
    while (size > 0) {
        poffset = addr & (at24cxx->page_size-1);
        wc = size < at24cxx->page_size-poffset ? 
            size : at24cxx->page_size-poffset;

        i2c_start(at24cxx->i2c);
        i2c_7b_addr(at24cxx->i2c, at24cxx->address, 0);
        i2c_write(at24cxx->i2c, addr);
        
        twc += wc;
        addr += wc;
        size -= wc;
        
        while (wc-- > 0) {
            i2c_write(at24cxx->i2c, *data++);
        }
        
        i2c_stop(at24cxx->i2c);
        
		// at24cxx inner delay, see datasheet.
        delay_ms(5);
    }
    
    return twc;
}

/****************************** Copy right 2019 *******************************/
