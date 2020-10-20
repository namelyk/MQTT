#include "hotel_netip.h"
#include "sys_include.h"



#ifdef HOTEL_ENABLE

#define HOTEL_PORT				(6006)




static int hotel_socket;
static unsigned short hotel_port;
extern unsigned long netip_broadcast;


void hotel_init(void)
{
	hotel_port = HOTEL_PORT;
	hotel_socket = socket_bind(hotel_port,hotel_read);
}



void hotel_read(char *packet,int len)
{
	//receive data
	
	
}


void hotel_write(char *packet,int len)
{
	
}


int hotel_send(int s,unsigned long ipaddress,unsigned short port,char *packet,int len)
{
	return udp_sendto(s,ipaddress,port,packet,len);
}


int hotel_sendto(char *packet,int len)
{
	return udp_sendto(hotel_socket,netip_broadcast,hotel_port+2,packet,len);
}


#endif

