/***************************************************************************************
 * Project  :RC522-UART-STM32
 * Describe :Read the UID of the S50 card and print the card number
 *			 Read block 8 data and change the first four bytes to 0x11, 0x22, 0x33, 0x44
 * Experimental Platform :STM32F103C8T6 + RC522 RFID HAT
 * Hardware Connection :STM32F103 -> RC522 RFID HAT
 *			5V	 ->	5V					PB10 -> TX
 *			GND	 ->	GND					PB11 -> RX
 *			PA8	 ->	P25
 *			PB4  ->	P29
 *			PB5	 ->	P24
 *			
 *	SW1:	RX   -> ON			SW2:	A1	 -> -
 *          TX   -> ON					A0	 -> -
 *          SDA  -> OFF					ADR5 -> 0
 *          SCL  -> OFF					ADR4 -> 0
 *          NSS  -> OFF					ADR3 -> 0
 *          MOSI -> OFF					ADR2 -> 0
 *          MISO -> OFF					ADR1 -> 0
 *          SCK  -> OFF					ADR0 -> 0
 * Library Version :ST_V3.5
 * Author		   :Christian
 * Web Site		   :www.seengreat.com
***************************************************************************************/
#include "gpio.h"

void IO_Init(void)
{	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE );
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable , ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;  // PB4 
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_4);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;  // PA8       RST
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOA, &GPIO_InitStructure);
 	GPIO_SetBits(GPIOA,GPIO_Pin_8);
} 

