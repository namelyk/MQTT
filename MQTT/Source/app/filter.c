#include "sys_include.h"
#include "filter.h"



#ifdef FILTER_ENABLE

#define FILTER_MAX_SIZE								(16)
#define FILTER_CONFIG_MAX_SIZE				(8)


#define NC2(X)			(((X & 0x00ff) << 8) | ((X & 0xff00) >>8))

#pragma pack(1)
struct filter_t
{
	unsigned char enable:1;
	unsigned char type:7;
	
	unsigned char src_subnet;
	unsigned char src_device;
	
	unsigned short cmd;
	
	unsigned char dest_subnet;
	unsigned char dest_device;
			
};

struct filter_commands_t
{
	unsigned char length;
	unsigned char src_subnet;
	unsigned char src_device;
	unsigned short device_type;
	unsigned short cmd;
	unsigned char dest_subnet;
	unsigned char dest_device;
	
};

#pragma pack()

static struct filter_t filter_struct[FILTER_MAX_SIZE];






//=======================Standard define API===============================
static int _get(unsigned char *config,int len)
{
	unsigned int addr;
	int index;
	
	index = config[0];
	if (index >= FILTER_MAX_SIZE) return 1;

	addr = FILTER_CONFIG_BASIC_ADDRESS;
	addr += FILTER_CONFIG_MAX_SIZE * index;
	
	eeprom_read(addr,&config[1],FILTER_CONFIG_MAX_SIZE);
	return 0;
}


static int _set(unsigned char *config,int len)
{
	unsigned int addr;
	int index;
	
	index = config[0];
	if (index >= FILTER_MAX_SIZE) return 1;
	
	if (config[1] != 0x01)
	{
		config[1] = 0;
	}

	filter_interface.write(config,len);
	
	addr = FILTER_CONFIG_BASIC_ADDRESS;
	addr += FILTER_CONFIG_MAX_SIZE * index;
	
	eeprom_write(addr,&config[1],FILTER_CONFIG_MAX_SIZE);
	return 0;
}




//====================================================
int i;
struct filter_t *filter;
unsigned char buffer[10];
static void _init(void)
{
	for (i = 0; i < FILTER_MAX_SIZE; i++)
	{
		filter = &filter_struct[i];
		buffer[0] = i;
		filter_interface.get(buffer, FILTER_CONFIG_MAX_SIZE);
	
		if (buffer[1])
		{
			filter->enable = 1;
		}
		else
		{
			filter->enable = 0;
		}
	
		filter->type = buffer[2];
		filter->src_subnet = buffer[3];
		filter->src_device = buffer[4];
		filter->cmd  = buffer[5];
		filter->cmd <<= 8;
		filter->cmd  |= buffer[6];
		filter->dest_subnet = buffer[7];
		filter->dest_device = buffer[8];
	}
}


static int _status(unsigned char *buffer,int len)
{
	
	return 0;
}

static int _read(unsigned char *buffer,int len)
{
	
	return 0;
}

static int _write(unsigned char *buffer,int len)
{
	struct filter_t *filter;
	unsigned char *message;
	int index;
	index = buffer[0];
	if (index >= FILTER_MAX_SIZE) return 1;
	filter = &filter_struct[index];
	message = &buffer[1];
	if (*message++)
		filter->enable = 1;
	else
		filter->enable = 0;

	filter->type = *message++;
	filter->src_subnet = *message++;
	filter->src_device = *message++;
	filter->cmd  = *message++;  
	filter->cmd <<= 8;
	filter->cmd  |= *message++;	//test
	filter->dest_subnet = *message++;
	filter->dest_device = *message++;
	
	return 0;
}

static int _handler(void *event)
{
	int i;
	struct filter_t *filter;
	struct filter_commands_t *frame = (struct filter_commands_t *)event;
	unsigned short cmd = 0;
	
	cmd = NC2(frame->cmd);
	for (i = 0; i < FILTER_MAX_SIZE; i++)
	{
		filter = &filter_struct[i];
		if (filter->enable == 0) continue;
		switch (filter->type)
		{
			case 1: //filter src subnet device   only subnet id 
				if (filter->src_subnet == frame->src_subnet 
				&& filter->src_device == frame->src_device)
				{
					return 0;
				}
				break;
				
			case 2://cmd
				if (filter->cmd == cmd)
				{
					return 0;
				}
				break;
				
			case 3: //filter src subnet device cmd
				if (filter->src_subnet == frame->src_subnet 
				&& filter->src_device == frame->src_device
				&& filter->cmd == cmd)
				{
					return 0;
				}
				break;
				
			case 4: //filter dest subnet device
				if (filter->dest_subnet == frame->dest_subnet 
				&& filter->dest_device == frame->dest_device)
				{
					return 0;
				}
				break;
				
			case 5: //filter src dest subnet device
				if (filter->src_subnet == frame->src_subnet 
				&& filter->src_device == frame->src_device
				&& filter->dest_subnet == frame->dest_subnet 
				&& filter->dest_device == frame->dest_device)
				{
					return 0;
				}
				break;
				
			case 6: //filter dest subnet device cmd
				if (filter->dest_subnet == frame->dest_subnet 
				&& filter->dest_device == frame->dest_device
				&& filter->cmd == cmd)
				{
					return 0;
				}
				break;	
				
			case 7: //filter src dest subnet device cmd
				if (filter->src_subnet == frame->src_subnet 
				&& filter->src_device == frame->src_device
				&& filter->dest_subnet == frame->dest_subnet 
				&& filter->dest_device == frame->dest_device
				&& filter->cmd == cmd)
				{
					return 0;
				}
				break;	
				
			default:
				break;
		}
	}
	return 1;
}




//====================app interface=====================
const struct app_interface_t filter_interface =
{
	_get,_set,_init,_status,_read,_write,_handler
};

#endif


