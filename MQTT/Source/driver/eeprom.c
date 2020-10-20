/***************************************************
ģ�����ƣ�  FLASH
����������	
�� �� ��:	  Ѧ����(steven sit)  
��дʱ�䣺	2012-05-30
***************************************************/
#include <stdio.h>
#include "eeprom.h" 
#include <string.h>
#include "STM32Fxx_flash.h"



#ifdef EEPROM_ENABLE


void eeprom_init(void)
{

}

int eeprom_write(unsigned int addr, unsigned char* buffer, int len)
{

	
	Flash_WriteMaxByte(addr,buffer,len);
	
	return 0;
}


int eeprom_read(unsigned int addr, unsigned char* buffer, int len)
{

	Flash_ReadMaxByte(addr,buffer,len);
	
	return 0;
}



int eeprom_erase(void)
{
	
	return 0;
}


void eeprom_handler(void)
{
	
}


#endif


