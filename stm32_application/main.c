
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

// Add images
#include "ap_setup.h"
#include "lost_connection.h"
#include "low_battery.h"
#include "sleep.h"
#include "dhcp_error.h"
#include "dns_error.h"
#include "connection_error.h"
#include "invalid_image.h"

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

const void * image_table[8] =
{
	ap_setup,
	lost_connection,
	low_battery,
	sleep_mode,
	dhcp_error,
	dns_error,
	connection_error,
	invalid_image
};

__IO uint16_t RxIdx = 0;
unsigned char scratch[60*1024]; // __attribute__((section(".extdata"), used));

// Interrupt routine make some advance to the 
void SPI1_Handler(void)
{
	scratch[RxIdx++] = SPI_I2S_ReceiveData(SPI1);
}

/**
  * @brief  Configures the DMA.
  * @param  None
  * @retval None
  */
void DMA_Configuration(void)
{
  DMA_InitTypeDef DMA_InitStructure;

  /* USART1 RX DMA1 Channel (triggered by USART1 Rx event) Config */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  DMA_DeInit(DMA1_Channel5);
  DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40013804;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)scratch;
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


void USART_Initialize() {
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

void USART_Write(const char * txt) {
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


 int main() {
	// Init HW for the micro
	initHW();

	// Fuckin SRAM memory has stopped working
	// That means only 60KB RAM, either B/W images (1bit) or compressed images!
	//FSMC_SRAM_Init();
    DMA_Configuration();

     sync_blink();

     memcpy(scratch, image_table[2], sizeof(scratch));

einkd_init(1);

// Power ON, draw and OFF again!
einkd_PowerOn();
einkd_refresh_compressed(scratch);
einkd_PowerOff();
     RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_SPI1, DISABLE);
     einkd_deinit();

USART_Initialize();

//  USART_Write("Starting eInk Panel /r/n");
//     USART_Write("/r/n");

     unsigned char read_int =0;

     while (1) {


         for (char ch = ' '; ch <= 'z'; ch++) {
             read_int = USART_ReadByteSync(USART1, 0);

         if (read_int==0x00) {
             delay_ms(999);
         } else {
             USART_WriteInt(read_int);
             read_int=0;
         }
         }

     }




	// Wait for the first byte, that tells us what to do:
	unsigned char cmd = 0; //USART_ReadByteSync(USART1, 0);
	// Wait for the magic word
        while (1) {
		while (cmd != 0x2d) { 
                   cmd = USART_ReadByteSync(USART1, 0);
                   USART_Write("Got 1st magic word: "); USART_WriteInt(cmd); USART_Write("\r\n");
                }
		cmd = USART_ReadByteSync(USART1, 0);
                USART_Write("Got 2nd magic word: "); USART_WriteInt(cmd); USART_Write("\r\n");
		if (cmd == 0x5a) break;
	}
        USART_Write("Got magic word: "); USART_WriteInt(cmd); USART_Write("\r\n");
	
	// Then read command
	cmd = USART_ReadByteSync(USART1, 0);


	// Bit   7 defines direction hint (which can be ignored by the device)
	// Bit   6 tells whether to show a predefined picture (0) or to load a picture (1)
	//         If the bit is 1, it will be followed by 120000 bytes with the picture content
	// Bit   5 indicates whether the battery icon should be overlayed to the image
	// Bit 2,0 defines which preloaded picture to show (from the 4 in-ROM available)

	int direction = (cmd & 0x80) ? 1 : 0;
	int int_image = ((cmd & 0x40) == 0);
	int show_bat  = ((cmd & 0x20) == 0);
	int imageidx  =  cmd & 0x7;

	if (!int_image) {
		// Keep reading for external image!
		unsigned int spointer = 0, bl = 1, waiter = 0;
		USART_StartDMA();
		USART_Write("Received cmd: "); USART_WriteInt(cmd); USART_Write("\r\n");
		/* Wait until USARTy RX DMA1 Channel Transfer Complete */
		while (DMA_GetFlagStatus(DMA1_FLAG_TC5) == RESET && waiter < 0xFF00000) { waiter++; }
		if (waiter == 0xFF00000) { USART_Write("Failed at: "); spointer = DMA_GetCurrDataCounter(DMA1_Channel5); USART_WriteInt(spointer); USART_Write(" last byte: "); USART_WriteInt(scratch[spointer - 2]); 
USART_Write("\r\n"); waiter = 0; }
/*
		while (spointer < sizeof(scratch)) {
			scratch[spointer++] = USART_ReadByteSync(USART1, &waiter);
			if ((spointer & 0x1FFF) == 0) { blink(bl); bl = !bl; }
			if (waiter) { USART_Write("Failed at: "); USART_WriteInt(spointer); USART_Write(" last byte: "); USART_WriteInt(scratch[spointer - 2]); USART_Write("\r\n"); waiter = 0; }
		}
*/

		USART_Write("Done receiving picture\r\n");
		blink(0);
	}
	else {
		// Copy the internal compressed image
		memcpy(scratch, image_table[imageidx], sizeof(scratch));
	}

        USART_Write("Init screen\r\n");
	// Initialize tables (according to direction)
	einkd_init(direction);

	// Power ON, draw and OFF again!
	einkd_PowerOn();
	einkd_refresh_compressed(scratch);
	einkd_PowerOff();

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_SPI1, DISABLE);
	einkd_deinit();
	USART_Write("Done setting the screen, sleeping now\r\n");

	// Turn ourselves OFF, hopefully save some power before final power gate off
	PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
	while (1);

	return 0;
}


