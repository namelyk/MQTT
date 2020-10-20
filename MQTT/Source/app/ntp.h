#ifndef _NTP_H_
#define _NTP_H_


//#define NTP_ENABLE
#ifdef NTP_ENABLE

#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


extern struct tm *tv;

extern void ntp_init(void);
extern int ntp_setserverip(int ntp_serverip);
extern int ntp_getserverip(void);
extern time_t ntp_gettime(void);
extern int ntp_setzone(int zone_hour,int zone_minute);
extern int ntp_getzone(int *zone_hour,int *zone_minute);
extern int ntp_request(void);

#endif
#endif

