#include "driver.h"
#include "air_conditioner.h"


#ifdef AIR_CONDITIONER_ENABLE




// max driver
#define AIR_CONDITIONER_MAX_SIZE									(4)
#define AIR_CONDITIONER_CONFIG_MAX_SIZE						(32)

#define AIR_CONDITIONER_TIMEOUT										(500)//5S



//update flag
#define AIR_UPDATE_SW_FLAG												(0x01)
#define AIR_UPDATE_TEMP_FLAG											(0x02)
#define AIR_UPDATE_MODE_FLAG											(0x04)
#define AIR_UPDATE_WIND_FLAG											(0x08)



//================================================================
#pragma pack(1)
struct air_conditioner_t
{
	unsigned char enable:1;
	unsigned char sw:1;
	unsigned char mode:4;
	unsigned char wind:3;
		
	unsigned char set_temperature:7;
	unsigned char temperature;
	
	unsigned char update;
};

struct air_conditioner_event_t
{
	unsigned char index;
	unsigned char status;
	unsigned int timeout;
};

#pragma pack()

static int air_conditioner_index = 0;
static struct air_conditioner_event_t air_struct;
static struct air_conditioner_t air_conditioner[AIR_CONDITIONER_MAX_SIZE];



//=======================User define API===============================
int air_conditioner_get(int index,unsigned char *buffer,int len)
{
	
	return 0;
}


int air_conditioner_set(int index,unsigned char *buffer,int len)
{
	
	return 0;
}






//=======================Standard define API===============================

static int _get(unsigned char *config,int len)
{
	unsigned int addr;
	int index;
	
	if (index >= AIR_CONDITIONER_MAX_SIZE) return 1;
	
	addr = AIR_CONDITIONER_CONFIG_BASIC_ADDRESS;
	addr += index * AIR_CONDITIONER_CONFIG_MAX_SIZE;
	
	eeprom_read(addr,config,AIR_CONDITIONER_CONFIG_MAX_SIZE);
	return 0;
}


static int _set(unsigned char *config,int len)
{
	unsigned int addr;
	int index;
	
	if (index >= AIR_CONDITIONER_MAX_SIZE) return 1;
	
	addr = AIR_CONDITIONER_CONFIG_BASIC_ADDRESS;
	addr += index * AIR_CONDITIONER_CONFIG_MAX_SIZE;
	
	eeprom_write(addr,config,AIR_CONDITIONER_CONFIG_MAX_SIZE);
	return 0;
}



//================================================================
static void _init(void)
{
	int i;
	unsigned char buf[AIR_CONDITIONER_CONFIG_MAX_SIZE];
	
	
	air_struct.status = 0;
	for (i = 0; i < AIR_CONDITIONER_MAX_SIZE; i++)
	{
		air_conditioner[i].update = 0;
		
//		air_conditioner_interface.get(i, buf,AIR_CONDITIONER_CONFIG_MAX_SIZE);
		if (buf[1] == 1)
		{
			air_conditioner[i].enable = 1;
		}
		else
		{
			air_conditioner[i].enable = 0;
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
	
	if (index >= AIR_CONDITIONER_MAX_SIZE) return 1;
	if (air_conditioner[index].enable == 0) return 1;
	
	buffer[0] = air_conditioner[index].sw;
	buffer[1] = air_conditioner[index].mode;
	buffer[2] = air_conditioner[index].wind;
	buffer[3] = air_conditioner[index].set_temperature;
	buffer[4] = air_conditioner[index].temperature;
	return 0;
}


static int _write(unsigned char *buffer,int len)
{
	int index;
	
	if (index >= AIR_CONDITIONER_MAX_SIZE) return 1;
	if (air_conditioner[index].enable == 0) return 1;
	
	if (air_conditioner[index].sw != (buffer[0] & 0x01))//sw
	{
		air_conditioner[index].update |= AIR_UPDATE_SW_FLAG;
		air_conditioner[index].sw = (buffer[1] & 0x01);
	}
	
	if (air_conditioner[index].mode != (buffer[1] & 0x0f))//mode
	{
		air_conditioner[index].update |= AIR_UPDATE_MODE_FLAG;
		air_conditioner[index].mode = (buffer[1] & 0x0f);
	}
	
	if (air_conditioner[index].wind != (buffer[2] & 0x07))//wind
	{
		air_conditioner[index].update |= AIR_UPDATE_WIND_FLAG;
		air_conditioner[index].wind = (buffer[2] & 0x07);
	}
	
	if (air_conditioner[index].set_temperature != (buffer[3] & 0x7f))//set temperature
	{
		air_conditioner[index].update |= AIR_UPDATE_TEMP_FLAG;
		air_conditioner[index].set_temperature = (buffer[3] & 0x7f);
	}
	
	air_conditioner[index].temperature = buffer[4];
	return 0;
}


static int _handler(void *event)
{
	struct air_conditioner_event_t *air = &air_struct;
	
	switch(air->status)
	{
		case 0:
			if (air_conditioner_index >= AIR_CONDITIONER_MAX_SIZE)
			{
				air_conditioner_index = 0;
			}
			
			if (air_conditioner[air_conditioner_index].enable == 0) 
			{
				air_conditioner_index++;
				break;
			}
			
			if (air_conditioner[air_conditioner_index].update)
			{
					if (!air_conditioner_set(air_conditioner_index,(unsigned char *)&air_conditioner[air_conditioner_index],10))
					{
						air_conditioner[air_conditioner_index].update = 0;
					}
					
					air->timeout = AIR_CONDITIONER_TIMEOUT;
			}
			else
			{
					if (air->timeout)
					{
						air->timeout--;
					}
					else
					{
						air->status = 1;
					}
			}
			air_conditioner_index++;
			break;
			
		case 1:
			if (air->index >= AIR_CONDITIONER_MAX_SIZE)
			{
				air->index = 0;
			}
			
			if (!air_conditioner_get(air->index,(unsigned char *)&air_conditioner[air_conditioner_index],10))
			{
				air->index++;
				air->status = 0;
				air->timeout = AIR_CONDITIONER_TIMEOUT;
			}
			break;

			
		default:
			air->status = 0;
			break;
		
	}
	return 0;
}


//====================low interface=====================
const struct driver_interface_t air_conditioner_interface =
{
	_get,_set,_init,_status,_read,_write,_handler
};
#endif





