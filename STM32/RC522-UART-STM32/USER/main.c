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
#include "sys.h"
#include "usart.h"
#include "gpio.h"
#include "delay.h"
#include "rc522.h"
#include "ic_card.h"
#include "timer.h"

int main(void)
{
	SystemInit();
    Uart1Init(9600,0,0);
	Uart3Init(9600,1,1);
	printf("\r\n---------- RC522-RFID-HAT ----------\r\n");
	printf("STM32F103C8T6 ");
	printf("V1.0.0 Build 2022/06/18 09:22\r\n");
	delay_init();
    IO_Init();
    RC522_Init();
	TIM3_PWM_Init(7000,6);//Tout=((arr)+(psc+1))/Tclk
    while(1)
	{
		ReadIDData();
	}
}

