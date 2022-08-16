/***************************************************************************************
 * Project  :RC522-IIC-STM32
 * Describe :Read the UID of the S50 card and print the card number
 *			 Read block 8 data and change the first four bytes to 0x11, 0x22, 0x33, 0x44
 * Experimental Platform :STM32F103C8T6 + RC522 RFID HAT
 * Hardware Connection :STM32F103 -> RC522 RFID HAT
 *			5V	 ->	5V					PB10 -> SCL1
 *			GND	 ->	GND					PB11 -> SDA1
 *			PA8	 ->	P25
 *			PB4  ->	P29
 *			PB5	 ->	P24
 *			
 *	SW1:	RX   -> OFF			SW2:	A1	 -> +
 *          TX   -> OFF					A0	 -> +
 *          SDA  -> ON					ADR5 -> +
 *          SCL  -> ON					ADR4 -> +
 *          NSS  -> OFF					ADR3 -> +
 *          MOSI -> OFF					ADR2 -> +
 *          MISO -> OFF					ADR1 -> +
 *          SCK  -> OFF					ADR0 -> +
 * Library Version :ST_V3.5
 * Author		   :Christian
 * Web Site		   :www.seengreat.com
***************************************************************************************/
#include "myiic.h"
#include "delay.h"

void IIC_Init(void)
{					     
 	RCC->APB2ENR|=1<<3;		 
	GPIOB->CRH&=0XFFF000FF;//PB10,11 
	GPIOB->CRH|=0X00033300;	   
	GPIOB->ODR|=3<<10;     //PB10,11 
}

void IIC_Start(void)
{
	SDA_OUT();  
	IIC_SDA=1;	  	  
	IIC_SCL=1;
	delay_us(4);
 	IIC_SDA=0;//START:when CLK is high,DATA change form high to low 
	delay_us(4);
	IIC_SCL=0;
}	  

void IIC_Stop(void)
{
	SDA_OUT();
	IIC_SCL=0;
	IIC_SDA=0;//STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	IIC_SCL=1; 
	IIC_SDA=1;
	delay_us(4);							   	
}

u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime=0;
	SDA_IN();    
	IIC_SDA=1;delay_us(1);	   
	IIC_SCL=1;delay_us(1);	 
	while(READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			IIC_Stop();
			return 1;
		}
	}
	IIC_SCL=0;
	return 0;  
} 

void IIC_Ack(void)
{
	IIC_SCL=0;
	SDA_OUT();
	IIC_SDA=0;
	delay_us(2);
	IIC_SCL=1;
	delay_us(2);
	IIC_SCL=0;
}
    
void IIC_NAck(void)
{
	IIC_SCL=0;
	SDA_OUT();
	IIC_SDA=1;
	delay_us(2);
	IIC_SCL=1;
	delay_us(2);
	IIC_SCL=0;
}					 				     
		  
void IIC_Send_Byte(u8 txd)
{                        
    u8 t;   
	SDA_OUT(); 	    
    IIC_SCL=0;
    for(t=0;t<8;t++)
    {              
        IIC_SDA=(txd&0x80)>>7;
        txd<<=1; 	  
		delay_us(2);   
		IIC_SCL=1;
		delay_us(2); 
		IIC_SCL=0;	
		delay_us(2);
    }	 
} 	    
 
u8 IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	SDA_IN();
    for(i=0;i<8;i++ )
	{
        IIC_SCL=0; 
        delay_us(2);
		IIC_SCL=1;
        receive<<=1;
        if(READ_SDA)receive++;   
		delay_us(1); 
    }					 
    if (!ack)
        IIC_NAck();
    else
        IIC_Ack();
    return receive;
}

