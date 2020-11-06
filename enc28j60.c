/**
  ******************************************************************************
  * \brief      enc28j60
  * \file       enc28j60.c
  * \author     doerthous
  * \date       2020-11-04
  * \details    
  ******************************************************************************
  */

#include "enc28j60.h"

#define bit(i) (1<<i)

#define CMD_RCR(addr)   (0x1F & (addr))
#define CMD_RBM         ((0x01 << 5)|(0x1A))
#define CMD_WCR(arg)    ((0x02 << 5)|(0x1F & (arg)))
#define CMD_WBM         ((0x03 << 5)|(0x1A))
#define CMD_BFS(arg)    ((0x04 << 5)|(0x1F & (arg)))
#define CMD_BFC(arg)    ((0x05 << 5)|(0x1F & (arg)))
#define CMD_SRC         (0xFF)

#define REG_EIE         (0x1B)
# define RXERIE         (bit(0))
# define TXERIE         (bit(1))
# define TXIE           (bit(3))
# define LINKIE         (bit(4))
# define DMAIE          (bit(5))
# define PKTIE          (bit(6))
# define INTIE          (bit(7))
#define REG_EIR         (0x1C)
# define RXERIF         (bit(0))
# define TXERIF         (bit(1))
# define TXIF           (bit(3))
# define LINKIF         (bit(4))
# define DMAIF          (bit(5))
# define PKTIF          (bit(6))
#define REG_ESTAT       (0x1D)
# define CLKRDY         (bit(0))
# define TXABRT         (bit(1))
# define RXBUSY         (bit(2))
# define LATECOL        (bit(4))
# define BUFER          (bit(6))
# define INT            (bit(7))
#define REG_ECON2       (0x1E)
# define VRPS           (bit(3))
# define PWRSV          (bit(5))
# define PKTDEC         (bit(6))
# define AUTOINC        (bit(7))
#define REG_ECON1       (0x1F)
# define RXEN           (bit(2))
# define TXRTS          (bit(3))
# define RXRST          (bit(6))
# define TXRST          (bit(7))

#define BANK0           (0)
#define REG_ERDPTL      (0x00) // reset: 0xFA
#define REG_ERDPTH      (0x01)
#define REG_EWRPTL      (0x02)
#define REG_EWRPTH      (0x03)
#define REG_ETXSTL      (0x04)
#define REG_ETXSTH      (0x05)
#define REG_ETXNDL      (0x06)
#define REG_ETXNDH      (0x07)
#define REG_ERXSTL      (0x08)
#define REG_ERXSTH      (0x09)
#define REG_ERXNDL      (0x0A)
#define REG_ERXNDH      (0x0B)
#define REG_ERXRDPTL    (0x0C)
#define REG_ERXRDPTH    (0x0D)
#define REG_ERXWRPTL    (0x0E)
#define REG_ERXWRPTH    (0x0F)

#define BANK1           (1)
#define REG_ERXFCON     ((BANK1<<5)|0x18) // reset: 0xA1
# define BCEN           (bit(0))
# define MCEN           (bit(1))
# define HTEN           (bit(2))
# define MPEN           (bit(3))
# define PMEN           (bit(4))
# define CRCEN          (bit(5))
# define ANDOR          (bit(6))
# define UCEN           (bit(7))

#define REG_EPKTCNT     ((BANK1<<5)|0x19)

#define BANK2           (2)
#define REG_MACON1      ((BANK2<<5)|0x00)
# define MARXEN         (bit(0))
# define PASSALL        (bit(1))
# define RXPAUS         (bit(2))
# define TXPAUS         (bit(3))
#define REG_MACON3      ((BANK2<<5)|0x02)
# define FULDPX         (bit(0))
# define FRMLNEN        (bit(1))
# define HFRMEN         (bit(2))
# define PHDREN         (bit(3))
# define TXCRCEN        (bit(4))
# define PADCFG0        (bit(5))
# define PADCFG1        (bit(6))
# define PADCFG2        (bit(7))
#  define PADCFG(cfg)   ((cfg) << 5)
#define REG_MACON4      ((BANK2<<5)|0x03)
# define NOBKOFF        (bit(4))
# define BPEN           (bit(5))
# define DEFER          (bit(6))
#define MABBIPG         ((BANK2<<5)|0x04)
#define MAIPGL          ((BANK2<<5)|0x06)
#define MAIPGH          ((BANK2<<5)|0x07)
#define REG_MACLCON1    ((BANK2<<5)|0x08) // reset: 0xXF
#define REG_MAMXFLL     ((BANK2<<5)|0x0A)
#define REG_MAMXFLH     ((BANK2<<5)|0x0B)
#define REG_MICMD       ((BANK2<<5)|0x12)
# define MIIRD          (bit(0))
# define MIISCAN        (bit(1))
#define REG_MIREGADR    ((BANK2<<5)|0x14)
#define REG_MIRDL       ((BANK2<<5)|0x18)
#define REG_MIRDH       ((BANK2<<5)|0x19)

#define BANK3           (3)
#define REG_MAADR5      ((BANK3<<5)|0x00)
#define REG_MAADR6      ((BANK3<<5)|0x01)
#define REG_MAADR3      ((BANK3<<5)|0x02)
#define REG_MAADR4      ((BANK3<<5)|0x03)
#define REG_MAADR1      ((BANK3<<5)|0x04)
#define REG_MAADR2      ((BANK3<<5)|0x05)
#define REG_MISTAT      ((BANK3<<5)|0x0A)
# define BUSY           (bit(0))
# define SCAN           (bit(1))
# define NVALID         (bit(2))
#define REG_EPAUSH      ((BANK3<<5)|0x19) // reset: 0x10

// PHY Registers
#define REG_PHCON1      (0x00)
# define PDPXMD         (bit(8))
# define PPWRSV         (bit(11))
# define PLOOPBK        (bit(14))
# define PRST           (bit(15))
#define REG_PHSTAT1     (0x01)
# define JBSTAT         (bit(1))
# define LLSTAT         (bit(2))
# define PHDPX          (bit(11))
# define PFDPX          (bit(12))
#define REG_PHID1       (0x02) // reset: 0x0083
#define REG_PHID2       (0x03) // reset: 0x1400
# define PHY_ID(phid1, phid2)   (0) // TODO:
# define PHY_REVISION(phid2)    ((phid2) & 0x000F)
# define PHY_PN(phid2)          (((phid2) & 0x03F0)>>4)
#define REG_PHCON2      (0x10)
# define HDLDIS         (bit(8))
# define JABBER         (bit(10))
# define TXDIS          (bit(13))
# define FRCLNK         (bit(14))
#define REG_PHSTAT2     (0x11)
# define PLRITY         (bit(5))
# define DPXSTAT        (bit(9))
# define LSTAT          (bit(10))
# define COLSTAT        (bit(11))
# define RXSTAT         (bit(12))
# define TXSTAT         (bit(13))
#define REG_PHIE        (0x12)
# define PGEIE          (bit(1))
# define PLNKIE         (bit(4))
#define REG_PHIR        (0x13)
# define PGIF           (bit(2))
# define PLNKIF         (bit(4))
#define REG_PHLCON      (0x14)
# define STRCH          (bit(1))
# define LFRQ(phlcon)   ((phlcon >> 2) & 0x0003)
# define LBCFG(phlcon)  ((phlcon >> 4) & 0x000F)
# define LACFG(phlcon)  ((phlcon >> 8) & 0x000F)


int enc28j60_write_ctrl_reg(enc28j60_t *enc28j60, 
    uint8_t reg, uint8_t *data, uint32_t size)
{
    int ret = 0;
    reg = CMD_WCR(reg);
    gpio_clear(&enc28j60->cs);
    if (spi_write(enc28j60->spi, &reg, 1) == 1) {
        if (spi_write(enc28j60->spi, data, size) == size) {
            ret = 1;
        }
    }
    gpio_set(&enc28j60->cs);
    return ret;
}

int enc28j60_read_ctrl_reg(enc28j60_t *enc28j60, 
    uint8_t reg, uint8_t *buff, uint32_t size)
{
    int ok = 0;
    reg = CMD_RCR(reg);
    for (int i = 0; i < 3 && !ok; ++i) {
        gpio_clear(&enc28j60->cs);
        if (spi_write(enc28j60->spi, &reg, 1) == 1) {
            if (spi_read(enc28j60->spi, buff, size) == size) {
                ok = 1;
            }
        }
        gpio_set(&enc28j60->cs);
    }
    return ok;
}

static int write_buffer_memory(enc28j60_t *enc28j60,
    uint8_t *buff, uint32_t size)
{
    int ok = 0;
    uint8_t cmd = CMD_WBM;
    for (int i = 0; i < 3 && !ok; ++i) {
        gpio_clear(&enc28j60->cs);
        if (spi_write(enc28j60->spi, &cmd, 1) == 1) {
            if (spi_write(enc28j60->spi, buff, size) == size) {
                ok = 1;
            }
        }
    }
    gpio_set(&enc28j60->cs);
    return ok;
}

static int read_buffer_memory(enc28j60_t *enc28j60,
    uint8_t *buff, uint32_t size)
{
    int ret = 0;
    uint8_t cmd = CMD_RBM;
    gpio_clear(&enc28j60->cs);
    if (spi_write(enc28j60->spi, &cmd, 1) == 1) {
        if (spi_read(enc28j60->spi, buff, size) == size) {
            ret = 1;
        }
    }
    gpio_set(&enc28j60->cs);
    return ret;
}

int enc28j60_bit_field_set(enc28j60_t *enc28j60, uint8_t reg, uint8_t data)
{
    int ok = 0;
    reg = CMD_BFS(reg);
    for (int i = 0; i < 3 && !ok; ++i) {
        gpio_clear(&enc28j60->cs);
        if (spi_write(enc28j60->spi, &reg, 1) == 1) {
            if (spi_write(enc28j60->spi, &data, 1) == 1) {
                ok = 1;
            }
        }
        gpio_set(&enc28j60->cs);
    }
    return ok;
}

int enc28j60_bit_field_clear(enc28j60_t *enc28j60, uint8_t reg, uint8_t data)
{
    int ok = 0;
    reg = CMD_BFC(reg);
    for (int i = 0; i < 3 && !ok; ++i) {
        gpio_clear(&enc28j60->cs);
        if (spi_write(enc28j60->spi, &reg, 1) == 1) {
            if (spi_write(enc28j60->spi, &data, 1) == 1) {
                ok = 1;
            }
        }
        gpio_set(&enc28j60->cs);
    }
    return ok;
}

int enc28j60_select_bank(enc28j60_t *enc28j60, int bank)
{
    uint8_t data;

    if (enc28j60->current_bank == bank) {
        return 1;
    }
    
    if (enc28j60_read_ctrl_reg(enc28j60, REG_ECON1, &data, 1)) {
        data &= 0xFC;
        data |= bank;
        if (enc28j60_write_ctrl_reg(enc28j60, REG_ECON1, &data, 1)) {
            enc28j60->current_bank = bank;
            return 1;
        }
    }

    return 0;
}

static int write_reg(enc28j60_t *enc28j60,
    uint8_t reg, uint8_t *data, uint32_t size)
{
    if (enc28j60_select_bank(enc28j60, (reg >> 5))) {
        return enc28j60_write_ctrl_reg(enc28j60, reg, data, size);
    }
    return 0;
}
static inline int write_a_reg(enc28j60_t *enc28j60, uint8_t reg, uint8_t data)
{
    return write_reg(enc28j60, reg, &data, 1);
}

static int read_reg(enc28j60_t *enc28j60, 
    uint8_t reg, uint8_t *buff, uint32_t size)
{
    if (enc28j60_select_bank(enc28j60, (reg >> 5))) {
        return enc28j60_read_ctrl_reg(enc28j60, reg, buff, size);
    }
    return 0;
}
static inline int read_a_reg(enc28j60_t *enc28j60, uint8_t reg, uint8_t *data)
{
    return read_reg(enc28j60, reg, data, 1);
}

static int read_mii_reg(enc28j60_t *enc28j60, uint8_t reg)
{
    int ret = -1;
    uint8_t buff[2];
    if (read_reg(enc28j60, reg, buff, 2)) {
        ret = buff[1];
    }
    return ret;
}
static inline int read_mac_reg(enc28j60_t *enc28j60, uint8_t reg)
{
    return read_mii_reg(enc28j60, reg);
}

static int set_bit(enc28j60_t *enc28j60, uint8_t reg, uint8_t bits)
{
    if (enc28j60_select_bank(enc28j60, (reg >> 5))) {
        return enc28j60_bit_field_set(enc28j60, reg, bits);
    }
    return 0;
}

static int clear_bit(enc28j60_t *enc28j60, uint8_t reg, uint8_t bits)
{
    if (enc28j60_select_bank(enc28j60, (reg >> 5))) {
        return enc28j60_bit_field_clear(enc28j60, reg, bits);
    }
    return 0;
}

static uint16_t read_phy_reg(enc28j60_t *enc28j60, uint8_t addr)
{
    int reg;
    write_a_reg(enc28j60, REG_MIREGADR, addr);
    write_a_reg(enc28j60, REG_MICMD, MIIRD);

    //for (int i = 0; i < 100; ++i); // wait for 10.24us
    reg = BUSY;
    while (reg & BUSY) {
        reg = read_mii_reg(enc28j60, REG_MISTAT);
    }

    write_a_reg(enc28j60, REG_MICMD, 0);

    uint16_t data;
    reg = read_mii_reg(enc28j60, REG_MIRDL);
    data = reg;
    reg = read_mii_reg(enc28j60, REG_MIRDH);
    data |= (reg << 8);
    
    return data;
}

static inline void system_reset(enc28j60_t *enc28j60)
{
    uint8_t cmd[2] = { CMD_SRC, CMD_SRC };
    gpio_clear(&enc28j60->cs);
    spi_write(enc28j60->spi, cmd, 2);
    gpio_set(&enc28j60->cs);
    enc28j60->current_bank = 0;
}

static inline void tx_reset(enc28j60_t *enc28j60)
{
    uint8_t cmd[2] = { CMD_BFS(REG_ECON1), TXRST };
    gpio_clear(&enc28j60->cs);
    spi_write(enc28j60->spi, cmd, 2);
    gpio_set(&enc28j60->cs);
}

static inline void rx_reset(enc28j60_t *enc28j60)
{
    uint8_t cmd[2] = { CMD_BFS(REG_ECON1), RXRST };
    gpio_clear(&enc28j60->cs);
    spi_write(enc28j60->spi, cmd, 2);
    gpio_set(&enc28j60->cs);
}
static inline int set_mac_addr(enc28j60_t *enc28j60, uint8_t addr[6])
{
    int ret = 0;
    ret += write_reg(enc28j60, REG_MAADR1, addr  , 1);
    ret += write_reg(enc28j60, REG_MAADR2, addr+1, 1);
    ret += write_reg(enc28j60, REG_MAADR3, addr+2, 1);
    ret += write_reg(enc28j60, REG_MAADR4, addr+3, 1);
    ret += write_reg(enc28j60, REG_MAADR5, addr+4, 1);
    ret += write_reg(enc28j60, REG_MAADR6, addr+5, 1);
    return ret == 6;
}

#define RX_BUFF_SIZE (6*1024)
int enc28j60_init(enc28j60_t *enc28j60)
{
    uint8_t data;

	gpio_set(&enc28j60->cs);

    system_reset(enc28j60);

    // Check ETH Registers
    if (!(read_a_reg(enc28j60, REG_ERDPTL, &data) && (data == 0xFA))) {
        goto init_error;
    }
    if (!(read_a_reg(enc28j60, REG_ERXFCON, &data) && (data== 0xA1))) {
        goto init_error;
    }

    // Wait OST
    /// MAC and PHY registers can only be access after OST stable!
    data = 100;
    while (data--) {
        if (read_reg(enc28j60, REG_ESTAT, &data, 1) && (data & CLKRDY)) {
            break;
        }
    }
    if (!data) {
        goto init_error;
    }

    // Check PHY Registers
    if (read_phy_reg(enc28j60, REG_PHID1) != 0x0083) {
        goto init_error;
    }
    if (read_phy_reg(enc28j60, REG_PHID2) != 0x1400) {
        goto init_error;
    }

    // Configure R/TX Buffer
    /// RX
    /// ENC28J60 Silicon Errata and Data Sheet Clarification, issue #4
    data = 0;
    write_reg(enc28j60, REG_ERXRDPTL, &data, 1);
    write_reg(enc28j60, REG_ERXRDPTH, &data, 1);
    write_reg(enc28j60, REG_ERXSTL, &data, 1);
    write_reg(enc28j60, REG_ERXSTH, &data, 1);
    data = (RX_BUFF_SIZE-1)&0xFF;
    write_reg(enc28j60, REG_ERXNDL, &data, 1);
    data = (RX_BUFF_SIZE-1)>>8;
    write_reg(enc28j60, REG_ERXNDH, &data, 1);
    /// TX
    data = (RX_BUFF_SIZE)&0xFF;
    write_reg(enc28j60, REG_ETXSTL, &data, 1);
    data = (RX_BUFF_SIZE)>>8;
    write_reg(enc28j60, REG_ETXSTH, &data, 1);
    data = (1024*8-1)&0xFF;
    write_reg(enc28j60, REG_ETXNDL, &data, 1);
    data = (1024*8-1)>>8;
    write_reg(enc28j60, REG_ETXNDH, &data, 1);

    // Set RX Filter
    /// Nothing to do

    // Configure MAC
    if (!enc28j60->half_mode) {
        set_bit(enc28j60, REG_MACON1, MARXEN|TXPAUS|RXPAUS);
        set_bit(enc28j60, REG_MACON3, PADCFG(1)|FRMLNEN|TXCRCEN|FULDPX);
        data = 0x15;
        write_reg(enc28j60, MABBIPG, &data, 1);
        data = 0x12;
        write_reg(enc28j60, MAIPGL, &data, 1);
    }
    else {
        set_bit(enc28j60, REG_MACON1, MARXEN);
        set_bit(enc28j60, REG_MACON3, PADCFG(1)|FRMLNEN|TXCRCEN);
        data = 0x12;
        write_reg(enc28j60, MABBIPG, &data, 1);
        data = 0x12;
        write_reg(enc28j60, MAIPGL, &data, 1);
        data = 0x0C;
        write_reg(enc28j60, MAIPGH, &data, 1);
        // program the Retransmission and Collision Window registers,
        // MACLCON1 and MACLCON2. Most applications will not need to 
        // change the default Reset values
    }
    set_bit(enc28j60, REG_MACON4, DEFER);

    /// Set Max Packet Length
    data = (1518 & 0xFF);
    if (!write_reg(enc28j60, REG_MAMXFLL, &data, 1)) {
        goto init_error;
    }
    data = (1518 >> 8);
    if (!write_reg(enc28j60, REG_MAMXFLH, &data, 1)) {
        goto init_error;
    }

    /// Set MAC Address
    if (!set_mac_addr(enc28j60, enc28j60->mac_addr)) {
        goto init_error;
    }

    // Configure PHY
    /// Nothing to do

    
    // Start Receiving Packets
    set_bit(enc28j60, REG_ECON1, RXEN);
    
    return 1;

init_error:
	return 0;
}

#include <string.h>
uint32_t enc28j60_send(enc28j60_t *enc28j60, uint8_t *data, uint32_t size)
{
    uint8_t ewrptl, ewrpth;
    uint8_t buff[16];

    // Always from ETXSTPH start, which is RX_BUFF_SIZE.
    ewrptl = (RX_BUFF_SIZE)&0xFF;
    ewrpth = (RX_BUFF_SIZE)>>8;
    write_reg(enc28j60, REG_EWRPTL, &ewrptl, 1);
    write_reg(enc28j60, REG_EWRPTH, &ewrpth, 1);

    // Copy data to buffer memory
    buff[0] = 0x0E;
    write_buffer_memory(enc28j60, buff, 1);
    write_buffer_memory(enc28j60, data, size);

    // Set ETXNDPT
    read_reg(enc28j60, REG_EWRPTL, &ewrptl, 1);
    read_reg(enc28j60, REG_EWRPTH, &ewrpth, 1);
	if (--ewrptl == 0xFF) {
        --ewrpth;
    }
    write_reg(enc28j60, REG_ETXNDL, &ewrptl, 1);
    write_reg(enc28j60, REG_ETXNDH, &ewrpth, 1);

	// Enable interrupt
    //clear_bit(enc28j60, REG_EIR, TXIF);
    //set_bit(enc28j60, REG_EIE, TXIE|INTIE);

    // Start transmit
    set_bit(enc28j60, REG_ECON1, TXRTS);

    // Wait for transmit logic complete
    while (read_reg(enc28j60, REG_ECON1, buff, 1) && (buff[0] & TXRTS));

    // Get Transmit Status Vector
    if (++ewrptl == 0) {
        ++ewrpth;
    }
    for (int i = 0; i < 4; ++i) {
        write_reg(enc28j60, REG_ERDPTL, &ewrptl, 1);
        write_reg(enc28j60, REG_ERDPTH, &ewrpth, 1);
        if (read_buffer_memory(enc28j60, buff, 7)) {
            break;
        }
        if (i == 3) {
            return 0;
        }
    }

    #define TX_BYTE_COUNT (buff[0]|(buff[1]<<8))
    #define TX_COLLISION_COUNT (buff[2]&0x0F)
    #define TX_CRC_ERROR (buff[2]&0x10)
    #define TX_LEN_ERROR (buff[2]&0x20)
    #define TX_LEN_OUT_OF_RANGE (buff[2]&0x40)
    #define TX_DONE (buff[2]&0x80)

    if (TX_CRC_ERROR|TX_LEN_ERROR|TX_LEN_OUT_OF_RANGE) {
        return 0;
    }

    return 1;
}

static inline void restore_next_pkt_ptr(enc28j60_t *enc28j60)
{
    uint8_t nxpktpth, nxpktptl;
    uint8_t rxndpth, rxndptl;
    read_reg(enc28j60, REG_ERXRDPTL, &nxpktptl, 1);
    read_reg(enc28j60, REG_ERXRDPTH, &nxpktpth, 1);
    if (nxpktptl & 0x01) { // if REG_ERXRDPT is odd address
        read_reg(enc28j60, REG_ERXNDL, &rxndptl, 1);
        read_reg(enc28j60, REG_ERXNDH, &rxndpth, 1);
        // ENC28J60 Silicon Errata and Data Sheet Clarification, issue #14
        // if ERXRDPT == ERXNDPT then Next_Packet_Pointer = ERXSTPT
        if (nxpktptl == rxndptl && nxpktpth == rxndpth) {
            read_reg(enc28j60, REG_ERXSTL, &nxpktptl, 1);
            read_reg(enc28j60, REG_ERXSTH, &nxpktpth, 1);
        }
        // else Next_Packet_Pointer = ERXRDPT+1
        else {
            if (++nxpktptl == 0x00) {
                ++nxpktpth;
            }
        }
    }
    write_reg(enc28j60, REG_ERDPTL, &nxpktptl, 1);
    write_reg(enc28j60, REG_ERDPTH, &nxpktpth, 1);
}
static inline void save_next_pkt_ptr(enc28j60_t *enc28j60,
    uint8_t nxpktpth, uint8_t nxpktptl)
{
    uint8_t rxstpth, rxstptl;
    read_reg(enc28j60, REG_ERXSTL, &rxstptl, 1);
    read_reg(enc28j60, REG_ERXSTH, &rxstpth, 1);
    
    // ENC28J60 Silicon Errata and Data Sheet Clarification, issue #14
    // if Next_Packet_Pointer == ERXSTPT then ERXRDPT = ERXNDPT
    // Note: because Next Packet Pointer always point to even address
    //       here assume rxstpt point to even address too.
    if (nxpktptl == rxstptl && nxpktpth == rxstpth) { 
        read_reg(enc28j60, REG_ERXNDL, &nxpktptl, 1);
        read_reg(enc28j60, REG_ERXNDH, &nxpktpth, 1);
    }
    // else ERXRDPT = Next_Packet_Pointer-1
    else {
        if (--nxpktptl == 0xFF) {
            --nxpktpth;
        }
    }
    write_reg(enc28j60, REG_ERXRDPTL, &nxpktptl, 1);
    write_reg(enc28j60, REG_ERXRDPTH, &nxpktpth, 1);
}
static void dump_phy_reg(enc28j60_t *enc28j60, uint16_t *phy_regs)
{
    *phy_regs++ = read_phy_reg(enc28j60, REG_PHSTAT1);
    *phy_regs++ = read_phy_reg(enc28j60, REG_PHSTAT2);
    *phy_regs++ = read_phy_reg(enc28j60, REG_PHIR);
    *phy_regs++ = read_phy_reg(enc28j60, REG_PHIE);
    *phy_regs++ = read_phy_reg(enc28j60, REG_PHCON1);
    *phy_regs++ = read_phy_reg(enc28j60, REG_PHCON2);
    *phy_regs++ = read_phy_reg(enc28j60, REG_PHLCON);
    *phy_regs++ = read_phy_reg(enc28j60, REG_PHID1);
    *phy_regs++ = read_phy_reg(enc28j60, REG_PHID2);
}

uint32_t enc28j60_recv(enc28j60_t *enc28j60, uint8_t *data, uint32_t size)
{
    uint8_t header[8];

    size = 0;
    if (read_reg(enc28j60, REG_EPKTCNT, header, 1) && header[0] > 0) {
        for (int i = 0; i < 4; ++i) {
            if (i == 3) {
                return 0;
            }

            restore_next_pkt_ptr(enc28j60);
            
            // get six-byte header
            if (!read_buffer_memory(enc28j60, header, 6)) {
                continue;
            }

            size = (header[2]|(header[3]<<8)); // include 4-byte crc
            if (!read_buffer_memory(enc28j60, data, size)) {
                continue;
            }

            break;
        }

        save_next_pkt_ptr(enc28j60, header[1], header[0]);

        set_bit(enc28j60, REG_ECON2, PKTDEC); // decrease pktcnt
        return size;
    }

    return 0;
}



int enc28j60_pack(enc28j60_packet_t *packet)
{
    uint8_t *ptr = packet->data;

    memcpy(ptr, packet->dist_mac_addr, 6);
    ptr += 6;

    memcpy(ptr, packet->src_mac_addr, 6);
    ptr += 6;

    // TODO: endian handle
    if (packet->type < 0x05DC) {
        if (packet->payload_size < 46) {
            *ptr++ = 0;
            *ptr++ = 46;
        }
    }
    else {
        *ptr++ = (packet->type >> 8);
        *ptr++ = (packet->type & 0xFF);
    }

    memcpy(ptr, packet->payload, packet->payload_size);
    packet->payload_size = packet->payload_size < 46? 46 : packet->payload_size;
    ptr += packet->payload_size;
    packet->data_size = ptr-packet->data;
    return 1;
}
int enc28j60_unpack(enc28j60_packet_t *packet)
{
    uint8_t *ptr = packet->data;

    if (packet->data_size < 64) {
        return 0;
    }

    memcpy(packet->dist_mac_addr, ptr, 6);
    ptr += 6;

    memcpy(packet->src_mac_addr, ptr, 6);
    ptr += 6;

    // TODO: endian handle
    packet->type = (*ptr << 8) | *(ptr+1);
    ptr += 2;

    packet->payload_size = packet->data_size-12-2-4;
    //memcpy(packet->payload, ptr, packet->payload_size);
	packet->payload = ptr;
    ptr += packet->payload_size;

    // FIXME:
    packet->crc = (ptr[0]|ptr[1]|ptr[2]|ptr[3]);

    return 1;
}

/****************************** Copy right 2020 *******************************/
