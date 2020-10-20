#ifndef _HOTEL_NETIP_H_
#define _HOTEL_NETIP_H_



//#define HOTEL_ENABLE
#ifdef HOTEL_ENABLE
#include "unet.h"




///
extern void hotel_init(void);
extern void hotel_read(char *packet,int len);
extern void hotel_write(char *packet,int len);
extern int hotel_send(int s,unsigned long ipaddress,unsigned short port,char *packet,int len);
extern int hotel_sendto(char *packet,int len);

#endif
#endif

