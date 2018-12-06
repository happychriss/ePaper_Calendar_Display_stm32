
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <stm32f10x/stm32f10x.h>
#include <stm32f10x/stm32f10x_pwr.h>
#include <stm32f10x/stm32f10x_dma.h>

#include "gde043a2.h"
#include "sram.h"
#include "uart.h"
#include "imgdec.h"
#include "painter.h"

#include "usart.h"
#include "support_functions.h"
#include "init_stm32.h"

unsigned char scratch[60 * 1024]; // __attribute__((section(".extdata"), used));

void DMA_Configuration_USART(void) {
    DMA_InitTypeDef DMA_InitStructure;

    /* USART1 RX DMA1 Channel (triggered by USART1 Rx event) Config */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    DMA_DeInit(DMA1_Channel5);
    DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40013804;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) scratch;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = sizeof(scratch);
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel5, &DMA_InitStructure);



}


/*

// Interrupt routine make some advance to the 
void SPI1_Handler(void)
{
	scratch[RxIdx++] = SPI_I2S_ReceiveData(SPI1);
}
*/


int main() {
    // Init HW for the micro
    initHW();

    FSMC_SRAM_Init();
    DMA_Configuration_USART();
    InitDisplayMemory();
    einkd_refresh_from_sram();
    

    einkd_init(1);

    // Power ON, draw and OFF again!
    einkd_PowerOn();
    InitDisplayMemory();
    einkd_refresh_from_sram();
    einkd_PowerOff();
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_SPI1, DISABLE);
    einkd_deinit();

    USART_Initialize();
    delay_ms(999);
    USART_Write("Done my  job /r/n");
    USART_Write("/r/n");

    // Turn ourselves OFF, hopefully save some power before final power gate off
    PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
    while (1);

    return 0;
}


