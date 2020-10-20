#include "client.h"
#include "sys_include.h"

#ifdef CLIENT_ENABLE
#include "MQTTLinux.h"

#define CLIENT_MAX_SIZE				4


#define CLIENT_WRITE							 netip_ptp_write


#pragma pack(1)

// client
struct client_t
{
	unsigned short enable;
	unsigned short timeout;
	unsigned int ipaddr;
	unsigned short port;
	
};


#pragma pack()


static struct client_t client_struct[CLIENT_MAX_SIZE];



//=======================User define API===============================

static int client_sendto(unsigned int ipaddr,unsigned short port,unsigned char *buffer,int len)
{
	
}

int client_set(unsigned int ipaddr,unsigned short port)
{
	int i;
	struct client_t *client;
	
	for (i = 0; i < CLIENT_MAX_SIZE; i++)
	{
		client = &client_struct[i];
		if (client->enable)
		{
			if (client->ipaddr == ipaddr
			 && client->port == port)
			{
				client->timeout = 1000;
				return 0;
			}
		}
	}
	
	for (i = 0; i < CLIENT_MAX_SIZE; i++)
	{
		client = &client_struct[i];
		if (client->enable == 0)
		{
			client->ipaddr = ipaddr;
			client->port = port;
			client->timeout = 1000;
			break;
		}
	}
	return 0;
}




void client_init(void)
{
	int i;
	struct client_t *client;
	
	for (i = 0; i < CLIENT_MAX_SIZE; i++)
	{
		client = &client_struct[i];
		client->enable = 0;
		client->timeout = 0;
	}
}


int client_send(int s,unsigned int cmd,unsigned char subnet,\
				unsigned char device,unsigned char *buffer,unsigned char size)
{
	int i;
	struct client_t *client;
	
	for (i = 0; i < CLIENT_MAX_SIZE; i++)
	{
		client = &client_struct[i];
		if (client->enable)
		{	
			client_sendto(client->ipaddr,client->port,buffer,len);
		}
	}
}


int client_write(unsigned char *buffer,int len)
{
	int i;
	struct client_t *client;
	
	for (i = 0; i < CLIENT_MAX_SIZE; i++)
	{
		client = &client_struct[i];
		if (client->enable)
		{	
			client_sendto(client->ipaddr,client->port,buffer,len);
		}
	}
	return 0;
}




int client_handler(void)
{
	int i;
	struct client_t *client;
	
	for (i = 0; i < CLIENT_MAX_SIZE; i++)
	{
		client = &client_struct[i];
		if (client->enable)
		{
			if (client->timeout)
				client->timeout--;
			else
				client->enable = 0;
		}
	}
	
	return 1;
}





#endif


