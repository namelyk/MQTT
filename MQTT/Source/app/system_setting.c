#include "system_setting.h"
#include "sys_include.h"
#include "STM32Fxx_flash.h"

#ifdef SYSTEM_SETTING_ENABLE

#define SYSTEM_SETTING_CONFIG_TIME			30//30S


extern unsigned char led_status;
extern unsigned int  led_timeout;
static unsigned char reset_config_flag = 0;
unsigned char restore_timeout = 0;

int system_setting_reset_config(unsigned char *buffer)
{
	//uint32_t pu32;
	if(buffer[0] == 0 && buffer[1] == 0)
	{
		if (sys_memcmp(devices->MAC,&buffer[2],8))
		{
			buffer[0] = 0xF5;
			buffer[1] = 0;
			buffer[2] = 0;
			buffer[3] = SYSTEM_SETTING_CONFIG_TIME;
			return 0;
		}
		devices->subnet = buffer[10];
		devices->device = buffer[11];
		buffer[0] = 0xF8;
	    buffer[1] = 0;
		buffer[2] = 0;
		buffer[3] = SYSTEM_SETTING_CONFIG_TIME;
		reset_config_flag = 1;
		
		//pu32 = 0xFFFFFFFF;
        //eeprom_write(EEPROM_GLOBALE_FALG_ADDR,(uint8_t*)&pu32,4);
	}
	
	return 0;
}

int system_setting_set_position(unsigned char *buffer)
{
	led_status = 3;           
	if(buffer[0] > 30)
	{
		led_timeout = buffer[0]*100;
	}
	else
	{
		led_timeout = 3000;
	}
	return 0;
}

int system_setting_get_program(unsigned char *buffer,unsigned char frame_len)
{
	static unsigned char redomn[4], Count, i, Recv_my_addr_flag = 0;
	unsigned char subnet_buffer[3];
	unsigned char Buffer[32];
	devices_get_subnet(subnet_buffer);
	Count = (frame_len - 13)/2;
	for(i = 0; i < Count; i++)
	{
		if(buffer[2+2*i] == subnet_buffer[0] && buffer[3 + i * 2] == subnet_buffer[1])
		{
			Recv_my_addr_flag = 1;
			return 0;
		}
	}
	if(redomn[0] != buffer[0] || redomn[1] != buffer[1])
	{
		;
	}
	else
	{
		if(Recv_my_addr_flag == 1) return 0;
	}
	Recv_my_addr_flag = 0;
	redomn[0] = buffer[0];
	redomn[1] = buffer[1];
	buffer[2] = 254;
	buffer[3] = 6;//yk 2019-12-30改 之前为1
	buffer[4] = 1;
	devices_get_mac(&buffer[5]); //8字节
	devices_get_remark(&buffer[13]); //读取备注 20byte
	netip_gateway_get_ipaddress(Buffer); 
	memcpy(&buffer[33],Buffer,14); // addr gateway mac
	
	//yk 2019-12-30改
	memcpy(&buffer[47],Buffer+16,5); //submask dhcp
	buffer[52] = devices_get_subnet(NULL);//子网号
	netcrypt_interface.get(&buffer[53], 17);//aes enable,aes
	//iot_config_interface.getAES((char *)&buffer[53], 17);//aes enable,aes
	memcpy(&buffer[70], Buffer + 22, 9); //dnsenable, dns1, dns2
	return 79;
	
}

int system_setting_set_program(unsigned char *buffer,unsigned char size)
{
	if(buffer[0] == 254 && buffer[1] == 6)//有效的
	{
		easy_netip_gateway_set_ipaddress(&buffer[31]);
	}
	return 0;
}

int Restore_factory_settings(void)
{
    unsigned char buffer[5] = {0};
	unsigned char refreshBuff[50] = {0};
	
	restore_timeout++;
	if(restore_timeout <= 50) return 1;
	restore_timeout = 0;
	
    if(reset_config_flag == 0)	return 1;
		
	buffer[0] = 0xF8;
	buffer[1] = 0;
	buffer[2] = 0;
	buffer[3] = 0;	          
	if(reset_config_flag == 1)                //如果启用此功能，则要分页擦除
	{	
		eeprom_read(EEPROM_DEVICE_BASIC_ADDR, refreshBuff, 50);
		Flash_EraseChip();  
		eeprom_write(EEPROM_DEVICE_BASIC_ADDR, refreshBuff, 50);//子网id 设备id mac恢复 备注 
		//LED_RED_ON();
	#ifdef NETIP_GATEWAY_ENABLE
		//netip_gateway_reset_ipaddress();  //不初始化ip
	#endif
	
	#ifndef RESET_DEBUG
		LED_RED_OFF();
	#endif
		system_reboot();
	}
	app_commands_send(1,RESET_TARGET_INFO_ACK, \
				0xff,0xff,buffer,4);
	
	reset_config_flag = 0;
	
	return 0;
}


void reset_restore_timeout(void)
{
	restore_timeout = 0;
}



#endif



