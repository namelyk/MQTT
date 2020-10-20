#include "ntp.h"
//#include "unet.h"





#ifdef NTP_ENABLE

// config linux
#define SOCKET_SELECT_STD	//
#ifdef SOCKET_SELECT_STD
#include "MQTTLinux.h"

extern char netip_tx_buffer[];
extern char netip_rx_buffer[];

unsigned char *ntp_wbuf = (unsigned char *)netip_tx_buffer;
unsigned char *ntp_rbuf = (unsigned char *)netip_rx_buffer;

extern unsigned long ticks;
#define NTimeMS()			ticks

#endif



//================================================================

#define NTP_PORT               		123  
#define TIME_PORT              		37     
#define NTP_SERVER_IP       			"17.253.84.253"//"210.72.145.44"
#define NTP_PORT_STR        			"123"
#define NTPV1                			"NTP/V1"
#define NTPV2                			"NTP/V2"
#define NTPV3                			"NTP/V3"
#define NTPV4                			"NTP/V4"
#define TIME                			"TIME/UDP"
 
#define NTP_PCK_LEN 48
#define LI 0
#define VN 3
#define MODE 3
#define STRATUM 0
#define POLL 4
#define PREC -6
 
#define TIMESTAMP_1970 						0x83aa7e80 				/* 1900 ~ 1970 s*/
#define NTPFRAC(x)     						(4294 * (x) + ((1981 * (x)) >> 11))
#define USEC(x)         					(((x) >> 12) - 759 * ((((x) >> 10) + 32768) >> 16))


#define NC4(X)										(((X >> 24) & 0X000000FF) | ((X >> 8) & 0X0000FF00) | ((X << 8) & 0X00FF0000) | ((X << 24) & 0XFF000000))

#pragma pack(1)
typedef struct _ntp_time
{
    unsigned int coarse;
    unsigned int fine;
}ntp_time;
 
struct ntp_packet
{
     unsigned char leap_ver_mode;
     unsigned char startum;
     char poll;
     char precision;
     int root_delay;
     int root_dispersion;
     int reference_identifier;
     ntp_time reference_timestamp;
     ntp_time originage_timestamp;
     ntp_time receive_timestamp;
     ntp_time transmit_timestamp;

};

#pragma pack()

static int ntp_socket = -1;
static int ntp_server_ipaddr;
static time_t local_timestamp;
static int local_zone_hour = 0;
static int local_zone_minute = 0;
struct tm *tv;



void ntp_handler(char *buffer,int len);

//===========================================================
unsigned long ntp_inet_addr(const char *addr)
{
	unsigned long temp , addr_temp;
	char  i,j;
	int addr_size;
	
	addr_temp = 0;
	temp = 0;
	j = 0;
	
	addr_size =strlen(addr);
	for(i = 0; i < addr_size; i++)
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

void ntp_init(void)
{

	//test
	ntp_server_ipaddr = ntp_inet_addr(NTP_SERVER_IP);
	ntp_setzone(8,0);
}


// set server ip
int ntp_setserverip(int ntp_serverip)
{
	ntp_server_ipaddr = ntp_serverip;
	return 0;
}

// get server ip
int ntp_getserverip(void)
{
	return ntp_server_ipaddr;
}


time_t ntp_gettime(void)
{
	return (local_timestamp);
}	

int ntp_setzone(int zone_hour,int zone_minute)
{
	local_zone_hour = zone_hour;
	local_zone_minute = zone_minute;
	return 0;
}


int ntp_getzone(int *zone_hour,int *zone_minute)
{
	*zone_hour = local_zone_hour;
	*zone_minute = local_zone_minute;
	return 0;
}	

int ntp_request(void)
{
#ifdef SOCKET_SELECT_STD
	struct ntp_packet packet;
	char *localAddr;
	
	ntp_socket = linux_open(0,	localAddr, 6886,0,0);
	if (ntp_socket < 0)return 1;
	
	memset((char *)&packet,0,sizeof(struct ntp_packet));
	packet.leap_ver_mode = 0xA3;
	linux_sendto(ntp_socket,ntp_server_ipaddr,NTP_PORT,(unsigned char *)&packet,sizeof(struct ntp_packet));
	
	// wait for
	unsigned long wwul1 = NTimeMS();
	int len,flen;
	struct sockaddr from;
	
	while (abs(NTimeMS() - wwul1) < 2000)
	{
		len = linux_recvfrom(ntp_socket, (char *)&ntp_rbuf[0], 1500,&from,&flen);
		
		if (len > 0)
		{
			ntp_handler((char *)ntp_rbuf,len);
			break;
		}
		
		linux_handler();
	}
#else
	struct ntp_packet packet;
	
	ntp_socket = socket_bind(6886,ntp_handler);
	if (ntp_socket < 0)return 1;
	memset((char *)&packet,0,sizeof(struct ntp_packet));
	packet.leap_ver_mode = 0xA3;
	udp_sendto(ntp_socket,ntp_server_ipaddr,NTP_PORT,(char *)&packet,sizeof(struct ntp_packet));
#endif
	
	return 0;
}



void ntp_handler(char *buffer,int len)
{
	time_t timep;    
	struct ntp_packet *packet;
	
#ifdef SOCKET_SELECT_STD	
	linux_close(ntp_socket);
#else
	socket_close(ntp_socket);
#endif

	packet = (struct ntp_packet *)buffer;
	timep = NC4(packet->transmit_timestamp.coarse);
  local_timestamp = timep - TIMESTAMP_1970;
  local_timestamp += (3600 * local_zone_hour) + (60 * local_zone_minute);
	
	tv = localtime(&local_timestamp); 
	tv->tm_year += 1900;
	tv->tm_mon += 1;

}


#endif

