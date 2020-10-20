#include "netcrypt.h"
#include <string.h>
#include "sys_include.h"

extern unsigned long ticks;
#define NTimeMS()			ticks


//点对点发送到HDL-ON
struct P2P_iplist_t{
	unsigned char enable;
	unsigned int ip;
	unsigned char timeout;
};
static struct P2P_iplist_t P2P_iplist[5] = {0};


int netcrypt_aes_encrypt(unsigned char *key,unsigned int length,unsigned char *input,unsigned char *output)
{
	mbedtls_aes_context aes_ctx;
	unsigned char iv[16];
	int i,len,padsize = 0;
	
	if (length % 16) //长度为16的倍数
	{
		padsize = 16 - (length % 16); //多出来的字节
	}
	else
	{
		padsize = 16;
	}
	len = length + padsize;
	for (i = 0; i < padsize; i++)
	{
		input[length + i] = padsize; //42个数则后面的6个数为全补6
	}
	
	memcpy(iv,key,16); //复制密码
	
	mbedtls_aes_init( &aes_ctx );//初始化密码，清0
	mbedtls_aes_setkey_enc( &aes_ctx, key, 128);//
	mbedtls_aes_crypt_cbc(&aes_ctx, MBEDTLS_AES_ENCRYPT, len, iv, input, output);
	//mbedtls_aes_free( &aes_ctx );
	return len;
}

int netcrypt_aes_decrypt(unsigned char *key,unsigned int length,unsigned char *input,unsigned char *output)
{
	mbedtls_aes_context aes_ctx;
	unsigned char iv[16];
	int i,len,padsize = 0;
	unsigned char padlen;
	
	if (length % 16)
	{
		padsize = 16 - (length % 16);
	}
	
	len = length + padsize;
	
	for (i = 0; i < padsize; i++)
	{
		input[length + i] = padsize;
	}
	memcpy(iv,key,16);
	mbedtls_aes_init( &aes_ctx );
	mbedtls_aes_setkey_dec( &aes_ctx, key, 128);
	mbedtls_aes_crypt_cbc(&aes_ctx, MBEDTLS_AES_DECRYPT, len, iv, input, output);
//	mbedtls_aes_free( &aes_ctx );
	padlen = output[len-1];
	
	return (len - padlen);
}

extern unsigned char temp_buffer[1200];
extern unsigned char base_buffer[1200];
void netcrypt_aes_test(void)
{
//	unsigned int olen;
	unsigned int Len;
	unsigned char key[16] = "1234567812345678";
	unsigned char plain[64] = {0xc0, 0xa8, 0x0c, 0x78, 0x48, 0x44, 0x4c, 0x4d, 0x49, 0x52, 0x41, 0x43, 0x4c, 0x45, 0xaa, 0xaa, 0x0b, 0xfd, 0xfe, 0xff, 0xfe, 0x00, 0x0e, 0xff, 0xff, 0xa0, 0x0d};
	unsigned char dec_plain[64]={0};
	unsigned char cipher[64]={0};
//	unsigned char base_buf[200];
//	unsigned char temp_buf[200];
	
	Len = netcrypt_aes_encrypt(key,27,plain,cipher);
	memset(base_buffer,0,sizeof(base_buffer));
	memcpy(base_buffer, cipher, Len);
	
	netcrypt_aes_decrypt(key,Len,cipher,dec_plain);
	memset(temp_buffer, 0, sizeof(temp_buffer));
	memcpy(temp_buffer, dec_plain, Len);
	
	
}

// NETIP CRYPT
#ifdef NETCRYPT_ENABLE

#include "mbedtls/base64.h"
#include "MQTTLinux.h"

extern unsigned char netip_rx_buffer[];
extern unsigned char netip_tx_buffer[];
extern unsigned long netip_broadcast;

#ifdef IOT_ENABLE
extern unsigned char temp_buffer[1200];
extern unsigned char base_buffer[1200];

#else

unsigned char temp_buffer[1200];
unsigned char base_buffer[1200];
#endif

#define HEAD_MIRACLE								"HDLMIRACLE"

#define NETCRYPT_PORT								6000

struct netcrypt_t
{
	unsigned char enable;
	unsigned char key[16];
	int fd;
};

static char netcrypt_status = 0;
static struct netcrypt_t netcrypt_struct;

#define NETCRYPT_RX_BUFFER			netip_rx_buffer
#define NETCRYPT_TX_BUFFER			netip_tx_buffer
#define NETCRYPT_BROADCAST			netip_broadcast
//=======================================================
extern struct netip_t netip_gateway;


//处理清空HDL-ON IP问题  2020-07-01加
//============================================================
/***********************************************************
Name:		add_IP_to_hdlonlist
Function:	将IP添加到点对点列表中，需要判断Buspro协议的设备类型 
Parameter:	deviceTypeH:	设备类型高位
			deviceTypeL:	设备类型低位
			addr 	   :	收到消息的IP地址
Return:		
Other:		先找到是否有空IP的槽，然后再和已经有IP的槽比较IP是否相同
***********************************************************/
#if 0
static void add_IP_to_hdlonlist(unsigned char deviceTypeH, unsigned char deviceTypeL, unsigned int addr)
{
	int i = 0, j = 0, break_flag = 0;
	if(deviceTypeH != 0x75 || deviceTypeL != 0x31)
	{
		return;
	}
	
	for(i = 0; i < 5; i++)//记录IP
	{
		if(P2P_iplist[i].enable != 0)
		{
			continue;
		}
		for(j = 0; j < 5; j++)
		{
			if(P2P_iplist[j].enable != 1)
			{
				continue;
			}
			if(P2P_iplist[j].ip == addr)
			{
				break_flag = 1; 
				P2P_iplist[j].timeout = 60;//恢复时间
				break;
			}
		}
		if(break_flag == 0) 
		{
			P2P_iplist[i].ip = addr;
			P2P_iplist[i].enable = 1;
			P2P_iplist[i].timeout = 60;
		}
		break;
	}	
}
/***********************************************************
Name:		remove_IP_from_hdlonlist
Function:	从列表中将hdlonlist
Parameter:	
Return:		
Other:		1分钟内如果HDL-ON没有发送任何消息 从列表中去掉它
***********************************************************/
static void remove_IP_from_hdlonlist(void)
{
	int i = 0;
	for(i = 0; i < 5 && P2P_iplist[i].enable == 1; i++)//记录IP
	{
		P2P_iplist[i].timeout--;
		if(P2P_iplist[i].timeout == 0)
		{
			P2P_iplist[i].enable = 0;
			P2P_iplist[i].ip = 0;
		}
	}
}
#endif

static int netcrypt_set_key(unsigned char *key)
{
	struct netcrypt_t *netcrypt;
	netcrypt = &netcrypt_struct;
	
	eeprom_write(NETCRYPT_KEY_ADDR,key,17);
	if (key[0] == 1)
		netcrypt->enable  = 1;
	else
		netcrypt->enable  = 0;
	memcpy(netcrypt->key,&key[1],16);
	
	devices_button_set_program(0); //清空无密码模式
	
	return 1;
}

static int netcrypt_get_key(unsigned char *key, unsigned char len)
{
	struct netcrypt_t *netcrypt;
	netcrypt = &netcrypt_struct;
	unsigned char mac[9] = {0};
	if(len < 17) return 0;
	eeprom_read(NETCRYPT_KEY_ADDR,key,17);
	if (key[0] == 1)
	{
		netcrypt->enable  = 1;
	}
	else
	{
		key[0] = 0;
		netcrypt->enable  = 0;
	}
	memcpy(netcrypt->key,&key[1],16);
	if(len < 25) return 17;
	eeprom_read(EEPROM_MAC_BASIC_ADDR,mac,8);
	memcpy(&key[17], mac, 8);
	return 25;
}

// 加密方式
unsigned char netcrypt_is_enable(void)
{
	struct netcrypt_t *netcrypt;
	netcrypt = &netcrypt_struct;
	
	//按键模式下，是非加密
	if (devices_button_get_mode())
		return 0;
	
	if (netcrypt->enable)
		return 1;
	else
		return 0;
}

static int netcrypt_read(char *packet,int len, struct ip_addr *addr, u16_t port);
void netcrypt_low_recvfrom(void *arg, struct udp_pcb *upcb, struct pbuf *p,struct ip_addr *addr, u16_t port)
{
	netcrypt_read((char *)p->payload, p->len, addr, port);
}

static void netcrypt_init(void)
{
	unsigned char key[40] = {0};
	struct netcrypt_t *netcrypt;
	
	
	netcrypt = &netcrypt_struct;
	linux_close(0);
	netcrypt_get_key(key, 40);
	netcrypt->fd = linux_open(0,	"224.0.168.188", NETCRYPT_PORT,0,1);  //组播地址
	if (netcrypt->fd >= 0)
	{
		udp_recv(netsock[netcrypt->fd].upcb, netcrypt_low_recvfrom, NULL);
	}

}


const unsigned char Bus_search[] = {0x00, 0x0E};//6 7
const unsigned char Bus_E548[] = {0xE5, 0x48};//6 7
const unsigned char Bus_E54A[] = {0xE5, 0x4A};//6 7
static int netcrypt_read(char *packet,int len, struct ip_addr *ip_addr, u16_t port)
{
	unsigned char *frame_buffer;
	unsigned int ret_num = 0;
	struct netcrypt_t *netcrypt;
	static unsigned long wwul1 = 0;
	netcrypt = &netcrypt_struct;
//	unsigned char flags = 0;
	if(len < 26) return 1;
	
	if (!memcmp((unsigned char *)(&packet[4]),(unsigned char *)HEAD_MIRACLE,10) && netcrypt_is_enable() ) //当前为明文发送；加密
	{
//		if(!memcmp((unsigned char *)(&packet[14]),(unsigned char *)"Password error",14) || !memcmp((unsigned char *)(&packet[14]),(unsigned char *)"Locked",6))
//		{
//			return 0;
//		}
		if(abs(NTimeMS() - wwul1) < 1000 && wwul1 != 0)//相差1s内
		{
			return 0;
		}
		wwul1 = NTimeMS();
		
		
		if(!memcmp((unsigned char *)(&packet[21]),(unsigned char *)Bus_search,2)
			|| !memcmp((unsigned char *)(&packet[21]),(unsigned char *)Bus_E548,2)
			|| !memcmp((unsigned char *)(&packet[21]),(unsigned char *)Bus_E54A,2))
		{
			memcpy((unsigned char *)(&netip_tx_buffer[0]),netip_gateway.addr.b,4);
			memcpy((unsigned char *)(&netip_tx_buffer[4]),(unsigned char *)HEAD_MIRACLE,10);
			memcpy(&netip_tx_buffer[14], "Locked", 6);
			linux_sendto(netcrypt->fd, NETCRYPT_BROADCAST, NETCRYPT_PORT, netip_tx_buffer, 20);
		}
		
		memcpy((unsigned char *)(&netip_tx_buffer[0]),netip_gateway.addr.b,4);
		memcpy((unsigned char *)(&netip_tx_buffer[4]),(unsigned char *)HEAD_MIRACLE,10);
		memcpy(&netip_tx_buffer[14], "Locked", 6);
		linux_sendto(netcrypt->fd, NETCRYPT_BROADCAST, NETCRYPT_PORT, netip_tx_buffer, 20);
		return 0;
	}
	else if(!memcmp((unsigned char *)(&packet[4]),(unsigned char *)HEAD_MIRACLE,10) && !netcrypt_is_enable() )//明文 不加密
	{
		frame_buffer = (unsigned char *)packet; //原数据
		if (frame_buffer[14] == 0xAA && frame_buffer[15] == 0xAA)
		{
		#if 0
			add_IP_to_hdlonlist(frame_buffer[19], frame_buffer[20], ip_addr->addr);
		#endif
			netip_commands_handler(1,&frame_buffer[16],len - 16);
		}
		return 0;
	}
	else if(memcmp((unsigned char *)(&packet[4]),(unsigned char *)HEAD_MIRACLE,10) && netcrypt_is_enable())  //密文 加密
	{
		ret_num = netcrypt_aes_decrypt(netcrypt->key,len,(unsigned char *)packet, netip_rx_buffer);
		frame_buffer = netip_rx_buffer;
		if(memcmp((unsigned char *)(&frame_buffer[4]),(unsigned char *)HEAD_MIRACLE,10))//密码不正确
		{
			memcpy((unsigned char *)(&netip_tx_buffer[0]),netip_gateway.addr.b,4);
			memcpy((unsigned char *)(&netip_tx_buffer[4]),(unsigned char *)HEAD_MIRACLE,10);
			memcpy(&netip_tx_buffer[14], "Password error", 14);
			linux_sendto(netcrypt->fd, NETCRYPT_BROADCAST, NETCRYPT_PORT, netip_tx_buffer, 28);
			return 0;
		}
		if (frame_buffer[14] == 0xAA && frame_buffer[15] == 0xAA)
		{
		#if 0
			add_IP_to_hdlonlist(frame_buffer[19], frame_buffer[20], ip_addr->addr);
		#endif
			netip_commands_handler(1,&frame_buffer[16],ret_num - 16);
			return 0;
		}
	}
	else if(memcmp((unsigned char *)(&packet[4]),(unsigned char *)HEAD_MIRACLE,10) && !netcrypt_is_enable())  //密文 不加密
	{
//		memcpy((unsigned char *)(&netip_tx_buffer[0]),netip_gateway.addr.b,4);
//		memcpy((unsigned char *)(&netip_tx_buffer[4]),(unsigned char *)HEAD_MIRACLE,10);
//		memcpy(&netip_tx_buffer[14], "Unlocked", 8);
//		linux_sendto(netcrypt->fd, NETCRYPT_BROADCAST, NETCRYPT_PORT, netip_tx_buffer, 20);
		return 0;
	}
	return 0;
}
//2020-07-01改 转发添加HDL-ON IP
static int netcrypt_write(char *packet,int len)
{
	int alen;
	struct netcrypt_t *netcrypt;
	netcrypt = &netcrypt_struct;
	unsigned char *send_buffer = NULL;
	
	memset(netip_tx_buffer, 0, 1200);
	memcpy((unsigned char *)(&netip_tx_buffer[0]),netip_gateway.addr.b,4);
	memcpy((unsigned char *)(&netip_tx_buffer[4]),(unsigned char *)HEAD_MIRACLE,10);
	
	netip_tx_buffer[14] = 0xAA;
	netip_tx_buffer[15] = 0xAA;
	
	memcpy(&netip_tx_buffer[16],(unsigned char *)packet,len);

	alen = len + 16;
	if (netcrypt_is_enable()) // 加密方式
	{
		alen = netcrypt_aes_encrypt(netcrypt->key,alen,netip_tx_buffer, base_buffer);
		send_buffer = base_buffer;
	}
	else
	{
		send_buffer = netip_tx_buffer;
	
	}
#if 0
	int i = 0;
	for(i = 0; i < 5; i++)//发送对应的数据到
	{
		if(P2P_iplist[i].enable == 0)
		{
			continue;
		}
		linux_sendto(netcrypt->fd,P2P_iplist[i].ip,NETCRYPT_PORT,send_buffer,alen);
	}
#endif
	return linux_sendto(netcrypt->fd,NETCRYPT_BROADCAST,NETCRYPT_PORT,send_buffer,alen);
}
int len_t = 1;
void netcrypt_handler(void)
{
#if 0
	int timeout = 10;
	
	struct sockaddr_in from;
	struct netcrypt_t *netcrypt;
	netcrypt = &netcrypt_struct;
	int flen;
	len_t = 1;
	while (timeout && (len_t > 0))
	{
		len_t = linux_recvfrom(netcrypt->fd, (char *)NETCRYPT_RX_BUFFER,1200,(struct sockaddr *)&from, &flen);
		timeout--;
		
		if (len_t > 26) 
		{
		//	linux_sendto(netcrypt->fd,NETCRYPT_BROADCAST,NETCRYPT_PORT,NETCRYPT_RX_BUFFER,len);
			
			netcrypt_read((char *)NETCRYPT_RX_BUFFER,len_t);
		}
	}
#endif
}


int netcrypt_send(unsigned int cmd,unsigned char src_subnet,\
					unsigned char src_device,unsigned char dest_subnet,\
					unsigned char dest_device,unsigned char *buffer,int len)
{
	int alen;
	unsigned int crc;
	unsigned int dev_type = netip_get_type();
	
	if (len > 67) return 1;
	
	struct netcrypt_t *netcrypt;
	netcrypt = &netcrypt_struct;
	
	memset(netip_tx_buffer, 0, 1200);
	memcpy((unsigned char *)(&netip_tx_buffer[0]),netip_gateway.addr.b,4);
	memcpy((unsigned char *)(&netip_tx_buffer[4]),(unsigned char *)HEAD_MIRACLE,10);

	netip_tx_buffer[14] = 0xAA;
	netip_tx_buffer[15] = 0xAA;
	netip_tx_buffer[16] = len + 11; 
	netip_tx_buffer[17] = src_subnet;  
	netip_tx_buffer[18] = src_device;   
	netip_tx_buffer[19] = (unsigned char)(dev_type >> 8);     
	netip_tx_buffer[20] = (unsigned char)(dev_type & 0X00FF); 
	netip_tx_buffer[21] = (unsigned char)(cmd >> 8);
	netip_tx_buffer[22] = (unsigned char)(cmd & 0x00FF);
	netip_tx_buffer[23] = dest_subnet;
	netip_tx_buffer[24] = dest_device;

	memcpy((unsigned char *)&(netip_tx_buffer[25]),buffer,len);

	crc = CRC16_XMODEM((unsigned char *)&netip_tx_buffer[16],netip_tx_buffer[16]-2);
	netip_tx_buffer[len + 25] = (unsigned char)(crc >> 8);
	netip_tx_buffer[len + 26] = (unsigned char)(crc);
	
	alen = len + 27;
	if (netcrypt_is_enable()) // 加密方式
	{
		alen = netcrypt_aes_encrypt(netcrypt->key,alen,netip_tx_buffer, base_buffer);
		return linux_sendto(netcrypt->fd,NETCRYPT_BROADCAST,NETCRYPT_PORT,base_buffer,alen);
	}
	else
	{
		return linux_sendto(netcrypt->fd,NETCRYPT_BROADCAST,NETCRYPT_PORT,netip_tx_buffer,alen);
	}
}




int netcrypt_long_send(unsigned int cmd,unsigned char src_subnet,\
												unsigned char src_device,unsigned char dest_subnet,\
												unsigned char dest_device,unsigned char *buffer,int len)
{
	int alen;
	unsigned int dev_type = netip_get_type();
	struct sockaddr_in ipaddr_temp;
	if (len > 1200) return 1;
	
	struct netcrypt_t *netcrypt;
	netcrypt = &netcrypt_struct;
	
	memset(netip_tx_buffer, 0, 1200);
	memcpy((unsigned char *)(&netip_tx_buffer[0]),netip_gateway.addr.b,4);
	memcpy((unsigned char *)(&netip_tx_buffer[4]),(unsigned char *)HEAD_MIRACLE,10);

	netip_tx_buffer[14] = 0xAA;
	netip_tx_buffer[15] = 0xAA;
	netip_tx_buffer[16] = 0xFF; 
	netip_tx_buffer[17] = src_subnet;  
	netip_tx_buffer[18] = src_device;   
	netip_tx_buffer[19] = (unsigned char)(dev_type >> 8);     
	netip_tx_buffer[20] = (unsigned char)(dev_type & 0X00FF); 
	netip_tx_buffer[21] = (unsigned char)(cmd >> 8);
	netip_tx_buffer[22] = (unsigned char)(cmd & 0x00FF);
	netip_tx_buffer[23] = dest_subnet;
	netip_tx_buffer[24] = dest_device;

	netip_tx_buffer[25] = (unsigned char)(len >> 8);
	netip_tx_buffer[26] = (unsigned char)(len & 0x00FF);
	
	memcpy(&netip_tx_buffer[27],buffer,len);
	
	// 
	alen = len + 27;
	if (netcrypt_is_enable()) // 加密方式
	{
		alen = netcrypt_aes_encrypt(netcrypt->key,alen,netip_tx_buffer, base_buffer);
		if (cmd == READ_PROGRAM_REMARK_ACK
			|| cmd == WRITE_PROGRAM_REMARK_ACK)
		{
			linux_sendto(netcrypt->fd,0xffffffff,NETCRYPT_PORT,base_buffer,alen);
			ipaddr_temp.sin_addr.s_addr = inet_addr("224.0.168.188");
			linux_sendto(netcrypt->fd,ipaddr_temp.sin_addr.s_addr,NETCRYPT_PORT,base_buffer,alen);
		}
		return linux_sendto(netcrypt->fd,NETCRYPT_BROADCAST,NETCRYPT_PORT,base_buffer,alen);
	}
	else
	{
		if (cmd == READ_PROGRAM_REMARK_ACK
			|| cmd == WRITE_PROGRAM_REMARK_ACK)
		{
			
			linux_sendto(netcrypt->fd,0xffffffff,NETCRYPT_PORT,netip_tx_buffer,alen);
			ipaddr_temp.sin_addr.s_addr = inet_addr("224.0.168.188");
			linux_sendto(netcrypt->fd,ipaddr_temp.sin_addr.s_addr,NETCRYPT_PORT,netip_tx_buffer,alen);
		}
		return linux_sendto(netcrypt->fd,NETCRYPT_BROADCAST,NETCRYPT_PORT,netip_tx_buffer,alen);
	}
}


int netcrypt_allbroadcast_send(unsigned int cmd,unsigned char src_subnet,unsigned char src_device, \
											unsigned char dest_subnet,unsigned char dest_device, \
											unsigned char *buffer,unsigned char len)
{
	int alen;
	unsigned int crc;
	unsigned int dev_type = netip_get_type();
	
	if (len > 67) return 1;
	
	struct netcrypt_t *netcrypt;
	netcrypt = &netcrypt_struct;
	
	memset(netip_tx_buffer, 0, 1200);
	memcpy((unsigned char *)(&netip_tx_buffer[0]),netip_gateway.addr.b,4);
	memcpy((unsigned char *)(&netip_tx_buffer[4]),(unsigned char *)HEAD_MIRACLE,10);

	netip_tx_buffer[14] = 0xAA;
	netip_tx_buffer[15] = 0xAA;
	netip_tx_buffer[16] = len + 11; 
	netip_tx_buffer[17] = src_subnet;  
	netip_tx_buffer[18] = src_device;   
	netip_tx_buffer[19] = (unsigned char)(dev_type >> 8);     
	netip_tx_buffer[20] = (unsigned char)(dev_type & 0X00FF); 
	netip_tx_buffer[21] = (unsigned char)(cmd >> 8);
	netip_tx_buffer[22] = (unsigned char)(cmd & 0x00FF);
	netip_tx_buffer[23] = dest_subnet;
	netip_tx_buffer[24] = dest_device;

	memcpy((unsigned char *)&(netip_tx_buffer[25]),buffer,len);

	crc = CRC16_XMODEM((unsigned char *)&netip_tx_buffer[16],netip_tx_buffer[16]-2);
	netip_tx_buffer[len + 25] = (unsigned char)(crc >> 8);
	netip_tx_buffer[len + 26] = (unsigned char)(crc);
	
	alen = len + 27;
	if (netcrypt_is_enable()) // 加密方式
	{
		alen = netcrypt_aes_encrypt(netcrypt->key,alen,netip_tx_buffer, base_buffer);
		return linux_sendto(netcrypt->fd,0xffffffff,NETCRYPT_PORT,base_buffer,alen);
	}
	else
	{
		return linux_sendto(netcrypt->fd,0xffffffff,NETCRYPT_PORT,netip_tx_buffer,alen);
	}
}

int netcrypt_multi_send(unsigned int cmd,unsigned char src_subnet,unsigned char src_device, \
											unsigned char dest_subnet,unsigned char dest_device, \
											unsigned char *buffer,unsigned char len)
{
	int alen;
	unsigned int crc;
	unsigned int dev_type = netip_get_type();
	struct sockaddr_in ipaddr_temp;
	
	if (len > 67) return 1;
	
	struct netcrypt_t *netcrypt;
	netcrypt = &netcrypt_struct;
	
	memset(netip_tx_buffer, 0, 1200);
	memcpy((unsigned char *)(&netip_tx_buffer[0]),netip_gateway.addr.b,4);
	memcpy((unsigned char *)(&netip_tx_buffer[4]),(unsigned char *)HEAD_MIRACLE,10);

	netip_tx_buffer[14] = 0xAA;
	netip_tx_buffer[15] = 0xAA;
	netip_tx_buffer[16] = len + 11; 
	netip_tx_buffer[17] = src_subnet;  
	netip_tx_buffer[18] = src_device;   
	netip_tx_buffer[19] = (unsigned char)(dev_type >> 8);     
	netip_tx_buffer[20] = (unsigned char)(dev_type & 0X00FF); 
	netip_tx_buffer[21] = (unsigned char)(cmd >> 8);
	netip_tx_buffer[22] = (unsigned char)(cmd & 0x00FF);
	netip_tx_buffer[23] = dest_subnet;
	netip_tx_buffer[24] = dest_device;

	memcpy((unsigned char *)&(netip_tx_buffer[25]),buffer,len);

	crc = CRC16_XMODEM((unsigned char *)&netip_tx_buffer[16],netip_tx_buffer[16]-2);
	netip_tx_buffer[len + 25] = (unsigned char)(crc >> 8);
	netip_tx_buffer[len + 26] = (unsigned char)(crc);
	
	alen = len + 27;
	ipaddr_temp.sin_addr.s_addr = inet_addr("224.0.168.188");
	
	if (netcrypt_is_enable()) // 加密方式
	{
		alen = netcrypt_aes_encrypt(netcrypt->key,alen,netip_tx_buffer, base_buffer);
		return linux_sendto(netcrypt->fd,ipaddr_temp.sin_addr.s_addr,NETCRYPT_PORT,base_buffer,alen);
	}
	else
	{
		return linux_sendto(netcrypt->fd,ipaddr_temp.sin_addr.s_addr,NETCRYPT_PORT,netip_tx_buffer,alen);
	}
}



//=======================Standard define API===============================
static int _get(unsigned char *config,int len)
{
	unsigned char return_num = 0; 
	return_num = netcrypt_get_key(config, len);
	
	return return_num;
}


static int _set(unsigned char *config,int len)
{
	unsigned char return_num = 0; 
	return_num = netcrypt_set_key(config);
	return return_num;
}

static void _init(void)
{
	netcrypt_status = 0;
}

static int _status(unsigned char *buffer,int len)
{

	return 0;
}

static int _read(unsigned char *buffer,int len)
{
	return 1;
	//return netcrypt_read((char *)buffer,len);
}


static int _write(unsigned char *buffer,int len)
{
	
	return netcrypt_write((char *)buffer,len);
}


static int _handler(void *event)
{
	static unsigned char mutex = 0;
	if(mutex == 1) return 0;
	mutex = 1;	
	switch (netcrypt_status)
	{
		case 0:
			netcrypt_init();
			netcrypt_status++;
			break;
		
		case 1:
			netcrypt_handler();
			break;
	}
	mutex = 0;
	return 0;
}
#if 0
/***********************************************************
Name:		netcrypt_tenMs_handler
Function:	局域网通信 10ms处理函数 IP老化
Parameter:	
Return:		
***********************************************************/
void netcrypt_tenMs_handler(void)
{
	static unsigned int timeout = 0;
	timeout++;
	if(timeout < 100) return;
	timeout = 0;	
	
	
	remove_IP_from_hdlonlist();

}
#endif

//====================app interface=====================
const struct app_interface_t netcrypt_interface =
{
	_get, _set,_init, _status, _read, _write, _handler
};


#endif


