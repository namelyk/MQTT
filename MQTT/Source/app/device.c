#include "device.h"
#include "sys_include.h"

#ifdef DEVICE_ENABLE

static struct devices_t device_struct;
struct devices_t *devices;

unsigned char led_status = 0;
unsigned char program_mode = 0;
unsigned int led_timeout = 6000;

extern char HDL_MQTT_USE_MAC[17];  //用于mqtt
extern char MQTT_MAC_UUID[37];

//=======================User define API===============================

#ifdef NETIP_ENABLE
static void devices_get_uid(unsigned char *uid)
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
#endif


static void device_get_ipmac(unsigned char *ipmac,unsigned char *uid)
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
#define CG_HEX_TO_ASCII_NUM(A) 			(A+48)
#define CG_HEX_TO_ASCII_CHAR(A)			(A+55)
static unsigned char Set_HDL_MQTT_USE_MAC(unsigned char *MAC)
{
	int i;
	
	for(i = 0; i < 8; i++)
	{
		if((MAC[i] >> 4) >= 0 && (MAC[i] >> 4) <= 9)
		{
			HDL_MQTT_USE_MAC[0+i*2] = CG_HEX_TO_ASCII_NUM( (MAC[i] >> 4) );
		}
		else
		{
			HDL_MQTT_USE_MAC[0+i*2] = CG_HEX_TO_ASCII_CHAR( (MAC[i] >> 4) );
		}
		
		if((MAC[i] & 0x0f) >= 0 && (MAC[i] & 0x0f) <= 9)
		{
			HDL_MQTT_USE_MAC[1+i*2] = CG_HEX_TO_ASCII_NUM( (MAC[i] & 0x0f));
		}
		else
		{
			HDL_MQTT_USE_MAC[1+i*2] = CG_HEX_TO_ASCII_CHAR( (MAC[i] & 0x0f));
		}
	}
	HDL_MQTT_USE_MAC[16] = '\0';
}

static void devices_init(void)
{
	unsigned char buffer[100];
	
	led_status = 0;
	program_mode = 0;
	devices = &device_struct;
	
	
	// subnet 
	eeprom_read(EEPROM_DEVICE_BASIC_ADDR,buffer,2);
	if(buffer[0]==255)buffer[0] = 100;
	if(buffer[1]==255)buffer[1] = 100;
	
	devices->subnet = buffer[0];
	devices->device = buffer[1];
	devices->device = 0;
	
	// MAC
	eeprom_read(EEPROM_MAC_BASIC_ADDR,devices->MAC,8);
	Set_HDL_MQTT_USE_MAC(devices->MAC);
	 
	// remark
	eeprom_read(EEPROM_REMARK_BASIC_ADDR,devices->name,20);
	
	
#ifdef NETIP_ENABLE
	// UID
	devices_get_uid(devices->UID);
	device_get_ipmac(netip_gateway.mac,&devices->UID[4]);
#endif
	
}
int devices_get_remark(unsigned char *remark)
{
	memcpy(remark,devices->name,20);
	return 0;
}

int devices_set_remark(unsigned char *remark)
{
	memcpy(devices->name,remark,20);
	eeprom_write(EEPROM_REMARK_BASIC_ADDR,remark,20);
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
	eeprom_write(EEPROM_MAC_BASIC_ADDR,mac,8);
	Set_HDL_MQTT_USE_MAC(mac);
	netip_MQTT_Set_Status();
	
	return 0;
}


int devices_get_subnet(unsigned char *buffer)
{
	if(buffer != NULL)
	{
		buffer[0] = devices->subnet;
		buffer[1] = devices->device;
	}
	return devices->subnet;//yk 2019-12-30改
}


int devices_set_subnet(unsigned char *buffer)
{
	if(buffer[0]==255)buffer[0]=100;
	if(buffer[1]==255)buffer[1]=100;
	
	devices->subnet = buffer[0];
	devices->device = buffer[1];
	
	eeprom_write(EEPROM_DEVICE_BASIC_ADDR,buffer,2);
	netip_MQTT_Set_Status();
	return 0;
}

int devices_check_set_subnet(unsigned char *buffer)
{
	if (sys_memcmp(devices->MAC,buffer,8))
	{
		return 1;
	}
	devices_set_subnet(&buffer[8]);
	return 0;
}


void devices_button_set_program(unsigned char value)
{
	led_timeout = 0;
	program_mode = value;
}

int devices_button_get_mode(void)
{
	return program_mode;
}


static void device_button_handler(void)
{
	static int timeout;
	
	if (KEY_EVENT_VALUE())
	{
		timeout++;
		if(program_mode == 1)
		{
			program_mode = 0;
			led_status = 0;
		}
		if (timeout > 1000)//10s
		{
			led_status = 3;
			program_mode = 0;
		}
		else if (timeout > 200)//2s
		{
			// led
			led_status = 2;
			program_mode = 1;
			led_timeout = 12000;
		}
	}
	else
	{
		// reboot system
		if (led_status == 3 && timeout > 1000)
		{
			system_reboot();
			
#ifdef NETIP_GATEWAY_ENABLE
			netip_gateway_reset_ipaddress();
#endif
		}
		timeout = 0;
	}
}


static void device_led_handler(void)
{
	static unsigned int timeout = 0;
	static unsigned char red_status = 0;
	
	// timeout
	if (led_timeout)
	{
		led_timeout--;
	}
	else
	{
		led_status = 0;
		program_mode = 0;
	}
	
	switch (led_status)
	{
		case 0://正常
			led_timeout = 12000;//60秒
			timeout++;
			if (timeout > 200)
			{
				timeout = 0;
				//if(iot_config_interface.get_status() != 3)
				{
					LED_GREEN_ON();
				}
//				else
//				{
//					
//					LED_RED_ON();
//				}
			#ifndef RESET_DEBUG
//				if(iot_config_interface.get_status() != 3)
				{
					LED_RED_OFF();
				}
//				else
//				{
//					LED_GREEN_OFF();
//				}
			#endif
			}
			else if (timeout > 20)
			{
//				if(iot_config_interface.get_status() != 3)
				{
					LED_GREEN_OFF();
				}
//				else
//				{
//					LED_RED_OFF();
//				}
			}
			break;
			
		case 1://按2S
			timeout++;
			if (timeout > 200)
			{
				timeout = 0;
				LED_GREEN_ON();
				LED_RED_ON();
			}
			else if (timeout > 20)
			{
				LED_GREEN_OFF();
			}
			break;
			
		case 2://KEY  2s
			timeout++;
			if (timeout > 200)
			{
				timeout = 0;
				LED_GREEN_ON();
				LED_RED_ON();
				
			}
			else if (timeout > 20)
			{
				LED_GREEN_OFF();
			}
			break;
			
		case 3:
			timeout++;
			if ((timeout%20) == 0)
			{
				if(red_status == 0)
				{
					LED_RED_ON();
					LED_GREEN_OFF();
					red_status = 1;
				}
				else if(red_status == 1)
				{
				#ifndef RESET_DEBUG
					LED_RED_OFF();
				#endif
					LED_GREEN_OFF();
					red_status = 0;
				}
				
				if(timeout >= 400)
				{
					led_status = 0;
				}
			}
			break;
		
		default:
			led_status = 0;
			break;
	}
}

int devices_is_broadcast(unsigned char subnet,unsigned char device)
{
	if (subnet == 0xFF && device == 0XFF)
		return 1;
	else if (subnet == devices->subnet && device == 0XFF)
		return 1;
	else
		return 0;
}

int devices_is_localaddress(unsigned char subnet,unsigned char device)
{
	if (subnet == devices->subnet && device == devices->device)
		return 1;
	else
		return 0;
}

int devices_is_local_subnet(unsigned char subnet)
{
	if (subnet == devices->subnet)
		return 1;
	else
		return 0;
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

	devices_init();
	
	
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
	device_led_handler();
	device_button_handler();
	
	Restore_factory_settings();
	return 0;
}


//====================app interface=====================
const struct app_interface_t device_interface =
{
	_get, _set,_init, _status, _read, _write, _handler
};


#endif
