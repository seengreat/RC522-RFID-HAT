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
#include <string.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <softPwm.h>
#include "rc522.h"

unsigned char fHasRATS = 0;
unsigned char CT[2];//Card type
unsigned char SN[4]; //card number
unsigned char sec[12] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; //IC card initial password
unsigned char blockdata1[16]= {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

/////////////////////////////////////////////////////////////////////
//function:Serial port read and write data processing
//Parameters that:TxData[IN]:Data to be sent
//return:Read data
/////////////////////////////////////////////////////////////////////
unsigned char Uart_ReadWriteByte(unsigned char TxData)
{
    unsigned char rev=1;
    int waittime=5;
    serialPutchar(serial_Fd, TxData);
    while(waittime--)
    {
		if(serialDataAvail(serial_Fd)==1)
		{
			rev = serialGetchar(serial_Fd);
			serialFlush(serial_Fd);
			break;
		}
		else delay(3);
    }
	if( !waittime )
	{
		rev=0;	
		printf("Wait for a timeout\r\n");								 
	}		
    return rev;
}

/////////////////////////////////////////////////////////////////////
//function:Read register RC522
//Parameters that:Address[IN]:Register address
//return:Read data
/////////////////////////////////////////////////////////////////////
unsigned char ReadRawRC(unsigned char Address)
{
    return Uart_ReadWriteByte((Address&0x3F) | 0x80);
}

/////////////////////////////////////////////////////////////////////
//function:Write register RC522
//Parameters that:Address[IN]:Register address
//                  value[IN]:Written data
/////////////////////////////////////////////////////////////////////
void WriteRawRC(unsigned char Address, unsigned char value)
{
    unsigned char ch = Uart_ReadWriteByte(Address & 0x3F);
    if(ch != Address)
    {
		printf("Not equal");
    }
    serialPutchar(serial_Fd, value);
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
    delay(10);
}

/////////////////////////////////////////////////////////////////////
//function:Close the antenna
/////////////////////////////////////////////////////////////////////
void PcdAntennaOff()
{
    ClearBitMask(TxControlReg, 0x03);
    delay(10);
}

/////////////////////////////////////////////////////////////////////
//function:Initialize for ISO14443A type card
//Parameters:ucType[IN]:Card type
/////////////////////////////////////////////////////////////////////
void M500PcdConfigISOType(unsigned char ucType)
{
    if ( ucType == 'A')   //ISO14443A
    {
        ClearBitMask ( Status2Reg, 0x08 );  //Contains status flags for receivers and transmitters
        WriteRawRC ( ModeReg, 0x3D );	    //Define common modes for sending and receiving
        WriteRawRC ( RxSelReg, 0x86 );      //Define received data
        WriteRawRC( RFCfgReg, 0x6F );       //Configuring receive Gain
        WriteRawRC( TReloadRegL, 30 );      //Describes the reinstallation value of a 16-bit timer
        WriteRawRC ( TReloadRegH, 0 );
        WriteRawRC ( TModeReg, 0x8D );	    //Internal timer setting
        WriteRawRC ( TPrescalerReg, 0x3E );	//Internal timer setting
        delay(1);
        PcdAntennaOn ();                    //Open the antenna
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
    delayMicrosecondsHard(100);
    WriteRawRC(TPrescalerReg,0xFF);
    WriteRawRC(TModeReg,0x87);		
    WriteRawRC(TReloadRegL,(unsigned char)TimeOut); 
    WriteRawRC(TReloadRegH,(unsigned char)(TimeOut>>8));
    delayMicrosecondsHard(10);
    WriteRawRC(ComIrqReg,0x7F);
    WriteRawRC(DivIrqReg,0x7F);
    WriteRawRC(FIFOLevelReg,0x80);
    SetBitMask(CommandReg,Command);
    delayMicrosecondsHard(1);
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
    delay(10);
    macRC522_Reset_Enable();	
    delay(10);	
    macRC522_Reset_Disable();	
    delay(10);
    WriteRawRC(CommandReg,PCD_RESETPHASE);  //Software reset
    delay(10);
    WriteRawRC(ControlReg,0x10);
    WriteRawRC(ModeReg,0x3F);               //Define sending and receiving common modes and Mifare card messages, CRC initial value 0x6363
    WriteRawRC(RFU23,0x00);
    WriteRawRC(RFU25,0x80);
    WriteRawRC(AutoTestReg,0x40);
    WriteRawRC(TxAutoReg,0x40);             //The modulation sends a signal of 100%ASK
    ReadRawRC(TxAutoReg); 
    SetBitMask(TxControlReg,0x03);
    WriteRawRC(TPrescalerReg,0x3D);         //Set the timer frequency division coefficient
    WriteRawRC(TModeReg,0x0D);              //Define internal timer Settings
    WriteRawRC(TReloadRegL,0x0A);           //Low value of 16-bit timer
    WriteRawRC(TReloadRegH,0x00);           //High value of 16-bit timer
    WriteRawRC(ComIrqReg,0x01);
    SetBitMask(ControlReg,0x40);
    do
    {
		Temp = ReadRawRC(ComIrqReg);
    }
    while(!(Temp&0x01));	
    WriteRawRC(ComIrqReg,0x01);
    WriteRawRC(CommandReg,0x00);	
    while( ReadRawRC(0x27) != 0x88)         //wait chip start ok
    {
		delay(10);
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
			delayMicrosecondsHard(5);
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
    delayMicrosecondsHard(200);
    WriteRawRC(BitFramingReg,0x00);
    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x20;
    status = PcdComMF522_P(PCD_TRANSCEIVE,ucComMF522Buf,2,ucComMF522Buf,&unLen,0x0020);
    delayMicrosecondsHard(100);
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
    SetBitMask(TxModeReg,0x80);//Enable tx crc generation during transmit
    SetBitMask(RxModeReg,0x80);//Enable rx crc generation during recieve
    delayMicrosecondsHard(10);
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
    delayMicrosecondsHard(10);   
    status = Opation_MF1Card(PCD_TRANSCEIVE,ucComMF522Buf,2,0X0010);//Step A:To query the block status, the card should respond with 4 bits,1010
    if((status == MI_OK) && ((ucComMF522Buf[0] &0x0F) == 0X0A))
    {
    	status = Opation_MF1Card(PCD_TRANSCEIVE,pData,16,0X0020);//Step B: Write data
    }
    else
    {
		status = MI_ERR;
    }
    return status;
}

/////////////////////////////////////////////////////////////////////
//function:Read the UID of the S50 card and print the card number
//         Read block 8 data and can change the first 4 bytes of data by keyboard input
/////////////////////////////////////////////////////////////////////
void ReadIDData()
{
    unsigned char i,a,b,ret;
    unsigned char ucStatusReturn = MI_ERR;           
    unsigned char bFind = 0;
    if((ucStatusReturn = PcdRequest(PICC_REQIDL,CT)) != MI_OK) //寻卡
	ucStatusReturn = PcdRequest(PICC_REQIDL,CT);//再寻卡
    if(ucStatusReturn != MI_OK)  //防止连续打卡                    
	bFind = 0;	
    if((ucStatusReturn == MI_OK) && (bFind == 0))
    {
		if(PcdAnticoll(SN) == MI_OK) //防止冲撞
		{
			delay(250);//等待卡片放置稳定
			if(PcdSelect(SN) == MI_OK)//选定卡片
			{
				printf("Card ID:");
				for(i=0;i<4;i++)
				{
					printf("%d",SN[i]);
				}
				printf("\r\n");
				softPwmWrite(PWM,9);
				delay(100);
				softPwmWrite(PWM,0);
				LED_Enable();
				delay(100);
				LED_Disable();
				if(PcdAuthState(C_A,8,sec,SN) == MI_OK)//验证密码
				{
					printf("Read block data\n");
					PcdRead(8,blockdata1);//读取块8数据
					printf("block_8=[ ");
					for(a=0;a<16;a++)
					{
						printf("%02X ",blockdata1[a]);
					}
					printf("]\n");
					printf("Please enter the first 4 bytes of block data on the keyboard\n");
					for(b=0;b<4;b++)
					{
						printf("input %d:",b);
						ret=scanf("%X", (unsigned int*)&blockdata1[b]);
						while(ret!=1)
						{
							printf("Input error, please re-enter\n");
							while(getchar()!='\n');
							ret=scanf("%X", (unsigned int*)&blockdata1[b]);
						}
					}					
					PcdWrite(8,blockdata1);
					printf("Read block data again\n");
					PcdRead(8,blockdata1);//读取块8数据
					printf("block_8=[ ");
					for(a=0;a<16;a++)
					{
						printf("%02X ",blockdata1[a]);
					}
					printf("]\n");		
					printf("----------------------------------\n");	
					delay(1000);			
				}
			}
		}
    }
}

