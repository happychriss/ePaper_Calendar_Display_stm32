//
// Created by development on 8/13/17.
//

#include <stm32f10x/stm32f10x_conf.h>
#include <stm32f10x/stm32f10x_dma.h>
#include "usart.h"


void Serial_USART_Initialize() {
    USART_InitTypeDef usartConfig;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOB, ENABLE);

    usartConfig.USART_BaudRate = 9600;//460800; //115200;
    usartConfig.USART_WordLength = USART_WordLength_8b;
    usartConfig.USART_StopBits = USART_StopBits_1;
    usartConfig.USART_Parity = USART_Parity_No;
    usartConfig.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    usartConfig.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART1, &usartConfig);

    GPIO_InitTypeDef gpioConfig;

    // PA9 = USART1.TX => Alternative Function Output
    gpioConfig.GPIO_Mode = GPIO_Mode_AF_PP;
    gpioConfig.GPIO_Pin = GPIO_Pin_9;
    gpioConfig.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpioConfig);

    // PA10 = USART1.RX => Input
    gpioConfig.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    gpioConfig.GPIO_Pin = GPIO_Pin_10;
    GPIO_Init(GPIOA, &gpioConfig);

    USART_Cmd(USART1, ENABLE);
}



void USART_StartDMA() {
    USART_Cmd(USART1, DISABLE);
    /* Enable USART1 DMA Rx request */
    USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);

    /* Enable USARTy RX DMA1 Channel */
    DMA_Cmd(DMA1_Channel5, ENABLE);

    // Reenable UART1
    USART_Cmd(USART1, ENABLE);
}



void USART_Write(const char *txt) {
    while (*txt) {
        while (!(USART1->SR & USART_SR_TXE)) {}
        USART_SendData(USART1, *txt);
        ++txt;
    }
}

void USART_WriteInt(int v) {
    char intV[11] = {'0', 'x', '0', '0', '0', '0', '0', '0', '0', '0', 0 };
    static const char hexMap[] = "0123456789ABCDEF";
    intV[2] = hexMap[(v & 0xF0000000) >> 28];
    intV[3] = hexMap[(v & 0x0F000000) >> 24];
    intV[4] = hexMap[(v & 0x00F00000) >> 20];
    intV[5] = hexMap[(v & 0x000F0000) >> 16];
    intV[6] = hexMap[(v & 0x0000F000) >> 12];
    intV[7] = hexMap[(v & 0x00000F00) >>  8];
    intV[8] = hexMap[(v & 0x000000F0) >>  4];
    intV[9] = hexMap[(v & 0x0000000F) >>  0];
    USART_Write(intV);
}

unsigned char USART_ReadByteSync(USART_TypeDef *USARTx, unsigned * waiter) {
    unsigned count = 0xFF00000;
    while ((USARTx->SR & USART_SR_RXNE) == 0 && --count) {}
    if (!count && waiter) *waiter = 1;
    return (unsigned char)USART_ReceiveData(USARTx);
}

