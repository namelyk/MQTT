#include "sys_include.h"
#include "device.h"


#ifdef DEVICE_ENABLE

extern struct netip_t netip_gateway;
static struct devices_t device_struct;
struct devices_t *devices;



//=======================User define API===============================


void devices_get_uid(unsigned char *uid)
{
	unsigned int temp;

	uid[0] = 'H';
	uid[1] = 'D';
	uid[2] = 'L';
	uid[3] = ':';
	
	temp = *(unsigned int*)(0x1FFFF7E8);
	uid[4] = (uint8_t)(temp);
	uid[5] = (uint8_t)(temp>>8);
	uid[6] = (uint8_t)(temp>>16);
	uid[7] = (uint8_t)(temp>>24);
	temp = *(unsigned int*)(0x1FFFF7EC);
	uid[8] = (uint8_t)(temp);
	uid[9] = (uint8_t)(temp>>8);
	uid[10] = (uint8_t)(temp>>16);
	uid[11] = (uint8_t)(temp>>24);
	temp = *(unsigned int*)(0x1FFFF7F0);
	uid[12] = (uint8_t)(temp);
	uid[13] = (uint8_t)(temp>>8);
	uid[14] = (uint8_t)(temp>>16);
	uid[15] = (uint8_t)(temp>>24);
	
}


void device_get_ipmac(unsigned char *ipmac,unsigned char *uid)
{
	unsigned char i;
	unsigned short temp;
	
	ipmac[0] = 0x40;
	
	temp = 0;
	for (i = 0; i < 12;i++)
	{
		temp += uid[i];
	}
	ipmac[1] = (uint8_t)(temp >> 8);
	ipmac[2] = (uint8_t)(temp >> 0);
	
	temp = CRC16_XMODEM(uid,12);
	ipmac[3] = (uint8_t)(temp >> 8);
	ipmac[4] = (uint8_t)(temp >> 0);
	
	ipmac[5] = 0;
	for (i = 0; i < 6;i++)
	{
		ipmac[5] += uid[i];
	}
	for (i = 0; i < 6;i++)
	{
		ipmac[5] ^= uid[6+i];
	}
}


//==========================================================




int devices_get_remark(unsigned char *remark)
{
	memcpy(remark,devices->name,20);
	return 0;
}

int devices_set_remark(unsigned char *remark)
{
	memcpy(devices->name,remark,20);
	
	return 0;
}

int devices_get_mac(unsigned char *mac)
{
	memcpy(mac,devices->MAC,8);
	
	return 0;
}

int devices_set_mac(unsigned char *mac)
{
	memcpy(devices->MAC,mac,8);
	
	return 0;
}


int devices_get_subnet(unsigned char *buffer)
{
	buffer[0] = devices->subnet;
	buffer[1] = devices->device;
	return 0;
}

int devices_set_subnet(unsigned char *buffer)
{
	devices->subnet = buffer[0];
	devices->device = buffer[1];
	
	return 0;
}

int devices_check_set_subnet(unsigned char *buffer)
{
	if (memcmp(devices->MAC,buffer,8))
	{
		return 1;
	}
	devices_set_subnet(&buffer[8]);
	return 0;
}

int devices_reset_config(unsigned char *buffer)
{
	
	return 0;
}

int devices_setting_position(unsigned char *buffer)
{
	
	return 0;
}

int devices_get_program(unsigned char *buffer,unsigned char size)
{
	
	return 0;
}

int devices_set_program(unsigned char *buffer,unsigned char size)
{
	
	return 0;
}

void devices_button_set_program(void)
{
	
}



int devices_is_localaddress(unsigned char subnet,unsigned char device)
{
	if (devices->subnet == subnet && devices->device == device)
		return 1;
	else
		return 0;
}



//获取命令
unsigned short devices_get_command(unsigned char opcode)
{
	unsigned short cmd;
	switch (opcode)
	{
		case 0:
			cmd = 0x1;
			break;
	}
	return cmd;
}


//处理本机命令（包括发送内容处理）
void devices_local_command_handler(unsigned short cmd,unsigned char *buffer,int len)
{
	
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



static void _init(void)
{
	devices = &device_struct;
	devices->subnet = 50;
	devices->device = 0;
	
	// UID
	devices_get_uid(devices->UID);
	
#ifdef NETIP_ENABLE
	device_get_ipmac(netip_gateway.mac,devices->UID);
#endif
}

static int _status(unsigned char *buffer,int len)
{
	int index;
	
	return 0;
}

static int _read(unsigned char *buffer,int len)
{
	
	return 0;
}


static int _write(unsigned char *buffer,int len)
{
	
	return 0;
}


static int _handler(void *event)
{
	
	return 0;
}


//====================app interface=====================
const struct app_interface_t device_interface =
{
	_get, _set,_init, _status, _read, _write, _handler
};


#endif
