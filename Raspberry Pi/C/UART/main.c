/***************************************************************************************
 * Project  :rc522-uart-c
 * Describe :Read the UID of the S50 card and print the card number
 *			 Read block 8 data and can change the first 4 bytes of data by keyboard input
 * Experimental Platform :Raspberry Pi 4B + RC522 RFID HAT
 * Hardware Connection :
 *	SW1:	RX   -> ON			SW2:	A1	 -> -
 *          TX   -> ON					A0	 -> -
 *          SDA  -> OFF					ADR5 -> 0
 *          SCL  -> OFF					ADR4 -> 0
 *          NSS  -> OFF					ADR3 -> 0
 *          MOSI -> OFF					ADR2 -> 0
 *          MISO -> OFF					ADR1 -> 0
 *          SCK  -> OFF					ADR0 -> 0
 * Library Version :WiringPi_V2.52
 * Author		   :Christian
 * Web Site		   :
***************************************************************************************/
#include <stdio.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <softPwm.h>
#include "rc522.h"

int main()
{
    printf(" =======================================================\n");
    printf(" |	SW1:	RX   -> ON			SW2:	A1	 -> -      |\n");
    printf(" |          TX   -> ON					A0	 -> -      |\n");
    printf(" |          SDA  -> OFF					ADR5 -> 0      |\n");
    printf(" |          SCL  -> OFF					ADR4 -> 0      |\n");
    printf(" |          NSS  -> OFF					ADR3 -> 0      |\n");
    printf(" |          MOSI -> OFF					ADR2 -> 0      |\n");
    printf(" |          MISO -> OFF					ADR1 -> 0      |\n");
    printf(" |          SCK  -> OFF					ADR0 -> 0      |\n");
    printf(" =======================================================\n");
    
	if(wiringPiSetup()==-1)
	{
		printf("init wiringPi error\n");
	}
	serial_Fd=serialOpen("/dev/ttyS0", 9600); 
	if(serial_Fd==-1)
	{
		printf("init serial error!\n");
	}
	pinMode(Res,OUTPUT);
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
