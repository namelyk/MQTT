#include <stdio.h>
#include "STM32Fxx_flash.h"



#ifdef STM32FXX_ENABLE

#include "hw_init.h"


//flash 页大小
#define K									(1024)
#define PAGE_SIZE					(2*K)


//后面64K用来保存数据
#define STM32_FLASH_DATA_ADD		0x8038000
#define STM32_FLASH_END_ADD			0x803F800




//stm32 内部BUFF
uint8_t FlashBuffer[PAGE_SIZE];


/***************************************************
				Flash API.s	 
***************************************************/

 union Flash_char32
{
      unsigned int  l_num;
      unsigned char   c_num[4]; 
} Flash_Data; 

/***************************************************
函数名称：	 StrCopy
输入参数：	
返 回 值:	
功能描述：	 
***************************************************/
static void StrCopy(unsigned char *dest,unsigned char *src,int len)
{
 	while(len--)
	{
	 	*dest++ = *src++;
	}
}
/***************************************************
函数名称：	 Flash_ReadWord
输入参数：	
返 回 值:	
功能描述：	 
***************************************************/
unsigned int Flash_ReadWord(unsigned int ulAddr)
{
	unsigned int read_word;

	read_word = *(__IO unsigned int*)(ulAddr);
	return read_word;
}
/***************************************************
函数名称：	 Flash_WriteWord
输入参数：	
返 回 值:	
功能描述：	 
***************************************************/
unsigned char Flash_WriteWord(unsigned int ulAddr,unsigned int wDdata)
{
	unsigned int Count;

	for(Count = 0; Count < 3; Count++)
	{
		FLASH_ProgramWord(ulAddr, wDdata);
		if (*(unsigned int*)ulAddr == wDdata)
		{
			return 0;
		}
	}
	return 1;
}
/***************************************************
函数名称：	 Flash_ReadPage
输入参数：	
返 回 值:	
功能描述：	 
***************************************************/
/*	
void Flash_ReadPage(unsigned int ulPageAddr)
{
 	volatile unsigned int ulFlashAddr;
	unsigned int RamSource;
	int usCount;

	ulFlashAddr = (ulPageAddr * PAGE_SIZE);
	RamSource = (unsigned int)FlashBuffer;

	for(usCount = 0; usCount < PAGE_SIZE;)
	{
		*(uint32_t*)RamSource = Flash_ReadWord(ulFlashAddr);
		RamSource += 4;
		usCount += 4;
		ulFlashAddr += 4;	
	}
}
*/

void Flash_ReadPage(unsigned int ulPageAddr)
{
 	volatile unsigned int ulFlashAddr;
	//unsigned int RamSource;
	int usCount;

	ulFlashAddr = (ulPageAddr * PAGE_SIZE);
	//RamSource = (uint32_t)FlashBuffer;

	for(usCount = 0; usCount < PAGE_SIZE;)
	{
		Flash_Data.l_num = Flash_ReadWord(ulFlashAddr);
		FlashBuffer[usCount]=Flash_Data.c_num[0];usCount++;
		FlashBuffer[usCount]=Flash_Data.c_num[1];usCount++;
		FlashBuffer[usCount]=Flash_Data.c_num[2];usCount++;
		FlashBuffer[usCount]=Flash_Data.c_num[3];usCount++;
		//RamSource += 4;
		//usCount += 4;
		ulFlashAddr += 4;	
	}
}
/***************************************************
函数名称：	 Flash_WritePage
输入参数：	
返 回 值:	
功能描述：	 
***************************************************/
unsigned char Flash_WritePage(unsigned int ulPageAddr)
{
	volatile unsigned int ulFlashAddr;
	unsigned int RamSource;
	unsigned int Count;

	ulFlashAddr = (ulPageAddr * PAGE_SIZE);
	RamSource = (uint32_t)FlashBuffer;
	
	
	__set_PRIMASK(1);
	FLASH_Unlock();
	
	FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR|FLASH_FLAG_OPTERR);
	
	while (FLASH_ErasePage(ulFlashAddr) != FLASH_COMPLETE)
	{
		RESET_WATCH_DOG();
	}

	for(Count = 0; Count < (PAGE_SIZE/4);)
	{
		if (Flash_WriteWord(ulFlashAddr,(*(uint32_t*)RamSource)))
		{
			__set_PRIMASK(0);
			return 1; 
		}
	    ulFlashAddr += 4;
	    RamSource += 4;
		Count++;
	}

	__set_PRIMASK(0);
	return 0;
}
/***************************************************
函数名称：	 Flash_ReadMaxByte
输入参数：	
返 回 值:	
功能描述：	 
***************************************************/
int Flash_ReadMaxByte(unsigned int ulAddr,unsigned char *pBuffer,  int usLen)
{

	int i;
//	if (usLen > PAGE_SIZE) return;
	if ((ulAddr + STM32_FLASH_DATA_ADD + usLen) >= STM32_FLASH_END_ADD)return 1;

	FLASH_Unlock();
	  for (i=0;i<usLen;i++)
	  {
	     Flash_Data.l_num=Flash_ReadWord(ulAddr + STM32_FLASH_DATA_ADD); 
	     pBuffer[i]=Flash_Data.c_num[0];
	     ulAddr++;
	  }  
	FLASH_Lock();
		return 0;
}


/***************************************************
函数名称：	 Flash_WriteMaxByte
输入参数：	
返 回 值:	
功能描述：	 
***************************************************/
int Flash_WriteMaxByte(unsigned int ulAddr, unsigned char * pBuffer, int usLen)
{
	volatile unsigned int ulFlashAddr;	
	unsigned int ulPageSize;
	int usSizeTemp;

	if (usLen > PAGE_SIZE) return 1;
	ulFlashAddr = STM32_FLASH_DATA_ADD + ulAddr;
	if ((ulFlashAddr + PAGE_SIZE) >= STM32_FLASH_END_ADD)return 1;

	FLASH_Unlock();

	ulPageSize = ulFlashAddr / PAGE_SIZE;
	usSizeTemp = (uint16_t)(ulFlashAddr % PAGE_SIZE);

	if (usSizeTemp != 0)
	{		
		if ((usSizeTemp + usLen) > PAGE_SIZE)
		{
		 	Flash_ReadPage(ulPageSize);	
			StrCopy(&FlashBuffer[usSizeTemp],pBuffer,(PAGE_SIZE - usSizeTemp));
			Flash_WritePage(ulPageSize);						

			Flash_ReadPage(ulPageSize+1);	
			StrCopy(FlashBuffer,&pBuffer[PAGE_SIZE-usSizeTemp],(usSizeTemp + usLen - PAGE_SIZE));
			Flash_WritePage(ulPageSize+1);
		}
		else
		{
		 	Flash_ReadPage(ulPageSize);	
			StrCopy(&FlashBuffer[usSizeTemp],pBuffer,usLen);
			Flash_WritePage(ulPageSize);
		}
	}
	else
	{
		Flash_ReadPage(ulPageSize);	
		StrCopy(FlashBuffer,pBuffer,usLen);
		Flash_WritePage(ulPageSize);
	}
	FLASH_Lock();
	
	return 0;
}

/***************************************************
函数名称：	 Flash_ReadMaxByte
输入参数：	
返 回 值:	
功能描述：	 
***************************************************/
#define STM32_FLASH_URL_ADD 0x8010000
int Flash_ReadURL(unsigned char * pBuffer, int usLen)
{
	
	unsigned int i;

	  for (i=0;i<64;)
	  {
		Flash_Data.l_num=Flash_ReadWord(i + STM32_FLASH_URL_ADD); 
		pBuffer[i]=Flash_Data.c_num[0];
		FlashBuffer[i]=Flash_Data.c_num[0]; 
		FlashBuffer[i+1]=Flash_Data.c_num[1];
		FlashBuffer[i+2]=Flash_Data.c_num[2];
		FlashBuffer[i+3]=Flash_Data.c_num[3];
		i += 4;
	  }  
	
		return 0;
}


/***************************************************
函数名称：	 Flash_WriteMaxByte
输入参数：	
返 回 值:	
功能描述：	 
***************************************************/
int Flash_WriteURL(unsigned char * pBuffer, int usLen)
{
	volatile unsigned int ulFlashAddr;	
	unsigned int ulPageSize;
	int usSizeTemp;

	if (usLen > PAGE_SIZE) return 1;
	ulFlashAddr = STM32_FLASH_URL_ADD;
	if ((ulFlashAddr + PAGE_SIZE) >= STM32_FLASH_END_ADD)return 1;

	FLASH_Unlock();

	ulPageSize = ulFlashAddr / PAGE_SIZE;
	usSizeTemp = (uint16_t)(ulFlashAddr % PAGE_SIZE);

	if (usSizeTemp != 0)
	{		
		if ((usSizeTemp + usLen) > PAGE_SIZE)
		{
		 	Flash_ReadPage(ulPageSize);	
			StrCopy(&FlashBuffer[usSizeTemp],pBuffer,(PAGE_SIZE - usSizeTemp));
			Flash_WritePage(ulPageSize);						

			Flash_ReadPage(ulPageSize+1);	
			StrCopy(FlashBuffer,&pBuffer[PAGE_SIZE-usSizeTemp],(usSizeTemp + usLen - PAGE_SIZE));
			Flash_WritePage(ulPageSize+1);
		}
		else
		{
		 	Flash_ReadPage(ulPageSize);	
			StrCopy(&FlashBuffer[usSizeTemp],pBuffer,usLen);
			Flash_WritePage(ulPageSize);
		}
	}
	else
	{
		Flash_ReadPage(ulPageSize);	
		StrCopy(FlashBuffer,pBuffer,usLen);
		Flash_WritePage(ulPageSize);
	}
	FLASH_Lock();
	
	return 0;
}







int Flash_ErasePage(unsigned int ulAddr)
{
	volatile unsigned int ulFlashAddr;	

	ulFlashAddr = ulAddr;
	if ((ulFlashAddr + PAGE_SIZE) > STM32_FLASH_END_ADD)return 1;
	ulFlashAddr = ulFlashAddr/PAGE_SIZE*PAGE_SIZE;

	FLASH_Unlock();

	__set_PRIMASK(1);	
	
	FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR|FLASH_FLAG_OPTERR);
	
	while (FLASH_ErasePage(ulFlashAddr) != FLASH_COMPLETE)
	{
		RESET_WATCH_DOG();
	}
	__set_PRIMASK(0);
	
	FLASH_Lock();
	
	return 0;
}


void Flash_EraseChip(void)
{
	volatile unsigned int ulFlashAddr;	

	ulFlashAddr = STM32_FLASH_DATA_ADD;  //mac和子网id 设备id不变

	FLASH_Unlock();
	__set_PRIMASK(1);
	
	ulFlashAddr = ulFlashAddr/PAGE_SIZE*PAGE_SIZE;
	while (ulFlashAddr < STM32_FLASH_END_ADD)
	{
		while (FLASH_ErasePage(ulFlashAddr) != FLASH_COMPLETE);
		ulFlashAddr += PAGE_SIZE;
	}
	__set_PRIMASK(0);
	FLASH_Lock();
}

#endif

