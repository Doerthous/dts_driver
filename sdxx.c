/**
  ******************************************************************************
  * \brief      sd card driver (sdsc, sdhc, sdxc)
  * \file       sdxx.c
  * \author     doerthous
  * \date       2019-09-19
  * \details    
  ******************************************************************************
  */


#include "sdxx.h"
#include <lib/delay.h>


//# Debug
//#define DEBUG
#ifdef DEBUG
  #include <lib/uart_printf.h>
  #define COMMA ,
  #define PRINTF(...) uart_printf(&uart1, ##__VA_ARGS__)
  static void __DUMP_STATUS(sdxx_t *sdxx, const char *func) 
  {
    int status;
    static const char *snm[] = { 
        "idle", "ready", "ident", "stby", "tran", "data", "rcv", "prg", "dis",
    };


    int sdxx_get_status(sdxx_t *sdxx, uint32_t *status);
    sdxx_get_status(sdxx, (uint32_t *)&status);
    PRINTF("%s status: %08X, ", func, status);
    status >>= 9;
    status &= 0x0F;
    if (status < 9) {
        PRINTF("%s\n", snm[status]);
    }
    else {
        PRINTF("reserved\n");
    }
  }
  static void DUMP_SCR(sdxx_scr_t *scr) {
    PRINTF("scr structure: %d\n", scr->scr_structure);
    PRINTF("scr sd_spec: %d\n", scr->sd_spec);  
    PRINTF("scr sd_spec3: %d\n", scr->sd_spec3);
    PRINTF("scr sd_spec4: %d\n", scr->sd_spec4);
    PRINTF("scr sd_specx: %d\n", (scr->sd_specx));  
    PRINTF("scr sd_bus_widths: %d\n", scr->sd_bus_widths);
    PRINTF("scr data_stat_after_erase: %d\n", scr->data_stat_after_erase);
    PRINTF("scr sd_security: %d\n", scr->sd_security);
    PRINTF("scr ex_security: %d\n", scr->ex_security); 
    PRINTF("scr cmd_support: %d\n", scr->cmd_support);
  }
  #define DUMP_STATUS(sdxx) __DUMP_STATUS(sdxx, __func__)
#else
  #define COMMA
  #define PRINTF(...)
  #define DUMP_STATUS(sdxx)
  #define DUMP_SCR(scr)
#endif


//# Command
//## Basic Command
#define SDXX_CMD0_N_GO_IDLE_STATE                                           0
#define SDXX_CMD2_R2_ALL_SEND_CID                                           2
#define SDXX_CMD3_R6_SEND_RELATIVE_ADDR                                     3
#define SDXX_CMD7_R1_SEL_DESEL_CARD                                         7
#define SDXX_CMD8_R7_SEND_IF_COND                                           8
#define SDXX_CMD9_R2_SEND_CSD                                               9
#define SDXX_CMD12_R1_STOP_TRANSTER                                         12
#define SDXX_CMD13_R1_SEND_STATUS_TASK_STATUS                               13
#define SDXX_CMD17_R1_READ_SINGLE_BLOCK                                     17
#define SDXX_CMD18_R1_READ_MULTIPLE_BLOCK                                   18
#define SDXX_CMD24_R1_WRITE_SINGLE_BLOCK                                    24
#define SDXX_CMD25_R1_WRITE_MULTIPLE_BLOCK                                  25
#define SDXX_CMD55_R1_APP_CMD                                               55
//## APP Command
#define SDXX_ACMD6_R1_SET_BUS_WIDTH                                         6
#define SDXX_ACMD23_R1_SET_BLOCK_COUNT                                      23
#define SDXX_ACMD41_R3_SD_SEND_OP_COND                                      41
#define SDXX_ACMD51_R1_SEND_SCR                                             51


//# Response
// start bit(sb), stop bit{pb), transmition bit(tb), command index(ci) and crc.
// rv: reserve
// va: voltage accepted
// check pattern: cp
// rca: relative card address
// csb: card status bit
// 
// array unit: bit.
// 
// +-------------------------------------------------------------------------+
// |res |sb |tb |ci      |data                                  |crc     |pb |
// +-------------------------------------------------------------------------+
// |R1  |0  |0  |xxxxxx  |card_state[32]                        |xxxxxx  |1  |
// |R2  |0  |0  |111111  |cid or csd[120]                       |xxxxxx  |1  |
// |R3  |0  |0  |111111  |ocr[32]                               |111111  |1  |
// |R6  |0  |0  |111111  |rca[16], csb[16]                      |111111  |1  |
// |R7  |0  |0  |001000  |rv[20],va[4],cp[8]                    |xxxxxx  |1  |
// +----+---+---+--------+--------------------------------------+--------+---+

/**
 * @brief      check the "return" of sdxx.ask and response "R1"
 *
 * @param      ret   the "return" of sdxx.ask
 * @param      r1    R1
 *
 * @return     return ret if ret != SDXX_OK, else return SDXX_ERROR if R1 has
 *             error, else return SDXX_OK
 */
#define r1_check(ret, r1) \
    (((ret) != SDXX_OK) ? \
        (PRINTF("%s: ret: %d\n", __func__, (ret))COMMA (ret)) : \
        (SDXX_CS_HAS_ERROR((r1)) ? \
            (PRINTF("%s: resp: %08X\n", __func__, (r1))COMMA SDXX_ERROR) : \
            SDXX_OK))


//# Card Status
//## Errors
#define SDXX_CS_CMD_CRC_ERROR                                          (1 << 23)
#define SDXX_CS_HAS_ERROR(status) ((status)&0xFDFFE008)
//## Misc
#define SDXX_CS_RDY_FOR_DAT                                             (1 << 8)
//## States
#define SDXX_CS_IDLE                                                           0
#define SDXX_CS_READY                                                          1
#define SDXX_CS_IDENT                                                          2
#define SDXX_CS_STBY                                                           3
#define SDXX_CS_TRAN                                                           4
#define SDXX_CS_DATA                                                           5
#define SDXX_CS_RCV                                                            6
#define SDXX_CS_PRG                                                            7
#define SDXX_CS_DIS                                                            8
#define SDXX_CS_CUR_STA(status) (((status)&(0x0F << 9))>>9)


//# Command Util
static int sdxx_get_status(sdxx_t *sdxx, uint32_t *status)
{
    return sdxx->ask(sdxx, 
        SDXX_CMD13_R1_SEND_STATUS_TASK_STATUS, sdxx->relative_card_addr<<16,
        (uint8_t *)status, 4);
}

static int sdxx_send_app_command(sdxx_t *sdxx, 
    uint8_t cmd, uint32_t arg, uint8_t *resp, uint32_t size)
{
    int ret;


    ret = sdxx->ask(sdxx, 
        SDXX_CMD55_R1_APP_CMD, sdxx->relative_card_addr<<16,
        resp, 4);

    ret = r1_check(ret, *resp);
    if (ret != SDXX_OK) {    
        return ret;
    }

    return sdxx->ask(sdxx, cmd, arg, resp, size);
}

static int sdxx_set_bus_width(sdxx_t *sdxx, int width)
{
    uint32_t resp;
    int ret;


    if (width == 1) {
        width = 0x00;
    }
    else if (width == 4) {
        width = 0x02;
    }
    else {
        return SDXX_INVALID_ARG;
    }

    ret = sdxx_send_app_command(sdxx, 
        SDXX_ACMD6_R1_SET_BUS_WIDTH, width, 
        (uint8_t *)&resp, 4);

    return r1_check(ret, resp);
}

static int sdxx_get_scr(sdxx_t *sdxx, uint8_t *scr)
{
    uint32_t resp;
    int ret;


    sdxx->recv(sdxx, scr, 8);

    ret = sdxx_send_app_command(sdxx, SDXX_ACMD51_R1_SEND_SCR, 0, 
        (uint8_t *)&resp, 4);
    
    ret = r1_check(ret, resp);
    if (ret != SDXX_OK) {    
        return ret;
    }

    ret = 3;
    while (sdxx->transfer_end(sdxx) != SDXX_OK) {
        DUMP_STATUS(sdxx);
        delay_us(250);
        ret -= 1;
        if (ret == 0) {
            return SDXX_ERROR;
        }
    }

    return SDXX_OK;    
}


//# Initialize and Identify
static int sdxx_initialize(sdxx_t *sdxx)
{
    uint32_t resp;
    uint8_t valid_vol = 0;
    uint32_t hcs = 0;
    int retry = 200;
    int ret;
    // argument for CMD8
    #define SD_VHS (1<<8)
    #define SD_CHECK_PATTERN (0xAA)
    // argument for ACMD41
    #define SD_HCS (1 << 30)
    #define SD_VDD_WIN (1 << 20) // OCR 20bit, 3.2 ~ 3.3V


    //
    ret = sdxx->ask(sdxx, 
        SDXX_CMD0_N_GO_IDLE_STATE, 0, 0, 0);
    if (ret != SDXX_OK) {
        return SDXX_ERROR; // not a valid sd card.
    }

    //
    ret = sdxx->ask(sdxx, 
        SDXX_CMD8_R7_SEND_IF_COND, SD_VHS|SD_CHECK_PATTERN,
        (uint8_t *)&resp, 4);
    if (ret == SDXX_OK) {
        if (resp != (SD_VHS|SD_CHECK_PATTERN)) {
            return SDXX_ERROR; // voltage supply not accepted.
        }
        hcs = SD_HCS; // tell card that host support high-capacity.
    }

    //
    while (!valid_vol) {
        ret = sdxx_send_app_command(sdxx, 
            SDXX_ACMD41_R3_SD_SEND_OP_COND, SD_VDD_WIN|hcs,
            (uint8_t *)&resp, 4);

        if (ret == SDXX_ERROR || ret == SDXX_TIMEOUT) {
            return SDXX_ERROR;
        }

        valid_vol = (((resp>>31) == 1) ? 1 : 0);

        if (--retry == 0) {
            return SDXX_ERROR; // initialize failed.
        }
    }

    if (resp & (1<<30)) {
        sdxx->type = SDXX_SDHC;
    }

    return SDXX_OK;
}

static int sdxx_identify(sdxx_t *sdxx)
{
    uint32_t resp;
    int ret;


    // if S18R == 1 && S18A == 1
    //      CMD11
    
    //
    ret = sdxx->ask(sdxx, 
        SDXX_CMD2_R2_ALL_SEND_CID, 0,
        (uint8_t *)sdxx->cid, 16);
    if (ret != SDXX_OK) {
        return ret; // not sd card.
    }

    // 
    ret = sdxx->ask(sdxx, 
        SDXX_CMD3_R6_SEND_RELATIVE_ADDR, 0,
        (uint8_t *)&resp, 4);
    if (ret != SDXX_OK) {
        return ret; // not sd card.
    }
    sdxx->relative_card_addr = (resp >> 16);

    //
    ret = sdxx->ask(sdxx, 
        SDXX_CMD9_R2_SEND_CSD, sdxx->relative_card_addr<<16,
        (uint8_t *)sdxx->csd, 16);
    if (ret != SDXX_OK) {
        return ret; // not sd card.
    }

    return ret;
}


//# Data Transter
/**
 * @brief      move card's state move to "tran".
 *
 * @param      sdxx     sdxx
 * @param[in]  timeout  timeout, ms
 *
 * @return     SDXX_OK when successed, else SDXX_ERROR
 */
static int sdxx_move_to_tran(sdxx_t *sdxx, int timeout)
{
    uint32_t state, resp;


    sdxx_get_status(sdxx, &state);
    while ((SDXX_CS_CUR_STA(state) != SDXX_CS_TRAN) && timeout > 0) {
        if ((SDXX_CS_CUR_STA(state) == SDXX_CS_RCV)
        || (SDXX_CS_CUR_STA(state) == SDXX_CS_DATA)) {
            sdxx->ask(sdxx, 
                SDXX_CMD12_R1_STOP_TRANSTER, 0,
                (uint8_t *)&resp, 4);
        }
        else if (SDXX_CS_CUR_STA(state) == SDXX_CS_STBY) {
            sdxx_select(sdxx);
        }

		timeout -= 1;
        delay_ms(1);
        sdxx_get_status(sdxx, &state);
    }

    DUMP_STATUS(sdxx);
    return timeout > 0 ? SDXX_OK : SDXX_ERROR;    
}

/**
 * @brief      read single block using CMD17
 *
 * @param      sdxx  sdxx
 * @param[in]  idx   block index, [0, max]
 * @param      buff  buff
 *
 * @return     the number of blocks read
 */
static uint32_t sdxx_read_single_block(sdxx_t *sdxx,
    uint32_t idx, uint8_t *buff)
{
    uint32_t resp;
    int ret;


    if (sdxx_move_to_tran(sdxx, 500) != SDXX_OK) {
        return 0;
    }

    sdxx->recv(sdxx, buff, sdxx->block_size);
    ret = sdxx->ask(sdxx, 
        SDXX_CMD17_R1_READ_SINGLE_BLOCK, idx,
        (uint8_t *)&resp, 4);
    if (ret != SDXX_OK) {
        PRINTF("%s: ret: %d resp: %08X\n", __func__, ret, resp);
        return 0;
    }
    while (sdxx->transfer_end(sdxx) != SDXX_OK);

    return 1;
}

/**
 * @brief      write single block using COM24
 *
 * @param      sdxx  sdxx
 * @param[in]  idx   block index, [0, max]
 * @param      data  data
 *
 * @return     the number of blocks written
 */
static uint32_t sdxx_write_single_block(sdxx_t *sdxx,
    uint32_t idx, uint8_t *data)
{
    uint32_t resp;
    int ret;


    if (sdxx_move_to_tran(sdxx, 500) != SDXX_OK) {
        return 0;
    }

    ret = sdxx->ask(sdxx, 
        SDXX_CMD24_R1_WRITE_SINGLE_BLOCK, idx,
        (uint8_t *)&resp, 4);
    if (ret != SDXX_OK) {
        PRINTF("%s: ret: %d resp: %08X\n", __func__, ret, resp);
        return 0;
    }
    sdxx->send(sdxx, data, sdxx->block_size);
    while (sdxx->transfer_end(sdxx) != SDXX_OK);

    return 1;
}

/**
 * @brief      read multi block using sdxx_read_single_block
 *
 * @param      sdxx  sdxx
 * @param[in]  idx   block index, [0, max]
 * @param[in]  cnt   the number of blocks need to read from card
 * @param      buff  buff
 *
 * @return     the number of blocks read
 */
static uint32_t sdxx_read_multi_block_iter(sdxx_t *sdxx, 
    uint32_t idx, uint32_t cnt, uint8_t *buff)
{
    uint32_t i;


    for (i = 0; i < cnt; ++i) {
        if (!sdxx_read_single_block(sdxx, idx, buff)) {
            return i;
        }

        idx += 1;
        buff += sdxx->block_size;
    }

    return i;    
}

/**
 * @brief      write multi block using sdxx_write_single_block
 *
 * @param      sdxx  sdxx
 * @param[in]  idx   block index, [0, max]
 * @param[in]  cnt   the number of blocks need to write to card
 * @param      data  data
 *
 * @return     the number of blocks written
 */
static uint32_t sdxx_write_multi_block_iter(sdxx_t *sdxx,
    uint32_t idx, uint32_t cnt, uint8_t *data)
{
    uint32_t i;
    

    for (i = 0; i < cnt; ++i) {
        if (!sdxx_write_single_block(sdxx, idx, data)) {
            break;
        }
        
        idx += 1;
        data += sdxx->block_size;       
    }

    return i;
}

/**
 * @brief      read multi block using CMD18
 *
 * @param      sdxx  sdxx
 * @param[in]  idx   block index, [0, max]
 * @param[in]  cnt   the number of blocks need to read from card
 * @param      buff  buff
 *
 * @return     the number of blocks read
 */
static uint32_t sdxx_read_multi_block(sdxx_t *sdxx,
    uint32_t idx, uint32_t cnt, uint8_t *buff)
{
    uint32_t resp;
    int ret;

    if (sdxx_move_to_tran(sdxx, 500) != SDXX_OK) {
        return 0;
    }

    sdxx->recv(sdxx, buff, sdxx->block_size * cnt);    
    ret = sdxx->ask(sdxx, 
        SDXX_CMD18_R1_READ_MULTIPLE_BLOCK, idx,
        (uint8_t *)&resp, 4);
    if (ret != SDXX_OK) {
        PRINTF("%s: ret: %d resp: %08X\n", __func__, ret, resp);
        return 0;
    }
    while (sdxx->transfer_end(sdxx) != SDXX_OK);
    
    return cnt;
}

/**
 * @brief      write multi block using CMD25
 *
 * @param      sdxx  sdxx
 * @param[in]  idx   block index, [0, max]
 * @param[in]  cnt   the number of blocks need to write to card
 * @param      data  data
 *
 * @return     the number of blocks written
 *
 * @note       will card be busy if we send a large amount of data to it?
 */
static uint32_t sdxx_write_multi_block(sdxx_t *sdxx,
    uint32_t idx, uint32_t cnt, uint8_t *data)
{
    uint32_t resp;
    int ret;


    if (sdxx_move_to_tran(sdxx, 500) != SDXX_OK) {
        return 0;
    }

    // use this pre-erase command before CMD25 could improve performance.
    sdxx_send_app_command(sdxx,
        SDXX_ACMD23_R1_SET_BLOCK_COUNT, cnt,
        (uint8_t *)&resp, 4);

    //
    ret = sdxx->ask(sdxx, 
        SDXX_CMD25_R1_WRITE_MULTIPLE_BLOCK, idx,
        (uint8_t *)&resp, 4);
    if (ret != SDXX_OK) {
        PRINTF("%s: ret: %d resp: %08X\n", __func__, ret, resp);
        return 0;
    }
    sdxx->send(sdxx, data, sdxx->block_size * cnt);
    while (sdxx->transfer_end(sdxx) != SDXX_OK);
    
    return cnt;
}


//# Basic Interface
int sdxx_init(sdxx_t *sdxx)
{
    int ret;
    sdxx_info_t info;
    #define SD_WIDE_BUS 0x04  


    //
    sdxx->md_init(sdxx);
    
    //
    if ((ret = sdxx_initialize(sdxx)) != SDXX_OK) {
        return ret;
    }

    //
    if ((ret = sdxx_identify(sdxx)) != SDXX_OK) {
        return ret;
    }

    // sdxx config
    /// set bus frequence to data transfer mode's frequency.
    sdxx->frequency = sdxx->frequency < 400000 ? 400000 : sdxx->frequency;
    sdxx->bus_width = 1;
    if (sdxx->config(sdxx) != SDXX_OK) {
        return SDXX_ERROR;
    }
    DUMP_STATUS(sdxx);
    sdxx_select(sdxx);
    DUMP_STATUS(sdxx);

    sdxx_get_info(sdxx, &info);
    DUMP_STATUS(sdxx);

    /// try to set to wide bus mode
    if (info.scr.sd_bus_widths & SD_WIDE_BUS) {
        sdxx->bus_width = 4;
        if (sdxx->config(sdxx) == SDXX_OK) {
            if (sdxx_set_bus_width(sdxx, 4) != SDXX_OK) {
                DUMP_STATUS(sdxx);
                sdxx->bus_width = 1;
                sdxx->config(sdxx);
            }
        }
        DUMP_STATUS(sdxx);        
    }

    /// set default read/write interface
    sdxx->read_block = sdxx_read_multi_block_iter;
    sdxx->write_block = sdxx_write_multi_block_iter;

    return SDXX_OK;
}

int sdxx_select(sdxx_t *sdxx)
{
    uint32_t resp;
    int ret;


    ret = sdxx->ask(sdxx, 
        SDXX_CMD7_R1_SEL_DESEL_CARD, sdxx->relative_card_addr<<16,
        (uint8_t *)&resp, 4);

    return r1_check(ret, resp);
}

uint32_t sdxx_read_block(sdxx_t *sdxx, 
    uint32_t idx, uint32_t cnt, uint8_t *buff)
{
    return sdxx->read_block(sdxx, idx, cnt, buff);
}

uint32_t sdxx_write_block(sdxx_t *sdxx,
    uint32_t idx, uint32_t cnt, uint8_t *data)
{
    return sdxx->write_block(sdxx, idx, cnt, data);
}


//# Extend Interface
#include <string.h>
int sdxx_get_info(sdxx_t *sdxx, sdxx_info_t *info)
{
    uint8_t buf[8];


    // cid
    /// get cid (this is done when init sd card)
    /// unpack cid
    memcpy(&info->cid, sdxx->cid, 16); // auto unpack


    // csd
    /// get csd (this is done when init sd card)
    /// unpack csd
    info->csd.csd_structure = (sdxx->csd[3] >> 30);
    info->csd.tran_speed = (sdxx->csd[3] & 0xFF); // [96,103] [3][0, 7]
    info->csd.read_bl_len = ((sdxx->csd[2] >> 16) & 0x0F); // [80,83] [2][16,19]
    if (info->csd.csd_structure == 0) {
        info->csd.c_size = (sdxx->csd[1] >> 30); // [62,63] [1][30,31]
        info->csd.c_size |= ((sdxx->csd[2] & 0x4FF) << 2); // [64,73] [2][0,9]
        // [47,49] [1][15,17]
        info->csd.c_size_mul = ((sdxx->csd[1] >> 15) & 0x07); 
        // todo compute capacity
    }
    else if (info->csd.csd_structure == 1) {
        info->csd.c_size = (sdxx->csd[1] >> 16); // [48,63] [1][16,31]
        info->csd.c_size |= ((sdxx->csd[2] & 0x4F) << 16); // [64,69] [2][0,5]

        // compute capacity
        sdxx->capacity = (info->csd.c_size + 1);
        sdxx->capacity <<= 19; // ~ * 512 * 1024 ~ * 512K
        sdxx->block_size = 512;
    }
    else {
        return SDXX_ERROR;
    }

    
    // scr
    /// get scr
    if (sdxx_get_scr(sdxx, (uint8_t *)buf) != SDXX_OK) {
        return SDXX_ERROR;
    }

    /// unpack scr
    info->scr.scr_structure = (buf[0] >> 4);
    info->scr.sd_spec = (buf[0] & 0x0F); 
    info->scr.sd_spec3 = (buf[2] >> 7);
    info->scr.sd_spec4 = ((buf[2] >> 2) & 0x01);
    info->scr.sd_specx = ((buf[2] & 0x03) << 2) | (buf[3] >> 6);
    info->scr.data_stat_after_erase = (buf[1] >> 7);
    info->scr.sd_bus_widths = (buf[1] & 0x0F);
    info->scr.sd_security = ((buf[1] >> 4) & 0x07);
    info->scr.ex_security = ((buf[2] >> 3) & 0x0F);
    info->scr.cmd_support = (buf[3] & 0x0F);
    DUMP_SCR(&info->scr); 



    return SDXX_OK;
}

#include <stdarg.h>
int sdxx_config(sdxx_t *sdxx, int cmd, ...)
{
    va_list args;
    int ret = SDXX_ERROR;
    
    
    va_start(args, cmd);

    switch (cmd) {
        case SDXX_CMD_RX_MODE:
            switch (va_arg(args, int)) {
                case SDXX_RX_DMA_SINGLE_BLK_ITER:
                    sdxx->read_block = sdxx_read_multi_block_iter;
                    ret = SDXX_OK;
                    PRINTF("sdxx set rx to dma single block iter mode.\n");
                    break;
                case SDXX_RX_DMA_MULTI_BLK:
                    sdxx->read_block = sdxx_read_multi_block;
                    ret = SDXX_OK;
                    PRINTF("sdxx set rx to dma multi block mode.\n");
                    break;
            }
            break;

        case SDXX_CMD_TX_MODE:
            switch (va_arg(args, int)) {
                case SDXX_TX_DMA_SINGLE_BLK_ITER:
                    sdxx->write_block = sdxx_write_multi_block_iter;
                    ret = SDXX_OK;
                    PRINTF("sdxx set tx to dma single block iter mode.\n");
                    break;
                case SDXX_TX_DMA_MULTI_BLK:
                    sdxx->write_block = sdxx_write_multi_block;
                    ret = SDXX_OK;
                    PRINTF("sdxx set tx to dma multi block mode.\n");
                    break;
            }
            ret = SDXX_OK;
            break;
    }

    return ret;
}
/****************************** Copy right 2019 *******************************/
