/***************************************************************************************
 * Project  :rc522-spi-c
 * Describe :Read the UID of the S50 card and print the card number
 *			 Read block 8 data and can change the first 4 bytes of data by keyboard input
 * Experimental Platform :Raspberry Pi 4B + RC522 RFID HAT
 * Hardware Connection :
 *	SW1:	RX   -> OFF			SW2:	A1	 -> -
 *          TX   -> OFF					A0	 -> +
 *          SDA  -> OFF					ADR5 -> 0
 *          SCL  -> OFF					ADR4 -> 0
 *          NSS  -> ON					ADR3 -> 0
 *          MOSI -> ON					ADR2 -> 0
 *          MISO -> ON					ADR1 -> 0
 *          SCK  -> ON					ADR0 -> 0
 * Library Version :WiringPi_V2.52
 * Author		   :Christian
 * Web Site		   :
***************************************************************************************/
#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <softPwm.h>
#include "rc522.h"

int main()
{
    printf(" =======================================================\n");
    printf(" |	SW1:	RX   -> OFF			SW2:	A1	 -> -      |\n");
    printf(" |          TX   -> OFF					A0	 -> +      |\n");
    printf(" |          SDA  -> OFF					ADR5 -> 0      |\n");
    printf(" |          SCL  -> OFF					ADR4 -> 0      |\n");
    printf(" |          NSS  -> ON					ADR3 -> 0      |\n");
    printf(" |          MOSI -> ON					ADR2 -> 0      |\n");
    printf(" |          MISO -> ON					ADR1 -> 0      |\n");
    printf(" |          SCK  -> ON					ADR0 -> 0      |\n");
    printf(" =======================================================\n");

	int spi_Fd;
	if(wiringPiSetup()==-1)
	{
		printf("init wiringPi error\n");
	}
	spi_Fd=wiringPiSPISetup(0,500000); 
	if(spi_Fd==-1)
	{
		printf("init spi failed!\n");
	}
	pinMode(RST,OUTPUT);
	pinMode(LED, OUTPUT);
	pinMode(PWM, PWM_OUTPUT);
	RC522_Init();
	softPwmCreate(PWM, 0, 10);
	while(1)
	{
		ReadIDData();
	}
	return 0;
}
