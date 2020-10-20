#include "gateway.h"
#include "sys_include.h"

#ifdef NETIP_GATEWAY_ENABLE

#define UDP_SERVER_PORT						6000

#define REMOTE_MAX_SIZE           5

#define HDL_MQTT_ENABLE


// reboot
#define NETIP_GATEWAY_REBOOT()				system_reboot()

#define NETIP_P2P_SEND								netip_gateway_dosend//	netip_ptp_send
#define NETIP_P2P_LONGSEND								netip_gateway_dolongsend//	netip_ptp_send
#define NETIP_P2P_WRITE								netip_gateway_dowrite//	netip_ptp_write
#define NETIP_P2P_RECV								netip_commands_handler

extern unsigned long ticks;
#define NTimeMS()			ticks

#define IPaddr 		UIPaddr
// server
struct netip_server_t
{
	UIPaddr ipaddr;
	unsigned short port;
};


// geteway
struct netip_device_t
{
	unsigned char point_hour;
	unsigned char point_minute;
	
	// project 
	unsigned char type;
	unsigned char group[20];
	unsigned char name[20];
	unsigned char user[8];
	unsigned char passwd[8];
	

};

// remote
struct netip_remote_t
{
	unsigned short enable:1;
	unsigned short timeout:15;
	struct netip_server_t addr;
};

struct netip_domain_t
{
	unsigned char enable;
	UIPaddr ipaddr;
	unsigned char name[65];
	unsigned char update_domainname;
};

struct netip_device_t device_local;


static struct netip_server_t clondServer[2];
static struct netip_remote_t remoteClient[REMOTE_MAX_SIZE];

static struct netip_domain_t domainClient;

struct netip_t netip_gateway;

int netip_gateway_doit(unsigned int ipaddr,unsigned short port,unsigned char *buffer,int len);
int netip_gateway_dowrite(unsigned int ipaddr,unsigned short port,\
												unsigned char *packet,int len);
int netip_gateway_dosend(unsigned int ipaddr,unsigned short port,\
									unsigned int cmd,unsigned char src_subnet,unsigned char src_device, \
											unsigned char dest_subnet,unsigned char dest_device, \
											unsigned char *buffer,unsigned char len);
void netip_gateway_setup_history(void);
void netip_gateway_setup_history(void);
//yk 2019-12-11加
int netip_gateway_set_dhcp(unsigned char dhcp);
											
char HDL_MQTT_USE_MAC[17] ={0};  //用于mqtt
char MQTT_MAC_UUID[37];	
											
extern char netip_tx_buffer[1500];
char netip_rx_buffer[1500];
#ifdef IOT_ENABLE
extern unsigned char Home_ID[37];
extern unsigned char Account[37];
extern unsigned char MQTT_Pname[21];
extern unsigned char MQTT_Gname[21];
extern unsigned char MQTT_user[9];
#endif

//=====================================================================
// test
//#define NETIP_GATEWAY_DEBUG_ENABLE
#define DNS_HDL_SERVER				"bus.hdlcontrol.com"

#ifdef NETIP_GATEWAY_DEBUG_ENABLE
void netip_gateway_debug(void)
{
	struct netip_t *gateway;
	gateway = &netip_gateway;
	
	gateway->dhcp = 0;
	
	strcpy((char *)domainClient.name,"bus.hdlcontrol.com");
	domainClient.enable = 1;
	
	memset((char *)device_local.group,0,sizeof(device_local.group));
	memset((char *)device_local.name,0,sizeof(device_local.name));
	memset((char *)device_local.user,0,sizeof(device_local.user));
	memset((char *)device_local.passwd,0,sizeof(device_local.passwd));

	strcpy((char *)device_local.group,"HDL_TEST");
	strcpy((char *)device_local.name,"HDL-STEVEN");
	strcpy((char *)device_local.user,"user");
	strcpy((char *)device_local.passwd,"12345678");
	device_local.type = 1;
}

#endif


#include "MQTTLinux.h"

#define HEAD_MIRACLE					"HDLMIRACLE"

static int netip_gateway_socket;
static int netip_socket_timeout;
static char netip_gateway_status = 0;
static int local_port = 6030;

int netip_gateway_dosend(unsigned int ipaddr,unsigned short port,\
									unsigned int cmd,unsigned char src_subnet,unsigned char src_device, \
											unsigned char dest_subnet,unsigned char dest_device, \
											unsigned char *buffer,unsigned char len)
{
	unsigned int crc;
	unsigned int type = netip_get_type();
	
	if (len > 67) return 1;

	memcpy((unsigned char *)(&netip_tx_buffer[0]),netip_gateway.addr.b,4);
	memcpy((unsigned char *)(&netip_tx_buffer[4]),(unsigned char *)HEAD_MIRACLE,10);

	netip_tx_buffer[14] = 0xAA;
	netip_tx_buffer[15] = 0xAA;
	netip_tx_buffer[16] = len + 11; 
	netip_tx_buffer[17] = src_subnet;  
	netip_tx_buffer[18] = src_device;   
	netip_tx_buffer[19] = (unsigned char)(type >> 8);     
	netip_tx_buffer[20] = (unsigned char)(type); 
	netip_tx_buffer[21] = (unsigned char)(cmd >> 8);
	netip_tx_buffer[22] = (unsigned char)(cmd);
	netip_tx_buffer[23] = dest_subnet;
	netip_tx_buffer[24] = dest_device;

	memcpy((unsigned char *)&(netip_tx_buffer[25]),buffer,len); //要发送的消息

	crc = CRC16_XMODEM((unsigned char *)&netip_tx_buffer[16],netip_tx_buffer[16]-2);
	netip_tx_buffer[len + 25] = (unsigned char)(crc >> 8);
	netip_tx_buffer[len + 26] = (unsigned char)(crc);
	
	return netip_gateway_doit(ipaddr,port,(unsigned char *)netip_tx_buffer,len + 27);
	
}

int netip_gateway_dolongsend(unsigned int ipaddr,unsigned short port,\
									unsigned int cmd,unsigned char src_subnet,unsigned char src_device, \
											unsigned char dest_subnet,unsigned char dest_device, \
											unsigned char *buffer,unsigned int len)
{
	unsigned int dev_type = netip_get_type();
	
	if (len > 1200) return 1;
	
	
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
	
	return netip_gateway_doit(ipaddr,port,(unsigned char *)netip_tx_buffer,len + 27);
}



int netip_gateway_dowrite(unsigned int ipaddr,unsigned short port,\
												unsigned char *packet,int len)
{
	memcpy((unsigned char *)(&netip_tx_buffer[0]),netip_gateway.addr.b,4);
	memcpy((unsigned char *)(&netip_tx_buffer[4]),(unsigned char *)HEAD_MIRACLE,10);
	
	netip_tx_buffer[14] = 0xAA;
	netip_tx_buffer[15] = 0xAA;
	memcpy((unsigned char *)(&netip_tx_buffer[16]),(unsigned char *)packet,len);

	return netip_gateway_doit(ipaddr,port,(unsigned char *)netip_tx_buffer,len + 27);
}

void netip_socket_timeout_handler(void)
{
	netip_socket_timeout++;
	if (netip_socket_timeout > 60)
	{
		netip_socket_timeout = 0;
		netip_gateway_status = 0;
	}
}

void netip_gateway_low_recvfrom(void *arg, struct udp_pcb *upcb, struct pbuf *p,struct ip_addr *addr, u16_t port)
//void netip_gateway_low_cb(uint32_t ipaddr,uint16_t port,uint8_t *buffer,int len)
{
	unsigned char *pbuffer;
	
	if (device_local.type == 0) return;
	if (device_local.type == GATEWAY_LINK_SERVER || device_local.type == GATEWAY_LINK_DNS)
	{
		if (p->len > 26)
		{
			pbuffer = p->payload;
			
			netip_socket_timeout = 0;
			remote_ipaddr = addr->addr;
			remote_port = port;
			netip_gateway_remote_link(remote_ipaddr,remote_port);
			NETIP_P2P_RECV(100,&pbuffer[16],p->len-16);
		}
	}
}




int netip_gateway_doit(unsigned int ipaddr,unsigned short port,unsigned char *buffer,int len)
{
	switch (netip_gateway_status)
	{
		case 0:
			netip_socket_timeout = 0;
			if (local_port == 6030)
			{
				local_port = 6030 + ticks % 100;
			}
			else
			{
				linux_close(netip_gateway_socket);
			}
			
			local_port++;
			if (local_port > 60000)
			{
				local_port = 6030;
			}				
			
			netip_gateway_socket = linux_open(0,"192.168.10.250",local_port,1,0);
			if (netip_gateway_socket >= 0)
			{
				udp_recv(netsock[netip_gateway_socket].upcb, netip_gateway_low_recvfrom, NULL);
				netip_gateway_status++;
			}
		case 1:
			linux_sendto(netip_gateway_socket, ipaddr,port, buffer, len);
			break;
		
		default:
			netip_gateway_status = 0;
			break;
	}
	return 0;
}


void netip_gateway_low_handler(void)
{
	#if 0
	int timeout = 10;
	int len = 1;
	struct sockaddr_in client_addr;
	int addr_len = sizeof(struct sockaddr_in);
	
	if (device_local.type == 0) return;
	if (device_local.type == GATEWAY_LINK_SERVER || device_local.type == GATEWAY_LINK_DNS)
	{
		while (len > 0 && timeout--)
		{
			len = linux_recvfrom(netip_gateway_socket,netip_rx_buffer, 1200,\
										(struct sockaddr *)&client_addr, &addr_len);
			
			if (len > 26)
			{
				netip_socket_timeout = 0;
				remote_ipaddr = client_addr.sin_addr.s_addr;
				remote_port = htons(client_addr.sin_port);
				netip_gateway_remote_link(remote_ipaddr,remote_port);
				NETIP_P2P_RECV(100,(unsigned char *)&netip_rx_buffer[16],len-16);
			}
			else if (len <= 0)
			{
				break;
			}
		}	
	}
	#endif
}





int netip_is_submark(unsigned char * array)
{
	unsigned int submark;
	int i;
	int status = 0;

	submark = array[0]<<24;
	submark |= array[1]<<16;
	submark |= array[2]<<8;
	submark |= array[3];//ffffF800

	for (i = 31; i >= 0; i--)
	{
		if (submark & (1 << i))
		{
			if(status)
				return 1;
		}
		else
		{
			status = 1;
		}
	}

	return 0;
}



/*******************************设定网关模式************************************************/
void netip_MQTT_Set_Status(void) //是否开启MQTT
{
	unsigned char buffer[4];
	struct netip_t *gateway = &netip_gateway;
#ifdef IOT_ENABLE
	if(device_local.type == MQTT_LINK_SERVER || device_local.type == STANDAR_MQTT_LINK_SERVER)
	{
		if(gateway->dhcp != 1)
		{
			memset(buffer,0,4);
			buffer[0] = 114;
			buffer[1] = 114;
			buffer[2] = 114;
			buffer[3] = 114;
			//memcpy(gateway->dns1_addr.b,buffer,4);
			//buffer[3] = 166;
			memcpy(gateway->dns2_addr.b,buffer,4);
		}
		if(device_local.type == MQTT_LINK_SERVER)
		{
			iot_config_interface.set_status(1); //将MQTT的标志位置位1 ，开启HDL MQTT
		}
		else
		{
			iot_config_interface.set_status(2); //将MQTT的标志位置位2，开启标准MQTT
		}
	}
	else
	{
		iot_config_interface.set_status(0); //将MQTT的标志位置位1 ，关闭MQTT
	}
#endif
	
}

//yk 2019-12-11加
extern int netip_gateway_set_type(unsigned char type);

void netip_gateway_init(void)
{
	struct netip_t *gateway;
	unsigned char buffer[60];

	// getway IP Address yk 2020-05-28改
	eeprom_read(IPADDR_CONFIG_BASIC_ADDRESS,buffer,19);
	gateway = &netip_gateway;

	memcpy(gateway->addr.b,buffer,4);
	memcpy(gateway->gw.b,&buffer[4],4);
	memcpy(gateway->mac,&buffer[8],6);
	
#if 1
	memcpy(gateway->submask.b,&buffer[14],4);
	gateway->dhcp = buffer[18];
	if (gateway->dhcp != 1)
	{
		gateway->dhcp = 0;
	}
	gateway->status = 0;
	eeprom_read(DNS_SERVER_IPADDR_ADDR, buffer, 9);
	// dns
	gateway->dns = buffer[0];
	if (gateway->dns != 1 || gateway->dhcp)
	gateway->dns = 0;
	memcpy(gateway->dns1_addr.b,&buffer[1],4);
	memcpy(gateway->dns2_addr.b,&buffer[5],4);
	
#else
	memcpy(gateway->submask.b,&buffer[16],4);

	gateway->dhcp = buffer[20];
	if (gateway->dhcp != 1)
	{
		gateway->dhcp = 0;
	}
	
	gateway->status = 0;

	// dns
	gateway->dns = buffer[22];
	if (gateway->dns != 1 || gateway->dhcp)
	gateway->dns = 0;

	memcpy(gateway->dns1_addr.b,&buffer[23],4);
	memcpy(gateway->dns2_addr.b,&buffer[27],4);	
#endif
	// set server
	//if (gateway->dns == 0)
	//	dns_setserver((char *)gateway->dns1_addr.b,(char *)gateway->dns2_addr.b);
	//dns_setserver((char *)gateway->dns1_addr.b,(char *)gateway->dns2_addr.b);
	//dns_setserver((char *)gateway->dns1_addr.b,(char *)gateway->dns2_addr.b);
	
	// check
	if ((gateway->addr.l == 0xFFFFFFFF) || (gateway->addr.l == 0)
										|| (gateway->gw.l == 0xFFFFFFFF) || (gateway->gw.l == 0)
										|| (gateway->submask.l == 0xFFFFFFFF) || (gateway->submask.l == 0)
										|| (netip_is_submark(gateway->submask.b)))
	{
		gateway->addr.b[0] = 192;
		gateway->addr.b[1] = 168;
		gateway->addr.b[2] = 10;
		gateway->addr.b[3] = 250; //test

		gateway->gw.b[0] = 192;
		gateway->gw.b[1] = 168;
		gateway->gw.b[2] = 10;
		gateway->gw.b[3] = 1;

		gateway->submask.b[0] = 255;
		gateway->submask.b[1] = 255;
		gateway->submask.b[2] = 255;
		gateway->submask.b[3] = 0;
		gateway->dhcp = 0;
		
		
		gateway->dns2_addr.l = 0x72727272;
	}

	// project name
	eeprom_read(PROJECT_CONFIG_BASIC_ADDRESS,buffer,60);
	device_local.type = buffer[0];
	if (device_local.type >= GATEWAY_LINK_MAX_NUM)
	{
		device_local.type = 0;
	}
	
	// get domain name
	if(device_local.type == GATEWAY_LINK_DNS)
	{
		eeprom_read(DOMAINNAME_CONFIG_BASIC_ADDRESS, domainClient.name, 65);
		if (domainClient.name[64] != 0x55)
		{
			memset(domainClient.name,0,64);
			domainClient.enable = 0;
		}
		else
		{
			domainClient.enable = 1;
		}
	}

	memcpy(device_local.group,&buffer[1],20);
	memcpy(device_local.name,&buffer[21],20);
	strncpy((char *)device_local.user,(char *)&buffer[41],8);
	strncpy((char *)device_local.passwd,(char *)&buffer[49],8);

	// clondServer
	eeprom_read(SERVER_CONFIG_BASIC_ADDRESS,buffer,15);
	memcpy(clondServer[0].ipaddr.b,buffer,4);
	clondServer[0].port = buffer[4];
	clondServer[0].port <<= 8;
	clondServer[0].port += buffer[5];

	memcpy(clondServer[1].ipaddr.b,&buffer[6],4);
	clondServer[1].port = buffer[10];
	clondServer[1].port <<= 8;
	clondServer[1].port += buffer[11];

	// hour area
	device_local.point_hour = buffer[13];
	device_local.point_minute = buffer[14];
	if (device_local.point_minute > 60)
	{
		device_local.point_minute = 0;
	}
	//remote
	memset((char *)remoteClient,0,(sizeof(struct netip_remote_t) * REMOTE_MAX_SIZE));

	//history
	netip_gateway_setup_history();

	#ifdef NETIP_GATEWAY_DEBUG_ENABLE	
	netip_gateway_debug();
	#endif	
	// netip setup
//yk 2019-12-11加 
	memset(buffer, 0, 60);
#ifdef IOT_ENABLE
	netip_MQTT_Set_Status();
#endif

//	gateway->dhcp = 1;
	netip_setup(gateway);
	
}
/*******************************初始化网关 end line************************************************/


/********************获取当前网关的类型 本地 远程 或者MQTT**********************/
int netip_gateway_type(void)
{
	return device_local.type;
}
int netip_gateway_set_type(unsigned char type)
{
	unsigned char buffer = 0;
	buffer = type; 
	device_local.type = type;
	return eeprom_write(PROJECT_CONFIG_BASIC_ADDRESS,&buffer,1);
}
//yk 2019-12-11加
int netip_gateway_set_dhcp(unsigned char dhcp)
{
	if(dhcp != 1)
	{
		dhcp = 0;
	}
	eeprom_write(IPADDR_CONFIG_BASIC_ADDRESS + 18, &dhcp, 1);
	return dhcp;
}


/********************获取当前网关的类型 end line**********************/

/***********************************************************
Name: 		netip_gateway_send
Function:	网关主动发送
Parameter:	cmd:命令 subnet:子网id device:设备id buffer:发送内容 size:发送长度
return:		0:远程服务器返回 1:本地返回 2 MQTT返回
***********************************************************/
int netip_gateway_send(unsigned int cmd,unsigned char subnet,\
					unsigned char device,unsigned char *buffer,unsigned char size)
{
	int i;
	struct netip_remote_t *remote;
	
	if (device_local.type == 0) return 1;
#ifdef IOT_ENABLE
	if( device_local.type == MQTT_LINK_SERVER || device_local.type == STANDAR_MQTT_LINK_SERVER) //如果开启mqtt
	{
		#ifdef IOT_ENABLE	
		if(0 != iot_config_interface.send(cmd,devices->subnet,devices->device,subnet,device,buffer,size))
		{
			iot_debug_status(4);
		}
		
		#endif
		return 2;
	}
#endif

	// remote
	for (i = 0; i < REMOTE_MAX_SIZE; i++)
	{
		remote = &remoteClient[i];
		if ((remote->addr.ipaddr.l != 0 && remote->addr.ipaddr.l != 0xFFFFFFFF)
		&& (remote->addr.port != 0 && remote->addr.port != 0xFFFF))
		{
			NETIP_P2P_SEND(remote->addr.ipaddr.l,remote->addr.port,cmd,\
						devices->subnet,devices->device,subnet,device,buffer,size);
		}
	}
	return 0;
}

/***********************************************************
Name: 		netip_gateway_longsend
Function:	网关主动发送
Parameter:	cmd:命令 subnet:子网id device:设备id buffer:发送内容 size:发送长度
return:		0:远程服务器返回 1:本地返回 2 MQTT返回
***********************************************************/
int netip_gateway_longsend(unsigned int cmd,unsigned char subnet,\
					unsigned char device,unsigned char *buffer,unsigned char size)
{
	int i;
	struct netip_remote_t *remote;
	
	if (device_local.type == 0) return 1;
	// remote
	for (i = 0; i < REMOTE_MAX_SIZE; i++)
	{
		remote = &remoteClient[i];
		if ((remote->addr.ipaddr.l != 0 && remote->addr.ipaddr.l != 0xFFFFFFFF)
		&& (remote->addr.port != 0 && remote->addr.port != 0xFFFF))
		{
			NETIP_P2P_LONGSEND(remote->addr.ipaddr.l,remote->addr.port,cmd,\
						devices->subnet,devices->device,subnet,device,buffer,size);
		}
	}
	return 0;
}

int netip_gateway_send_special(unsigned int cmd,unsigned char src_subnet, unsigned char src_device, unsigned char subnet,\
					unsigned char device,unsigned char *buffer,unsigned char size)
{
	int i;
	struct netip_remote_t *remote;
	
	
	if (device_local.type == 0) return 1;
#ifdef IOT_ENABLE
	if( device_local.type == MQTT_LINK_SERVER || device_local.type == STANDAR_MQTT_LINK_SERVER) //如果开启mqtt
	{
	#ifdef IOT_ENABLE	
		if(0 != iot_config_interface.push(cmd, src_subnet, src_device, subnet, device, buffer, size))  //2020-05-21改
		{
			iot_debug_status(4);
		}
	#endif
		return 2;
	}
#endif

	// remote
	for (i = 0; i < REMOTE_MAX_SIZE; i++)
	{
		remote = &remoteClient[i];
		if ((remote->addr.ipaddr.l != 0 && remote->addr.ipaddr.l != 0xFFFFFFFF)
		&& (remote->addr.port != 0 && remote->addr.port != 0xFFFF))
		{
			NETIP_P2P_SEND(remote->addr.ipaddr.l,remote->addr.port,cmd,\
						src_subnet, src_device, subnet, device, buffer, size);
		}
	}
	return 0;
}

/***********************网关主动发送 end line*****************************************************/

/***********************************************************
Name: 		netip_gateway_write
Function:	网关转发
Parameter:	buffer:转发的内容 len:转发的长度
return:		0:远程服务器返回 1:本地返回 2 MQTT返回
***********************************************************/
int netip_gateway_write(unsigned char *buffer,int len)
{
	int i;
	struct netip_remote_t *remote;
	unsigned char debug_buffer[60] = {0};
	memcpy(debug_buffer, buffer, 60);
	if (device_local.type == 0) return 1; //本地
#ifdef IOT_ENABLE
	if( device_local.type == MQTT_LINK_SERVER || device_local.type == STANDAR_MQTT_LINK_SERVER) //如果开启mqtt
	{
		if(0 != iot_config_interface.write((char *)buffer,len))
		{
			iot_debug_status(4);
		}
		return 2;
	}
#endif
	// server 1
	if ((clondServer[0].ipaddr.l != 0 && clondServer[0].ipaddr.l != 0xFFFFFFFF)
		&& (clondServer[0].port != 0 && clondServer[0].port != 0xFFFF))
	{
			NETIP_P2P_WRITE(clondServer[0].ipaddr.l,clondServer[0].port,buffer,len);
	}
		
	// server 2
	if ((clondServer[1].ipaddr.l != 0 && clondServer[1].ipaddr.l != 0xFFFFFFFF)
		&& (clondServer[1].port != 0 && clondServer[1].port != 0xFFFF))
	{
			NETIP_P2P_WRITE(clondServer[1].ipaddr.l,clondServer[1].port,buffer,len);
	}
	
	// remote
	for (i = 0; i < REMOTE_MAX_SIZE; i++)
	{
		remote = &remoteClient[i];
		if ((remote->addr.ipaddr.l != 0 && remote->addr.ipaddr.l != 0xFFFFFFFF)
		&& (remote->addr.port != 0 && remote->addr.port != 0xFFFF))
		{
			if ((remote->addr.ipaddr.l == clondServer[0].ipaddr.l && remote->addr.port == clondServer[0].port)
			|| (remote->addr.ipaddr.l == clondServer[1].ipaddr.l && remote->addr.port == clondServer[1].port))
			{
				continue;
			}
			else
			{
				NETIP_P2P_WRITE(remote->addr.ipaddr.l,remote->addr.port,buffer,len);
			}
		}
	}
	
	return 0;
}
/*****************网关转发 end line***************************************************/


/***************重置设备IP***************/
void netip_gateway_reset_ipaddress(void)
{
	unsigned char buffer[80];
	unsigned char Buffer[30];
	struct netip_t *gateway;
	gateway = &netip_gateway;
	
	gateway->addr.b[0] = 192;
	gateway->addr.b[1] = 168;
	gateway->addr.b[2] = 10;
	gateway->addr.b[3] = 250;
		
	gateway->gw.b[0] = 192;
	gateway->gw.b[1] = 168;
	gateway->gw.b[2] = 10;
	gateway->gw.b[3] = 1;
		
	gateway->submask.b[0] = 255;
	gateway->submask.b[1] = 255;
	gateway->submask.b[2] = 255;
	gateway->submask.b[3] = 0;
		
	gateway->dhcp = 0;
	gateway->dns = 0;
	gateway->dns1_addr.l = 0;
	gateway->dns2_addr.l = 0x72727272;
	//yk 2020-05-28改
	netip_gateway_get_ipaddress(buffer);
	memcpy(Buffer,buffer,14);
	memcpy(&Buffer[14],&buffer[16],5);
	
	eeprom_write(IPADDR_CONFIG_BASIC_ADDRESS,Buffer,19);
	eeprom_write(DNS_SERVER_IPADDR_ADDR, &buffer[22], 9); //DNS
	netip_gateway_set_type(GATEWAY_LINK_NULL);
	return;
}
/***************重置设备IP end line***************/

/*********************获取和设置设备IP**************************/
int netip_gateway_get_ipaddress(unsigned char *buffer)
{
	struct netip_t *gateway;
	gateway = &netip_gateway;
	
	memcpy(buffer,gateway->addr.b,4);
	memcpy(&buffer[4],gateway->gw.b,4);
	
	memcpy(&buffer[8],gateway->mac,6);
	
	buffer[14] = (UDP_SERVER_PORT >> 8);
	buffer[15] = (UDP_SERVER_PORT & 0x00ff);
	
	memcpy(&buffer[16],gateway->submask.b,4);
	
	buffer[20] = gateway->dhcp;
	buffer[21] = gateway->status;
	
	// dns
	buffer[22] = gateway->dns;
	memcpy(&buffer[23],gateway->dns1_addr.b,4);
	memcpy(&buffer[27],gateway->dns2_addr.b,4);
	
	return 0;
}

int netip_gateway_set_ipaddress(unsigned char *buffer)
{
	unsigned char Buffer[30];
	NETIP_GATEWAY_REBOOT();
	memcpy(Buffer,buffer,14);
	memcpy(&Buffer[14],&buffer[16],5);
	
	unsigned char flag = 0;
	eeprom_read(IPADDR_CONFIG_BASIC_ADDRESS+19, &flag, 1);
	if(flag != 0xaa)
	{
		flag = 0xaa;
		eeprom_write(IPADDR_CONFIG_BASIC_ADDRESS+19, &flag, 1);//写标志位
	}
	
	//yk 2020-05-27改
	eeprom_write(DNS_SERVER_IPADDR_ADDR, &buffer[22], 9); //DNS
	return eeprom_write(IPADDR_CONFIG_BASIC_ADDRESS,Buffer,19);
}
/*********************获取和设置设备IP end line****************/

/*********************简易编程设置设备IP*********************************/
int easy_netip_gateway_set_ipaddress(unsigned char *buffer)
{
	unsigned char Buffer[80] = {0};

	memcpy(Buffer, buffer, 19);  //status
	memcpy(&Buffer[20], &buffer[19+18], 9);
	eeprom_write(IPADDR_CONFIG_BASIC_ADDRESS,Buffer,19);
	
	unsigned char flag = 0;
	eeprom_read(IPADDR_CONFIG_BASIC_ADDRESS+19, &flag, 1);
	if(flag != 0xaa)
	{
		flag = 0xaa;
		eeprom_write(IPADDR_CONFIG_BASIC_ADDRESS+19, &flag, 1);//写标志位
	}
	
	eeprom_write(DNS_SERVER_IPADDR_ADDR, &Buffer[20], 9); //DNS
	NETIP_GATEWAY_REBOOT();
	return 1;
}
/*********************简易编程设置设备IP end line*************************/

/************************获取和设置服务器名字******************************/
int netip_gateway_get_domainname(unsigned char *buffer)
{
	strncpy((char *)buffer,(char *)domainClient.name,64);
	return 0;
}



int netip_gateway_set_domainname(unsigned char *buffer)
{
	struct in_addr temp_addr;
	//char domainname1[] = "www.baidu.com";
	
	buffer[64] = 0x55;
	memset(domainClient.name,0,65);
	strncpy((char *)domainClient.name,(char *)buffer,64);
	domainClient.update_domainname = 1;
	return eeprom_write(DOMAINNAME_CONFIG_BASIC_ADDRESS,buffer,65);
}


void netip_gateway_update_domainname(void)
{
	struct in_addr temp_addr;
	static unsigned char mutex = 0;
	struct in_addr ip_addr;
	
	if(mutex == 1) return;
	
	if (domainClient.update_domainname)
	{
		memset((char *)remoteClient,0,(sizeof(struct netip_remote_t) * REMOTE_MAX_SIZE));
		
		mutex = 1;
		ip_addr = linux_gethostbyname((char *)domainClient.name);
		if (ip_addr.s_addr == 0)
		{
		//	domainClient.update_domainname = 1;
			mutex = 0;
			return;
		}
		temp_addr = ip_addr;	
		domainClient.ipaddr.l = temp_addr.s_addr;	
		clondServer[0].ipaddr.l = temp_addr.s_addr;
		if (domainClient.ipaddr.l > 2)
		{
			clondServer[0].port = 9999;
			domainClient.enable = 0;
		}
		domainClient.update_domainname = 0;
		mutex = 0;
	}
	
}
/*************************获取和设置服务器名字end line**********************/

#ifdef IOT_ENABLE
/*********************************************************************************************************************************/
//获取工程名 组名 用户名和密码
int netip_gateway_get_message(unsigned char *buffer,int len)
{
	buffer[0] = device_local.type;
	memcpy(&buffer[1],device_local.group,20);
	memcpy(&buffer[21],device_local.name,20);
	strncpy((char *)&buffer[41],(char *)device_local.user,8);
	strncpy((char *)&buffer[49],(char *)device_local.passwd,8);
	memcpy(MQTT_Pname, &buffer[21], 20);
	memcpy(MQTT_Gname, &buffer[1], 20);
	memcpy(MQTT_user, &buffer[41], 8);

	return 0;
}
int netip_gateway_set_message(unsigned char *buffer,int len)
{
	unsigned char BUFFER[60];
	device_local.type = buffer[0];
	if (device_local.type >= GATEWAY_LINK_MAX_NUM)
		device_local.type = 0;
#ifdef IOT_ENABLE
	netip_MQTT_Set_Status();
#endif
	memcpy(device_local.group,&buffer[1],20);
	memcpy(device_local.name,&buffer[21],20);
	strncpy((char *)device_local.user,(char *)&buffer[41],8);
	strncpy((char *)device_local.passwd,(char *)&buffer[49],8);
	memcpy(MQTT_Pname, &buffer[21], 20);
	memcpy(MQTT_Gname, &buffer[1], 20);
	memcpy(MQTT_user, &buffer[41], 8);
	
	if (device_local.type == GATEWAY_LINK_SERVER
						||  device_local.type == GATEWAY_LINK_DNS) 
	{
		memcpy(BUFFER,device_local.name,20);
		memcpy(&BUFFER[20],device_local.group,20);
		memcpy(&BUFFER[40],device_local.user,8);
		memcpy(&BUFFER[48],devices->MAC,8);
		
		// server 1
		if ((clondServer[0].ipaddr.l != 0 && clondServer[0].ipaddr.l != 0xFFFFFFFF)
			&& (clondServer[0].port != 0 && clondServer[0].port != 0xFFFF))
		{
			NETIP_P2P_SEND(clondServer[0].ipaddr.l,clondServer[0].port,MODULE_REGISTER_SERVER_CMD,\
											devices->subnet,devices->device,0XFF,0XFF,BUFFER,56);
		
		}
		
		// server 2
		if ((clondServer[1].ipaddr.l != 0 && clondServer[1].ipaddr.l != 0xFFFFFFFF)
			&& (clondServer[1].port != 0 && clondServer[1].port != 0xFFFF))
		{
			NETIP_P2P_SEND(clondServer[1].ipaddr.l,clondServer[1].port,MODULE_REGISTER_SERVER_CMD,\
											devices->subnet,devices->device,0XFF,0XFF,BUFFER,56);
		}
	}
	return eeprom_write(PROJECT_CONFIG_BASIC_ADDRESS,buffer,60);
}
/*********************************************************************************************************************************/
//MAC home ID的校验和(yk 2019-11-29加)
unsigned short netip_gateway_get_message_CRC(char *crc)
{
	unsigned char CRC_Buffer[100] = {0};
	unsigned char MAC_Buffer[9] = {0};
	unsigned char Home_ID[37] = {0};
	unsigned short crc_int = 0;
	
	memcpy(MAC_Buffer, devices->MAC, 8);
	
	netip_gateway_get_MQTT_message(CRC_Buffer);
	memcpy(Home_ID, &CRC_Buffer[37], 36);
	
	memset(CRC_Buffer, 0, 100);
	sprintf((char *)CRC_Buffer,"%s%s",Home_ID,MAC_Buffer); 
	
	crc_int = CRC16_XMODEM((unsigned char *)CRC_Buffer,8 + 36);
	return crc_int;
}

/*********************************************************************************************************************************/
//单独获取密码
void netip_gateway_get_passwd(unsigned char *buffer)
{
	unsigned char aes_buffer[8] = {0};
	
	eeprom_read(PROJECT_CONFIG_BASIC_ADDRESS + 49, aes_buffer, 8);
	memcpy(buffer, aes_buffer, 8);
}
/*********************************************************************************************************************************/
//HDLMQTT获取和修改管理员信息
int netip_gateway_get_MQTT_message(unsigned char *buffer)
{
	eeprom_read(MQTT_CONFIG_ADDRESS, buffer, MQTT_CONFIG_SIZE); 
	memcpy(Account, buffer, 36);
	memcpy(Home_ID, &buffer[37], 36);
	return MQTT_CONFIG_SIZE;
}

int netip_gateway_set_MQTT_message(unsigned char *buffer)
{
	eeprom_write(MQTT_CONFIG_ADDRESS, buffer, MQTT_CONFIG_SIZE);
	memcpy(Account, buffer, 36);
	memcpy(Home_ID, &buffer[37], 36);
	
	buffer[0] = 0xF8;
#ifdef IOT_ENABLE
	//iot_config_interface.set_status(1); //将MQTT的标志位置位1 ，开启HDL MQTT
#endif
	return 1;
}
/*********************************************************************************************************************************/
//标准MQTT获取客户端ID 用户名和密码
int netip_gateway_get_Standar_MQTT_message(unsigned char *buffer)
{
	eeprom_read(STANDAR_MQTT_CONFIG_ADDRESS, buffer, STANDAR_MQTT_CONFIG_SIZE);
	
	return STANDAR_MQTT_CONFIG_SIZE;
}
int netip_gateway_set_Standar_MQTT_message(unsigned char *buffer)
{
	eeprom_write(STANDAR_MQTT_CONFIG_ADDRESS, buffer, STANDAR_MQTT_CONFIG_SIZE);
	buffer[0] = 0xF8;
	return 1;
}
/*********************************************************************************************************************************/
//标准MQTT获取URL和端口号
int netip_gateway_get_Standar_MQTT_URL_message(unsigned char *buffer)
{
	eeprom_read(STANDAR_MQTT_CONFIG_URL_ADDRESS, buffer, STANDAR_MQTT_CONFIG_URL_SIZE);
	return STANDAR_MQTT_CONFIG_URL_SIZE;
}
int netip_gateway_set_Standar_MQTT_URL_message(unsigned char *buffer)
{
	eeprom_write(STANDAR_MQTT_CONFIG_URL_ADDRESS, buffer, STANDAR_MQTT_CONFIG_URL_SIZE);
	
	buffer[0] = 0xF8;
	return 1;
}
/*********************************************************************************************************************************/
//标准MQTT获取AES加密信息
int netip_gateway_get_STANDAR_MQTT_AES(unsigned char *buffer)
{
	eeprom_read(STANDAR_MQTT_CONFIG_AES_ADDRESS, buffer, STANDAR_MQTT_CONFIG_AES_SIZE);
	return STANDAR_MQTT_CONFIG_AES_SIZE;
}

int netip_gateway_set_STANDAR_MQTT_AES(unsigned char *buffer)
{
	eeprom_write(STANDAR_MQTT_CONFIG_AES_ADDRESS, buffer, STANDAR_MQTT_CONFIG_AES_SIZE);
#ifdef IOT_ENABLE
	iot_config_interface.setAES_enable((char *)buffer, 17);
#endif
	buffer[0] = 0xF8;
	return 1;
}
/*********************************************************************************************************************************/
//标准MQTT获取和修改主题
int netip_gateway_get_STANDAR_MQTT_TOPIC(unsigned char *buffer)
{
	if(buffer[0] == 1)
	{
		eeprom_read(STANDAR_MQTT_CONFIG_SUBTOPIC_ADDRESS + 1, &buffer[1], STANDAR_MQTT_CONFIG_SUBTOPIC_SIZE-1);
	}
	else if(buffer[0] == 2)
	{
		eeprom_read(STANDAR_MQTT_CONFIG_PUBTOPIC_ADDRESS + 1, &buffer[1], STANDAR_MQTT_CONFIG_PUBTOPIC_SIZE-1);
	}
	return STANDAR_MQTT_CONFIG_PUBTOPIC_SIZE;
}

int netip_gateway_set_STANDAR_MQTT_TOPIC(unsigned char *buffer)
{
#ifdef IOT_ENABLE
	iot_config_interface.settopic((char *)buffer,buffer[0]);
#endif
	if(buffer[0] == 1)
	{
		eeprom_write(STANDAR_MQTT_CONFIG_SUBTOPIC_ADDRESS, buffer, STANDAR_MQTT_CONFIG_SUBTOPIC_SIZE);
	}
	else if(buffer[0] == 2)
	{
		eeprom_write(STANDAR_MQTT_CONFIG_PUBTOPIC_ADDRESS, buffer, STANDAR_MQTT_CONFIG_PUBTOPIC_SIZE);
	}
	return 2;
}
#endif
/*********************************************************************************************************************************/
int netip_gateway_get_server(unsigned char *buffer,int len)
{
	memcpy(buffer,clondServer[0].ipaddr.b,4);
	buffer[4] = clondServer[0].port >> 8;
	buffer[5] = clondServer[0].port;
	
	memcpy(&buffer[6],clondServer[1].ipaddr.b,4);
	buffer[10] = clondServer[1].port >> 8;
	buffer[11] = clondServer[1].port;
	
	buffer[12] = 0;
	// hour area
	buffer[13] = device_local.point_hour;
	buffer[14] = device_local.point_minute;
	
	return 0;
}

int netip_gateway_set_server(unsigned char *buffer,int len)
{
	memcpy(clondServer[0].ipaddr.b,buffer,4);
	clondServer[0].port = buffer[4];
	clondServer[0].port <<= 8;
	clondServer[0].port += buffer[5];

	memcpy(clondServer[1].ipaddr.b,&buffer[6],4);
	clondServer[1].port = buffer[10];
	clondServer[1].port <<= 8;
	clondServer[1].port += buffer[11];
	
	// hour area
	device_local.point_hour = buffer[13];
	if(device_local.point_hour >= 24 && device_local.point_hour != 255) device_local.point_hour = 0;
	
	device_local.point_minute = buffer[14];
	if (device_local.point_minute > 60)
		device_local.point_minute = 0;
		
	return eeprom_write(SERVER_CONFIG_BASIC_ADDRESS,buffer,15);
}

int netip_gateway_get_hour_minute_area(int *hour, int *minute)
{
	//yk 2020-05-27改
	if(device_local.point_hour == 0xff)
	{
		return 0;  
	}
	
	*hour = device_local.point_hour;
	*minute = device_local.point_minute;
	return 1;
}

/************************************************************************
Name:		netip_gateway_setup_history， netip_gateway_get_history, netip_gateway_set_history
Function:	读历史记录和写历史记录
called by:  command.c
return :	1
*************************************************************************/
// history
unsigned char history_record_index = 0;
unsigned char history_record_number = 0;

void netip_gateway_setup_history(void)//用于 init
{
	unsigned char buff[10];
	
	eeprom_read(HISTORY_CONFIG_BASIC_ADDRESS,buff,2);
	
	if (buff[0] != buff[1])
		history_record_number = 0;
	else
		history_record_number = buff[0];

}

int netip_gateway_get_history(unsigned long ipaddr,unsigned short port,unsigned char *buffer,int len)
{
	unsigned char history_number_addr;  
   unsigned long addr;
   unsigned char buff[2];
   
   
   history_number_addr = buffer[0];
   if (history_number_addr == 0)
   {
      if (buffer[1] != 0x55 || buffer[2] != 0x55)
			{		
      	buffer[0] = history_number_addr;
      	buffer[1] = history_record_number; 
      	buffer[2] = 0xf5; 
      	return (3);
			}
      
      history_record_number = 0;
      history_record_index = 0;
      buff[0] = history_record_number;
      buff[1] = history_record_number;
    
      eeprom_write(HISTORY_CONFIG_BASIC_ADDRESS,buff,2);
      
      buffer[0] = history_number_addr;
      buffer[1] = history_record_number; 
      buffer[2] = 0xf8; 
      return (3);  
      
   }
   else if (history_number_addr > history_record_number)
   {
      buffer[0] = history_number_addr;
      buffer[1] = history_record_number; 
      buffer[2] = 0xf8; 
      return (3);   
   }
   else
   { 
      addr = ((unsigned long)(history_record_index-1) * 30);
			addr += HISTORY_CONFIG_BASIC_ADDRESS + 2; 
  
     eeprom_read(addr,&buffer[2],28);
         
     buffer[0] = history_number_addr;
     buffer[1] = history_record_number; 
     return (30);
   }
}


int netip_gateway_set_history(unsigned long ipaddr,unsigned short port,unsigned char *buffer,int len)
{
	unsigned long addr;
	unsigned char buff[2];

	history_record_index++;
	if(history_record_index > 50)
	{
		history_record_index = 1; 
		history_record_number = 50;
	}
	else
	{ 
		history_record_number++;
		buff[0] = history_record_number;
		buff[1] = history_record_number;

		eeprom_write(HISTORY_CONFIG_BASIC_ADDRESS,buff,2);
	}

	addr = ((unsigned long)(history_record_index-1) * 30);
	addr += HISTORY_CONFIG_BASIC_ADDRESS + 2; 

	eeprom_write(addr,buffer,28);
		
	return 0;
}

// link

/******************************读历史记录和写历史记录end line*************************************/


/*************************************************************************
Name:		netip_gateway_remote_link_endpoint
Function:	读取连接数
called by:  command.c
return :	1
*************************************************************************/
int netip_gateway_remote_link_endpoint(unsigned char *buffer)
{
	int i;
	struct netip_remote_t *remote;
	
	if (device_local.type)
	{
		for (i = 0; i < REMOTE_MAX_SIZE; i++)
		{
				remote = &remoteClient[i];
				
		  buffer[i * 10 + 0] = remote->enable;
		  buffer[i * 10 + 1] = remote->addr.ipaddr.b[0];
		  buffer[i * 10 + 2] = remote->addr.ipaddr.b[1];
		  buffer[i * 10 + 3] = remote->addr.ipaddr.b[2];
		  buffer[i * 10 + 4] = remote->addr.ipaddr.b[3];
		  buffer[i * 10 + 5] = (unsigned char)(remote->addr.port >> 8);
		  buffer[i * 10 + 6] = (unsigned char)(remote->addr.port);  
		  buffer[i * 10 + 7] = (unsigned char)(remote->timeout >> 8);
		  buffer[i * 10 + 8] = (unsigned char)(remote->timeout);
		  buffer[i * 10 + 9] = 0;
		}
	}
	else
	{
		memset(buffer,0,(10*REMOTE_MAX_SIZE));
	}
	
	return 0;
}

/*************************************************************************
Name:		netip_gateway_remote_disable
Function:	断开连接
called by:  command.c
return :	1
*************************************************************************/
void netip_gateway_remote_disable(unsigned long ipaddr,unsigned short port)
{
	int i;
	struct netip_remote_t *remote;
	
	for (i = 0; i < REMOTE_MAX_SIZE; i++)
	{
		remote = &remoteClient[i];
		
		if (remote->addr.ipaddr.l == ipaddr
		&& remote->addr.port == port)
		{
			remote->enable = 0;
			remote->timeout = 2;
		}
	}
}

/*************************************************************************
Name:		netip_gateway_remote_link
Function:	远程连接
called by:  netip_gateway_low_handler
return :	1
*************************************************************************/
void netip_gateway_remote_link(unsigned long ipaddr,unsigned short port)
{
	int i;
	struct netip_remote_t *remote;
	
	for (i = 0; i < REMOTE_MAX_SIZE; i++)
	{
		remote = &remoteClient[i];
		
		if (remote->enable)
		{
			if (remote->addr.ipaddr.l == ipaddr
			&& remote->addr.port == port)
			{
				remote->enable = 1;
				remote->timeout = 1000;
				remote->addr.ipaddr.l = ipaddr;
				remote->addr.port = port;
				break;
			}
		}
	}
}

/*************************************************************************
Name:		netip_gateway_remote_add
Function:	添加远程连接
called by:  netip_gateway_password_module
return :	1
*************************************************************************/
int netip_gateway_remote_add(unsigned long ipaddr,unsigned short port)
{
	int i;
	struct netip_remote_t *remote;
	
	for (i = 0; i < REMOTE_MAX_SIZE; i++)
	{
		remote = &remoteClient[i];
		
		if (remote->enable)
		{
			if (remote->addr.ipaddr.l == ipaddr
			&& remote->addr.port == port)
			{
				remote->enable = 1;
				remote->timeout = 1000;
				remote->addr.ipaddr.l = ipaddr;
				remote->addr.port = port;
				return 1;
			}
		}
	}
	
	for (i = 0; i < REMOTE_MAX_SIZE; i++)
	{
		remote = &remoteClient[i];
		
		if (remote->enable == 0)
		{
			remote->enable = 1;
			remote->timeout = 1000;
			remote->addr.ipaddr.l = ipaddr;
			remote->addr.port = port;
			return 1;
		}
	}
	
	return 0;
}

/*************************************************************************
Name:		netip_gateway_remote_timeout
Function:	远程超时处理
called by:  netip_gateway_low_handler
return :	1
*************************************************************************/
void netip_gateway_remote_timeout(void)
{
	int i;
	struct netip_remote_t *remote;

	for (i = 0; i < REMOTE_MAX_SIZE; i++)
	{
		remote = &remoteClient[i];

		if (remote->timeout)
		{
			remote->timeout--;
		}
		else
		{
			remote->enable = 0;
			remote->addr.ipaddr.l = 0;
			remote->addr.port = 0;
		}
	}
}


/************************************************************************
Name:		netip_gateway_password_module
Function:	验证输入密码
called by:  command.c
return :	1
*************************************************************************/
int netip_gateway_password_module(unsigned long ipaddr,unsigned short port,\
							unsigned char subnet,unsigned char device,unsigned char *buffer,int len)
{
	unsigned char key[10];
	unsigned char passwd_status;
	
	if (memcmp(buffer,device_local.name,20)) return 1;
	if (memcmp(&buffer[20],device_local.user,8)) return 1;
	
	strncpy((char *)key,(char *)device_local.passwd,8);
	remote_crypt(&buffer[36],8,key,8);
	
	if (!memcmp(key,&buffer[28],8))
	{
		if (netip_gateway_remote_add(ipaddr,port))
		{
				passwd_status = 0xf8;
				NETIP_P2P_SEND(ipaddr,port,MODULE_PASSWORD_LINK_ACK,\
						devices->subnet,devices->device,subnet,device,&passwd_status,1);
				return 0;
		}
	}
	
	passwd_status = 0xf5;
	NETIP_P2P_SEND(ipaddr,port,MODULE_PASSWORD_LINK_ACK,\
					devices->subnet,devices->device,subnet,device,&passwd_status,1);
	return 1;
}
/****************************验证输入密码end line**********************************/



/********************************空函数***************************************************/
void netip_gateway_connect_client(void)
{
	
}

void netip_gateway_connect_client_response(unsigned long ipaddr,unsigned short port)
{
	
}

int netip_gateway_conneting_module(unsigned long ipaddr,unsigned short port,\
					unsigned char subnet,unsigned char device,unsigned char *buffer,int len)
{
	
	return 0;
}


void netip_gateway_register_error_report(unsigned char *report)
{
	
}
/***************************空函数end line**********************************************/

/************************************************************************
Name:		netip_gateway_register_request
Function:	发送注册包 (分以前的版本和MQTT版本)
called by:  handler
return :	空
*************************************************************************/
void netip_gateway_register_request(void)
{
	static unsigned char timeout = 0;
	
	unsigned char buffer[90];
	struct netip_t *gateway;
	gateway = &netip_gateway;
#if 1
	if (device_local.type == GATEWAY_LINK_SERVER
						||  device_local.type == GATEWAY_LINK_DNS) 
	{
		//复位
		if (gateway->dhcp)
		{
			if (gateway->status != 1)
			{
				return;
			}
		}
				
		timeout++;
		if (timeout > 20)
		{
			timeout = 0;
			
			memcpy(buffer,device_local.name,20);
			memcpy(&buffer[20],device_local.group,20);
			memcpy(&buffer[40],device_local.user,8);
			memcpy(&buffer[48],devices->MAC,8);
			
			// server 1
			if ((clondServer[0].ipaddr.l != 0 && clondServer[0].ipaddr.l != 0xFFFFFFFF)
				&& (clondServer[0].port != 0 && clondServer[0].port != 0xFFFF))
			{
				NETIP_P2P_SEND(clondServer[0].ipaddr.l,clondServer[0].port,MODULE_REGISTER_SERVER_CMD,\
												devices->subnet,devices->device,0XFF,0XFF,buffer,56);
			
			}
			
			// server 2
			if ((clondServer[1].ipaddr.l != 0 && clondServer[1].ipaddr.l != 0xFFFFFFFF)
				&& (clondServer[1].port != 0 && clondServer[1].port != 0xFFFF))
			{
				NETIP_P2P_SEND(clondServer[1].ipaddr.l,clondServer[1].port,MODULE_REGISTER_SERVER_CMD,\
												devices->subnet,devices->device,0XFF,0XFF,buffer,56);
			}
		}
	}
#endif
}



/************************************************************************
Name:		netip_gateway_register_respons
Function:	注册包返回消息处理
called by:  netip_gateway_handler
return :	空
*************************************************************************/
#ifdef DATETIME_ENABLE
extern void datetime_get_localtime(unsigned char area_hour,unsigned char area_minute,unsigned char *datetime);
#endif

void netip_gateway_register_respons(unsigned char *buffer)
{
	netip_socket_timeout = 0;
	
	if (device_local.type)
	{	
		//yk 2020-05-27改
		if(device_local.point_hour == 0xff)
		{
		#ifdef DATETIME_ENABLE
			set_datetime_enable(0);
		#endif
			return ;  
		}
#ifdef DATETIME_ENABLE
		datetime_get_localtime(device_local.point_hour,device_local.point_minute,&buffer[9]);
		datetime_interface.set(&buffer[9],7);
#endif
	}
}
/***********************************注册及其反馈函数end line*********************************************************/


#if (DNS == 2)
//extern IPaddr DNSiid[2];
#endif
void netip_gateway_dns_handler(void)
{
	struct ip_addr dns_ipaddr;
	struct netip_t *gateway;
	gateway = &netip_gateway;
	struct in_addr temp_addr;
	// get dhcp ip
	if (gateway->dhcp)
	{
		if (gateway->status == 1)
		{
			gateway->status = 1;
			gateway->dns = 1;
			//gateway->dns1_addr.l = DNSiid[0].l;
			//gateway->dns2_addr.l = DNSiid[1].l;
			#if LWIP_DNS 
			dns_ipaddr = dns_getserver(0);
			gateway->dns1_addr.l = dns_ipaddr.addr;
			dns_ipaddr = dns_getserver(1);
			gateway->dns2_addr.l = dns_ipaddr.addr;
			#endif
		}
	}
	
	// handler dns domain
	
	if (domainClient.enable)
	{
		domainClient.update_domainname = 1;
		domainClient.enable = 0;
	}
}

/************************************************************************
Name:		netip_gateway_handler
Function:	网关处理函数
called by:  netip_gateway_handler
return :	空
*************************************************************************/
void netip_gateway_handler(void)
{
	static unsigned long wwull = 0;
	netip_gateway_low_handler();
	
	if ((abs(NTimeMS() - wwull) > 1000))
	{
		wwull = NTimeMS();
		
		netip_gateway_register_request();
	
		netip_gateway_remote_timeout();
	
		netip_gateway_dns_handler();
		
		netip_socket_timeout_handler();
		
		netip_gateway_update_domainname();
	}
}

#endif
