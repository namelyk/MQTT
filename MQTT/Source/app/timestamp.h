#ifndef _TIMESTAMP_H_
#define _TIMESTAMP_H_


#define TIMESTAMP_ENABLE
#ifdef TIMESTAMP_ENABLE
#include <time.h>


extern time_t datetime_to_timestamp(struct tm *datetime);
extern int timestamp_to_datetime(time_t timestamp, struct tm *datetime);

extern time_t localtime_to_timestamp(int hour,int minute,int second,struct tm *datetime);
extern int timestamp_to_localtime(int hour,int minute,int second,time_t timestamp,struct tm *datetime);

extern char * datetime_str_to_timestamp(char *datetime_str_in, int len, char * datetime_str_out);

#endif
#endif

