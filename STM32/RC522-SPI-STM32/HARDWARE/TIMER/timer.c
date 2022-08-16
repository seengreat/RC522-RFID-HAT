/***************************************************************************************
 * Project  :RC522-SPI-STM32
 * Describe :Read the UID of the S50 card and print the card number
 *			 Read block 8 data and change the first four bytes to 0x11, 0x22, 0x33, 0x44
 * Experimental Platform :STM32F103C8T6 + RC522 RFID HAT
 * Hardware Connection :STM32F103 -> RC522 RFID HAT
 *			5V	 ->	5V					PB12 -> CE0
 *			GND	 ->	GND					PB13 -> SCK
 *			PA8	 ->	P25					PB14 -> MISO
 *			PB4  ->	P29					PB15 -> MOSI
 *			PB5	 ->	P24
 *			
 *	SW1:	RX   -> OFF			SW2:	A1	 -> -
 *          TX   -> OFF					A0	 -> +
 *          SDA  -> OFF					ADR5 -> 0
 *          SCL  -> OFF					ADR4 -> 0
 *          NSS  -> ON					ADR3 -> 0
 *          MOSI -> ON					ADR2 -> 0
 *          MISO -> ON					ADR1 -> 0
 *          SCK  -> ON					ADR0 -> 0
 * Library Version :ST_V3.5
 * Author		   :Christian
 * Web Site		   :www.seengreat.com
***************************************************************************************/
#include "timer.h"
#include "delay.h"

void TIM3_PWM_Init(u16 arr,u16 psc)
{  
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);	
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB  | RCC_APB2Periph_AFIO, ENABLE);  
	
	GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE); 
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
 
	TIM_TimeBaseStructure.TIM_Period = arr; 
	TIM_TimeBaseStructure.TIM_Prescaler =psc; 
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; 
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; 
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; 
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; 
	TIM_OC2Init(TIM3, &TIM_OCInitStructure); 

	TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable); 
 
	TIM_Cmd(TIM3, ENABLE);  
}

void beep(u8 num)
{
	while(num--)
    {
		TIM_SetCompare2(TIM3,3000);
		delay_ms(100);
		TIM_SetCompare2(TIM3,0);
		delay_ms(100);
	}
}

