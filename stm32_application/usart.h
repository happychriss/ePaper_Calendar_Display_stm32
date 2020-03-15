//
// Created by development on 8/13/17.
//

#ifndef STM32_DISPLAY_USART_C_H
#define STM32_DISPLAY_USART_C_H

void Serial_USART_Initialize();
void USART_StartDMA();
void USART_Write(const char * txt);
void USART_WriteInt(int v);
uint32_t USART_ReadString(char *data, uint32_t max_size);
void USART_WriteStatus (uint8_t status);
unsigned char USART_ReadByteSync(USART_TypeDef *USARTx, unsigned * waiter);

#endif //STM32_DISPLAY_USART_C_H
