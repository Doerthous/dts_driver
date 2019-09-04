/**
  ******************************************************************************
  * \brief      w25qxx
  * \file       w25qxx.h
  * \author     doerthous
  * \date       2019-06-07
  * \details    
  ******************************************************************************
  */

#ifndef W25QXX_H_
#define W25QXX_H_

#include <gpio.h>
#include <spi.h>

typedef struct
{
    gpio_t cs;
    spi_t *spi;

    uint32_t capacity; // Byte
    uint32_t sector_size; // Byte
} w25qxx_t;


int w25qxx_init(w25qxx_t *w25qxx);
// no erase before write
uint32_t w25qxx_write(w25qxx_t *w25qxx, 
    uint32_t addr, uint8_t *data, uint32_t size);
uint32_t w25qxx_read(w25qxx_t *w25qxx, 
    uint32_t addr, uint8_t *buff, uint32_t size);
int w25qxx_erase_sector(w25qxx_t *w25qxx, uint32_t addr);
int w25qxx_erase_chip(w25qxx_t *w25qxx);

#endif /* W25QXX_H_ */

/****************************** Copy right 2019 *******************************/
