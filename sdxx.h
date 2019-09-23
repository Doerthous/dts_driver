/**
  ******************************************************************************
  * \brief      sd card driver (sdsc, sdhc, sdxc)
  * \file       sdxx.h
  * \author     doerthous
  * \date       2019-09-19
  * \details    
  ******************************************************************************
  */

#ifndef SDXX_H_
#define SDXX_H_

#include <stdint.h>


// -----------------------------------------------------------------------------
// basic interfaces
enum
{
    SDXX_OK = 0,
    SDXX_ERROR,
    SDXX_TIMEOUT,
    SDXX_CRC_ERROR,
    SDXX_INVALID_ARG,
    SDXX_NOT_SUPPORT,
    //
    SDXX_STATE_CODE_COUNT,
};

typedef struct sdxx
{
    #define SDXX_SDSC   0
    #define SDXX_SDHC   1
    #define SDXX_SDXC   2
    uint8_t type;
    uint64_t capacity;
    uint32_t block_size;
    uint32_t relative_card_addr;

    uint8_t bus_width;
    uint32_t frequency;

    uint32_t cid[4];
    uint32_t csd[4];

    // interfaces
    uint32_t (*read_block)(struct sdxx *sdxx, 
        uint32_t idx, uint32_t cnt, uint8_t *data);
    uint32_t (*write_block)(struct sdxx *sdxx, 
        uint32_t idx, uint32_t cnt, uint8_t *data);

    //
    void (*md_init)(struct sdxx *sdxx);
    int (*config)(struct sdxx *sdxx);

    int (*ask)(struct sdxx *sdxx, 
        uint8_t cmd, uint32_t arg, uint8_t *res, uint32_t size);

    void (*recv)(struct sdxx *sdxx, uint8_t *buff, uint64_t size);
    void (*send)(struct sdxx *sdxx, uint8_t *data, uint64_t size);
    int (*transfer_end)(struct sdxx* sdxx); // int check(sdxx, flag)
} sdxx_t;

int sdxx_init(sdxx_t *sdxx);
int sdxx_select(sdxx_t *sdxx);
uint32_t sdxx_read_block(sdxx_t *sdxx, 
    uint32_t idx, uint32_t cnt, uint8_t *data);
uint32_t sdxx_write_block(sdxx_t *sdxx,
    uint32_t idx, uint32_t cnt, uint8_t *data);



// -----------------------------------------------------------------------------
// extend interfaces
typedef struct sdxx_cid
{
    uint32_t :1;
    uint32_t crc: 7;
    uint32_t manufact_date: 12; /*!< Manufacturing Date */
    uint32_t :4;
    uint32_t producv_sn2: 8; /*!< Product Serial Number */

    uint32_t producv_sn1: 24; /*!< Product Serial Number */   
    uint32_t product_ver: 8; /*!< Product Revision */

    uint32_t product_name2; /*!< Product Name part2 */

    uint32_t product_name1: 8; /*!< Product Name part1 */
    uint32_t oem_app_id: 16; /*!< OEM/Application ID */
    uint32_t manufacturer_id: 8; /*!< ManufacturerID */
} sdxx_cid_t;

typedef struct sdxx_csd
{
    uint32_t csd_structure: 2; 
    uint32_t read_bl_len: 4; /*< max. read data block length */
    uint32_t c_size_mul: 3; /*< device size multiplier, only in csd.v1 */
    uint32_t tran_speed: 8;        
    uint32_t c_size; /*< device size */
} sdxx_csd_t;

typedef struct sdxx_scr
{
    uint64_t scr_structure: 4;

    uint64_t sd_spec: 4;
    uint64_t sd_specx: 4;
    uint64_t sd_spec4: 1;
    uint64_t sd_spec3: 1;    

    uint64_t sd_bus_widths: 4;

    uint64_t data_stat_after_erase: 1;

    uint64_t sd_security: 3;
    uint64_t ex_security: 4;

    uint64_t cmd_support: 4;
} sdxx_scr_t;

typedef struct sdxx_info
{
    sdxx_cid_t cid;
    sdxx_csd_t csd;
    sdxx_scr_t scr;
} sdxx_info_t;

int sdxx_get_info(sdxx_t *sdxx, sdxx_info_t *info);



enum SDXX_RX_MODE
{
    SDXX_RX_DMA_SINGLE_BLK_ITER,
    SDXX_RX_DMA_MULTI_BLK,
};

enum SDXX_TX_MODE
{
    SDXX_TX_DMA_SINGLE_BLK_ITER,
    SDXX_TX_DMA_MULTI_BLK,
};

enum SDXX_CMD
{
    // (enum SDXX_RX_MODE)
    SDXX_CMD_RX_MODE,
    
    // (enum SDXX_TX_MODE)
    SDXX_CMD_TX_MODE,
};

int sdxx_config(sdxx_t *sdxx, int cmd, ...);

#endif /* SDXX_H_ */

/****************************** Copy right 2019 *******************************/
