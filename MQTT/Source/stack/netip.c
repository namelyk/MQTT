#include "netip.h"
#include "stack.h"

#include "hw_init.h"
#include "MQTTLinux.h"


//声明函数
#ifdef NETIP_ENABLE


#define SERVER_PORT						6000
#define MULTICAST_IP					"224.0.168.188"
#define BROADCAST_IP					0xFFFFFFFF	

#define HEAD_MIRACLE					"HDLMIRACLE"


#define NC2(val) 		((((unsigned short)val&0xff) << 8) | ((unsigned short)val >> 8))


static unsigned int netip_device_type = 0;


static int netip_socket;
static unsigned short netip_port;
unsigned long netip_broadcast;
unsigned long netip_multicast;


extern struct netip_interface_t netip_low_interface;

unsigned long remote_ipaddr;
unsigned short remote_port;

/***************************************************
***************************************************/
struct netip_t *netip;

char netip_tx_buffer[1500];


static void (*_callback)(int s,unsigned char *buffer,int size);


// low
void EthernetIntHandler(void);
void netip_arp_handler(void);
void netip_dns_handler(void);

int low_udp_sendto(int s, char *buf, int len, int flags,
           struct sockaddr * to, int tolen);
void low_netip_init(void);
void low_udp_recvform(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);
int socket_open(int port,void (* recv)(void *arg, struct udp_pcb *upcb, struct pbuf *p,
                       struct ip_addr *addr, u16_t port),int igmp_flag);
void socket_close(int s);


/***************************************************
函数名称：   inet_addr
输入参数：	
返 回 值:	 
功能描述：	  
***************************************************/
static unsigned long inet_addr(const char *addr)
{
	unsigned long temp , addr_temp;
	int i,j,len;
	
	addr_temp = 0;
	temp = 0;
	j = 0;

	len = strlen(addr);
	for(i = 0; i < len; i++)
	{
		if (*addr == '.')
		{
			addr++;
			addr_temp |= (temp << j);
			j += 8;
			temp = 0;			
		}
		else if (*addr >= '0' || *addr <= '9' )
		{
			temp *= 10;
			temp += (*addr - 48);
			addr++;
		}
		else
		{
			
			break;
		}
	}
	addr_temp |= (temp << j);
	
	return addr_temp;
}
/***************************************************
函数名称：   inet_ipaddr
输入参数：	
返 回 值:	 
功能描述：	  
***************************************************/
unsigned long inet_ipaddr(unsigned char *val)
{
	unsigned long temp;
	
	temp = val[3];  	
	temp <<= 8;
	temp += val[2]; 	
	temp <<= 8;
	temp += val[1];  
	temp <<= 8;
	temp += val[0];
	
	return temp;
}








/***************************************************
函数名称：   netip_handler
输入参数：	
返 回 值:	 
功能描述：	  
***************************************************/
void netip_handler(void)
{
//	while(ETH_GetRxPktSize())
//	{
//		 EthernetIntHandler();
//				LED_GREEN_ON();
//	}
//	
	netip_arp_handler();

	//netip_dns_handler();
}


/***************************************************
函数名称：   netip_broadcast_send
输入参数：	
返 回 值:	 
功能描述：	  
***************************************************/
int netip_broadcast_send(char *packet,int len)
{
	int retval;
	struct sockaddr_in to;
	
	netip_broadcast = ~netip->submask.l;
	netip_broadcast |= (netip->addr.l & netip->submask.l);
	
	//to.sin_family = AF_INET;  
  to.sin_port = netip_port;
  to.sin_addr.s_addr = netip_broadcast;//inet_addr("192.168.1.255"); 
    
  retval = low_udp_sendto(netip_socket, packet, len, 0,\
														(struct sockaddr*)&to, sizeof(to));
	return retval;
	
	//return udp_sendto(netip_socket,netip_broadcast,netip_port,packet,len);
}

/***************************************************
函数名称：   netip_p2p_send
输入参数：	
返 回 值:	 
功能描述：	  
***************************************************/
int netip_p2p_send(unsigned int ipaddr,unsigned short port,char *packet,int len)
{
	int retval;
	struct sockaddr_in to;
    
 // to.sin_family = AF_INET;  
  to.sin_port = port;
  to.sin_addr.s_addr = ipaddr;//inet_addr("192.168.1.255"); 
    
  retval = low_udp_sendto(netip_socket, packet, len, 0,\
														(struct sockaddr*)&to, sizeof(to));
	return retval;
	//return udp_sendto(netip_socket,ipaddr,port,packet,len);
}

/***************************************************
函数名称：   netip_multicast_send
输入参数：	
返 回 值:	 
功能描述：	  
***************************************************/
int netip_multicast_send(char *packet,int len)
{
	int retval;
	struct sockaddr_in to;
    
  //to.sin_family = AF_INET;  
  to.sin_port = netip_port;
  to.sin_addr.s_addr = netip_multicast;//inet_addr("192.168.1.255"); 
    
  retval = low_udp_sendto(netip_socket, packet, len, 0,\
														(struct sockaddr*)&to, sizeof(to));
	return retval;
	//return udp_sendto(netip_socket,netip_multicast,netip_port,packet,len);
}

/***************************************************
函数名称：   netip_broadcast_all_send
输入参数：	
返 回 值:	 
功能描述：	  
***************************************************/
int netip_broadcast_all_send(char *packet,int len)
{
	int retval;
	struct sockaddr_in to;
    
  //to.sin_family = AF_INET;  
  to.sin_port = netip_port;
  to.sin_addr.s_addr = BROADCAST_IP;//inet_addr("192.168.1.255"); 
    
  retval = low_udp_sendto(netip_socket, packet, len, 0,\
														(struct sockaddr*)&to, sizeof(to));
	return retval;
	
	//return udp_sendto(netip_socket,BROADCAST_IP,netip_port,packet,len);
}




/***************************************************
函数名称：	netip_set_type
输入参数：	
返 回 值:	
功能描述：	
***************************************************/
int netip_set_type(unsigned int type,unsigned char *encode,\
					void (*callback)(int ,unsigned char*, int))
{
	
		netip_device_type = type;
		_callback = callback;
		return 0;

}
/***************************************************
函数名称：	netip_get_type
输入参数：	
返 回 值:	
功能描述：	
***************************************************/
unsigned int netip_get_type(void)
{
	return netip_device_type;
}

/***************************************************
函数名称：   netip_setup
输入参数：	
返 回 值:	 
功能描述：	  
***************************************************/
void netip_setup(struct netip_t *ipaddr)
{
	netip = ipaddr;
}

/***************************************************
函数名称：   netip_init
输入参数：	
返 回 值:	 
功能描述：	  
***************************************************/
void netip_init(void)
{
	if (netip_device_type == 0) return;
	
	low_netip_init();
	//ethernet_init(0,0,0,0,(char *)netip->mac);
	//uencode_compiledate(__DATE__,__TIME__);
	
	//dhcp
//	if (netip->dhcp)
//	{
//		dhcp_init((char *)netip->addr.b,(char *)netip->submask.b,\
//							(char *)netip->gw.b,(char *)netip->mac,\
//								"steven-home",(netip_low_interface.watchdog_clear));
//		
//		//netip_low_interface.interrupt_enable();
//		
//		netip->status = dhcp_start();
//	}

	// open socket
	netip_broadcast = ~netip->submask.l;
	netip_broadcast |= (netip->addr.l & netip->submask.l);
	
	netip_port = SERVER_PORT;
	//ethernet_setnetip(netip->gw.l,netip->submask.l,netip->addr.l,netip_broadcast);
	#if 0
	netip_socket = socket_open(netip_port,low_udp_recvform,1);
	#endif
	
	netip_multicast = inet_addr(MULTICAST_IP);
	//multicast_open(netip_socket,netip_multicast);
	//socket_close(netip_socket);
	

	//dns
	//netip_low_interface.interrupt_enable();
	//dns_init(netip_low_interface.watchdog_clear);	
	
	//netip->dns = 1;
	//netip->dns1_addr.l = dns_getserver(0);
	//netip->dns2_addr.l = dns_getserver(1);
	
	
	
	


}

/***************************************************
函数名称：   netip_close
输入参数：	
返 回 值:	 
功能描述：	  
***************************************************/
void netip_close(void)
{
	socket_close(netip_socket);
}

/***************************************************
函数名称：   netip_read
输入参数：	
返 回 值:	 
功能描述：	  
***************************************************/
void netip_read(char *packet, int len)
{
	unsigned char *buffer = (unsigned char *)packet;
	unsigned short crc;
	
	//接收
	if(netip->addr.l == remote_ipaddr) return;
	remote_port	= NC2(remote_port);
	
	if (memcmp((&buffer[4]),(char *)HEAD_MIRACLE,10)) return;
	
	if (buffer[14] != 0xAA || buffer[15] != 0xAA)	return;
	
	// callback
	if (buffer[16] < 79 && buffer[16] >= 11)
	{
		
		crc = CRC16_XMODEM(&buffer[16], buffer[16]-2);
		if (buffer[buffer[16] + 14] != (unsigned char)(crc >> 8)
		|| buffer[buffer[16] + 15] != (unsigned char)(crc))
		{
			return;
		}				
	}
	
//	netip_write(&buffer[16], buffer[16]);
	
	// callback
	if ((netip->addr.l & netip->submask.l) == (remote_ipaddr & netip->submask.l))
	{
		_callback(0,&buffer[16],len-16);
	}
	else
	{
		_callback(2,&buffer[16],len-16);
	}
}



/***************************************************
函数名称：   netip_write
输入参数：	
返 回 值:	 
功能描述：	  
***************************************************/
int netip_write(char *packet,int len)
{
	memcpy((unsigned char *)(&netip_tx_buffer[0]),netip->addr.b,4);
	memcpy((unsigned char *)(&netip_tx_buffer[4]),(unsigned char *)HEAD_MIRACLE,10);
	
	netip_tx_buffer[14] = 0xAA;
	netip_tx_buffer[15] = 0xAA;
	memcpy((unsigned char *)(&netip_tx_buffer[16]),(unsigned char *)packet,len);

	return netip_broadcast_send(netip_tx_buffer,len + 16);

}

/***************************************************
函数名称：   netip_ptp_write
输入参数：	
返 回 值:	 
功能描述：	  
***************************************************/
int netip_ptp_write(unsigned int ipaddr,unsigned short port,char *packet,int len)
{
	memcpy((unsigned char *)(&netip_tx_buffer[0]),netip->addr.b,4);
	memcpy((unsigned char *)(&netip_tx_buffer[4]),(unsigned char *)HEAD_MIRACLE,10);
	
	netip_tx_buffer[14] = 0xAA;
	netip_tx_buffer[15] = 0xAA;
	memcpy((unsigned char *)(&netip_tx_buffer[16]),(unsigned char *)packet,len);

	return netip_p2p_send(ipaddr,port,netip_tx_buffer,len + 16);

}



/***************************************************
			小包	
***************************************************/
/***************************************************
函数名称：  netip_allbroadcast_send
输入参数：	
返 回 值:	
功能描述：	
***************************************************/
int netip_allbroadcast_send(unsigned int cmd,unsigned char src_subnet,unsigned char src_device, \
											unsigned char dest_subnet,unsigned char dest_device, \
											unsigned char *buffer,unsigned char len)
{
	unsigned int crc;
	
	if (len > 67) return 1;

	memcpy((unsigned char *)(&netip_tx_buffer[0]),netip->addr.b,4);
	memcpy((unsigned char *)(&netip_tx_buffer[4]),(unsigned char *)HEAD_MIRACLE,10);

	netip_tx_buffer[14] = 0xAA;
	netip_tx_buffer[15] = 0xAA;
	netip_tx_buffer[16] = len + 11;  //小包 
	netip_tx_buffer[17] = src_subnet;  
	netip_tx_buffer[18] = src_device;   
	netip_tx_buffer[19] = (unsigned char)(netip_device_type >> 8);     
	netip_tx_buffer[20] = (unsigned char)(netip_device_type & 0X00FF); 
	netip_tx_buffer[21] = (unsigned char)(cmd >> 8);
	netip_tx_buffer[22] = (unsigned char)(cmd & 0x00FF);
	netip_tx_buffer[23] = dest_subnet;
	netip_tx_buffer[24] = dest_device;

	memcpy((unsigned char *)&(netip_tx_buffer[25]),buffer,len);

	crc = CRC16_XMODEM((unsigned char *)&netip_tx_buffer[16],netip_tx_buffer[16]-2);
	netip_tx_buffer[len + 25] = (unsigned char)(crc >> 8);
	netip_tx_buffer[len + 26] = (unsigned char)(crc);
	
	
	return netip_broadcast_all_send(netip_tx_buffer,len + 27);
	
}
/***************************************************
函数名称：  netip_multi_send
输入参数：	
返 回 值:	
功能描述：	
***************************************************/
int netip_multi_send(unsigned int cmd,unsigned char src_subnet,unsigned char src_device, \
											unsigned char dest_subnet,unsigned char dest_device, \
											unsigned char *buffer,unsigned char len)
{
	unsigned int crc;
	
	if (len > 67) return 1;

	memcpy((unsigned char *)(&netip_tx_buffer[0]),netip->addr.b,4);
	memcpy((unsigned char *)(&netip_tx_buffer[4]),(unsigned char *)HEAD_MIRACLE,10);

	netip_tx_buffer[14] = 0xAA;
	netip_tx_buffer[15] = 0xAA;
	netip_tx_buffer[16] = len + 11;  //小包 
	netip_tx_buffer[17] = src_subnet;  
	netip_tx_buffer[18] = src_device;   
	netip_tx_buffer[19] = (unsigned char)(netip_device_type >> 8);     
	netip_tx_buffer[20] = (unsigned char)(netip_device_type & 0X00FF); 
	netip_tx_buffer[21] = (unsigned char)(cmd >> 8);
	netip_tx_buffer[22] = (unsigned char)(cmd & 0x00FF);
	netip_tx_buffer[23] = dest_subnet;
	netip_tx_buffer[24] = dest_device;

	memcpy((unsigned char *)&(netip_tx_buffer[25]),buffer,len);

	crc = CRC16_XMODEM((unsigned char *)&netip_tx_buffer[16],netip_tx_buffer[16]-2);
	netip_tx_buffer[len + 25] = (unsigned char)(crc >> 8);
	netip_tx_buffer[len + 26] = (unsigned char)(crc);
	

	return netip_multicast_send(netip_tx_buffer,len + 27);
	
}

/***************************************************
函数名称：  netip_send
输入参数：	
返 回 值:	
功能描述：	
***************************************************/
int netip_send(unsigned int cmd,unsigned char src_subnet,unsigned char src_device, \
											unsigned char dest_subnet,unsigned char dest_device, \
											unsigned char *buffer,unsigned char len)
{
	unsigned int crc;
	
	if (len > 67) return 1;

	memcpy((unsigned char *)(&netip_tx_buffer[0]),netip->addr.b,4);
	memcpy((unsigned char *)(&netip_tx_buffer[4]),(unsigned char *)HEAD_MIRACLE,10);

	netip_tx_buffer[14] = 0xAA;
	netip_tx_buffer[15] = 0xAA;
	netip_tx_buffer[16] = len + 11;  //小包 
	netip_tx_buffer[17] = src_subnet;  
	netip_tx_buffer[18] = src_device;   
	netip_tx_buffer[19] = (unsigned char)(netip_device_type >> 8);     
	netip_tx_buffer[20] = (unsigned char)(netip_device_type & 0X00FF); 
	netip_tx_buffer[21] = (unsigned char)(cmd >> 8);
	netip_tx_buffer[22] = (unsigned char)(cmd & 0x00FF);
	netip_tx_buffer[23] = dest_subnet;
	netip_tx_buffer[24] = dest_device;

	memcpy((unsigned char *)&(netip_tx_buffer[25]),buffer,len);

	crc = CRC16_XMODEM((unsigned char *)&netip_tx_buffer[16],netip_tx_buffer[16]-2);
	netip_tx_buffer[len + 25] = (unsigned char)(crc >> 8);
	netip_tx_buffer[len + 26] = (unsigned char)(crc);
	
	return netip_broadcast_send(netip_tx_buffer,len + 27);

}

/***************************************************
函数名称：  netip_ptp_send
输入参数：	
返 回 值:	
功能描述：	
***************************************************/
int netip_ptp_send(unsigned int ipaddr,unsigned short port,\
			unsigned int cmd,unsigned char src_subnet,unsigned char src_device, \
											unsigned char dest_subnet,unsigned char dest_device, \
											unsigned char *buffer,unsigned char len)
{
	unsigned int crc;
	
	if (len > 67) return 1;

	memcpy((unsigned char *)(&netip_tx_buffer[0]),netip->addr.b,4);
	memcpy((unsigned char *)(&netip_tx_buffer[4]),(unsigned char *)HEAD_MIRACLE,10);

	netip_tx_buffer[14] = 0xAA;
	netip_tx_buffer[15] = 0xAA;
	netip_tx_buffer[16] = len + 11;  //小包 
	netip_tx_buffer[17] = src_subnet;  
	netip_tx_buffer[18] = src_device;   
	netip_tx_buffer[19] = (unsigned char)(netip_device_type >> 8);     
	netip_tx_buffer[20] = (unsigned char)(netip_device_type & 0X00FF); 
	netip_tx_buffer[21] = (unsigned char)(cmd >> 8);
	netip_tx_buffer[22] = (unsigned char)(cmd & 0x00FF);
	netip_tx_buffer[23] = dest_subnet;
	netip_tx_buffer[24] = dest_device;

	memcpy((unsigned char *)&(netip_tx_buffer[25]),buffer,len);

	crc = CRC16_XMODEM((unsigned char *)&netip_tx_buffer[16],netip_tx_buffer[16]-2);
	netip_tx_buffer[len + 25] = (unsigned char)(crc >> 8);
	netip_tx_buffer[len + 26] = (unsigned char)(crc);
	
	return netip_p2p_send(ipaddr,port,netip_tx_buffer,len + 27);
	
}



/***************************************************
				大包	
***************************************************/

/***************************************************
函数名称：  netip_long_send
输入参数：	
返 回 值:	
功能描述：	
***************************************************/
int netip_long_send(unsigned int cmd,unsigned char src_subnet,unsigned char src_device, \
											unsigned char dest_subnet,unsigned char dest_device, \
											unsigned char *buffer,int len)
{
	if (len > 1200) return 1;

	memcpy((unsigned char *)(&netip_tx_buffer[0]),netip->addr.b,4);
	memcpy((unsigned char *)(&netip_tx_buffer[4]),(unsigned char *)HEAD_MIRACLE,10);

	netip_tx_buffer[14] = 0xAA;
	netip_tx_buffer[15] = 0xAA;
	netip_tx_buffer[16] = 0xFF;  //大包 
	netip_tx_buffer[17] = src_subnet;  
	netip_tx_buffer[18] = src_device;   
	netip_tx_buffer[19] = (unsigned char)(netip_device_type >> 8);     
	netip_tx_buffer[20] = (unsigned char)(netip_device_type & 0X00FF); 
	netip_tx_buffer[21] = (unsigned char)(cmd >> 8);
	netip_tx_buffer[22] = (unsigned char)(cmd & 0x00FF);
	netip_tx_buffer[23] = dest_subnet;
	netip_tx_buffer[24] = dest_device;

	netip_tx_buffer[25] = (unsigned char)(len >> 8);
	netip_tx_buffer[26] = (unsigned char)(len & 0x00FF);
	
	memcpy((unsigned char *)&(netip_tx_buffer[27]),buffer,len);
	
	return netip_broadcast_send(netip_tx_buffer,len + 27);

}

/***************************************************
函数名称：  netip_ptp_long_send
输入参数：	
返 回 值:	
功能描述：	
***************************************************/
int netip_ptp_long_send(unsigned int ipaddr,unsigned short port,\
				unsigned int cmd,unsigned char src_subnet,unsigned char src_device, \
											unsigned char dest_subnet,unsigned char dest_device, \
											unsigned char *buffer,int len)
{
	if (len > 1200) return 1;

	memcpy((unsigned char *)(&netip_tx_buffer[0]),netip->addr.b,4);
	memcpy((unsigned char *)(&netip_tx_buffer[4]),(unsigned char *)HEAD_MIRACLE,10);

	netip_tx_buffer[14] = 0xAA;
	netip_tx_buffer[15] = 0xAA;
	netip_tx_buffer[16] = 0xFF;  //大包 
	netip_tx_buffer[17] = src_subnet;  
	netip_tx_buffer[18] = src_device;   
	netip_tx_buffer[19] = (unsigned char)(netip_device_type >> 8);     
	netip_tx_buffer[20] = (unsigned char)(netip_device_type & 0X00FF); 
	netip_tx_buffer[21] = (unsigned char)(cmd >> 8);
	netip_tx_buffer[22] = (unsigned char)(cmd & 0x00FF);
	netip_tx_buffer[23] = dest_subnet;
	netip_tx_buffer[24] = dest_device;

	netip_tx_buffer[25] = (unsigned char)(len >> 8);
	netip_tx_buffer[26] = (unsigned char)(len & 0x00FF);
	
	memcpy((unsigned char *)&(netip_tx_buffer[27]),buffer,len);
	
	return netip_p2p_send(ipaddr,port,netip_tx_buffer,len + 27);
}



#if 1
// low ethernetif
//=======================================================================
#include <inttypes.h>

#define MAX_SOCKET						5
#define MAX_DHCP_TRIES        6//15 //yk 2020-08-26改
#define SELECTED              1
#define NOT_SELECTED		  (!SELECTED)
#define CLIENTMAC6            2

#define __IO 			volatile

struct netif netif;

__IO uint32_t TCPTimer = 0;
__IO uint32_t  IGMPTimer=0;

#ifdef LWIP_DHCP
__IO uint32_t DHCPfineTimer = 0;
__IO uint32_t DHCPcoarseTimer = 0;
static uint32_t IPaddress = 0;
#endif

#if LWIP_DNS 
char DNS_status = 0;
__IO uint32_t DNScoarseTimer;

#endif

unsigned long ticks;



typedef struct n_socket
{
	char status;
	struct udp_pcb *upcb;
	struct tcp_pcb *tpcb;
}nsocket;

struct n_socket netps[MAX_SOCKET];


typedef enum
{
  DHCP_START=0,
  DHCP_WAIT_ADDRESS,
  DHCP_ADDRESS_ASSIGNED,
  DHCP_TIMEOUT
}dhcp_state;


int netip_ticks;
void netip_reset(void)
{
#ifdef NETIP_ENABLE	
	#if 1
	netip_ticks++;
	if (netip_ticks < 30)
		return;
	netip_ticks = 0;
	
	netip_low_interface.interrupt_disable();
	ETH_DeInit();
	RCC_Configuration();
	
	Set_MAC_Address(netip->mac);
	
	netip_low_interface.interrupt_enable();	
	#endif
#endif
	
	
}


void EthernetIntHandler(void)
{
  ethernetif_input(&netif);//网络包的入口
}


void netip_arp_handler(void)
{
	static uint32_t ARPTimer = 0;
	
	if (ticks - ARPTimer >= ARP_TMR_INTERVAL)
  {
    ARPTimer =  ticks;
		netip_low_interface.interrupt_disable();	
    etharp_tmr();	
		netip_low_interface.interrupt_enable();	
  }
	
#if LWIP_TCP
    if(ticks - TCPTimer >= TCP_TMR_INTERVAL)
		{
        TCPTimer =  ticks;
        tcp_tmr();
    }
#endif /* LWIP_TCP */
		
#if LWIP_DNS
	if (ticks - DNScoarseTimer >= DNS_TMR_INTERVAL)
  {
		DNScoarseTimer = ticks;
		dns_tmr();
	}
#endif

#if LWIP_DHCP	
	if (ticks - DHCPcoarseTimer >= DHCP_COARSE_TIMER_MSECS)
	{
		DHCPcoarseTimer =  ticks;
		dhcp_coarse_tmr();
	}
#endif					

#ifdef LWIP_IGMP
	if(ticks - IGMPTimer >= IGMP_TMR_INTERVAL*100)
	{
		IGMPTimer = ticks;
		igmp_tmr();
	}
#endif
}

void netip_dhcp_handler(void)
{
	#define DHCP_TMR_INTERVAL			250
	static uint32_t DHCPTimer = 0;
	struct ip_addr ipaddr;
	struct ip_addr netmask;
	struct ip_addr gw; 
	static char DHCP_state = DHCP_START;
	unsigned long wait_Time = ticks;
	
	if (netip->dhcp == 0)
	{
		DHCP_state = DHCP_ADDRESS_ASSIGNED;
	}
	else
	{
		netip_low_interface.interrupt_enable();
	}
	
	//while(abs(ticks - wait_Time) < 10000)
	while(1)
	{
		netip_low_interface.watchdog_clear();
		
		if (DHCP_state == DHCP_TIMEOUT) 
		{
			break;
		}
		
		// netip arp handler
		netip_arp_handler();
	
		switch (DHCP_state)
		{
			case DHCP_START:
#if LWIP_DHCP
				dhcp_start(&netif);
#endif
				IPaddress = 0;
				DHCP_state = DHCP_WAIT_ADDRESS;
				break;
			
			case DHCP_WAIT_ADDRESS:
				
#if LWIP_DHCP
					if (ticks - DHCPfineTimer >= DHCP_FINE_TIMER_MSECS)
					{
						DHCPfineTimer =  ticks;
						dhcp_fine_tmr();
					}

					if (ticks - DHCPcoarseTimer >= DHCP_COARSE_TIMER_MSECS)
					{
						DHCPcoarseTimer =  ticks;
						dhcp_coarse_tmr();
					}
#endif				
					if (ticks - DHCPTimer >= DHCP_TMR_INTERVAL)
					{
						DHCPTimer = ticks;
					
					 IPaddress = netif.ip_addr.addr;
					 if (IPaddress!=0) 
					 {
						DHCP_state = DHCP_ADDRESS_ASSIGNED;	
						#if LWIP_DHCP
						dhcp_stop(&netif);
						#endif
						netip->status = 1;
						netip->addr.b[3] = (uint8_t)(netif.ip_addr.addr >> 24);
						netip->addr.b[2] = (uint8_t)(netif.ip_addr.addr >> 16);
						netip->addr.b[1] = (uint8_t)(netif.ip_addr.addr >> 8);
						netip->addr.b[0] = (uint8_t)(netif.ip_addr.addr);
						
						netip->submask.b[3] = (uint8_t)(netif.netmask.addr >> 24);
						netip->submask.b[2] = (uint8_t)(netif.netmask.addr >> 16);
						netip->submask.b[1] = (uint8_t)(netif.netmask.addr >> 8);
						netip->submask.b[0] = (uint8_t)(netif.netmask.addr);
						
						netip->gw.b[3] = (uint8_t)(netif.gw.addr >> 24);
						netip->gw.b[2] = (uint8_t)(netif.gw.addr >> 16);
						netip->gw.b[1] = (uint8_t)(netif.gw.addr >> 8);
						netip->gw.b[0] = (uint8_t)(netif.gw.addr);
						
					 }
					}
					else
					{
						#if LWIP_DHCP
						if (netif.dhcp->tries > MAX_DHCP_TRIES)
						#endif
						{
							DHCP_state = DHCP_ADDRESS_ASSIGNED;
							#if LWIP_DHCP
							dhcp_stop(&netif);
							#endif
							netip->addr.b[0] = 192;
							netip->addr.b[1] = 168;
							netip->addr.b[2] = 10;
							netip->addr.b[3] = 250;
							netip->submask.b[0]  = 255;
							netip->submask.b[1]  = 255;
							netip->submask.b[2]  = 255;
							netip->submask.b[3]  = 0;
							netip->gw.b[0] = 192;
							netip->gw.b[1] = 168;
							netip->gw.b[2] = 10;
							netip->gw.b[3] = 1;	 
						}
					}
					break;
					
			case DHCP_ADDRESS_ASSIGNED:
					IP4_ADDR(&ipaddr, netip->addr.b[0] ,netip->addr.b[1] , netip->addr.b[2] , netip->addr.b[3]);
					IP4_ADDR(&netmask, netip->submask.b[0] ,netip->submask.b[1] , netip->submask.b[2] , netip->submask.b[3]);
					IP4_ADDR(&gw, netip->gw.b[0] ,netip->gw.b[1] , netip->gw.b[2] , netip->gw.b[3]);
					netif_set_addr(&netif, &ipaddr , &netmask, &gw);
					DHCP_state = DHCP_TIMEOUT;
					
					break;
			
			default:
					break;
		}
	}
	
	netip_low_interface.interrupt_disable();
}



static void dns_user_found(const char *name, struct ip_addr *ipaddr, void *callback_arg)
{
	
}

int netip_dns_gethostname(const char *hostname)
{
#if LWIP_DNS 
	if (strlen(hostname) > 64)return 1;
	netip->domain_ipaddr.addr = 0;
	strcpy(netip->domain,hostname);
	
	DNS_status = 2;
	return 0;
#endif	
}


void netip_dns_handler(void)
{
	char arg;
	struct ip_addr dns_ipaddr;
	
#if LWIP_DNS 
	switch (DNS_status)
	{
		case 0:
			dns_init();
			DNS_status = 1;
			break;
		case 1:
			if (netip->dhcp)
			{
				if (netip->status)
				{
					dns_ipaddr = dns_getserver(0);
					netip->dns1_addr.l = dns_ipaddr.addr;
					dns_ipaddr = dns_getserver(1);
					netip->dns2_addr.l = dns_ipaddr.addr;
					
				//	netip->dns1_addr = dns_getserver(0);
				//	netip->dns2_addr = dns_getserver(1);
				}
			}
			else
			{
				dns_ipaddr.addr = netip->dns1_addr.l;
				dns_setserver(0,&dns_ipaddr);
				dns_ipaddr.addr = netip->dns2_addr.l;
				dns_setserver(1,&dns_ipaddr);
				
				//dns_setserver(0,&netip->dns1_addr);
				//dns_setserver(1,&netip->dns2_addr);
			}
			DNS_status = 2;
			break;
			
		case 2:
			if (dns_gethostbyname(netip->domain,&netip->domain_ipaddr,&dns_user_found,(void *)&arg) == 0)
			{
				DNS_status = 3;
			}
			break;
			
		case 3:
			if ((ticks - DNScoarseTimer) >= DNS_TMR_INTERVAL) 
			{ 
				DNScoarseTimer =  ticks; 
				dns_tmr(); 
			} 
			break;
	}
#endif
}

void low_udp_recvform(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
	remote_ipaddr = addr->addr;
	remote_port = port;
  //REMOTE_PORT <<= 8;
  //REMOTE_PORT |= port;

	LED_RED_ON();
	
	
	netip_read(p->payload,p->len);
}

int low_udp_sendto(int s, char *buf, int len, int flags,
           struct sockaddr * to, int tolen)
{
	struct sockaddr_in *ipaddr;
//	struct ip_addr ipaddr;
	
	struct udp_pcb *upcb;                                 
  struct pbuf *p;
	int stat = -1;
	
	netip_low_interface.interrupt_disable();
	
	ipaddr = (struct sockaddr_in *)to;
	upcb = netps[s].upcb;
	
	udp_connect(upcb, (struct ip_addr *)&(ipaddr->sin_addr.s_addr), (ipaddr->sin_port));
	p = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
	if (p != NULL)
	{
		p->payload = (void *)buf;
		p->len = len;
		p->tot_len = len;
		stat = udp_send(upcb, p);
	}
	udp_disconnect(upcb);
	pbuf_free(p);
	
	netip_low_interface.interrupt_enable();
	
	return stat;
}




void low_netip_init(void)
{
	
	struct ip_addr ipaddr;
  struct ip_addr netmask;
  struct ip_addr gw;
  mem_init();
  memp_init();

#if LWIP_DHCP
  ipaddr.addr = 0;
  netmask.addr = 0;
  gw.addr = 0;
#else
  IP4_ADDR(&ipaddr, 192, 168, 10, 250);
  IP4_ADDR(&netmask, 255, 255, 255, 0);
  IP4_ADDR(&gw, 192, 168, 10, 1);
#endif
	
	if (netip->dhcp == 0)
	{
		ipaddr.addr = netip->addr.l;			
		netmask.addr = netip->submask.l;
		gw.addr	= netip->gw.l;
	}
	
	
  netif.hwaddr[0] =  netip->mac[0];
  netif.hwaddr[1] =  netip->mac[1];
  netif.hwaddr[2] =  netip->mac[2];
  netif.hwaddr[3] =  netip->mac[3];
  netif.hwaddr[4] =  netip->mac[4];
  netif.hwaddr[5] =  netip->mac[5];
	
  Set_MAC_Address(netip->mac);
  netif_add(&netif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernet_input);
  netif_set_default(&netif);
  netif_set_up(&netif);
	
	// dns handler
	#if LWIP_DNS 
	dns_init();
	#endif
	
	// dhcp handler
	netip_dhcp_handler();
	
	
	
}



int socket_open(int port,void (* recv)(void *arg, struct udp_pcb *upcb, struct pbuf *p,
                       struct ip_addr *addr, u16_t port),int igmp_flag)
{
	struct udp_pcb *upcb; 
	int s = MAX_SOCKET;
	int i;
	struct ip_addr ipgroup;

	for (i = 0; i < MAX_SOCKET; i++)
	{
		if (netps[i].status == 0)
		{
			netps[i].status = 1;
			s = i;
			break;
		}
	}
	if (s == MAX_SOCKET) return -1;
	
	upcb = udp_new();
	netps[s].upcb = upcb;
	if (igmp_flag)
	{
		IP4_ADDR(&ipgroup, 224,0,168,188);
		igmp_joingroup(IP_ADDR_ANY,(struct ip_addr *)(&ipgroup));
		udp_bind(upcb, &ipgroup, port);
	}
  udp_bind(upcb, IP_ADDR_ANY, port);
  udp_recv(upcb, recv, NULL);
	 
	return s;
}


void socket_close(int s)
{
	struct udp_pcb *upcb; 
	
	if (s >= MAX_SOCKET) return;
	
	upcb = netps[s].upcb;
	udp_remove(upcb);
	netps[s].status = 0;
}


#if 1
// TCP
#include "string.h"

struct tcp_pcb *TcpPCB;

/******************************************************************************

******************************************************************************/
static err_t tcp_client_recv(void *arg, struct tcp_pcb *tpcb,
                             struct pbuf *p, err_t err)
{
    uint32_t i;
    
    if (p != NULL)
    {
//        struct pbuf *ptmp = p;
        
//        printf("get msg from %d:%d:%d:%d port:%d:\r\n",
//            *((uint8_t *)&tpcb->remote_ip.addr),
//            *((uint8_t *)&tpcb->remote_ip.addr + 1),
//            *((uint8_t *)&tpcb->remote_ip.addr + 2),
//            *((uint8_t *)&tpcb->remote_ip.addr + 3),
//            tpcb->remote_port);
//        
//        while(ptmp != NULL)
//        {
//            for (i = 0; i < p->len; i++)
//            {
//                printf("%c", *((char *)p->payload + i));
//            }
//            
//            ptmp = p->next;
//        }
			
				
        
        tcp_recved(tpcb, p->tot_len);
        
        pbuf_free(p);
    }
    else if (err == ERR_OK)
    {
        tcp_recved(tpcb, p->tot_len);
        
        return tcp_close(tpcb);
    }

    return ERR_OK;
}

/******************************************************************************

******************************************************************************/
static err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
   // printf("tcp client connected\r\n");
    
    //tcp_write(tpcb, "tcp client connected", strlen("tcp client connected"), 0);

   // tcp_recv(tpcb, tcp_client_recv);
	
    return ERR_OK;
}


/******************************************************************************

******************************************************************************/
int tcp_send(unsigned char *frame,int len)
{
  err_t status;
	
  status = tcp_write(TcpPCB, frame, len, 1);
	tcp_output(TcpPCB); 
	if (status == ERR_OK)
   return 0;
	else
	 return -1;
  
}


/******************************************************************************

******************************************************************************/
#define TCP_REMOTE_PORT    3310 
#define TCP_LOCAL_PORT     8880
int tcpfd;
void tcp_client_init(void)
{
	#if 1
    struct tcp_pcb *tpcb;
    struct ip_addr serverIp;
		
    IP4_ADDR(&serverIp, 192, 168, 1, 26);

    tpcb = tcp_new();  
    if (tpcb != NULL)
    {
        err_t err;
        
        err = tcp_bind(tpcb, IP_ADDR_ANY, TCP_LOCAL_PORT);

        if (err == ERR_OK)
        {
            tcp_connect(tpcb, &serverIp, TCP_REMOTE_PORT, tcp_client_connected);
						TcpPCB = tpcb;
						tcp_recv(tpcb, tcp_client_recv);
        }
        else
        {
            memp_free(MEMP_TCP_PCB, tpcb);
            
          //  printf("can not bind pcb\r\n");
        }
    }
		
		#endif
		
}






struct hostent *hp;
char kk[20];
void do_tcp_test(void)
{
	static char status = 0;
	int arg;
	
	switch (status)
	{
		case 0:
			tcpfd = linux_open(1,"192.168.1.26",3310,1,0);
		//	tcp_client_init();
			
			//dns_init();
		
			//dns_ipaddr = dns_getserver(0);

		 
			//hp = linux_gethostbyname("www.baidu.com");
		//	if (hp != NULL)
		//	if (dns_gethostbyname("www.baidu.com",&domain_ipaddr,&user_dns_found,(void *)&arg) == 0)
			{
			
			//	hp->h_addr = (char *)&kk[0];
			//	memcpy(kk, inet_ntoa(*(struct in_addr *)hp->h_addr), 16);
				//memcpy(kk,inet_ntoa(hp->h_addr),4);
				status++;//domain_flag = 3;
			}
			break;
		
		case 1:
			linux_send(tcpfd,"hello",5);
		//	if (tcp_send("hello",5))
		//		status = 0;
			break;
	}
}

#endif

#endif


#endif
