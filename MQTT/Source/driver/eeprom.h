
#ifndef _EEPROM_H_
#define _EEPROM_H_


#define EEPROM_ENABLE
#ifdef EEPROM_ENABLE

//===================================================
extern void eeprom_init(void);
extern int eeprom_write(unsigned int addr, unsigned char* buffer, int len);
extern int eeprom_read(unsigned int addr, unsigned char* buffer, int len);
extern void eeprom_handler(void);


#endif
#endif

