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

#include "MQTTLinux.h"



#define SHUT_RD    		0
#define	SHUT_WR				1
#define SHUT_RDWR 		2

extern char netip_tx_buffer[];
extern char netip_rx_buffer[];

unsigned char *wbuffer = (unsigned char *)netip_tx_buffer;
unsigned char *rbuffer = (unsigned char *)netip_rx_buffer;

extern unsigned long ticks;
#define NTimeMS()			ticks

static unsigned short local_port = 0;

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

int linux_read(struct Network* n, unsigned char* buffer, int len, int timeout_ms)
{
//	struct timeval interval = {timeout_ms / 1000, (timeout_ms % 1000) * 1000};
//	if (interval.tv_sec < 0 || (interval.tv_sec == 0 && interval.tv_usec <= 0))
//	{
//		interval.tv_sec = 0;
//		interval.tv_usec = 100;
//	}
//	setsockopt(n->my_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&interval, sizeof(struct timeval));

	unsigned long wwul1 = NTimeMS();
  unsigned long wtimeout_ms = 1000;

	int bytes = 0;
	while (bytes < len)
	{
		//YIELD();
		int rc = recv(n->my_socket, (char *)&buffer[bytes], (size_t)(len - bytes), 0);
		if (rc == -1)
		{
			;
//			if (errno != ENOTCONN && errno != ECONNRESET)
//			{
//				bytes = -1;
//				break;
//			}
		}
		else if (rc == 0)
		{
			break; 
		}
		else
		{
			bytes += rc;
		}
		n->mqtthandler();
		
		if (abs(NTimeMS() - wwul1) >= wtimeout_ms)
		{
			break;
		}
	}
	return bytes;
}


int linux_write(struct Network* n, unsigned char* buffer, int len, int timeout_ms)
{
//	struct timeval tv;

//	tv.tv_sec = 0;  /* 30 Secs Timeout */
//	tv.tv_usec = timeout_ms * 1000;  // Not init'ing this can cause strange errors

//	setsockopt(n->my_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));

	//int	rc = write(n->my_socket, buffer, len);
	
 	//YIELD();
	linux_send_enable = 1;
	int rc = send(n->my_socket, (char *)buffer, len,0);
	linux_send_enable = 0;
	return rc;
}

struct hostent *linux_gethostbyname(char *hnp)
{
	struct hostent *host;
	linux_send_enable = 1;
	host = gethostbyname(hnp);
	linux_send_enable = 0;
	return host;
}





int linux_open(int type,char *addr,int port,int isclient,int ismcast)
{
	extern unsigned int netip_get_type();
	int mode = 1;
	int opt = 1;
	int sockfd;
	struct sockaddr_in server;
	
	if(netip_get_type() == 0) return -1;
	
	
	if (type)
	{
		struct netconn *conn, *newconn;
		err_t err;
		/* Create a new connection identifier. */
		conn = netconn_new(NETCONN_TCP);

		if (isclient)
		{
			//client
			
		}
		else
		{
			//server
			
			
		}
	}
	else
	{
		
		
		
		YIELD();
		// open socket
		memset((char *) &(server), 0, sizeof(struct sockaddr_in));
		server.sin_family   = AF_INET;
		server.sin_port 	= htons(port);
		server.sin_addr.s_addr 	= htonl(INADDR_ANY);
		sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		
		
		setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(int));
		bind(sockfd, (struct sockaddr *)&server,sizeof(struct sockaddr_in));
		setsockopt(sockfd,SOL_SOCKET,SO_BROADCAST,0,0); 
	 
		if (ismcast)
		{
			struct ip_mreq mcast;
			mcast.imr_multiaddr.s_addr = inet_addr(addr);  
			mcast.imr_interface.s_addr = htonl(INADDR_ANY);     
			setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mcast, sizeof(mcast));	
		}
		
		SOCKET_NOBLOCK(sockfd);
	}
	return sockfd;
}

int linux_accept(int s, struct sockaddr * name, int *namelen)
{
	//YIELD();
	return accept(s,name,namelen);
	
}

int linux_recv(int s,unsigned char *buffer,int len)
{
	//YIELD();
	return recv(s, (char *)buffer, len,0);
}

int linux_recvfrom(int s, char *buf, int len,struct sockaddr * from, int *fromlen)
{
	
	YIELD();
	if (SOCKET_HASDATA(s))
	{
		return recvfrom(s,buf, len, 0,from, fromlen);
	}
	return 0;
}

int linux_send(int s,unsigned char *buffer,int len)
{
	int rc;
	
	YIELD();
	linux_send_enable = 1;
	rc = send(s, (char *)buffer, len,0);
	linux_send_enable = 0;
	return rc;
}

int linux_sendto(int s,unsigned int ipaddr,int port,unsigned char *buffer,int len)
{
	int rc;
	struct sockaddr_in to;
	
  YIELD();
  to.sin_family = AF_INET;  
  to.sin_port = htons(port);
  to.sin_addr.s_addr = ipaddr;
  linux_send_enable = 1;  
  rc = sendto(s, (char*)buffer,len, 0, (struct sockaddr*)&to, sizeof(to)); 
	linux_send_enable = 0;
	return rc;
}

void linux_close(int s)
{
	shutdown(s, SHUT_RDWR);
	recv(s, NULL, (size_t)0, 0);
	closesocket(s);
}






void NetworkInit(Network* n)
{
	n->my_socket = -1;
	n->mqttread = linux_read;
	n->mqttwrite = linux_write;
	n->mqttdisconnect = NetworkDisconnect;
	n->mqtthandler = linux_handler;
}



int NetworkConnect(struct Network* n, unsigned int addr, int port)
{
	
	int rc = -1;
	int mode = 1;
	struct sockaddr_in server;
	struct sockaddr_in localAddr;
	
	
	YIELD();
	n->my_socket = socket(AF_INET, SOCK_STREAM, 0);

	if (local_port == 0)
		local_port = 6942 + rand()%1000;
	local_port++;
	if (local_port > 60000)
		local_port = 7000;
	if (local_port == 6000 || local_port == 6006)
		local_port++;
	
  localAddr.sin_family = AF_INET;
  localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  localAddr.sin_port = htons(local_port);
  bind(n->my_socket, (struct sockaddr*) &localAddr, sizeof(localAddr));
//	
	//server
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = addr;//inet_addr(addr);//inet_addr("192.168.1.26");
	
	ioctlsocket(n->my_socket,FIONBIO,&mode);
	connect(n->my_socket, (struct sockaddr *)&server, sizeof(struct sockaddr_in));
	
//	unsigned long wwul1 = NTimeMS();
//	int rlen;
//	
//	while (abs(NTimeMS() - wwul1) < 3000)
//	{
//		rlen = recv(n->my_socket, (char *)&rbuffer[0], 1500 , 0);
//		if (rlen > 0)
//		{
//			rc = 0;
//			break;
//		}
//		
//		n->mqtthandler();
//	}
	return rc;
}



void NetworkDisconnect(struct Network* n)
{

	shutdown(n->my_socket, SHUT_RDWR);
	recv(n->my_socket, NULL, (size_t)0, 0);
	closesocket(n->my_socket);
	//close(n->my_socket);
}
