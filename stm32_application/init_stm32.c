//
// Created by development on 8/13/17.
//

#include <stm32f10x/stm32f10x.h>
#include <stm32f10x/stm32f10x_pwr.h>
#include <stm32f10x/stm32f10x_dma.h>



#include "init_stm32.h"

/**
  * @brief  Configures the DMA.
  * @param  None
  * @retval None
  */


/**
  * @brief  Configure the nested vectored interrupt controller.
  * @param  None
  * @retval None
  */
void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* 1 bit for pre-emption priority, 3 bits for subpriority */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

    /* Configure and enable SPI_SLAVE interrupt --------------------------------*/
    NVIC_InitStructure.NVIC_IRQChannel = SPI1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void initHW() {
    // Init basic stuff
    SystemInit();

    /* Debug support for low power modes: */
    DBGMCU_Config(DBGMCU_SLEEP, ENABLE);
    DBGMCU_Config(DBGMCU_STOP, ENABLE);
    DBGMCU_Config(DBGMCU_STANDBY, ENABLE);

    // Enable clocks
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

    // Disable the fucking JTAG!
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
//	GPIO_PinRemapConfig(GPIO_Remap_SPI1, ENABLE);

    //GPIO_PinRemapConfig(GPIO_FullRemap_USART3, DISABLE);
    //GPIO_PinRemapConfig(GPIO_PartialRemap_USART3, ENABLE);

    // Prepare to switch on the leds
    GPIO_InitTypeDef gpioConfig;
    gpioConfig.GPIO_Mode = GPIO_Mode_Out_PP;
    gpioConfig.GPIO_Pin = GPIO_Pin_7;
    gpioConfig.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOB, &gpioConfig);

}



