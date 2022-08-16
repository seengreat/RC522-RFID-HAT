#ifndef __GPIO_H
#define __GPIO_H
#include "sys.h"

#define	NSS PBout(12)  
#define	LED PBout(4) 

#define macRC522_Reset_Enable() GPIO_ResetBits( GPIOA, GPIO_Pin_8 )
#define macRC522_Reset_Disable() GPIO_SetBits( GPIOA, GPIO_Pin_8)

void IO_Init(void);

#endif

