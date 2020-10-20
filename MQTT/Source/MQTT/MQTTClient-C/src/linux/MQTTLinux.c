/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Allan Stockdill-Mander - initial API and implementation and/or initial documentation
 *******************************************************************************/
#include "netip.h"
#include "MQTTLinux.h"

#define SHUT_RD    		0
#define	SHUT_WR			1
#define SHUT_RDWR 		2

extern char netip_tx_buffer[];
extern char netip_rx_buffer[];


extern int system_ticks;
extern unsigned long ticks;
#define NTimeMS()			ticks

static unsigned short local_port = 0;

//============mqtt=============
#define MAX_SOCKET   6
#define DISABLE_INT()		__set_PRIMASK(1) 		
//\
//{ETH_DMAITConfig(ETH_DMA_IT_R,DISABLE);\
//ETH_DMAITConfig(ETH_DMA_IT_NIS,DISABLE);}
#define ENABLE_INT() 		__set_PRIMASK(0)
//\
{ETH_DMAITConfig(ETH_DMA_IT_R,ENABLE);\
	ETH_DMAITConfig(ETH_DMA_IT_NIS,ENABLE);}

struct net_socket netsock[MAX_SOCKET];
char TimerIsExpired(Timer* timer)
{
	//struct timeval now, res;
	//gettimeofday(&now, NULL);
	//timersub(&timer->end_time, &now, &res);	
	
	struct timeval res;
	struct timeval now;
	
	now.tv_sec = (NTimeMS())/1000;
	now.tv_usec = ((NTimeMS()) / 1000) * 1000;
	res.tv_sec = timer->end_time.tv_sec -  now.tv_sec;
	res.tv_usec = timer->end_time.tv_usec - now.tv_usec;
	
	return res.tv_sec < 0 || (res.tv_sec == 0 && res.tv_usec <= 0);
}


void TimerCountdownMS(Timer* timer, unsigned int timeout)
{
	//struct timeval now;
	//gettimeofday(&now, NULL);

	//struct timeval interval = {timeout / 1000, (timeout % 1000) * 1000};
	//timeradd(&now, &interval, &timer->end_time);

	struct timeval now;
	
	now.tv_sec = (NTimeMS())/1000;
	now.tv_usec = ((NTimeMS()) / 1000) * 1000;
	
	struct timeval interval = {timeout / 1000, (timeout % 1000) * 1000};
	
	timer->end_time.tv_sec = interval.tv_sec + now.tv_sec;
	timer->end_time.tv_usec = interval.tv_usec + now.tv_usec;
}


void TimerCountdown(Timer* timer, unsigned int timeout)
{
	//struct timeval now;
	//gettimeofday(&now, NULL);
	//struct timeval interval = {timeout, 0};
	//timeradd(&now, &interval, &timer->end_time);
	
	struct timeval now;
	
	now.tv_sec = (NTimeMS())/1000;
	now.tv_usec = ((NTimeMS()) / 1000) * 1000;
	struct timeval interval = {timeout, 0};
	
	timer->end_time.tv_sec = interval.tv_sec + now.tv_sec;
	timer->end_time.tv_usec = interval.tv_usec + now.tv_usec;
}


int TimerLeftMS(Timer* timer)
{
	//struct timeval now, res;
	//gettimeofday(&now, NULL);
	//timersub(&timer->end_time, &now, &res);
	
	struct timeval res;
	struct timeval now;
	
	now.tv_sec = (NTimeMS())/1000;
	now.tv_usec = ((NTimeMS()) / 1000) * 1000;
	
	res.tv_sec = timer->end_time.tv_sec -  now.tv_sec;
	res.tv_usec = timer->end_time.tv_usec - now.tv_usec;
	
	//printf("left %d ms\n", (res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_usec / 1000);
	return (res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_usec / 1000;
}


void TimerInit(Timer* timer)
{
	timer->end_time = (struct timeval){0, 0};
}

extern void RESET_WATCH_DOG(void);
static char linux_send_enable = 0;
void linux_tick(void)
{
	if (linux_send_enable)
	{
		RESET_WATCH_DOG();
	}
}


//==================================================
static err_t tcp_cb1(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    if (p != NULL)
    {
        
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

static err_t tcp_cb2(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    if (p != NULL)
    {
        
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


static err_t tcp_cb3(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    if (p != NULL)
    {
		//tpcb->remote_ip.addr;
		//tpcb->remote_port
		//p->payload
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


static err_t tcp_cb4(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    if (p != NULL)
    {
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


static err_t tcp_con(void *arg, struct tcp_pcb *tpcb, err_t err)
{
    //tcp_recv(tpcb, tcp_cb);
    return ERR_OK;
}

void udp_cb1(void *arg, struct udp_pcb *upcb, struct pbuf *p,struct ip_addr *addr, u16_t port)
{
	
}

void udp_cb2(void *arg, struct udp_pcb *upcb, struct pbuf *p,struct ip_addr *addr, u16_t port)
{
	
}

void udp_cb3(void *arg, struct udp_pcb *upcb, struct pbuf *p,struct ip_addr *addr, u16_t port)
{
	
}

void udp_cb4(void *arg, struct udp_pcb *upcb, struct pbuf *p,struct ip_addr *addr, u16_t port)
{
	
}




int linux_read(struct Network* n, unsigned char* buffer, int len, int timeout_ms)
{

	return 0;
}

int linux_write(struct Network* n, unsigned char* buffer, int len, int timeout_ms)
{
	err_t status;
	int s = n->my_socket;

	if (s >= MAX_SOCKET)
	{
		return -1;
	}

	DISABLE_INT();
	struct tcp_pcb *pcb = netsock[s].tpcb;
	if (tcp_write(pcb, buffer, len, 1) == ERR_CONN)
	{
		ENABLE_INT();
		return -1;
	}
	status = tcp_output(pcb); 
	ENABLE_INT();
	if (status == ERR_OK)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}



static char linux_dns_status = 0;
struct hostent s_hostent;
struct ip_addr s_hostent_addr;
const struct ip_addr *s_phostent_addr = &s_hostent_addr;
static void linux_dns_found(const char *name, struct ip_addr *ipaddr, void *callback_arg)
{
	//struct ip_addr *s_phostent_addr;
	s_hostent_addr = *ipaddr;
	//s_phostent_addr = &s_hostent_addr;
	s_hostent.h_name = (char*)name;
	//s_hostent.h_aliases = &s_aliases;
	//s_hostent.h_addrtype = AF_INET;
	s_hostent.h_length = sizeof(struct ip_addr);
	s_hostent.h_addr_list = (char**)&s_phostent_addr;

	linux_dns_status = 2;
}
extern unsigned long ticks;
#define NTimeMS()			ticks
struct in_addr linux_gethostbyname(char *hnp)
{
	int arg;
	struct in_addr in_addr = {0};
	in_addr.s_addr = 0;
	
	linux_dns_status = 0;

	linux_send_enable = 1;
#if LWIP_DNS 
	long wwul1 = NTimeMS();
	while (abs(NTimeMS() - wwul1) < 3000)
	{
		linux_handler();
		
		if (linux_dns_status == 0)
		{
			if (dns_gethostbyname(hnp,&s_hostent_addr,&linux_dns_found,(void *)&arg) == ERR_OK)
			{
				struct ip_addr *s_phostent_addr;
				s_phostent_addr = &s_hostent_addr;
				//s_hostent.h_name = (char*)name;
				//s_hostent.h_aliases = &s_aliases;
				//s_hostent.h_addrtype = AF_INET;
				s_hostent.h_length = sizeof(struct ip_addr);
				s_hostent.h_addr_list = (char**)&s_phostent_addr;
				in_addr.s_addr = s_hostent_addr.addr;
				linux_dns_status = 2;
			}
			else
			{
				linux_dns_status = 1;
				//break;
			}
		}
		else if (linux_dns_status == 2)
		{
			break;
		}
	}
#endif	
//	host = gethostbyname(hnp);
	linux_send_enable = 0;
	if (linux_dns_status == 2)
		in_addr.s_addr = s_hostent_addr.addr;
	
	return in_addr;

}



extern unsigned int netip_get_type(void);
#define UDP_CB_ADD(A)				udp_cb##A
#define TCP_CB_ADD(A)				tcp_cb##A

int linux_open(int type,char *addr,int port,int isclient,int ismcast)
{
	
	int sockfd = -1;
	struct ip_addr serverIP;
	int i;
	int s = MAX_SOCKET;
	struct ip_addr ipgroup;

	static char linux_setup = 0;
	if (!linux_setup)
	{
		linux_setup = 1;
		
		netsock[0].tcp_cb = TCP_CB_ADD(1);
		netsock[0].udp_cb = UDP_CB_ADD(1);
		netsock[1].tcp_cb = TCP_CB_ADD(2);
		netsock[1].udp_cb = UDP_CB_ADD(2);
		netsock[2].tcp_cb = TCP_CB_ADD(3);
		netsock[2].udp_cb = UDP_CB_ADD(3);
		netsock[3].tcp_cb = TCP_CB_ADD(4);
		netsock[3].udp_cb = UDP_CB_ADD(4);
		
	}
	
	if(netip_get_type() == 0) return -1;
	
	
	if (type)// tcp
	{
		if (isclient)// client
		{
			struct tcp_pcb *tpcb; 
			
			for (i = 0; i < MAX_SOCKET; i++)
			{
				if (netsock[i].status == 0)
				{
					netsock[i].status = 2;
					s = i;
					break;
				}
			}
			if (s == MAX_SOCKET) return -1;
			
		 // IP4_ADDR(&serverIp, 192, 168, 1, 26);
			serverIP.addr = (inet_addr(addr));//htonl(addr);
		
			tpcb = tcp_new();
			netsock[s].tpcb = tpcb;  
			
			if (tpcb != NULL)
			{
					err_t err;
				
					local_port++;
					if (local_port < 8000)
						local_port = 8000;
					
					err = tcp_bind(tpcb, IP_ADDR_ANY, local_port);
					if (err == ERR_OK)
					{
						if (tcp_connect(tpcb, &serverIP, port, tcp_con) == ERR_OK)
						{
							//tcp_recv(tpcb, netsock[s].tcp_cb);
							sockfd = s;
						}
						else
						{
							netsock[i].status = 0;
							memp_free(MEMP_TCP_PCB, tpcb);
						}
					}
					else
					{
						netsock[i].status = 0;
						memp_free(MEMP_TCP_PCB, tpcb);
					}
			}
		} 
	}
	else // udp
	{
		
		struct udp_pcb *upcb; 
		
		for (i = 0; i < MAX_SOCKET; i++)
		{
			if (netsock[i].status == 0)
			{
				netsock[i].status = 1;
				s = i;
				break;
			}
		}
		if (s == MAX_SOCKET) return -1;
		
		upcb = udp_new();
		netsock[s].upcb = upcb;
		
		if (ismcast)
		{
			IP4_ADDR(&ipgroup, 224,0,168,188);
			igmp_joingroup(IP_ADDR_ANY,(struct ip_addr *)(&ipgroup));
			udp_bind(upcb, &ipgroup, port);
		}
		if (udp_bind(upcb, IP_ADDR_ANY, port) == ERR_OK)
		{
			//udp_recv(upcb, netsock[s].udp_cb, NULL);
			sockfd = s;
		}
	}
	
	return sockfd;
}

int linux_accept(int s, struct sockaddr * name, int *namelen)
{
	
	return 0;
}

int linux_recv(int s,unsigned char *buffer,int len)
{

	return 0;
}

int linux_recvfrom(int s, char *buf, int len,struct sockaddr * from, int *fromlen)
{
	
	
	return 0;
}

int linux_send(int s,unsigned char *buffer,int len)
{
	err_t status;
	if (s >= MAX_SOCKET)
	{
		return -1;
	}

	struct tcp_pcb *pcb = netsock[s].tpcb;
	DISABLE_INT();
	if (tcp_write(pcb, buffer, len, 1) == ERR_CONN)
	{
		ENABLE_INT();
		return -1;
	}
	status = tcp_output(pcb);
	ENABLE_INT();
	if (status == ERR_OK)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}
	

int linux_sendto(int s,unsigned int ipaddr,int port,unsigned char *buffer,int len)
{
	struct sockaddr_in to;
	struct udp_pcb *upcb;                                 
	struct pbuf *p;
	int stat = -1;

	if (s >= MAX_SOCKET) 
	{
		return -1;
	}
	DISABLE_INT();

	to.sin_port = port;
	to.sin_addr.s_addr = ipaddr;
	
	upcb = netsock[s].upcb;
	udp_connect(upcb, (struct ip_addr *)&(to.sin_addr.s_addr), to.sin_port);
	p = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
	if (p != NULL)
	{
		p->payload = (void *)buffer;
		p->len = len;
		p->tot_len = len;
		stat = udp_send(upcb, p);
	}
	udp_disconnect(upcb);
	pbuf_free(p);

	ENABLE_INT();
	//linux_send_enable = 0;
	return stat;
}

void linux_close(int s)
{
	if (s >= MAX_SOCKET)
	{
		return;
	}
	DISABLE_INT();
	if (netsock[s].status == 2)
	{
		//tcp_abort(netsock[s].tpcb);
		tcp_close(netsock[s].tpcb);
		//tcp_free(netsock[s].tpcb);
	}
	else //udp
	{
		udp_remove(netsock[s].upcb);
	}
	ENABLE_INT();
	netsock[s].status = 0;
	
	return;
}
//===================MQTT TCP CONFIG============================
void NetworkInit(Network* n)
{
	n->my_socket = -1;
	n->mqttread = linux_read;
	n->mqttwrite = linux_write;
	n->mqttdisconnect = NetworkDisconnect;
	//n->mqtthandler = linux_handler;
	return;
}

int NetworkConnect(struct Network* n, unsigned int addr, int port)
{
	int rc = -1;
	struct in_addr server;
	char *server_ip;
	
	// IP4_ADDR(&serverIp, 192, 168, 1, 26);
	server.s_addr = addr;//htonl(addr);
	server_ip = inet_ntoa(server);

	rc = linux_open(1,server_ip,port,1,0);
	n->my_socket = rc; 

	return rc;
}

void NetworkDisconnect(struct Network* n)
{
	linux_close(n->my_socket);
	n->my_socket = -1;
	
	return;
}
