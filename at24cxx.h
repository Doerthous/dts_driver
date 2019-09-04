/**
  ******************************************************************************
  * \brief      at24cxx
  * \details    at24c01/02/04/08/16
  *             capacity: 128x8(1K)/256x8(2K)/512x8(4K)/1024x8(8K)/2048x8(16K)
  * \file       at24cxx.h
  * \author     doerthous
  * \date       2019-08-26
  * \details    
  ******************************************************************************
  */

#ifndef AT24CXX_H_
#define AT24CXX_H_

#include <i2c.h>

typedef struct 
{
    i2c_t *i2c;
    uint32_t page_size; // Byte
    uint32_t address;
    uint32_t capacity; // Byte
} at24cxx_t;

uint32_t at24cxx_read(at24cxx_t *at24cxx,
    uint32_t addr, uint8_t *buff, uint32_t size);
uint32_t at24cxx_write(at24cxx_t *at24cxx,
    uint32_t addr, uint8_t *data, uint32_t size);

#endif /* AT24CXX_H_ */

/****************************** Copy right 2019 *******************************/
