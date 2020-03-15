//
// Created by development on 8/13/17.
//

#ifndef STM32_DISPLAY_SUPPORT_FUNCTIONS_H
#define STM32_DISPLAY_SUPPORT_FUNCTIONS_H

// #define MYDEBUG

#ifdef MYDEBUG
#define DP(x)     USART_Write(x);
#define DPD(x)     USART_WriteInt(x);USART_WRITE("\n\r");
#define DPL(x)  USART_Write(x);USART_WRITE("\n\r");
#else
#define DP(x)
#define DPD(x)
#define DPL(x)
#endif

#define DB_BUFFER 600
extern char mbuf[DB_BUFFER];
#define DB(x) dp(x,mbuf)

void dp(int x,char *mbuf);

void delay_us(uint32_t time_us);
void delay_ms(uint16_t time_ms);
void sync_blink();
void blink(int v);
#endif //STM32_DISPLAY_SUPPORT_FUNCTIONS_H
