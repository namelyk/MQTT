#include "led.h"
#include "driver.h"



#ifdef LED_ENABLE

#define LED_MAX_SIZE															(11)



//==========================================================

struct led_t
{
	unsigned char enable:1;
	unsigned char status:7;
	
	unsigned char level;
	
	int delay;
};


static unsigned char led_index;
static struct led_t led_struct[LED_MAX_SIZE];


#define LED_SET(a,b,c)



//=======================User define API===============================



//=======================Standard define API===============================
static int _get(unsigned char *config,int len)
{

	return 0;
}


static int _set(unsigned char *config,int len)
{

	return 0;
}

//==========================================================
static void _init(void)
{
	int i;
	struct led_t *led;
	
	led_index = 0;
	for (i = 0; i < LED_MAX_SIZE; i++)
	{
		led = &led_struct[i];
		led->delay = 0;
		led->status = 0;
		led->enable = 1;
		led->level = 0;
	}
}

static int _status(unsigned char *buffer,int len)
{
	
	return 0;
}

static int _read(unsigned char *buffer,int len)
{
	int index;
	
	index = buffer[0];
	if (index >= LED_MAX_SIZE) return 1;
	if (led_struct[index].enable == 0) return 1;
	
	*buffer = led_struct[index].level;
	
	return 0;
}


static int _write(unsigned char *buffer,int len)
{
	int index;
	
	index = buffer[0];
	if (index >= LED_MAX_SIZE) return 1;
	if (led_struct[index].enable == 0) return 1;
	
	if (buffer[0] > 100) return 1;
	
	led_struct[index].level = buffer[0];
	led_struct[index].status = 1;
	led_struct[index].delay = buffer[1];
	led_struct[index].delay <<= 8;
	led_struct[index].delay |= buffer[2];
	if (led_struct[index].delay < 3600)
		led_struct[index].delay *= 100;
	else
		led_struct[index].delay = 0;
	
	return 0;
}


static int _handler(void *event)
{
	struct led_t *led;
	
	led_index++;
	if (led_index >= LED_MAX_SIZE)
	{
		led_index = 0;	
	}
	
	led = &led_struct[led_index];
	if (led->enable == 0) return 1;
	
	switch(led->status)
	{
		case 1:
			if (led->delay)
			{
				led->delay--;
			}
			else
			{
				led->status = 2;
			}
			break;
			
		case 2:
			LED_SET(led_index,&(led->level),1);
			led->status = 0;
			break;
			
		default:
			break;
	}
	
	return 0;
}



//====================low interface=====================
const struct driver_interface_t led_interface =
{
	_get,_set,_init,_status,_read,_write,_handler
};


#endif

