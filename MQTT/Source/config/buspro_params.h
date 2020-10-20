#ifndef _BUSPRO_PARAMS_H_
#define _BUSPRO_PARAMS_H_

#define BUSPRO_PARAMS_ENABLE
#ifdef BUSPRO_PARAMS_ENABLE

extern int buspro_get_baudrate(unsigned char *baudrate);
extern int buspro_set_baudrate(unsigned char *baudrate);

#endif
#endif
