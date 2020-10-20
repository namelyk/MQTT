#include "gateway.h"
#include "sys_include.h"

#ifdef GATEWAY_ENABLE

struct netip_t netip_gateway;


//===========================================
int _get(unsigned char *config, int len)
{
	
	return 0;
}

int _set(unsigned char *config, int len)
{
	
	return 0;
}


static void _init(void)
{
	struct netip_t *gateway;
	gateway = &netip_gateway;
	
	gateway->addr.b[0] = 192;
	gateway->addr.b[1] = 168;
	gateway->addr.b[2] = 1;
	gateway->addr.b[3] = 189;
	
	gateway->gw.b[0] = 192;
	gateway->gw.b[1] = 168;
	gateway->gw.b[2] = 10;
	gateway->gw.b[3] = 1;
	
	gateway->submask.b[0] = 255;
	gateway->submask.b[1] = 255;
	gateway->submask.b[2] = 255;
	gateway->submask.b[3] = 0;
	
	gateway->dhcp = 1;
	memset(gateway->domain,0,sizeof(gateway->domain));
	strcpy(gateway->domain,"www.baidu.com");
	
	netip_setup(gateway);
}

int _status(unsigned char *buffer, int len)
{
	return 0;
}

int _read(unsigned char *buffer,int len)
{
	
	return 0;
}


int _write(unsigned char *buffer,int len)
{
	
	return 0;
}


int _handler(void *event)
{
	
	
	return 0;
}


//
const struct app_interface_t gateway_interface =
{
	_get,_set,_init,_status,_read,_write,_handler
};

#endif
