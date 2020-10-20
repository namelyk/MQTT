#include "sys_include.h"
#include "objects.h"




#ifdef OBJECTS_ENABLE

#define OBJECTS_MAX_SIZE				(8)


//==========================================================
struct objects_t
{
	unsigned char enable:4;
	unsigned char retry:4;
	unsigned char timeout;
	
	int cmd;
	unsigned char subnet;
	unsigned char device;
	unsigned char size;
	unsigned char buffer[20];
};


static unsigned char objects_index = 0;
static struct objects_t objects_struct[OBJECTS_MAX_SIZE];


//=====================================================================

#define OBJECTS_SEND			app_commands_send

#define IS_LOCAL(A,B)			devices_is_localaddress(A,B)	
#define IS_LOCAL_SUBNET(A)		devices_is_local_subnet(A)	



//=======================User define API===============================
void objects_init(void)
{
	int i;
	struct objects_t *object;
	
	objects_index = 0;
	for(i = 0; i < OBJECTS_MAX_SIZE; i++)
	{
		object = &objects_struct[i];
		object->enable = 0;
	}
}

int objects_request(int cmd,unsigned char subnet,unsigned char device,unsigned char *buffer,unsigned char len)
{
	int i;
	struct objects_t *object;
	
	if (IS_LOCAL(subnet,device)) return 0;
	
	for(i = 0; i < OBJECTS_MAX_SIZE; i++)
	{
		object = &objects_struct[i];
		if (object->enable == 0)
		{
			object->cmd = cmd;
			object->subnet = subnet;
			object->device = device;
			object->size = len;
			memcpy(object->buffer,buffer,len);
			object->enable = 1;
			object->retry = 0;
			object->timeout = 50;
			return 0;
		}
		
	}
	return 1;
}

int objects_response(int cmd,unsigned char subnet,unsigned char device,unsigned char *buffer,unsigned char len)
{
	int i;
	struct objects_t *object;
	
	for(i = 0; i < OBJECTS_MAX_SIZE; i++)
	{
		object = &objects_struct[i];
		if (object->enable == 2)
		{
			if (object->cmd == cmd
			&& object->subnet == subnet
			&& object->device == device)
			{
				object->enable = 0;
				object->retry = 0;
				object->timeout = 0;
			}
		}
		
	}
	
	return 0;
}


void objects_handler(void)
{
	int i;
	int retval;
	struct objects_t *object;
	
	
	for (i = 0; i < OBJECTS_MAX_SIZE; i++)
	{
		object = &objects_struct[i];
		if (object->enable == 0) continue;
		
		object->timeout++;
		if (object->timeout < 50)continue;// 500ms
		object->timeout = 0;
	
		// send buspro
		if (IS_LOCAL_SUBNET(object->subnet))
		{
			// request
			retval = OBJECTS_SEND(0,object->cmd,object->subnet,object->device,\
							object->buffer,object->size);
		}
		else//send netip
		{
			retval = OBJECTS_SEND(1,object->cmd,object->subnet,object->device,\
							object->buffer,object->size);
		}
		
		if (retval == 0)
		{
			object->enable = 2;
			object->retry++;
			if (object->retry > 3)
			{
				object->retry = 0;
				object->enable = 0;
			}
		}
		break;
	}
}



#endif
