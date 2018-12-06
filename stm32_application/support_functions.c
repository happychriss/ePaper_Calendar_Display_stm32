//
// Created by development on 8/13/17.
//


#include <sched.h>
#include <stm32f10x/stm32f10x.h>
#include <stm32f10x/core_cm3.h>
#include <stm32f10x/stm32f10x.h>
#include "support_functions.h"

void delay_us(uint32_t time_us)
{
    SysTick->LOAD  = 72 * time_us-1;
    SysTick->VAL   = 0;                                          /* Load
the SysTick Counter Value */
    SysTick->CTRL  |= SysTick_CTRL_ENABLE_Msk;                   /* Enable
SysTick Timer */

    do
    {
    } while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG)==0);
    SysTick->CTRL  &= ~SysTick_CTRL_ENABLE_Msk;                  /*
Disable SysTick Timer */
    SysTick->VAL   = 0;                                          /* Load
the SysTick Counter Value */
}

void delay_ms(uint16_t time_ms)
{
    while (time_ms>0)
    {
        delay_us(1000);
        time_ms--;
    }
}

void sync_blink() {
    // Some blink
    int i;
    GPIO_SetBits(GPIOB, GPIO_Pin_7);
    for (i = 0; i < 3000000; i++) { asm volatile(""); }
    GPIO_ResetBits(GPIOB, GPIO_Pin_7);
    for (i = 0; i < 3000000; i++) { asm volatile(""); }
    GPIO_SetBits(GPIOB, GPIO_Pin_7);
    for (i = 0; i < 3000000; i++) { asm volatile(""); }
    GPIO_ResetBits(GPIOB, GPIO_Pin_7);
    for (i = 0; i < 3000000; i++) { asm volatile(""); }
}

void blink(int v) {
    v ? GPIO_SetBits(GPIOB, GPIO_Pin_7) : GPIO_ResetBits(GPIOB, GPIO_Pin_7);
}
