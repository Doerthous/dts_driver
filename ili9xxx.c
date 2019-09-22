/**
  ******************************************************************************
  * \brief      ili9xxx
  * \file       ili9xxx.c
  * \author     doerthous
  * \date       2019-09-07
  * \details    
  ******************************************************************************
  */

#include "ili9xxx.h"
#include <lib/delay.h> // delay_ms



#define ILI9341
//#define ILI9486




// ---------------------------------- Common -----------------------------------
// common command and function for ili9341 and ili9486
#define ILI9XXX_READ_DISPLAY_STATUS_09H                                     0x09
#define ILI9XXX_ENTER_SLEEP_MODE_10H                                        0x10
#define ILI9XXX_EXIT_SLEEP_MODE_11H                                         0x11
#define ILI9XXX_DISPLAY_OFF_28H                                             0x28
#define ILI9XXX_DISPLAY_ON_29H                                              0x29
#define ILI9XXX_COLUMN_ADDRESS_SET_2AH                                      0x2A
#define ILI9XXX_PAGE_ADDRESS_SET_2BH                                        0x2B
#define ILI9XXX_MEMORY_WRITE_2CH                                            0x2C
#define ILI9XXX_MEMORY_READ_2EH                                             0x2E
#define ILI9XXX_VERTICAL_SCROLLING_DEFINITION_33H                           0x33
#define ILI9XXX_MEMORY_ACCESS_CONTROL_36H                                   0x36
#define ILI9XXX_VERTICAL_SCROLLING_START_ADDRESS_37H                        0x37
#define ILI9XXX_PIXEL_FORMAT_CONTROL_3AH                                    0x3A
#define ILI9XXX_POSITIVE_GAMMA_CORRECTION_E0H                               0xE0
#define ILI9XXX_NEGATIVE_GAMMA_CORRECTION_E1H                               0xE1
#define ILI9XXX_POWER_CONTROL1_C0H                                          0xC0
#define ILI9XXX_POWER_CONTROL2_C1H                                          0xC1
#define ILI9XXX_VCOM_CONTROL1_C5H                                           0xC5
#define ILI9XXX_VCOM_CONTROL2_C7H                                           0xC7
#define ILI9XXX_READ_ID4_D3H                                                0xD3
#define ILI9XXX_FRAME_RATE_CONTROL_B1H                                      0xB1
#define ILI9XXX_FRAME_RATE_CONTROL_B2H                                      0xB2
#define ILI9XXX_FRAME_RATE_CONTROL_B3H                                      0xB3
#define ILI9XXX_DISPLAY_INVERSION_CONTROL_B4H                               0xB4
#define ILI9XXX_DISPLAY_FUNCTION_CONTROL_B6H                                0xB6



#define ili9xxx_display_on(ili9xxx) \
    ili9xxx->write_address(ili9xxx, ILI9XXX_DISPLAY_ON_29H)
#define ili9xxx_exit_sleep_mode(ili9xxx) \
    ili9xxx->write_address(ili9xxx, ILI9XXX_EXIT_SLEEP_MODE_11H); \
    delay_ms(120); \
    ili9xxx_display_on(ili9xxx)
#define ili9xxx_display_off(ili9xxx) \
    ili9xxx->write_address(ili9xxx, ILI9XXX_DISPLAY_OFF_28H) \
    delay_ms(10)
#define ili9xxx_enter_sleep_mode(ili9xxx) \
    ili9xxx_display_off(ili9xxx); \
    ili9xxx->write_address(ili9xxx, ILI9XXX_ENTER_SLEEP_MODE_10H); \
    delay_ms(120); 


#define ili9xxx_15_data(ili9xxx, address, \
    d1,d2,d3,d4,d5,d6,d7,d8,d9,d10,d11,d12,d13,d14,d15) \
    ili9xxx->write_address(ili9xxx, address); \
    ili9xxx->write_data(ili9xxx, d2); \
    ili9xxx->write_data(ili9xxx, d1); \
    ili9xxx->write_data(ili9xxx, d3); \
    ili9xxx->write_data(ili9xxx, d4); \
    ili9xxx->write_data(ili9xxx, d5); \
    ili9xxx->write_data(ili9xxx, d6); \
    ili9xxx->write_data(ili9xxx, d8); \
    ili9xxx->write_data(ili9xxx, d7); \
    ili9xxx->write_data(ili9xxx, d9); \
    ili9xxx->write_data(ili9xxx, d10); \
    ili9xxx->write_data(ili9xxx, d11); \
    ili9xxx->write_data(ili9xxx, d12); \
    ili9xxx->write_data(ili9xxx, d13); \
    ili9xxx->write_data(ili9xxx, d14); \
    ili9xxx->write_data(ili9xxx, d15)
#define ili9xxx_positive_gamma_correction(ili9xxx, \
    d1,d2,d3,d4,d5,d6,d7,d8,d9,d10,d11,d12,d13,d14,d15) \
    ili9xxx_15_data(ili9xxx, ILI9XXX_POSITIVE_GAMMA_CORRECTION_E0H, \
        d1,d2,d3,d4,d5,d6,d7,d8,d9,d10,d11,d12,d13,d14,d15)
#define ili9xxx_negative_gamma_correction(ili9xxx, \
    d1,d2,d3,d4,d5,d6,d7,d8,d9,d10,d11,d12,d13,d14,d15) \
    ili9xxx_15_data(ili9xxx, ILI9XXX_NEGATIVE_GAMMA_CORRECTION_E1H, \
        d1,d2,d3,d4,d5,d6,d7,d8,d9,d10,d11,d12,d13,d14,d15)


#define ili9xxx_set_range(ili9xxx, row_start, row_end, col_start, col_end) \
    ili9xxx->write_address(ili9xxx, ILI9XXX_COLUMN_ADDRESS_SET_2AH); \
    ili9xxx->write_data(ili9xxx, ((col_start)>>8)&0xFF); \
    ili9xxx->write_data(ili9xxx, (col_start)&0xFF); \
    ili9xxx->write_data(ili9xxx, ((col_end)>>8)&0xFF); \
    ili9xxx->write_data(ili9xxx, (col_end)&0xFF); \
    ili9xxx->write_address(ili9xxx, ILI9XXX_PAGE_ADDRESS_SET_2BH); \
    ili9xxx->write_data(ili9xxx, ((row_start)>>8)&0xFF); \
    ili9xxx->write_data(ili9xxx, (row_start)&0xFF); \
    ili9xxx->write_data(ili9xxx, ((row_end)>>8)&0xFF); \
    ili9xxx->write_data(ili9xxx, (row_end)&0xFF)


#define ili9xxx_memory_access_control(ili9xxx, rao, cao, exchange, \
    vro, bgr, hro) \
    ili9xxx->write_address(ili9xxx, ILI9XXX_MEMORY_ACCESS_CONTROL_36H); \
    ili9xxx->write_data(ili9xxx, (rao << 7) | (cao << 6) | (exchange << 5) \
        | (vro << 4) | (bgr << 3) | (hro << 2))

/**
 * @brief      read id
 *
 * @param      ili9xxx  The ili9xxx
 *
 * @return     device id, 0x9486,0x9341
 */
static inline int ili9xxx_read_id(ili9xxx_t *ili9xxx)
{
    int data = 0; 

    ili9xxx->write_address(ili9xxx, ILI9XXX_READ_ID4_D3H);
    data <<= 8;
    data |= ili9xxx->read_data(ili9xxx);
    data <<= 8;
    data |= ili9xxx->read_data(ili9xxx);
    data <<= 8;
    data |= ili9xxx->read_data(ili9xxx);
    data <<= 8;
    data |= ili9xxx->read_data(ili9xxx);
    
    return data;
}



struct __32bits
{
    uint32_t b0: 1;
    uint32_t b1: 1;
    uint32_t b2: 1;
    uint32_t b3: 1;
    uint32_t b4: 1;       
    uint32_t b5: 1;
    uint32_t b6: 1;
    uint32_t b7: 1;
    uint32_t b8: 1;
    uint32_t b9: 1;
    uint32_t b10: 1;
    uint32_t b11: 1;
    uint32_t b12: 1;
    uint32_t b13: 1;
    uint32_t b14: 1;
    uint32_t b15: 1;
    uint32_t b16: 1;
    uint32_t b17: 1;
    uint32_t b18: 1;
    uint32_t b19: 1;
    uint32_t b20: 1;
    uint32_t b21: 1;
    uint32_t b22: 1;
    uint32_t b23: 1;
    uint32_t b24: 1;
    uint32_t b25: 1;
    uint32_t b26: 1;
    uint32_t b27: 1;
    uint32_t b28: 1;   
    uint32_t b29: 1;          
    uint32_t b30: 1;
    uint32_t b31: 1;
};

int ili9xxx_read_config(ili9xxx_t *ili9xxx, ili9xxx_config_t *config)
{
    int data = 0;
    struct __32bits *b = (struct __32bits *)&data;


    ili9xxx->write_address(ili9xxx, ILI9XXX_READ_DISPLAY_STATUS_09H);
    ili9xxx->read_data(ili9xxx); // dummy read
    data <<= 8;
    data |= ili9xxx->read_data(ili9xxx);
    data <<= 8;
    data |= ili9xxx->read_data(ili9xxx);
    data <<= 8;
    data |= ili9xxx->read_data(ili9xxx);
    data <<= 8;
    data |= ili9xxx->read_data(ili9xxx);

    config->vertical_scan_order = b->b27;
    config->horizonal_scan_order = b->b25;
    config->bgr = b->b26;
    config->row_addr_order = b->b30;
    config->col_addr_order = b->b29;
    config->row_col_exchange = b->b28;

    return 1;
}

int ili9xxx_scan_order(ili9xxx_t *ili9xxx, int vso, int hso)
{
    ili9xxx_config_t config;


    ili9xxx_read_config(ili9xxx, &config);

    if (config.horizonal_scan_order != hso 
        || config.vertical_scan_order != vso) {
        ili9xxx_memory_access_control(ili9xxx, 
            config.row_addr_order, config.col_addr_order,
            config.row_col_exchange, vso, config.bgr, hso);
    }

    return 1;
}

int ili9xxx_addr_order(ili9xxx_t *ili9xxx, int rao, int cao)
{
    ili9xxx_config_t config;


    ili9xxx_read_config(ili9xxx, &config);

    if (config.row_addr_order != rao 
        || config.col_addr_order != cao) {
        ili9xxx_memory_access_control(ili9xxx, 
            rao, cao, config.row_col_exchange, 
            config.vertical_scan_order, config.bgr, 
            config.horizonal_scan_order);
    }

    return 1;
}


/**
 * @brief      define ili9xxx vertical scrolling area and fixed area.
 *
 * @param      ili9xxx  The ili9xxx
 * @param[in]  tfa      Top-fixed area
 * @param[in]  vsa      Vertical-scrolling area
 * @param[in]  bfa      Bottom-fixed area
 */
static inline void ili9xxx_vertical_scrolling_define
(
    ili9xxx_t *ili9xxx,
    uint16_t tfa, 
    uint16_t vsa, 
    uint16_t bfa
)
{
    ili9xxx->write_address(ili9xxx,
        ILI9XXX_VERTICAL_SCROLLING_DEFINITION_33H);
    ili9xxx->write_data(ili9xxx, (tfa>>8)&0xFF);
    ili9xxx->write_data(ili9xxx, tfa&0xFF);
    ili9xxx->write_data(ili9xxx, (vsa>>8)&0xFF);
    ili9xxx->write_data(ili9xxx, vsa&0xFF);
    ili9xxx->write_data(ili9xxx, (bfa>>8)&0xFF);
    ili9xxx->write_data(ili9xxx, bfa&0xFF);
}

/**
 * @brief      define ili9xxx vertical scrolling start address.
 *
 * @param      ili9xxx  The ili9xxx
 * @param[in]  vsa      vertical start address. valid value range: [0, 480-1].
 */
static inline void ili9xxx_vertical_scrolling_start_address
(
    ili9xxx_t *ili9xxx,
    uint16_t vsa
)
{
    ili9xxx->write_address(ili9xxx, 
        ILI9XXX_VERTICAL_SCROLLING_START_ADDRESS_37H);
    ili9xxx->write_data(ili9xxx, (vsa>>8)&0xFF);
    ili9xxx->write_data(ili9xxx, vsa&0xFF);
}




// ---------------------------------- ILI9341 ----------------------------------
#if defined(ILI9341)

#define ILI9341_GAMMA_SET_26H                                               0x26



static void ili9341_init_seq(ili9xxx_t *ili9xxx) 
{
    // ?
    ili9xxx->write_address(ili9xxx, 0xCF);  
    ili9xxx->write_data(ili9xxx, 0x00); 
    ili9xxx->write_data(ili9xxx, 0xC1); 
    ili9xxx->write_data(ili9xxx, 0X30); 
    ili9xxx->write_address(ili9xxx, 0xED);  
    ili9xxx->write_data(ili9xxx, 0x64); 
    ili9xxx->write_data(ili9xxx, 0x03); 
    ili9xxx->write_data(ili9xxx, 0X12); 
    ili9xxx->write_data(ili9xxx, 0X81); 
    ili9xxx->write_address(ili9xxx, 0xE8);  
    ili9xxx->write_data(ili9xxx, 0x85); 
    ili9xxx->write_data(ili9xxx, 0x10); 
    ili9xxx->write_data(ili9xxx, 0x7A); 
    ili9xxx->write_address(ili9xxx, 0xCB);  
    ili9xxx->write_data(ili9xxx, 0x39); 
    ili9xxx->write_data(ili9xxx, 0x2C); 
    ili9xxx->write_data(ili9xxx, 0x00); 
    ili9xxx->write_data(ili9xxx, 0x34); 
    ili9xxx->write_data(ili9xxx, 0x02); 
    ili9xxx->write_address(ili9xxx, 0xF7);  
    ili9xxx->write_data(ili9xxx, 0x20); 
    ili9xxx->write_address(ili9xxx, 0xEA);  
    ili9xxx->write_data(ili9xxx, 0x00); 
    ili9xxx->write_data(ili9xxx, 0x00);


    ili9xxx->write_address(ili9xxx, ILI9XXX_POWER_CONTROL1_C0H);
    ili9xxx->write_data(ili9xxx, 0x1B);
    ili9xxx->write_address(ili9xxx, ILI9XXX_POWER_CONTROL2_C1H);
    ili9xxx->write_data(ili9xxx, 0x01);
    ili9xxx->write_address(ili9xxx, ILI9XXX_VCOM_CONTROL1_C5H);
    ili9xxx->write_data(ili9xxx, 0x30);
    ili9xxx->write_data(ili9xxx, 0x30);
    ili9xxx->write_address(ili9xxx, ILI9XXX_VCOM_CONTROL2_C7H);
    ili9xxx->write_data(ili9xxx, 0XB7); 


    ili9xxx_memory_access_control(ili9xxx, 0, 0, 0, 0, 1, 0);


    ili9xxx->write_address(ili9xxx, ILI9XXX_PIXEL_FORMAT_CONTROL_3AH);   
    ili9xxx->write_data(ili9xxx, 0x55);
    ili9xxx->write_address(ili9xxx, ILI9XXX_FRAME_RATE_CONTROL_B1H);   
    ili9xxx->write_data(ili9xxx, 0x00);   
    ili9xxx->write_data(ili9xxx, 0x1A);
    ili9xxx->write_address(ili9xxx, ILI9XXX_DISPLAY_FUNCTION_CONTROL_B6H);
    ili9xxx->write_data(ili9xxx, 0x0A); 
    ili9xxx->write_data(ili9xxx, 0xA2);

    ili9xxx->write_address(ili9xxx, 0xF2);    // 3Gamma Function Disable 
    ili9xxx->write_data(ili9xxx, 0x00);

    ili9xxx->write_address(ili9xxx, ILI9341_GAMMA_SET_26H);
    ili9xxx->write_data(ili9xxx, 0x01);


    ili9xxx_positive_gamma_correction(ili9xxx,
        0x0F,0x2A,0x28,0x08,0x0E,0x08,0x54,0XA9,
        0x43,0x0A,0x0F,0x00,0x00,0x00,0x00);

    ili9xxx_negative_gamma_correction(ili9xxx,
        0x00,0x15,0x17,0x07,0x11,0x06,0x2B,0x56,
        0x3C,0x05,0x10,0x0F,0x3F,0x3F,0x0F);

    ili9xxx_set_range(ili9xxx, 0, ili9xxx->width-1, 0, ili9xxx->height-1);

    ili9xxx_exit_sleep_mode(ili9xxx);
}

#endif // ILI9341







// ---------------------------------- ILI9486 ----------------------------------
#if defined(ILI9486)

#define ILI9486_VCOM_COMTROL_C5H                       ILI9XXX_VCOM_CONTROL1_C5H



static void ili9486_init_seq(ili9xxx_t *ili9xxx) 
{
    ili9xxx->write_address(ili9xxx, 0xfd);
    ili9xxx->write_data(ili9xxx, 0x18);
    ili9xxx->write_data(ili9xxx, 0xa3);
    ili9xxx->write_data(ili9xxx, 0x12);
    ili9xxx->write_data(ili9xxx, 0x02);
    ili9xxx->write_data(ili9xxx, 0xb2);
    ili9xxx->write_data(ili9xxx, 0x12);
    ili9xxx->write_data(ili9xxx, 0xff);
    ili9xxx->write_data(ili9xxx, 0x10);
    ili9xxx->write_data(ili9xxx, 0x00);
    
    ili9xxx->write_address(ili9xxx, 0xf8);
    ili9xxx->write_data(ili9xxx, 0x21);
    ili9xxx->write_data(ili9xxx, 0x04);
    
    ili9xxx->write_address(ili9xxx, 0xf9);
    ili9xxx->write_data(ili9xxx, 0x00);
    ili9xxx->write_data(ili9xxx, 0x08);
    
    ili9xxx->write_address(ili9xxx, ILI9XXX_MEMORY_ACCESS_CONTROL_36H);
    ili9xxx->write_data(ili9xxx, 0x08);
    
    ili9xxx->write_address(ili9xxx, ILI9XXX_PIXEL_FORMAT_CONTROL_3AH);
    ili9xxx->write_data(ili9xxx, 0x05);
    
    ili9xxx->write_address(ili9xxx, ILI9XXX_DISPLAY_INVERSION_CONTROL_B4H);
    ili9xxx->write_data(ili9xxx, 0x01);
    
    ili9xxx->write_address(ili9xxx, ILI9XXX_DISPLAY_FUNCTION_CONTROL_B6H);
    ili9xxx->write_data(ili9xxx, 0x02);
    ili9xxx->write_data(ili9xxx, 0x22);
    
    ili9xxx->write_address(ili9xxx, ILI9XXX_POWER_CONTROL2_C1H);
    ili9xxx->write_data(ili9xxx, 0x41);
    
    ili9xxx->write_address(ili9xxx, ILI9486_VCOM_COMTROL_C5H);
    ili9xxx->write_data(ili9xxx, 0x00);
    ili9xxx->write_data(ili9xxx, 0x07);
    

    ili9xxx_positive_gamma_correction(ili9xxx,
        0x0F,0x1F,0x1C,0x0C,0x0F,0x08,0x48,0x98,
        0x37,0x0A,0x13,0x04,0x11,0x0D,0x00);

    ili9xxx_negative_gamma_correction(ili9xxx,
        0x0F,0x32,0x2E,0x0B,0x0D,0x05,0x47,0x75,
        0x37,0x06,0x10,0x03,0x24,0x20,0x00);
    
    ili9xxx_exit_sleep_mode(ili9xxx);
}

#endif // ILI9486


// -----------------------------------------------------------------------------
static int vsp;
static void ili9xxx_to_pixel(ili9xxx_t *ili9xxx, int x, int y) {
    ili9xxx->write_address(ili9xxx, ILI9XXX_COLUMN_ADDRESS_SET_2AH);
    ili9xxx->write_data(ili9xxx, (x & 0xFF00) >> 8);  
    ili9xxx->write_data(ili9xxx, x & 0x00FF);
    ili9xxx->write_data(ili9xxx, ((x + 1) & 0xFF00) >> 8);  
    ili9xxx->write_data(ili9xxx, (x + 1) & 0x00FF);
    
    ili9xxx->write_address(ili9xxx, ILI9XXX_PAGE_ADDRESS_SET_2BH); 
    ili9xxx->write_data(ili9xxx, (y & 0xFF00) >> 8);  
    ili9xxx->write_data(ili9xxx, y & 0x00FF);
    ili9xxx->write_data(ili9xxx, ((y + 1) & 0xFF00) >> 8);  
    ili9xxx->write_data(ili9xxx, (y + 1) & 0x00FF);
}

void ili9xxx_set_pixel(ili9xxx_t * ili9xxx, int x, int y, int color)
{
    ili9xxx_to_pixel(ili9xxx, x, (y+vsp)%ili9xxx->height);
    ili9xxx->write_address(ili9xxx, ILI9XXX_MEMORY_WRITE_2CH);
    ili9xxx->write_data(ili9xxx, color);
}

int  ili9xxx_get_pixel(ili9xxx_t * ili9xxx, int x, int y)
{
    ili9xxx_to_pixel(ili9xxx, x, (y+vsp)%ili9xxx->height);
    ili9xxx->write_address(ili9xxx, ILI9XXX_MEMORY_READ_2EH);
    ili9xxx->read_data(ili9xxx); // dummy read
    return ili9xxx->read_data(ili9xxx);
}

static void __ili9xxx_fill(ili9xxx_t *ili9xxx, 
    int xs, int ys, int xe, int ye, int color) 
{
    int i, c;
    ili9xxx->write_address(ili9xxx, ILI9XXX_COLUMN_ADDRESS_SET_2AH);
    ili9xxx->write_data(ili9xxx, (xs & 0xFF00) >> 8);  
    ili9xxx->write_data(ili9xxx, xs & 0x00FF);
    ili9xxx->write_data(ili9xxx, (xe & 0xFF00) >> 8);  
    ili9xxx->write_data(ili9xxx, xe & 0x00FF);
    
    ili9xxx->write_address(ili9xxx, ILI9XXX_PAGE_ADDRESS_SET_2BH); 
    ili9xxx->write_data(ili9xxx, (ys & 0xFF00) >> 8);  
    ili9xxx->write_data(ili9xxx, ys & 0x00FF);
    ili9xxx->write_data(ili9xxx, (ye & 0xFF00) >> 8);  
    ili9xxx->write_data(ili9xxx, ye & 0x00FF);
    
    ili9xxx->write_address(ili9xxx, ILI9XXX_MEMORY_WRITE_2CH);
    c = ((ye - ys + 1) * (xe - xs + 1));
    for(i = 0; i < c; i++) {
        ili9xxx->write_data(ili9xxx, color);
    }  
}
void ili9xxx_fill(ili9xxx_t *ili9xxx, 
    int xs, int ys, int xe, int ye, int color) 
{
    ys = (ys+vsp)%ili9xxx->height;
    ye = (ye+vsp)%ili9xxx->height;

    if (ye > ys) {
        __ili9xxx_fill(ili9xxx, xs, ys, xe, ye, color);
    }
    else {
        __ili9xxx_fill(ili9xxx, xs, ys, xe, ili9xxx->height-1, color);
        __ili9xxx_fill(ili9xxx, xs, 0, xe, ye, color);
    }
}








int ili9xxx_init(ili9xxx_t *ili9xxx)
{    
    ili9xxx->md_init(ili9xxx);

    ili9xxx->id = ili9xxx_read_id(ili9xxx);

    switch (ili9xxx->id) {
      #ifdef ILI9341
        case 0x9341:
            ili9341_init_seq(ili9xxx);
            break;
      #endif


      #ifdef ILI9486
        case 0x9486:
            ili9486_init_seq(ili9xxx);
            break;
      #endif


        default:
            return 0;
    }

    ili9xxx_fill(ili9xxx, 0, 0, ili9xxx->width-1, 
                    ili9xxx->height-1, 0xffff);
    
    return 1;
}



static void ili9xxx_vertical_scrolling_enable(ili9xxx_t *ili9xxx)
{
    ili9xxx_vertical_scrolling_define(ili9xxx, 0, ili9xxx->height, 0);

    ili9xxx_vertical_scrolling_start_address(ili9xxx, 0);
    vsp = 0;    
}

/****************************** Copy right 2019 *******************************/
