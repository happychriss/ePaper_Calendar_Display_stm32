
#include <stm32f10x/stm32f10x.h>
#include <stm32f10x/stm32f10x_fsmc.h>


#include "uart.h"
#include "usart.h"

// SRAM initialization for the waveshare e-ink board.
// It ships an IS62WV51216BLL (1MB RAM, 512K * 16)
// STM32 runs with 72MhZ


// #define Bank1_SRAM3_ADDR ((uint32_t)0x68000000)
//

// Put the Memory for the screen buffer at the end of SRAM:
// SRAM: 512*1204*16
// SRAM: 512*1204*16 - 60.0000 for the screen
#define Bank1_SRAM3_ADDR ((uint32_t)0x68000000)
//#define Bank1_SRAM3_ADDR ((uint32_t)0x687F1590)


void FSMC_SRAM_Init() {
	// Enable FSMC clock
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);

	// Enable ports DEFG
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOD | 
		RCC_APB2Periph_GPIOG| RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOF, ENABLE);

	// Init all ports to AF_PP, this covers A19-0, D15-0, NOE, NWE, NE3, NE4, NBL0 & NBL1
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;

	GPIO_InitStructure.GPIO_Pin = 0xff33;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = 0xFF88 | 0x3; // PE15-7, PE3  PE0 PE1
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = 0xf03f; // PF15-12, PF5-0
	GPIO_Init(GPIOF, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = 0x3f | 0x1400; // PG5-0 PG12 PG10
	GPIO_Init(GPIOG, &GPIO_InitStructure);

	// Set up SRAM timings and stuff
	FSMC_NORSRAMTimingInitTypeDef timing;
	FSMC_NORSRAMInitTypeDef sram;

	timing.FSMC_AddressSetupTime = 6;   // 3 HCLK cycles ...? 41ns? I think I should double this for safety!
	timing.FSMC_AddressHoldTime = 0;
	timing.FSMC_DataSetupTime = 6;
	timing.FSMC_BusTurnAroundDuration = 0;
	timing.FSMC_CLKDivision = 0;
	timing.FSMC_DataLatency = 0;
	timing.FSMC_AccessMode = 0;

	sram.FSMC_Bank = FSMC_Bank1_NORSRAM3;
	sram.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
	sram.FSMC_MemoryType = FSMC_MemoryType_SRAM;
	sram.FSMC_MemoryDataWidth = 16;
	sram.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
	sram.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
	sram.FSMC_WrapMode = FSMC_WrapMode_Disable;
	sram.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
	sram.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
	sram.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
	sram.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
	sram.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
	sram.FSMC_ReadWriteTimingStruct = &timing;
	sram.FSMC_WriteTimingStruct = &timing;

	// Setup RAM!
	FSMC_NORSRAMInit(&sram);

	// Enable it!
	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM3, ENABLE);
}


/**
  * @brief  Writes a Half-word buffer to the FSMC SRAM memory.
  * @param  pBuffer : pointer to buffer.
  * @param  WriteAddr : SRAM memory internal address from which the data
  *        will be written.
  * @param  NumHalfwordToWrite : number of half-words to write.
  * @retval None
  */
void FSMC_SRAM_WriteBuffer(uint16_t* pBuffer, uint32_t WriteAddr, uint32_t NumHalfwordToWrite)
{


    for(; NumHalfwordToWrite != 0; NumHalfwordToWrite--) /* while there is data to write */
    {
        /* Transfer data to the memory */
        *(uint16_t *) (Bank1_SRAM3_ADDR + WriteAddr) = *pBuffer++;

        /* Increment the address*/
        WriteAddr += 2;
    }
}
// Word in the ARM context is 32-bit
// SRAM: 512K words by 16 bits
// NumHalfwordToWrite = number of 16bit words to write

void FSMC_SRAM_FillBuffer(uint16_t value, uint32_t WriteAddr, uint32_t NumHalfwordToWrite)
{
    for(; NumHalfwordToWrite != 0; NumHalfwordToWrite--) /* while there is data to write */
    {
        /* Transfer data to the memory */
        *(uint16_t *) (Bank1_SRAM3_ADDR + WriteAddr) = value;

        /* Increment the address*/
        WriteAddr += 2;
    }
}



/**
  * @brief  Reads a block of data from the FSMC SRAM memory.
  * @param  pBuffer : pointer to the buffer that receives the data read
  *        from the SRAM memory.
  * @param  ReadAddr : SRAM memory internal address to read from.
  * @param  NumHalfwordToRead : number of half-words to read.
  * @retval None
  */
void FSMC_SRAM_ReadBuffer(uint16_t* pBuffer, uint32_t ReadAddr, uint32_t NumHalfwordToRead)
{
    for(; NumHalfwordToRead != 0; NumHalfwordToRead--) /* while there is data to read */
    {
        /* Read a half-word from the memory */
        *pBuffer++ = *(__IO uint16_t*) (Bank1_SRAM3_ADDR + ReadAddr);

        /* Increment the address*/
        ReadAddr += 2;
    }
}