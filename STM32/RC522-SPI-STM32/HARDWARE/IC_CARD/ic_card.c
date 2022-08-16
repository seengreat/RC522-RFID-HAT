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
#include "ic_card.h"
#include "rc522.h"
#include "delay.h"
#include "gpio.h"
#include "usart.h"
#include "timer.h"

u8 CT[2];//Card type
u8 SN[4];//Card number
u8 sec[12] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};//IC card initial password
u8 WBlockData1[16] = {0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0xAA,0xBB,0xCC,0xFF,0xFF,0xFF,0xFF,0xFF};
u8 blockdata1[16]= {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
u8 blockdata2[16]= {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
u8 blockdata3[16]= {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
u8 blockdata4[16]= {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

void ReadIDData(void)
{
	u8 i,a;
	u8 ucStatusReturn = MI_ERR;
	u8 bFind = 0;
	if((ucStatusReturn = PcdRequest(PICC_REQIDL,CT)) != MI_OK) 
		ucStatusReturn = PcdRequest(PICC_REQIDL,CT);
	if(ucStatusReturn != MI_OK)                      
		bFind = 0;	
	if((ucStatusReturn == MI_OK) && (bFind == 0))
	{
		if(PcdAnticoll(SN) == MI_OK) 
		{
			delay_ms(250);
			if(PcdSelect(SN) == MI_OK)
			{
				printf("Card ID:");
				for(i=0;i<4;i++)
				{
					printf("%d",SN[i]);
				}
				printf("\r\n");
				beep(1);
				LED=0;
				delay_ms(200);
				LED=1;
				delay_ms(200);
				if(PcdAuthState(C_A,8,sec,SN) == MI_OK)
				{
					printf("Read block data\r\n");
					PcdRead(8,blockdata1);
					printf("block_8=[ ");
					for(a=0;a<16;a++)
					{
						printf("%02X ",blockdata1[a]);
					}
					printf("]\r\n");
					blockdata1[0]=0x11;
					blockdata1[1]=0x22;
					blockdata1[2]=0x33;
					blockdata1[3]=0x44;
					PcdWrite(8,blockdata1);
					printf("----------------------------------\r\n");
					delay_ms(1000);
				}
			}
		}
	}
}

