#ifndef _STM32FXX_FLASH_H_
#define _STM32FXX_FLASH_H_


#define STM32FXX_ENABLE
#ifdef STM32FXX_ENABLE


extern int Flash_ReadMaxByte(unsigned int ulAddr,unsigned char *pBuffer, int usLen);
extern int Flash_WriteMaxByte(unsigned int ulAddr,unsigned char *pBuffer,  int usLen);
extern int Flash_ErasePage(unsigned int ulAddr);
extern void Flash_EraseChip(void);
extern int Flash_WriteURL(unsigned char * pBuffer, int usLen);
extern int Flash_ReadURL(unsigned char * pBuffer, int usLen);

#endif
#endif
