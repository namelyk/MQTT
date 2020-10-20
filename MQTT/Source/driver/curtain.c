#include "driver.h"
#include "curtain.h"


#ifdef CURTAIN_ENABLE



#define CURTAIN_MAX_SIZE									(4)
#define CURTAIN_CONFIG_MAX_SIZE						(32)



#define CURTAIN_RUNNING_IDLE							0
#define CURTAIN_RUNNING_OPEN							1
#define CURTAIN_RUNNING_CLOSE							2

//====================================================
struct curtain_t
{
	unsigned char enable:1;
	unsigned char sw:1;
	unsigned char status:4;
	unsigned char running:2;
	
	unsigned char percent;
	
	unsigned int time;
	unsigned int count;
};


static unsigned char curtain_index = 0;
static struct curtain_t curtain_struct[CURTAIN_MAX_SIZE];



//=======================User define API===============================
int curtain_get(int index,unsigned char *buffer,int len)
{
	
	return 0;
}


int curtain_set(int index,unsigned char *buffer,int len)
{
	
	return 0;
}





//=======================Standard define API===============================

static int _get(unsigned char *config,int len)
{
	unsigned int addr;
	int index;
	
	if (index >= CURTAIN_MAX_SIZE) return 1;
	
	addr = CURTAIN_CONFIG_BASIC_ADDRESS;
	addr += index * CURTAIN_CONFIG_MAX_SIZE;
	
	eeprom_read(addr,config,CURTAIN_CONFIG_MAX_SIZE);
	return 0;
}


static int _set(unsigned char *config,int len)
{
	unsigned int addr;
	int index;
	
	if (index >= CURTAIN_MAX_SIZE) return 1;
	
	addr = CURTAIN_CONFIG_BASIC_ADDRESS;
	addr += index * CURTAIN_CONFIG_MAX_SIZE;
	
	eeprom_write(addr,config,CURTAIN_CONFIG_MAX_SIZE);
	return 0;
}


//====================================================

static void _init(void)
{
	int i;
	struct curtain_t *curtain;
	unsigned char buffer[CURTAIN_CONFIG_MAX_SIZE];
	
	
	for (i = 0; i < CURTAIN_MAX_SIZE; i++)
	{
		curtain = &curtain_struct[i];
	//	curtain_interface.get(i,buffer,CURTAIN_CONFIG_MAX_SIZE);
		
		curtain->time = (int)((buffer[21]) * 60);
		curtain->time += buffer[22];
		if (curtain->time < 2)
			curtain->time = 2;
		
		curtain->time *= 10;
		
		curtain->status = 0;
		
		curtain->running = 0;
		curtain->count = 0;
		
		if (buffer[0] == 1)
		{
			curtain->enable = 1;
		}
		else
		{
			curtain->enable = 0;
		}
	}
}


static int _status(unsigned char *buffer,int len)
{
	int index;
	
	
	return 0;
}

static int _read(unsigned char *buffer,int len)
{
	int index;
	if (index >= CURTAIN_MAX_SIZE) return 1;
	if (curtain_struct[index].enable == 0) return 1;
	
	buffer[0] = curtain_struct[index].sw;//open close
	buffer[1] = curtain_struct[index].percent;// percent

	return 0;
}


static int _write(unsigned char *buffer,int len)
{
	int index;
	
	if (index >= CURTAIN_MAX_SIZE) return 1;
	if (curtain_struct[index].enable == 0) return 1;
	
	if ((curtain_struct[index].sw != (buffer[0] & 0x01))
	|| (curtain_struct[index].percent != buffer[1]))
	{
			curtain_struct[index].status = 1;
	}
	
	curtain_struct[index].sw = (buffer[0] & 0x01);
	curtain_struct[index].percent = buffer[1];
	
	return 0;
}




static int _handler(void *event)
{
	unsigned char buf[10];
	struct curtain_t *curtain;
	
	curtain_index++;
	if (curtain_index >= CURTAIN_MAX_SIZE)
	{
		curtain_index = 0;
	}
	curtain = &curtain_struct[curtain_index];
	
	if (curtain->enable == 0) return 1;

	switch (curtain->status)
	{
		case 1://search
			if (curtain->running == CURTAIN_RUNNING_OPEN)
			{
				if (curtain->sw)
				{
					;//curtain->status = 1;
				}
				else
				{
					curtain->status = 4;
				}
			}
			else if (curtain->running == CURTAIN_RUNNING_CLOSE)
			{
				if (curtain->sw)
				{
					curtain->status = 8;
				}
				else
				{
					;//curtain->status = 6;
				}
			}
			else
			{
				if (curtain->sw)
				{
					curtain->status = 1;
				}
				else
				{
					curtain->status = 6;
				}
			}
			break;
			
		//open
		case 2:
			curtain->running = CURTAIN_RUNNING_OPEN;
		
			buf[0] = 1;//open
			buf[1] = curtain->percent;
			if (!curtain_set(curtain_index,buf,10))
			{
				curtain->status = 3;
				curtain->count = 0;
			}
			break;
		
		//open delay
		case 3:
			curtain->count++;
			if (curtain->count >= curtain->time)
			{
				curtain->count = 0;
				curtain->status = 5;
				
				buf[0] = 0;//open close
				buf[1] = curtain->percent;
				curtain_set(curtain_index,buf,10);
			}
			break;
			
		// open close
		case 4:
			curtain->count = 0;
			curtain->status = 5;
		
			buf[0] = 0;//open close
			buf[1] = curtain->percent;
			curtain_set(curtain_index,buf,10);
			break;

		// open close delay 2s
		case 5:
			curtain->count++;
			if (curtain->count >= 200)
			{
				curtain->count = 0;
				curtain->running = CURTAIN_RUNNING_IDLE;
				
				if (curtain->sw)//if open curtain
				{
					curtain->status = 0;
				}
				else
				{
					curtain->status = 6;
				}
			}
			break;	
		
		//close
		case 6:
			curtain->running = CURTAIN_RUNNING_CLOSE;
		
			buf[0] = 0x81;//close
			buf[1] = curtain->percent;
			if (!curtain_set(curtain_index,buf,10))
			{
				curtain->status = 7;
				curtain->count = 0;
			}
			break;
			
		//close delay
		case 7:
			curtain->count++;
			if (curtain->count >= curtain->time)
			{
				curtain->count = 0;
				curtain->status = 9;

				buf[0] = 0x80;//close close
				buf[1] = curtain->percent;
				curtain_set(curtain_index,buf,10);
			}
			break;	
			
		case 8:
			curtain->count = 0;
			curtain->status = 9;
			buf[0] = 0x80;//close close
			buf[1] = curtain_struct[curtain_index].percent;
			curtain_set(curtain_index,buf,10);
			break;	
			
		case 9:
			curtain->count++;
			if (curtain->count >= 200)
			{
				curtain->count = 0;
				curtain->running = CURTAIN_RUNNING_IDLE;
				if (curtain->sw)//if open curtain
				{
					curtain->status = 2;
				}
				else
				{
					curtain->status  = 0;
				}
			}
			break;
	
		default:
			curtain->status = 0;
			curtain->running = 0;
			break;
		
	}
	
	return 0;
}

//====================low interface=====================
const struct driver_interface_t curtain_interface =
{
	_get,_set,_init,_status,_read,_write,_handler
};

#endif




