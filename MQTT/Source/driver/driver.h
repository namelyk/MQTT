#ifndef _DRIVER_H_
#define _DRIVER_H_



#define DRIVER_ENABLE	
#ifdef DRIVER_ENABLE	


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hw_init.h"

#include "eeprom_address_config.h"
#include "button.h"
#include "eeprom.h"
#include "air_conditioner.h"
#include "curtain.h"
#include "dimmer.h"
#include "led.h"
#include "relay.h"
#include "rs485.h"
#include "scene.h"
#include "temperature.h"
#include "objects.h"
#include "datetime.h"

#include "pannel.h"
#include "STM32Fxx_flash.h"



//=============Standard driver name======================
#define DRIVER_NAME(a)	(a##_interface)



//=============Standard driver API=======================


struct driver_interface_t
{
	int (*get)(unsigned char*, int);
	int (*set)(unsigned char*, int);
	void(*init)(void);
	int (*status)(unsigned char*, int);
	int (*read)(unsigned char*, int);
	int (*write)(unsigned char*, int);
	int (*handler)(void*);
};


//=============API=======================
extern void driver_init(void);
extern void driver_handler(void);

#endif
#endif
