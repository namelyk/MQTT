
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

//#if !defined(__MQTT_LINUX_)

#ifndef  __MQTT_LINUX_
#define __MQTT_LINUX_

//#if defined(WIN32_DLL) || defined(WIN64_DLL)
//  #define DLLImport __declspec(dllimport)
//  #define DLLExport __declspec(dllexport)
//#elif defined(LINUX_SO)
//  #define DLLImport extern
//  #define DLLExport  __attribute__ ((visibility ("default")))
//#else
//  #define DLLImport
//  #define DLLExport
//#endif

//#include <sys/types.h>
//#include <sys/socket.h>
//#include <sys/param.h>
//#include <sys/time.h>
//#include <sys/select.h>
//#include <netinet/in.h>
//#include <netinet/tcp.h>
//#include <arpa/inet.h>
//#include <netdb.h>
//#include <stdio.h>
//#include <unistd.h>
//#include <errno.h>
//#include <fcntl.h>

//#include <stdlib.h>
//#include <string.h>
//#include <signal.h>
//#include<stdio.h>
//#include<sys/time.h>
 
#include "netip.h"

struct timeval {              
    long            tv_sec;    
    long            tv_usec;    
};


typedef struct Timer
{
	struct timeval end_time;
} Timer;

void TimerInit(Timer*);
char TimerIsExpired(Timer*);
void TimerCountdownMS(Timer*, unsigned int);
void TimerCountdown(Timer*, unsigned int);
int TimerLeftMS(Timer*);

typedef struct Network
{
	int my_socket;
	int (*mqttread) (struct Network*, unsigned char*, int, int);
	int (*mqttwrite) (struct Network*, unsigned char*, int, int);
	void (*mqttdisconnect) (struct Network*);
	void (*mqtthandler)(void);
} Network;


typedef struct net_socket
{
	char status;
	struct udp_pcb *upcb;
	struct tcp_pcb *tpcb;
	
	err_t (*tcp_cb)(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
	void (*udp_cb)(void *arg, struct udp_pcb *upcb, struct pbuf *p,struct ip_addr *addr, u16_t port);
	
}net_socket;

extern struct net_socket netsock[];

extern void linux_tick(void);
extern int linux_read(Network*, unsigned char*, int, int);
extern int linux_write(Network*, unsigned char*, int, int);
extern void linux_disconnect(struct Network*);
extern void linux_handler(void);

extern void NetworkInit(struct Network*);
extern int NetworkConnect(struct Network*, unsigned int, int);
extern void NetworkDisconnect(struct Network*);

//extern struct hostent *linux_gethostbyname(char *hnp);
extern struct in_addr linux_gethostbyname(char *hnp);
extern int linux_open(int type,char *addr,int port,int isclient,int ismcast);  //type: 0 udp 1 tcp/isclient 1 客户端 0 服务端/ismcast: 1打开组播 0不打开组播

//extern int linux_accept(int s, struct sockaddr * name, int *namelen);
extern int linux_recv(int s,unsigned char *buffer,int len);
extern int linux_send(int s,unsigned char *buffer,int len);
//extern int linux_recvfrom(int s, char *buf, int len,struct sockaddr * from, int *fromlen);
extern int linux_sendto(int s,unsigned int ipaddr,int port,unsigned char *buffer,int len);
extern void linux_close(int s);

extern unsigned char get_tcp_recv_flag(void);  //用于确认连接是否正常
extern void set_tcp_recv_flag(unsigned char flag);	

#endif
