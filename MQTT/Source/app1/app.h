#ifndef _APP_H_
#define _APP_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"


//#include "iap.h"
//#include "commands.h"
#include "gateway.h"
//#include "remote.h"
//#include "route.h"
//#include "wechat.h"
//#include "filter.h"

#include "device.h"
//#include "hotel_netip.h"



//=============Standard APP name======================
#define DRIVER_NAME(a)	(a##_interface)


//=============Standard APP API=======================
struct app_interface_t
{
	int (*get)(unsigned char*, int);
	int (*set)(unsigned char*, int);
	void(*init)(void);
	int (*status)(unsigned char*, int);
	int (*read)(unsigned char*, int);
	int (*write)(unsigned char*, int);
	int (*handler)(void*);
};


extern int system_ticks;


//==========================
extern void app_init(void);
extern void app_handler(void);


extern void app_commands_handler(int s,unsigned char *buffer,unsigned char size);
extern void netip_commands_handler(int s,unsigned char *buffer, int size);

#endif
