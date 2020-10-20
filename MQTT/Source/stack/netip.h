#ifndef _NETIP_H_
#define _NETIP_H_


#define NETIP_ENABLE

#ifdef NETIP_ENABLE
//#include "unet.h"

#include "stm32_eth.h"


#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "lwip/igmp.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "lwip/dns.h"
#include <string.h>
#include <stdio.h>
#include "ethernetif.h"


#define YIELD()	

#define Iid_SZ 4

typedef union {
    unsigned char   c[Iid_SZ];
    unsigned short  s[Iid_SZ / 2];
    unsigned long   l;
}Iid;

struct sockaddr {            
    unsigned short  sa_family;
    char            sa_data[14];
};



struct sockaddr_in {
    short           sin_family; 
    unsigned short  sin_port; 
    struct in_addr  sin_addr;
    char            sin_zero[8];
};


struct hostent {
    char  *h_name;      /* Official name of the host. */
    char **h_aliases;   /* A pointer to an array of pointers to alternative host names,
                           terminated by a null pointer. */
    int    h_addrtype;  /* Address type. */
    int    h_length;    /* The length, in bytes, of the address. */
    char **h_addr_list; /* A pointer to an array of pointers to network addresses (in
                           network byte order) for the host, terminated by a null pointer. */
#define h_addr h_addr_list[0] /* for backward compatibility */
};


//======================================================================
struct netip_interface_t
{
	void (*interrupt_enable)(void);
	void (*interrupt_disable)(void);
	void (*watchdog_clear)(void);
};



//=================================================
#pragma pack(1)

typedef union {
    unsigned char   c[4];
    unsigned short  s[2];
    unsigned long   l;
} UIid;

typedef union {
    unsigned char   b[4];
    unsigned long   l;
}  UIPaddr;


struct netip_t 
{
	UIPaddr addr;
	UIPaddr gw;
	UIPaddr submask;
	
	unsigned char mac[6];
	
	unsigned char dhcp;
	unsigned char status;
	
	unsigned char dns;
	
	UIPaddr dns1_addr;
	UIPaddr dns2_addr;
	//struct ip_addr dns1_addr;
	//struct ip_addr dns2_addr;
	struct ip_addr domain_ipaddr;
	char domain[65];
};

#pragma pack()


extern unsigned long remote_ipaddr;
extern unsigned short remote_port;



//===========================================================================
extern void netip_setup(struct netip_t *ipaddr);
extern int netip_set_type(unsigned int type,unsigned char *encode,\
					void (*callback)(int ,unsigned char*,int));
extern unsigned int netip_get_type(void);

extern void netip_reset(void);
extern void netip_init(void);
extern void netip_close(void);
extern void netip_read(char *packet, int len);
extern int netip_write(char *packet,int len);
extern int netip_p2p_write(unsigned int ipaddr,unsigned short port,char *packet,int len);
extern void netip_handler(void);

extern int netip_broadcast_send(char *packet,int len);
extern int netip_p2p_send(unsigned int ipaddr,unsigned short port,char *packet,int len);
extern int netip_multicast_send(char *packet,int len);
extern int netip_broadcast_all_send(char *packet,int len);

extern int netip_allbroadcast_send(unsigned int cmd,unsigned char src_subnet,unsigned char src_device, \
											unsigned char dest_subnet,unsigned char dest_device, \
											unsigned char *buffer,unsigned char len);
											
extern int netip_multi_send(unsigned int cmd,unsigned char src_subnet,unsigned char src_device, \
											unsigned char dest_subnet,unsigned char dest_device, \
											unsigned char *buffer,unsigned char len);
											
extern int netip_send(unsigned int cmd,unsigned char src_subnet,unsigned char src_device, \
											unsigned char dest_subnet,unsigned char dest_device, \
											unsigned char *buffer,unsigned char len);
											
extern int netip_ptp_send(unsigned int ipaddr,unsigned short port,\
				unsigned int cmd,unsigned char src_subnet,unsigned char src_device, \
											unsigned char dest_subnet,unsigned char dest_device, \
											unsigned char *buffer,unsigned char len);
											
extern int netip_long_send(unsigned int cmd,unsigned char src_subnet,unsigned char src_device, \
											unsigned char dest_subnet,unsigned char dest_device, \
											unsigned char *buffer,int len);
											
extern int netip_ptp_long_send(unsigned int ipaddr,unsigned short port,\
							unsigned int cmd,unsigned char src_subnet,unsigned char src_device, \
											unsigned char dest_subnet,unsigned char dest_device, \
											unsigned char *buffer,int len);

// low ether											
extern int netip_dns_gethostname(const char *hostname);
extern int socket_open(int port,void (* recv)(void *arg, struct udp_pcb *upcb, struct pbuf *p,
                       struct ip_addr *addr, u16_t port),int igmp_flag);
extern void socket_close(int s);

#endif
#endif
