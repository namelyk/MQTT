#include "driver.h"
#include "relay.h"


#ifdef RELAY_ENABLE

#define RELAY_MAX_SIZE					(33)

struct relay_t
{
	int timeout;
	int count;
	unsigned char update;
	unsigned char level;
};


static struct relay_t relay_struct[RELAY_MAX_SIZE];



extern void dimmer_low_set(int index,unsigned char level);
//=====================================================================



#define RELAY_SET(A,B,C)		



//=======================User define API===============================
struct relay_datapoint_t
{
	const int reg_addr;
};

const struct relay_datapoint_t _relay_datapoint_table[]=
{
	// big relay
	57,56,55,54,
	// relay
	53,52,51,50,49,48,47,46,45,44,43,42,41,40,
	// D led
	39,38,37,36,35,
	// led
	34,33,32,31,30,29
};


void relay_low_set(int index, unsigned char level)
{
	int value;
	int reg_addr;
	
	if (level == 100)
		value = 1;
	else if (level == 0)
		value = 0;
	else
		return;
	
	reg_addr = _relay_datapoint_table[index].reg_addr;
	
	RELAY_SET(reg_addr,value,0);
	
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

}



static int _status(unsigned char *buffer,int len)
{
	int index,i;
	struct relay_t *relay;
	
	
	buffer[2] = buffer[1];
	buffer[1] = 0xf8;
	buffer[3] = RELAY_MAX_SIZE;
	
	for (i = 0; i < RELAY_MAX_SIZE; i++)
	{
		relay = &relay_struct[i];
		
		if (relay->level)
		{
			buffer[3 + i/8] |= (i%8); 
		}
		else
		{
			buffer[3 + i/8] &= ~(i%8); 
		}
	}
	
	return 0;
}

static int _read(unsigned char *buffer,int len)
{
	int i;
	struct relay_t *relay;
	
	buffer[0] = RELAY_MAX_SIZE;
	
	for (i = 0; i < RELAY_MAX_SIZE; i++)
	{
		relay = &relay_struct[i];
		buffer[1 + i] = relay->level;
	}
	
	return 0;
}


static int _write(unsigned char *buffer,int len)
{
	int i,index;
	struct relay_t *relay;
	
	index = buffer[0];
	if (index == 255)
	{
		for (i = 0; i < RELAY_MAX_SIZE; i++)
		{
			relay = &relay_struct[i];
			relay->level = buffer[1];
	
			relay->timeout = buffer[2];
			relay->timeout <<= 8;
			relay->timeout += buffer[3];
			if (relay->timeout > 3600)//s
			{
				relay->timeout = 0;
			}
			relay->count = 0;
			relay->update = 1;
			
		}
		return _status(buffer,len);
	}
	
	if (index == 0 || index > RELAY_MAX_SIZE)
		return 1;
	
	relay = &relay_struct[index-1];
	relay->level = buffer[1];
	
	relay->timeout = buffer[2];
	relay->timeout <<= 8;
	relay->timeout += buffer[3];
	if (relay->timeout > 3600)//s
	{
		relay->timeout = 0;
	}
	relay->count = 0;
	relay->update = 1;
	
	// status
	return _status(buffer,len);

}


static int _handler(void *event)
{
	static int timeout = 100;
	int i;
	struct relay_t *relay;
	
	timeout++;
	if (timeout < 100) return 0;
	timeout = 0;
	
	for (i = 0; i < RELAY_MAX_SIZE; i++)
	{
		relay = &relay_struct[i];
		if (relay->update == 0)
			continue;
		
		if (relay->timeout)
		{
			relay->timeout--;
		}
		else
		{
			relay->update = 0;
			
			if (i < 4)// dimmer
			{
				dimmer_low_set(i,relay->level);
			}
			else 
			{
				relay_low_set(i-4,relay->level);
			}
		}
	}
	
	return 0;
}

//====================low interface=====================
const struct driver_interface_t relay_interface =
{
	_get,_set,_init,_status,_read,_write,_handler
};

#endif


