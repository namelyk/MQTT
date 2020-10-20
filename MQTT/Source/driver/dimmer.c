#include "driver.h"
#include "dimmer.h"


#ifdef DIMMER_ENABLE

#define DIMMER_MAX_SIZE					(4)



//=============================================================
struct dimmer_t
{
	unsigned char enable:1;
	unsigned char update:1;
	unsigned char dimmer_dir:1;
	
	unsigned char level;
	
};

static int dimmer_index = 0;
static struct dimmer_t *dimmer;
static struct dimmer_t dimmer_struct[DIMMER_MAX_SIZE];

//=====================================================================

#define DIMMER_SET(A,B,C)	



//=======================User define API===============================

void dimmer_low_set(int index,unsigned char level)
{
	int value;
	
	value = (int)level * 512;
	value = value/100;
	if (dimmer_struct[index].dimmer_dir)
		value |= 0x8000;
	
	switch (index)
	{
		case 0:
			DIMMER_SET(58,(value >> 8),0);
			DIMMER_SET(59,(value >> 0),0);
			break;
		case 1:
			DIMMER_SET(60,(value >> 8),0);
			DIMMER_SET(61,(value >> 0),0);
			break;
		case 2:
			DIMMER_SET(62,(value >> 8),0);
			DIMMER_SET(63,(value >> 0),0);
			break;
		case 3:
			DIMMER_SET(64,(value >> 8),0);
			DIMMER_SET(65,(value >> 0),0);
			break;
	}
}


//=======================Standard define API===============================

static int _get(unsigned char *config,int len)
{
	
	return 0;
}


static int _set(unsigned char *config,int len)
{
	
	return 0;
}




//=============================================================
static void _init(void)
{
	int i;
	
	dimmer_index = 0;
	for(i = 0; i < DIMMER_MAX_SIZE; i++)
	{
		dimmer = &dimmer_struct[i];
		dimmer->update = 0;
	}
}



static int _status(unsigned char *buffer,int len)
{
	int i;
	
	for (i = 0; i < DIMMER_MAX_SIZE; i++)
	{
		buffer[i] = dimmer_struct[i].level;
	}
	
	return DIMMER_MAX_SIZE;
}

static int _read(unsigned char *buffer,int len)
{
	int index;
	struct dimmer_t *dimmer;
	
	index = buffer[0];
	if (index >= DIMMER_MAX_SIZE) return 1;
	
	dimmer = &dimmer_struct[index];

	
	buffer[1] = dimmer->level;
	
	return 0;
}


static int _write(unsigned char *buffer,int len)
{
	int index;
	int i;
	struct dimmer_t *dimmer;
	
	index = buffer[0];
	if (buffer[1] > 100) return 1;
	
	if (index == 255)
	{
		for(i = 0; i < DIMMER_MAX_SIZE; i++)
		{
			dimmer = &dimmer_struct[i];
			dimmer->level = buffer[1];
			dimmer->update = 1;
		}
		
	}
	else
	{
		if (index >= DIMMER_MAX_SIZE) 
		{	
			return 1;
		}
		
		dimmer = &dimmer_struct[index];
		dimmer->level = buffer[1];
		dimmer->update = 1;
	}
	return 0;
}


static int _handler(void *event)
{
	struct dimmer_t *dimmer;
	int i;
	
	for (i = 0; i < DIMMER_MAX_SIZE; i++)
	{
		dimmer = &dimmer_struct[i];
		//if (dimmer->enable == 0) return 1;
		
		// handler dimmer level
		if(dimmer->update)
		{
			dimmer->update = 0;
			DIMMER_SET(dimmer_index,dimmer->level,1);
		}
	}

	return 0;
}

//====================low interface=====================
const struct driver_interface_t dimmer_interface =
{
	_get,_set,_init,_status,_read,_write,_handler
};

#endif


