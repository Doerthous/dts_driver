/**
  ******************************************************************************
  * \brief      w25qxx
  * \file       w25qxx.c
  * \author     doerthous
  * \date       2019-06-07
  * \details    
  ******************************************************************************
  */

#include "w25qxx.h"



#define W25QXX_CMD_READ_ID			0x90
#define W25QXX_CMD_WRITE_ENABLE		0x06
#define W25QXX_CMD_WRITE_DISABLE	0x04
#define W25QXX_CMD_READ_DATA		0x03
#define W25QXX_CMD_SECTOR_ERASE		0x20
#define W25QXX_CMD_CHIP_ERASE		0x60
#define W25QXX_CMD_READ_SR1			0x05
#define W25QXX_CMD_READ_SR2			0x35
#define W25QXX_CMD_READ_SR3			0x15
#define W25QXX_CMD_PAGE_PROGRAM		0x02



static inline void w25qxx_send_cmd(w25qxx_t *w25qxx, uint8_t cmd)
{
	spi_write(w25qxx->spi, &cmd, 1);
}

static inline void w25qxx_send_addr(w25qxx_t *w25qxx, uint32_t addr)
{
	uint8_t byte;
	
	
	byte = (addr & 0xFF0000) >> 16;
	spi_write(w25qxx->spi, &byte, 1);
	byte = (addr & 0xFF00  ) >>  8;
	spi_write(w25qxx->spi, &byte, 1);
	byte = (addr & 0xFF    );
	spi_write(w25qxx->spi, &byte, 1);
}

static uint16_t w25qxx_read_id(w25qxx_t *w25qxx)
{
	uint16_t id;
	
	
	gpio_clear(&w25qxx->cs);
	
	w25qxx_send_cmd(w25qxx, W25QXX_CMD_READ_ID);
	w25qxx_send_addr(w25qxx, 0x00000000);
	spi_read(w25qxx->spi, ((uint8_t *)&id), 1);
	id <<= 8;
	spi_read(w25qxx->spi, ((uint8_t *)&id), 1);
	
	gpio_set(&w25qxx->cs);
		
	return id;
}

static inline void w25qxx_write_enable(w25qxx_t *w25qxx)
{
	gpio_clear(&w25qxx->cs);
	w25qxx_send_cmd(w25qxx, W25QXX_CMD_WRITE_ENABLE);
	gpio_set(&w25qxx->cs);
}

static inline void w25qxx_write_disable(w25qxx_t *w25qxx)
{
	gpio_clear(&w25qxx->cs);
	w25qxx_send_cmd(w25qxx, W25QXX_CMD_WRITE_DISABLE);
	gpio_set(&w25qxx->cs);
}

static uint8_t w25qxx_read_sr1(w25qxx_t *w25qxx)
{
	uint8_t sr;
	
	
	gpio_clear(&w25qxx->cs);
	w25qxx_send_cmd(w25qxx, W25QXX_CMD_READ_SR1);
	spi_read(w25qxx->spi, &sr, 1);
	gpio_set(&w25qxx->cs);
	
	return sr;
}

static void w25qxx_wait(w25qxx_t *w25qxx)
{
	while (w25qxx_read_sr1(w25qxx) & 0x01);
}



static void w25qxx_page_program(w25qxx_t *w25qxx, uint32_t addr, 
    uint8_t *data, uint32_t size)
{
	w25qxx_write_enable(w25qxx);
	
	gpio_clear(&w25qxx->cs);
	w25qxx_send_cmd(w25qxx, W25QXX_CMD_PAGE_PROGRAM);
	w25qxx_send_addr(w25qxx, addr);
	spi_write(w25qxx->spi, data, size);
	gpio_set(&w25qxx->cs);
	
	w25qxx_write_disable(w25qxx);
	
	w25qxx_wait(w25qxx);
}

uint32_t w25qxx_write(w25qxx_t *w25qxx, uint32_t addr, 
    uint8_t *data, uint32_t size)
{
	uint32_t pwc;
	uint32_t _size = size;
	
	
	addr %= w25qxx->capacity;
	
	while (size) {
		pwc = 256 - (addr & 0xFF);
		pwc = pwc > size ? size : pwc;
		pwc = addr + pwc < w25qxx->capacity ? pwc : w25qxx->capacity - addr;
		w25qxx_page_program(w25qxx, addr, data, pwc);
		addr = (addr + pwc) % w25qxx->capacity;
		data += pwc;
		size -= pwc;
	}
	
	return _size;
}

uint32_t w25qxx_read(w25qxx_t *w25qxx, uint32_t addr, 
    uint8_t *buff, uint32_t size)
{	
	addr %= w25qxx->capacity;
	
	gpio_clear(&w25qxx->cs);
	
	w25qxx_send_cmd(w25qxx, W25QXX_CMD_READ_DATA);
	w25qxx_send_addr(w25qxx, addr);
	spi_read(w25qxx->spi, buff, size);
	
	gpio_set(&w25qxx->cs);
	
	return size;
}

int w25qxx_erase_sector(w25qxx_t *w25qxx, uint32_t addr)
{
	addr %= w25qxx->capacity;
	
	w25qxx_write_enable(w25qxx);
	
	gpio_clear(&w25qxx->cs);
	w25qxx_send_cmd(w25qxx, W25QXX_CMD_SECTOR_ERASE);
	w25qxx_send_addr(w25qxx, addr);
	gpio_set(&w25qxx->cs);
	
	w25qxx_write_disable(w25qxx);
	
	w25qxx_wait(w25qxx);
	
	return 1;
}


int w25qxx_erase_chip(w25qxx_t *w25qxx)
{	
	w25qxx_write_enable(w25qxx);
	
	gpio_clear(&w25qxx->cs);
	w25qxx_send_cmd(w25qxx, W25QXX_CMD_CHIP_ERASE);
	gpio_set(&w25qxx->cs);
	
	w25qxx_write_disable(w25qxx);
	
	w25qxx_wait(w25qxx);
	
	return 1;
}


int w25qxx_init(w25qxx_t *w25qxx)
{
	uint16_t id;
	int ret = 1;
	
	
	gpio_clear(&w25qxx->cs);
	id = w25qxx_read_id(w25qxx);
	gpio_set(&w25qxx->cs);	
	
	
	switch (id) {
		case 0xEF17:
			w25qxx->sector_size = 4096;
			w25qxx->capacity = 16 * 1024 * 1024;
			break;
		default:
			ret = 0;
	}
	
	return ret;
}

/****************************** Copy right 2019 *******************************/
