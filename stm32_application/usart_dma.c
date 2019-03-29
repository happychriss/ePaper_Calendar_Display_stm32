//
// Created by development on 16.12.18.
//
#include <stm32f10x/stm32f10x.h>
#include <stm32f10x/stm32f10x_pwr.h>
#include <stm32f10x/stm32f10x_dma.h>
#include <malloc.h>
#include "usart_dma.h"


// unsigned char scratch[60 * 1024] __attribute__((section(".extdata"), used));

void DMA_Configuration_USART(void) {
    char *scratch = (char *) calloc((size_t ) 60*1024 ,sizeof(char));
   
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