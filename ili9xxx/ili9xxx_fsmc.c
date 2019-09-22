/**
  ******************************************************************************
  * \brief      
  * \file       ili9xxx_fsmc.c
  * \author     doerthous
  * \date       2019-09-07
  * \details    
  ******************************************************************************
  */


#include "../ili9xxx.h"
#include <stm32f10x.h>


/*
 * LCD line:
 * 
 * D0~D15:          PD14,PD15,PD0,PD1,PE7~PE15,PD8~PD10
 * CS,RS,WR,RD:     PG12,PG0,PD5,PD4
 * BL:              PB0
 */

static void fsmc_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
    FSMC_NORSRAMTimingInitTypeDef  readWriteTiming; 
    FSMC_NORSRAMTimingInitTypeDef  writeTiming;


    //# GPIO configuration
    //## enable clock of PORTB,D,E,G
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB
        | RCC_APB2Periph_GPIOD
        | RCC_APB2Periph_GPIOE
        | RCC_APB2Periph_GPIOG,
        ENABLE);

    //##PB0 backlight
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;                
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_WriteBit(GPIOB, GPIO_Pin_0, Bit_SET);
    
    //##PORTD
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0
        | GPIO_Pin_1
        | GPIO_Pin_4
        | GPIO_Pin_5
        | GPIO_Pin_8
        | GPIO_Pin_9
        | GPIO_Pin_10
        | GPIO_Pin_14
        | GPIO_Pin_15;   
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure); 

    //##PORTE 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7
        | GPIO_Pin_8
        | GPIO_Pin_9
        | GPIO_Pin_10
        | GPIO_Pin_11
        | GPIO_Pin_12
        | GPIO_Pin_13
        | GPIO_Pin_14
        | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIO_InitStructure);                                                           

    //##PORTG
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOG, &GPIO_InitStructure); 


    //# FSMC configuration
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC,ENABLE); //使能FSMC时钟
    readWriteTiming.FSMC_AddressSetupTime = 0x01;    //地址建立时间（ADDSET）为2个HCLK 1/36M=27ns
    readWriteTiming.FSMC_AddressHoldTime = 0x00;   //地址保持时间（ADDHLD）模式A未用到 
    readWriteTiming.FSMC_DataSetupTime = 0x0f;         // 数据保存时间为16个HCLK,因为液晶驱动IC的读数据的时候，速度不能太快，尤其对1289这个IC。
    readWriteTiming.FSMC_BusTurnAroundDuration = 0x00;
    readWriteTiming.FSMC_CLKDivision = 0x00;
    readWriteTiming.FSMC_DataLatency = 0x00;
    readWriteTiming.FSMC_AccessMode = FSMC_AccessMode_A;   //模式A 
    
    writeTiming.FSMC_AddressSetupTime = 0x00;    //地址建立时间（ADDSET）为1个HCLK  
    writeTiming.FSMC_AddressHoldTime = 0x00;   //地址保持时间（A     
    writeTiming.FSMC_DataSetupTime = 0x03;         ////数据保存时间为4个HCLK  
    writeTiming.FSMC_BusTurnAroundDuration = 0x00;
    writeTiming.FSMC_CLKDivision = 0x00;
    writeTiming.FSMC_DataLatency = 0x00;
    writeTiming.FSMC_AccessMode = FSMC_AccessMode_A;   //模式A 

    FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM4;//  这里我们使用NE4 ，也就对应BTCR[6],[7]。
    FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable; // 不复用数据地址
    FSMC_NORSRAMInitStructure.FSMC_MemoryType =FSMC_MemoryType_SRAM;// FSMC_MemoryType_SRAM;  //SRAM   
    FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;//存储器数据宽度为16bit   
    FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode =FSMC_BurstAccessMode_Disable;// FSMC_BurstAccessMode_Disable; 
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
    FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait=FSMC_AsynchronousWait_Disable; 
    FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;   
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;  
    FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;   //  存储器写使能
    FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;   
    FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Enable; // 读写使用不同的时序
    FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable; 
    FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &readWriteTiming; //读写时序
    FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &writeTiming;  //写时序

    FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);  //初始化FSMC配置

    FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM4, ENABLE);  // 使能BANK1 
}

static void write_address(ili9xxx_t *ili9xxx, uint32_t address)
{
    *((uint16_t *)(0x6C000000 | 0x00000000)) = address;
}

static void write_data(ili9xxx_t *ili9xxx, uint32_t data) 
{
    *((uint16_t *)(0x6C000000 | 0x00000800)) = data;
}

static uint32_t read_data(ili9xxx_t *ili9xxx) 
{
    return *((uint16_t *)(0x6C000000 | 0x00000800));
}

void ili9xxx_fsmc_init(ili9xxx_t *ili9xxx)
{
    fsmc_init();
    ili9xxx->write_address = write_address;
    ili9xxx->write_data = write_data;
    ili9xxx->read_data = read_data;
}

/****************************** Copy right 2019 *******************************/
