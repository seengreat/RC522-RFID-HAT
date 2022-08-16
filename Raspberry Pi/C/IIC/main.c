/***************************************************************************************
 * Project  :rc522-iic-c
 * Describe :Read the UID of the S50 card and print the card number
 *			 Read block 8 data and can change the first 4 bytes of data by keyboard input
 * Experimental Platform :Raspberry Pi 4B + RC522 RFID HAT
 * Hardware Connection :
 *	SW1:	RX   -> OFF			SW2:	A1	 -> +
 *          TX   -> OFF					A0	 -> +
 *          SDA  -> ON					ADR5 -> +
 *          SCL  -> ON					ADR4 -> +
 *          NSS  -> OFF					ADR3 -> +
 *          MOSI -> OFF					ADR2 -> +
 *          MISO -> OFF					ADR1 -> +
 *          SCK  -> OFF					ADR0 -> +
 * Library Version :WiringPi_V2.52
 * Author		   :Christian
 * Web Site		   :
***************************************************************************************/
#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <softPwm.h>
#include "rc522.h"

int main()
{
	printf(" =======================================================\n");
    printf(" |	SW1:	RX   -> OFF			SW2:	A1	 -> +      |\n");
    printf(" |          TX   -> OFF					A0	 -> +      |\n");
    printf(" |          SDA  -> ON					ADR5 -> +      |\n");
    printf(" |          SCL  -> ON					ADR4 -> +      |\n");
    printf(" |          NSS  -> OFF					ADR3 -> +      |\n");
    printf(" |          MOSI -> OFF					ADR2 -> +      |\n");
    printf(" |          MISO -> OFF					ADR1 -> +      |\n");
    printf(" |          SCK  -> OFF					ADR0 -> +      |\n");
    printf(" =======================================================\n");

	if(wiringPiSetup()==-1)
	{
		printf("init wiringPi error\n");
	}
	i2c_Fd=wiringPiI2CSetup(0x3F); //addr:EA=1 ADR_0-ADR_5=1 =>0111111 =>00111111=>0x3F
	if(i2c_Fd==-1)
	{
		printf("init iic error!\n");
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

