#include "scene.h"
#include "driver.h"


#ifdef SCENE_ENABLE

#define SCENE_MAX_SIZE							(200)
#define OBJECT_MAX_SIZE							(16)
#define SCENE_OBJECT_MAX_SIZE				(10 * OBJECT_MAX_SIZE)


//===============================================================
struct scene_t 
{
	unsigned char enable:1;
	unsigned char status:1;
	unsigned char update:1;
	unsigned char reserved:5;
};


struct scene_event_t
{
	unsigned char status;
	unsigned char state;
	unsigned char count;
	unsigned char event[SCENE_OBJECT_MAX_SIZE];
};



static struct scene_t scene_struct[SCENE_MAX_SIZE];
static struct scene_event_t scene_event;




//=======================User define API===============================

static int scene_send(unsigned char *buffer,unsigned char len)
{
	
	return 0;
}




int scene_get_event(int index, unsigned char status, unsigned char *event)
{
	unsigned int addr;
	
	addr = SCENE_EVENT_BASIC_ADDRESS;
	if (status)
	{
		addr += (index * 2 + 1) * SCENE_OBJECT_MAX_SIZE;
	}
	else
	{
		addr += (index * 2) * SCENE_OBJECT_MAX_SIZE;
	}
	
	eeprom_read(addr,event,SCENE_OBJECT_MAX_SIZE);
	
	return 0;
}


int scene_set_event(int index, unsigned char status, unsigned char *event)
{
	unsigned int addr;

	
	addr = SCENE_EVENT_BASIC_ADDRESS;
	if (status)
	{
		addr += (index * 2 + 1) * SCENE_OBJECT_MAX_SIZE;
	}
	else
	{
		addr += (index * 2) * SCENE_OBJECT_MAX_SIZE;
	}
	
	eeprom_write(addr,event,SCENE_OBJECT_MAX_SIZE);
	return 0;
}


//=======================Standard define API===============================

static int _get(unsigned char *config,int len)
{
	unsigned int addr;
	int index;
	
	addr = SCENE_CONFIG_BASIC_ADDRESS;

	return 0;
}


static int _set(unsigned char *config,int len)
{
	unsigned int addr;
	int index;
	
	addr = SCENE_CONFIG_BASIC_ADDRESS;

	return 0;
}



//===============================================================
static void _init(void)
{
	int i;
	struct scene_t *scene;
	
	scene_event.state = 0;
	scene_event.count = 0;
	scene_event.status = 0;
	
	for (i = 0; i < SCENE_MAX_SIZE; i++)
	{
		scene = &scene_struct[i];
		scene->enable = 0;
		scene->status = 0;
		scene->update = 0;
	}
}


static int _status(unsigned char *buffer,int len)
{
	int index;
	
	index = buffer[0];
	if (index == 0 || index >= SCENE_MAX_SIZE) return 1;
	index--;
	
	buffer[1] = scene_struct[index].status;
	
	return 0;
}

static int _read(unsigned char *buffer,int len)
{
	int index;
	
	index = buffer[0];
	if (index == 0 || index >= SCENE_MAX_SIZE) return 1;
	index--;
	
	buffer[0] = scene_struct[index].status;
	
	return 0;
}


static int _write(unsigned char *buffer,int len)
{
	int index;
	
	index = buffer[0];
	if (index == 0 || index >= SCENE_MAX_SIZE) return 1;
	index--;
	
	if (buffer[1])
	{
		scene_struct[index].status = 1;
		scene_struct[index].update = 1;
	}
	else
	{
		scene_struct[index].status = 0;
		scene_struct[index].update = 1;
	}
	return 0;
}



static int _handler(void *event)
{
	static int scene_index = 0;
	int i;
  struct scene_event_t *scene;
	scene = &scene_event;
	
	switch (scene->state)
	{
		// search
		case 0:
			for (i = 0; i < 32; i++)
			{
				scene_index++;
				if (scene_index >= SCENE_MAX_SIZE)
				{
					scene_index = 0;
				}
				
				if (scene_struct[scene_index].update)
				{
					//get scene event
					scene_get_event(scene_index,scene_struct[scene_index].status,scene->event);
					scene->status = scene_struct[scene_index].status;
					scene_struct[scene_index].update = 0;
					
					scene->count = 0;
					scene->state = 1;
					break;
				}
			}
			break;
			
		//get event	
		case 1:
			if (!scene_send(&scene->event[(int)(scene->count) * 10],10))
			{
				scene->count++;
			}
			
			if (scene->count >= OBJECT_MAX_SIZE)
			{
				scene->state = 0;
				if (scene->status == scene_struct[scene_index].status)//∑¿÷π÷ÿ∏¥¥•∑¢
				scene_struct[scene_index].update = 0;
			}
			break;
		
		default:
			break;
	}
	return 0;
}

//====================low interface=====================
const struct driver_interface_t scene_interface =
{
	_get,_set,_init,_status,_read,_write,_handler
};
#endif
