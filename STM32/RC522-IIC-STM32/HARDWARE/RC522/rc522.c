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
#include "rc522.h"
#include <string.h>
#include "gpio.h"
#include "delay.h"
#include "usart.h"
#include "myiic.h"

unsigned char fHasRATS = 0;

void RC522_WR_Reg(u8 RCsla,u8 addr,u8 val)
{
	IIC_Start();  				 
	IIC_Send_Byte(RCsla);     	
	IIC_Wait_Ack();	   
    IIC_Send_Byte(addr);
	IIC_Wait_Ack(); 	 										  		   
	IIC_Send_Byte(val);     			   
	IIC_Wait_Ack();  		    	   
    IIC_Stop();				
}

unsigned char RC522_RD_Reg(u8 RCsla,u8 addr)
{
	u8 temp=0;		 
	IIC_Start();  				 
	IIC_Send_Byte(RCsla);	
	temp=IIC_Wait_Ack();	   
    IIC_Send_Byte(addr);   	
	temp=IIC_Wait_Ack(); 	 										  		   
	IIC_Start();  	 	   		
	IIC_Send_Byte(RCsla+1);	 
	temp=IIC_Wait_Ack();	   
    temp=IIC_Read_Byte(0);		    	   
    IIC_Stop();				 
	return temp;			
}  

/////////////////////////////////////////////////////////////////////
//function:Read register RC522
//Parameters that:Address[IN]:Register address
//return:Read data
/////////////////////////////////////////////////////////////////////
unsigned char ReadRawRC(unsigned char Address)
{
	unsigned char ucResult;
	ucResult = RC522_RD_Reg(I2C_ADDR,Address);    //read reg
	return ucResult;
}

/////////////////////////////////////////////////////////////////////
//function:Write register RC522
//Parameters that:Address[IN]:Register address
//                  value[IN]:Written data
/////////////////////////////////////////////////////////////////////
void WriteRawRC(unsigned char Address, unsigned char value)
{
	RC522_WR_Reg(I2C_ADDR,Address,value);
}

/////////////////////////////////////////////////////////////////////
//function:Set register RC522 bit
//Parameters:reg[IN]:Register address
//          mask[IN]:Setting value
/////////////////////////////////////////////////////////////////////
void SetBitMask(unsigned char reg,unsigned char mask)
{
    unsigned char  tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg,tmp | mask);  // set bit mask
}

/////////////////////////////////////////////////////////////////////
//function:Clear register bit RC522
//Parameters:reg[IN]:Register address
//          mask[IN]:Setting value
/////////////////////////////////////////////////////////////////////
void ClearBitMask(unsigned char reg,unsigned char mask)
{
    unsigned char  tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg, tmp & ~mask);  // clear bit mask
} 

/////////////////////////////////////////////////////////////////////
//function:Open the antenna
//There shall be a minimum interval of 1ms between each start or close of the celestial launch
/////////////////////////////////////////////////////////////////////
void PcdAntennaOn()
{
    SetBitMask(TxControlReg, 0x03);
    delay_ms(10);
}

/////////////////////////////////////////////////////////////////////
//function:Close the antenna
/////////////////////////////////////////////////////////////////////
void PcdAntennaOff()
{
    ClearBitMask(TxControlReg, 0x03);
    delay_ms(10);
}

/////////////////////////////////////////////////////////////////////
//function:Initialize for ISO14443A type card
//Parameters:ucType[IN]:Card type
/////////////////////////////////////////////////////////////////////
void M500PcdConfigISOType(unsigned char ucType)
{
    if ( ucType == 'A')   //ISO14443A
    {
        ClearBitMask ( Status2Reg, 0x08 ); 
        WriteRawRC ( ModeReg, 0x3D );
        WriteRawRC ( RxSelReg, 0x86 );   
        WriteRawRC( RFCfgReg, 0x6F );   	
        WriteRawRC( TReloadRegL, 30 );
        WriteRawRC ( TReloadRegH, 0 );
        WriteRawRC ( TModeReg, 0x8D );	
        WriteRawRC ( TPrescalerReg, 0x3E );	
        delay_ms(1);
        PcdAntennaOn();
    }
}

/////////////////////////////////////////////////////////////////////
//function:Through RC522 and ISO14443 cartoon news
//Parameters:Command[IN]:RC522 command word
//           pInData[IN]:Data sent to the card via RC522
//         InLenByte[IN]:Length of sent data in bytes
//	       pOutData[OUT]:The received card returns data
//	     pOutLenBit[OUT]:The bit length of the returned data
//		     TimeOut[IN]:timeout
/////////////////////////////////////////////////////////////////////
unsigned char PcdComMF522_P(unsigned char Command, 
							 unsigned char *pInData, 
							 unsigned char InLenByte,
							 unsigned char *pOutData, 
							 unsigned int  *pOutLenBit,
							 unsigned short TimeOut)
{
    unsigned char n,status;
    unsigned int i;
    Delay1us(100);
    WriteRawRC(TPrescalerReg,0xFF);
    WriteRawRC(TModeReg,0x87);		
    WriteRawRC(TReloadRegL,(unsigned char)TimeOut); 
    WriteRawRC(TReloadRegH,(unsigned char)(TimeOut>>8));
    Delay1us(10);
    WriteRawRC(ComIrqReg,0x7F);
    WriteRawRC(DivIrqReg,0x7F);
    WriteRawRC(FIFOLevelReg,0x80);
    SetBitMask(CommandReg,Command);
    Delay1us(1);
    SetBitMask(ComIEnReg,0xA1);		
    SetBitMask(DivlEnReg,0x00);		
    //-------------------------------
    for(n=0;n<InLenByte;n++)		
    {
		WriteRawRC(FIFODataReg,pInData[n]);
	}
    SetBitMask(BitFramingReg,0x80);
    while(1)
    {
		n = ReadRawRC(ComIrqReg);
		status = ReadRawRC(DivIrqReg);
		if(n & 0x20)
			break;
		if(n & 0x01)
			break;
    }
    if(!(n&0x01))
    {
		SetBitMask(ControlReg,0x90);
		ClearBitMask(ComIEnReg,0x7F);
		ClearBitMask(DivlEnReg,0xFF);
		n = ReadRawRC(FIFOLevelReg);
		ReadRawRC(ControlReg);
		status = ReadRawRC(ErrorReg);
		for (i=0; i<n; i++)
		{   
			pOutData[i] = ReadRawRC(FIFODataReg);    
		}
		*pOutLenBit = n*8;
		WriteRawRC(ComIrqReg,0x20);
    }
    else
    {
		ClearBitMask(ComIEnReg,0x7F);
		ClearBitMask(DivlEnReg,0xFF);
		if(pInData[0] == PICC_HALT)
		{
			ClearBitMask(ComIrqReg,0xFF);
			return 0;	
		}
		WriteRawRC(ComIrqReg,0x01);
		status = MI_TIMEOUT;
    }
    WriteRawRC(DivIrqReg,0x00);
    WriteRawRC(FIFOLevelReg,0x80);
    WriteRawRC(ComIrqReg,0x01);
    WriteRawRC(BitFramingReg,0x00);
    return status;
}

/////////////////////////////////////////////////////////////////////
//function:Initialize RC522
/////////////////////////////////////////////////////////////////////
void RC522_Init()
{
    unsigned char Temp;
    printf("RC522 RST:");	
    macRC522_Reset_Disable();	
    delay_ms(10);
    macRC522_Reset_Enable();	
    delay_ms(10);	
    macRC522_Reset_Disable();	
    delay_ms(10);
    WriteRawRC(CommandReg,PCD_RESETPHASE);
    delay_ms(10);
    WriteRawRC(ControlReg,0x10);
    WriteRawRC(ModeReg,0x3F);   
    WriteRawRC(RFU23,0x00);
    WriteRawRC(RFU25,0x80);
    WriteRawRC(AutoTestReg,0x40);
    WriteRawRC(TxAutoReg,0x40);
    ReadRawRC(TxAutoReg);
    SetBitMask(TxControlReg,0x03);
    WriteRawRC(TPrescalerReg,0x3D);
    WriteRawRC(TModeReg,0x0D);	
    WriteRawRC(TReloadRegL,0x0A);
    WriteRawRC(TReloadRegH,0x00);
    WriteRawRC(ComIrqReg,0x01);
    SetBitMask(ControlReg,0x40);
    do
    {
		Temp = ReadRawRC(ComIrqReg);
    }
    while(!(Temp&0x01));	
    WriteRawRC(ComIrqReg,0x01);
    WriteRawRC(CommandReg,0x00);	
    while( ReadRawRC(0x27) != 0x88)
    {
		delay_ms(10);
    }
    printf("OK\r\n");	
    M500PcdConfigISOType ('A');
}

/////////////////////////////////////////////////////////////////////
//function:Find card
//Parameters:req_code[IN]:Find card way
//                0x52 = Find all cards in the zone conforming to 14443A standard
//                0x26 = Find the card that has not entered hibernation
//          pTagType[OUT]:Card type code
//              0x4400 = Mifare_UltraLight
//              0x0400 = Mifare_One(S50)
//              0x0200 = Mifare_One(S70)
//              0x0800 = Mifare_Pro(X)
//              0x4403 = Mifare_DESFire
//return:Successfully returns MI_OK
/////////////////////////////////////////////////////////////////////
unsigned char PcdRequest(unsigned char req_code,unsigned char *pTagType)
{
    char status = 0;
    unsigned int  unLen = 0;
    unsigned char ucComMF522Buf[MAXRLEN] = {0};
    char i=0;
    if(fHasRATS != 1)
    {
		do
		{
			status = ReadRawRC(Status2Reg);	
			WriteRawRC(Status2Reg,status&0xf7);	
			WriteRawRC(CollReg,0x80);		
			ClearBitMask(TxModeReg,0x80);	
			ClearBitMask(RxModeReg,0x80);			
			WriteRawRC(BitFramingReg,0x07); 
			SetBitMask(TxControlReg,0x03);  
			ucComMF522Buf[0] = req_code;
			i++;
			status = PcdComMF522_P(PCD_TRANSCEIVE,ucComMF522Buf,1,ucComMF522Buf,&unLen,0x0002);
			if(i>=2)
			break;
			Delay1us(5);
		}while(status ==MI_TIMEOUT);
    }
    else
    {
		SetBitMask(TxModeReg,0x80);
		SetBitMask(RxModeReg,0x80);
		ucComMF522Buf[0] = 0xCA;
		ucComMF522Buf[1] = 0x00;
		status = PcdComMF522_P(PCD_TRANSCEIVE,ucComMF522Buf,2,ucComMF522Buf,&unLen,0x00F0);
		//add below 3lines
		status = ReadRawRC(Status2Reg);	
		WriteRawRC(Status2Reg,status&0xf7);
		WriteRawRC(CollReg,0x80);		
		ClearBitMask(TxModeReg,0x80);	
		ClearBitMask(RxModeReg,0x80);	
		WriteRawRC(BitFramingReg,0x07);	
		//add 1 line
		SetBitMask(TxControlReg,0x03);
		ucComMF522Buf[0] = req_code;
		status = PcdComMF522_P(PCD_TRANSCEIVE,ucComMF522Buf,1,ucComMF522Buf,&unLen,0x0002);
		if(status == 0)
		{
			fHasRATS = 0;
		}
    }
    if ((status == MI_OK) && (unLen == 0x10))
    {
		*pTagType     = ucComMF522Buf[0];
		*(pTagType+1) = ucComMF522Buf[1];
    }
    else
    {   
		status = MI_ERR;  
    }
    return status;
}

/////////////////////////////////////////////////////////////////////
//function:Prevent a collision
//Parameters:pSnr[OUT]:Card serial number, 4 bytes
//return:Successfully returns MI_OK
/////////////////////////////////////////////////////////////////////  
unsigned char PcdAnticoll(unsigned char *pSnr)
{
    unsigned char status;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN];
    unsigned char i;
    unsigned char snr_check=0;    
    ClearBitMask(TxModeReg,0x80);
    ClearBitMask(RxModeReg,0x80);
    WriteRawRC(CollReg,0x00);	
    Delay1us(200);
    WriteRawRC(BitFramingReg,0x00);
    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x20;
    status = PcdComMF522_P(PCD_TRANSCEIVE,ucComMF522Buf,2,ucComMF522Buf,&unLen,0x0020);
    Delay1us(100);
    WriteRawRC(BitFramingReg,0x00);
    WriteRawRC(CollReg,0x80);
    if (status == MI_OK)
    {
		for (i=0; i<4; i++)
		{   
			*(pSnr+i)  = ucComMF522Buf[i];
			snr_check ^= ucComMF522Buf[i];
		}
		if (snr_check != ucComMF522Buf[i])
		{  
			status = MI_ERR;   
		}
    }
    return status;
}

/////////////////////////////////////////////////////////////////////
//function:Selected card
//Parameters:pSnr[IN]:Card serial number, 4 bytes
//return:Successfully returns MI_OK
/////////////////////////////////////////////////////////////////////
unsigned char PcdSelect(unsigned char *pSnr)
{
    char status;
    unsigned char ucComMF522Buf[MAXRLEN];
    unsigned char i;
    unsigned int  unLen;
    SetBitMask(TxModeReg,0x80);
    SetBitMask(RxModeReg,0x80);
    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x70;
    ucComMF522Buf[6] = 0;
    for (i=0; i<4; i++)
    {
    	ucComMF522Buf[i+2] = *(pSnr+i);
		ucComMF522Buf[6]  ^= *(pSnr+i);
    }
    status = PcdComMF522_P(PCD_TRANSCEIVE,ucComMF522Buf,7,ucComMF522Buf,&unLen,0x0010);
    if ((status == MI_OK))
    {  
		status = MI_OK;
    }
    else
    {   
		status = MI_ERR;    
    }  
    return status; 
}

/////////////////////////////////////////////////////////////////////
//function:Verify card password
//Parameters: auth_mode[IN]:Password authentication mode
//                 0x60 = Verifying A key
//                 0x61 = Verifying B key 
//                 addr[IN]:Block address
//                 pKey[IN]:password
//                 pSnr[IN]:Card serial number, 4 bytes
//return:Successfully returns MI_OK
/////////////////////////////////////////////////////////////////////               
unsigned char PcdAuthState(unsigned char auth_mode,unsigned char addr,unsigned char *pKey,unsigned char *pSnr)
{
    unsigned char status;
    unsigned char ucComMF522Buf[MAXRLEN]; 
    switch(auth_mode)
    {
		case 0x01:
			ucComMF522Buf[0] = 0X60;
			break;
		case 0x02:
			ucComMF522Buf[0] = 0X61;
			break;
		default:
			return MI_ERR;
    }
    ucComMF522Buf[1] = addr;
    memcpy(&ucComMF522Buf[2], pKey, 6); 
    memcpy(&ucComMF522Buf[8], pSnr, 4);  
    status =Opation_MF1Card(PCD_AUTHENT,ucComMF522Buf,12,0X0020);
    if(status == MI_OK)
    {
		status=ReadRawRC(Status2Reg);
		if(status & 0x08)
		status = MI_OK;
    }
    return status;
}

/////////////////////////////////////////////////////////////////////
//function:Through RC522 and ISO14443 cartoon news
//Parameters:Command[IN]:RC522 command word
//           pInData[IN]:Data sent to the card via RC522
//         InLenByte[IN]:Length of sent data in bytes
//		     TimeOut[IN]:timeout
/////////////////////////////////////////////////////////////////////
unsigned char Opation_MF1Card(unsigned char Command, 
							 unsigned char *pInData, 
							 unsigned char InLenByte,
							 unsigned short TimeOut)
{
    unsigned char irqEn  ;
    unsigned char waitFor ;
    unsigned char lastBits;
    unsigned char n;
    unsigned int i;
    unsigned  status = MI_ERR;
    switch (Command)
    {
		case PCD_AUTHENT:
			irqEn   = 0x12;
			waitFor = 0x10;
			break;
		case PCD_TRANSCEIVE:
			irqEn   = 0x77;
			waitFor = 0x30;
			break;
		default:
			irqEn   = 0x12;
			waitFor = 0x10;
			break;
    }
    WriteRawRC(TPrescalerReg,0xFF);
    WriteRawRC(TModeReg,0x87);		
    WriteRawRC(TReloadRegL,(unsigned char)TimeOut); 
    WriteRawRC(TReloadRegH,(unsigned char)(TimeOut>>8));
    WriteRawRC(ComIEnReg,irqEn|0x80);
    ClearBitMask(ComIrqReg,0x80);
    WriteRawRC(CommandReg,PCD_IDLE);
    SetBitMask(FIFOLevelReg,0x80);
    for (i=0; i<InLenByte; i++)
    {   
		WriteRawRC(FIFODataReg, pInData[i]);    
    }
    WriteRawRC(CommandReg, Command);
    if (Command == PCD_TRANSCEIVE)
    {    
		SetBitMask(BitFramingReg,0x80);  
    }
    else
    {
		SetBitMask(ControlReg,0x40);//start time 
    }
    do 
    {
         n = ReadRawRC(ComIrqReg);
    }
    while ( !(n&0x01) && !(n&waitFor));
    ClearBitMask(BitFramingReg,0x80); 
    status = ReadRawRC(ErrorReg); 
    if (!(n&0x01))	
    {
		SetBitMask(ControlReg,0x80);//stop time   
		if(!(status&0x1B))
		{
			status = MI_OK;
			if (n & irqEn & 0x01)
			{
				status = MI_NOTAGERR;   
			}
			if (Command == PCD_TRANSCEIVE)
			{
				n = ReadRawRC(FIFOLevelReg);
				lastBits = ReadRawRC(ControlReg) & 0x07;
				if (lastBits)
				{   
					InLenByte = (n-1)*8 + lastBits;   
				}
				else
				{   
					InLenByte = n*8;   
				}
				for (i=0; i<n; i++)
				{   
					pInData[i] = ReadRawRC(FIFODataReg);    
				}
			}
		}
		else
		{   
			status = MI_ERR;   
		}
    }
    WriteRawRC(CommandReg,PCD_IDLE); 
    return status;
}

/////////////////////////////////////////////////////////////////////
//function:Read a piece of data from M1 card
//Parameters:addr[IN]:Block address
//         pData[OUT]:Read data, 16 bytes
//return:Successfully returns MI_OK
///////////////////////////////////////////////////////////////////// 
unsigned char PcdRead(unsigned char addr,unsigned char *pData)
{
    unsigned char status;
    unsigned char ucComMF522Buf[MAXRLEN]; 
    ucComMF522Buf[0] = PICC_READ;
    ucComMF522Buf[1] = addr;
    SetBitMask(TxModeReg,0x80);
    SetBitMask(RxModeReg,0x80);
    Delay1us(10);
    status = Opation_MF1Card(PCD_TRANSCEIVE,ucComMF522Buf,2,0X0020);
    if ((status == MI_OK)) 
    {
		memcpy(pData, ucComMF522Buf, 16);   
    }
    else
    {
		status = MI_ERR; 
    }
    return status;
}

/////////////////////////////////////////////////////////////////////
//function:Write data to M1 card block
//Parameters:addr[IN]:Block address
//          pData[IN]:Write data, 16 bytes
//return:Successfully returns MI_OK
/////////////////////////////////////////////////////////////////////     
unsigned char PcdWrite(unsigned char addr,unsigned char *pData)
{
    unsigned char status=0;
    unsigned char ucComMF522Buf[MAXRLEN];
    ucComMF522Buf[0] = PICC_WRITE;
    ucComMF522Buf[1] = addr;
    SetBitMask(TxModeReg,0x80);
    SetBitMask(RxModeReg,0x80);
    Delay1us(10);   
    status = Opation_MF1Card(PCD_TRANSCEIVE,ucComMF522Buf,2,0X0010);
    if((status == MI_OK) && ((ucComMF522Buf[0] &0x0F) == 0X0A))
    {
    	status = Opation_MF1Card(PCD_TRANSCEIVE,pData,16,0X0020);
    }
    else
    {
		status = MI_ERR;
    }
    return status;
}

void Delay1us(unsigned char us)
{
	while (us)
	{
		delay_us(dt);delay_us(dt); delay_us(dt); delay_us(dt); delay_us(dt);
		--us;
	}
}

