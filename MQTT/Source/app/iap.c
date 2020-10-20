#include "iap.h"
#include "sys_include.h"
#include "netcrypt.h"


#ifdef IAP_ENABLE




//============================================================
#define IAP_PROGRAM_ENABLE
#ifdef IAP_PROGRAM_ENABLE

#include "bltable.h"

#define K																	(1024)


//升级地址
#define APPLICATION_BACKUP_ADDRESS				(0x08020000)

//升级标志
#define SIGNAL_FLAG_ADDR									(0x08001000)

/*升级标志*/
#define PROGRAM_FLAG_ADDR									(0X0803F800)

//升级包数据
#define PROGRAM_PACKET_MAX_SIZE						(K)

//页大小
#define PAGE_SIZE				 									(2*K)

// MCU容量
#define CHIP_MAX_SIZE         						(90) 	//90k

//升级类型
#define BOOTLOAD_DEVICE_TYPE							(65498)


extern unsigned char FlashBuffer[];

//===================================================
static unsigned short spm_updata_cur_number = 0;
static unsigned short spm_updata_totol_number = 0;
static unsigned short spm_updata_cur_point = 0;
static unsigned int  spm_updata_flash_addr = 0;
static unsigned char spm_updata_buffer[PAGE_SIZE];

static unsigned int spm_updata_ipaddr = 0;
static unsigned short spm_updata_port = 0;

static unsigned char spm_updata_status = 0;
static unsigned short spm_updata_timeout = 0;

union D32
{
      unsigned int  l;
      unsigned char  b[4]; 
}u32_data; 


//===================================================
static char system_reboot_enable = 0;
static int system_reboot_timeout = 0;

static unsigned char update_source = 0;  

static void system_reboot_init(void)
{
	system_reboot_enable = 0;
	system_reboot_timeout = 0;
}

//如果当前正在重启 不处理任何网络包 2020-07-01
char system_reboot_get_enable(void)
{
	return system_reboot_enable;
}

void system_reboot(void)
{
	system_reboot_enable = 1;
	system_reboot_timeout = 200;
}

void iap_set_update_source(unsigned char source)
{
	update_source = source;
}

unsigned char iap_get_update_source(void)
{
	return update_source;
}

static void system_reboot_handler(void)
{
	if (system_reboot_enable)
	{
		if (system_reboot_timeout)
		{
			system_reboot_timeout--;
		}
		else
		{
			while(1);
		}
	}
}


static void iap_program_packet(void)
{
	static unsigned char mutex = 0; 
	if(mutex == 1) return;
	mutex = 1;
	
	unsigned char packet[40];
	
	unsigned char length;
	unsigned short  crc;
	
	if (spm_updata_cur_number == 0)
	{
		length = 35;
		
		strncpy((char *)&packet[13],(char *)&MCU_CHIP[0],20);
		
		packet[33] = (unsigned char)(CHIP_MAX_SIZE>>8);
		packet[34] = (unsigned char)(CHIP_MAX_SIZE);
	}
	else
	{
		length = 13;
	}
	
	packet[0] = 0xaa;	
	packet[1] = 0xaa;
	packet[2] = length;
	packet[3] = devices->subnet;
	packet[4] = devices->device;;
	packet[5] = (unsigned char)(BOOTLOAD_DEVICE_TYPE>>8);
	packet[6] = (unsigned char)(BOOTLOAD_DEVICE_TYPE);
	packet[7] = 0x15;
	packet[8] = 0xFE;
	packet[9] = 0xFF;
	packet[10] = 0xFF;
	
	packet[11] = (unsigned char)(spm_updata_cur_number >> 8);
	packet[12] = (unsigned char)(spm_updata_cur_number);
	
	crc = CRC16_XMODEM(&packet[2],length - 2);
	
	packet[length] 		= (unsigned char)(crc >> 8);
	packet[length + 1] 	= (unsigned char)(crc);
#ifdef IOT_ENABLE	
//mark	
	if(netip_gateway_type() == MQTT_LINK_SERVER || netip_gateway_type() == STANDAR_MQTT_LINK_SERVER)
	{
		if(iap_get_update_source() == MQTT_UPDATE)
		{
			iot_config_interface.write((char *)&packet[2], length);	
			if(spm_updata_cur_number == 0xf8f8) //清空当前升级标志
			{
				iap_set_update_source(LOCAL_UPDATE);
			}
		}
		else
		{
			netcrypt_interface.write(&packet[2], length);
		}
		//netip_gateway_dowrite(spm_updata_ipaddr,spm_updata_port,&packet[2],length);
	}
	else if(netip_gateway_type() == GATEWAY_LINK_SERVER || netip_gateway_type() == GATEWAY_LINK_DNS)

	{
		if(iap_get_update_source() == REMOTE_UPDATE)
		{
			netip_gateway_dowrite(spm_updata_ipaddr,spm_updata_port,&packet[2],length);
			if(spm_updata_cur_number == 0xf8f8) //清空当前升级标志
			{
				iap_set_update_source(LOCAL_UPDATE);
			}
		}
		else
		{
			netcrypt_interface.write(&packet[2], length);
		}
	}
	else
	{
		netcrypt_interface.write(&packet[2], length);
	}
#endif
	mutex = 0;
}


static int iap_program_signal(void)
{
	unsigned int read_signal = 0;
	
	read_signal = *(__IO uint32_t*)(SIGNAL_FLAG_ADDR);
	if (read_signal == 0x8866aa55)
		return 1;
	else
		return 0;
}


static unsigned int iap_read_word(unsigned int addr)
{
	unsigned int read_word;

	read_word = *(volatile unsigned int*)(addr);
	return read_word;
}

static int iap_write_word(unsigned int addr,unsigned int value)
{
 	unsigned int Count;

	for(Count = 0; Count < 3; Count++)
	{
		FLASH_ProgramWord(addr, value);
		if (*(unsigned int*)addr == value)
		{
			return 0;
		}
	}

	return 1;
}


static int iap_program_write_page(unsigned int addr,unsigned char *buffer,int len)
{
	
	volatile uint32_t ulFlashAddr;
	unsigned int RamSource;
	unsigned int Count;

	ulFlashAddr = addr;
	RamSource = (unsigned int)buffer;
	
	FLASH_Unlock();
	__set_PRIMASK(1);
	
	FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR|FLASH_FLAG_OPTERR);
	
	RESET_WATCH_DOG();	 //清看门狗
	while (FLASH_ErasePage(ulFlashAddr) != FLASH_COMPLETE);
	RESET_WATCH_DOG();	 //清看门狗
	for(Count = 0; Count < (PAGE_SIZE/4);)
	{
		if (iap_write_word(ulFlashAddr,(*(unsigned int*)RamSource)))
		{
			__set_PRIMASK(0);
			FLASH_Lock();
			
			return 1; 
		}
	  ulFlashAddr += 4;
	  RamSource += 4;
		Count++;
	}
	RESET_WATCH_DOG();	 //清看门狗
	__set_PRIMASK(0);
	FLASH_Lock();
	return 0;
}



static int iap_program_read_page(unsigned int addr,unsigned char *buffer,int len)
{
	volatile unsigned int ulFlashAddr;
	int usCount;
	
	ulFlashAddr = addr;

	for(usCount = 0; usCount < PAGE_SIZE;)
	{
		u32_data.l = iap_read_word(ulFlashAddr);
		
		buffer[usCount++] = u32_data.b[0];
		buffer[usCount++] = u32_data.b[1];
		buffer[usCount++] = u32_data.b[2];
		buffer[usCount++] = u32_data.b[3];
		
		//RamSource += 4;
		//usCount += 4;
		ulFlashAddr += 4;	
	}
	
	return 0;
}





#endif


static void iap_program_bootloader(void)
{
#ifdef BL_PROGRAM

	unsigned int FlashAddr = 0;
	unsigned int FlashSize = 0;
	
	while (FlashSize < PAGE_SIZE)
	{
		RESET_WATCH_DOG();
		
		iap_program_write_page(FlashAddr,(unsigned char *)(&Bltable[FlashSize]),PAGE_SIZE);
		FlashSize += PAGE_SIZE;
		FlashAddr += PAGE_SIZE;
		
	}
#endif
}

static void iap_program_upgrade_flag(unsigned char flag)
{
	unsigned int ulPageAddr = PROGRAM_FLAG_ADDR;
	unsigned int fsize;
	
	fsize = (uint32_t)(spm_updata_totol_number+1) * K;
	
	iap_program_read_page( ulPageAddr,FlashBuffer,PAGE_SIZE);
	
	FlashBuffer[0] = flag;
	FlashBuffer[1] = devices->subnet;
	FlashBuffer[2] = devices->device;
	
	// cpy ipaddr
	memcpy(&FlashBuffer[3],netip_gateway.addr.b,4);
	memcpy(&FlashBuffer[7],netip_gateway.gw.b,4);	
	memcpy(&FlashBuffer[11],netip_gateway.mac,6);
	memcpy(&FlashBuffer[17],netip_gateway.submask.b,4);
	FlashBuffer[21] = 0;
	FlashBuffer[22] = 0xaa;
	
	FlashBuffer[30] = (uint8_t)(fsize >> 0);
	FlashBuffer[31] = (uint8_t)(fsize >> 8);
	FlashBuffer[32] = (uint8_t)(fsize >> 16);
	FlashBuffer[33] = (uint8_t)(fsize >> 24);
	FlashBuffer[34] = (uint8_t)((~fsize) >> 0);
	FlashBuffer[35] = (uint8_t)((~fsize) >> 8);
	FlashBuffer[36] = (uint8_t)((~fsize) >> 16);
	FlashBuffer[37] = (uint8_t)((~fsize) >> 24);
	
	iap_program_write_page(ulPageAddr,FlashBuffer,PAGE_SIZE);
}



//===========================================================


extern char netip_tx_buffer[];
extern char netip_rx_buffer[];

static int iap_program_write(unsigned char *buffer,int len)
{
#ifdef IAP_PROGRAM_ENABLE	
	unsigned short program_length;
	unsigned short packet_number;
	memcpy(netip_tx_buffer,buffer,len);
	program_length = buffer[0];
	program_length <<= 8;
	program_length += buffer[1];

	packet_number = buffer[2];
	packet_number <<= 8;
	packet_number += buffer[3];

	if (packet_number == 0)
	{
		if (program_length != 4)	return 1;
		
		// 升级开始
		spm_updata_totol_number = buffer[4];
		spm_updata_totol_number <<= 8;
		spm_updata_totol_number += buffer[5];
		
		if (spm_updata_totol_number < CHIP_MAX_SIZE)
		{
			spm_updata_status = 1;// start
			spm_updata_timeout = 0;
			
			spm_updata_cur_number = 1;
			spm_updata_cur_point = 0;
			spm_updata_flash_addr = APPLICATION_BACKUP_ADDRESS;//ApplicationBackup;
		}
	}
	else
	{
		// 升级包处理
		if (spm_updata_cur_number == packet_number 
			&& spm_updata_cur_number < spm_updata_totol_number
			&& program_length == (PROGRAM_PACKET_MAX_SIZE+2))
		{
			spm_updata_cur_number++;
			spm_updata_timeout = 0;
			
			if (spm_updata_cur_point < PROGRAM_PACKET_MAX_SIZE)
			{
				memcpy(&spm_updata_buffer[spm_updata_cur_point],&buffer[4],PROGRAM_PACKET_MAX_SIZE);
				spm_updata_cur_point += PROGRAM_PACKET_MAX_SIZE;
			}
			else
			{
				memcpy(&spm_updata_buffer[spm_updata_cur_point],&buffer[4],PROGRAM_PACKET_MAX_SIZE);
				spm_updata_cur_point = 0;
			
				if (!iap_program_write_page(spm_updata_flash_addr,spm_updata_buffer,PAGE_SIZE))
				{
					spm_updata_flash_addr += PAGE_SIZE;	
				}
				else
				{
					spm_updata_cur_number -= (PAGE_SIZE / PROGRAM_PACKET_MAX_SIZE);
				}
			}
		}

		// 升级最后包
		if (spm_updata_cur_number == packet_number 
		&& spm_updata_cur_number == spm_updata_totol_number)
		{
			spm_updata_timeout = 0;
			
			if (spm_updata_cur_point < PROGRAM_PACKET_MAX_SIZE)
			{
				memcpy(&spm_updata_buffer[spm_updata_cur_point],&buffer[4],(program_length - 2));
				spm_updata_cur_point += (program_length - 2);
				
				if (!iap_program_write_page(spm_updata_flash_addr,spm_updata_buffer,PAGE_SIZE))
				{
					spm_updata_cur_number = 0xF8F8;//finish
					spm_updata_status = 2;
				}
				else
				{
					spm_updata_cur_number -= (spm_updata_cur_point / PROGRAM_PACKET_MAX_SIZE);
					spm_updata_cur_point = 0;
				}
			}
			else
			{
				memcpy(&spm_updata_buffer[spm_updata_cur_point],&buffer[4],(program_length - 2));
				spm_updata_cur_point += (program_length - 2);
				
				if (!iap_program_write_page(spm_updata_flash_addr,spm_updata_buffer,PAGE_SIZE))
				{
					spm_updata_cur_number = 0xF8F8;//finish
					spm_updata_status = 2;
				}
				else
				{
					spm_updata_cur_number -= (spm_updata_cur_point / PROGRAM_PACKET_MAX_SIZE);
					spm_updata_cur_point = 0;
				}
			}
		}
	}
#endif
	return 0;
}




static void iap_program_request(void)
{
	if (spm_updata_timeout)
	{
#ifdef IAP_PROGRAM_ENABLE	
		if (iap_program_signal())
		{	
			spm_updata_ipaddr = remote_ipaddr;
			spm_updata_port = remote_port;
			
			spm_updata_status = 1;
			return;
		}
		
#endif	
		// 写程序标志
		iap_program_upgrade_flag(1);
		while(1);
	}
	else
	{
		spm_updata_timeout = 20;
	}
}


//=======================Standard define API===============================
static int _get(unsigned char *config,int len)
{
	
	return 0;
}


static int _set(unsigned char *config,int len)
{
	//spm_updata_timeout = 0;
	spm_updata_status = 0;
	spm_updata_cur_number = 0;
	
	iap_program_request();
	return 0;
}

static void _init(void)
{
	system_reboot_init();
#ifdef IAP_PROGRAM_ENABLE		
	if (iap_program_signal())
	{	
			iap_program_bootloader();
	}
	spm_updata_status = 0;
	spm_updata_timeout = 0;	
	spm_updata_cur_number = 0;
	spm_updata_totol_number = 0;
	spm_updata_cur_point = 0;
	spm_updata_flash_addr = 0;
#endif
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
	iap_program_write(buffer,len);
	
	iap_program_packet();
	return 0;
}


static int _handler(void *event)
{
	static unsigned char curNumber = 0;//用于判断d当前升级的包
	static unsigned char curTimeout = 0;
	
	// reboot
	system_reboot_handler();
	
	
	switch (spm_updata_status)
	{
		case 0:
			if (spm_updata_timeout)
			{
				spm_updata_timeout--;
			}
			break;
			
#ifdef IAP_PROGRAM_ENABLE		
		case 1://正在升级
			spm_updata_timeout++;
			if (spm_updata_timeout > 100)//1s   //test 
			{
				spm_updata_timeout = 0;
				iap_program_packet();
				if(curNumber == spm_updata_cur_number)
				{
					curTimeout++;
					if(curTimeout > 60)
					{
						curTimeout = 0;
						curNumber = 0;
						spm_updata_status = 0;
						spm_updata_cur_number = 0;
						spm_updata_timeout = 0;
					}
				}
				else
				{
					curTimeout = 0;
					curNumber = spm_updata_cur_number;
				}
			}
			
			
			break;
		
		case 2://升级结束
			iap_program_upgrade_flag(0x55);
			system_reboot();
			spm_updata_status = 0;
			break;
#endif
		
		default:
			break;
	}
	return 0;
}

//====================app interface=====================
const struct app_interface_t iap_interface =
{
	_get, _set,_init,_status, _read, _write, _handler
};

#endif

