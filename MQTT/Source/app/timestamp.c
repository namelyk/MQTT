#include "timestamp.h"
#include "sys_include.h"

#ifdef TIMESTAMP_ENABLE

time_t datetime_to_timestamp(struct tm *datetime)
{
	time_t timep;    
	struct tm p;   

	p.tm_year = datetime->tm_year - 1900;
	p.tm_mon = datetime->tm_mon - 1;
	p.tm_mday = datetime->tm_mday;
	p.tm_hour = datetime->tm_hour;
	p.tm_min = datetime->tm_min;
	p.tm_sec = datetime->tm_sec;
	
	timep = mktime(&p);
	return timep;
}


typedef union {
    char   b[4];
    unsigned int   l;
}        Union_Time;
char * datetime_str_to_timestamp(char *datetime_str_in, int len, char * datetime_str_out)
{
	Union_Time timep;    
	struct tm p;   
	
	if(len < 6) return 0; 
	p.tm_year = datetime_str_in[0] + 100;  //year为减去2000的值
	p.tm_mon = datetime_str_in[1] - 1;
	p.tm_mday = datetime_str_in[2];
	p.tm_hour = datetime_str_in[3];
	p.tm_min = datetime_str_in[4];
	p.tm_sec = datetime_str_in[5];
	timep.l = mktime(&p);
	memcpy(datetime_str_out, timep.b, 4);
	return datetime_str_out;

}




int timestamp_to_datetime(time_t timestamp, struct tm *datetime)
{
	struct tm *p;
	
	p = localtime(&timestamp); 
	datetime->tm_year = p->tm_year + 1900;
	datetime->tm_mon = 1 + p->tm_mon;
	datetime->tm_mday = p->tm_mday;
	datetime->tm_hour = p->tm_hour;
	datetime->tm_min = p->tm_min;
	datetime->tm_sec = p->tm_sec;

	return 0;
}



time_t localtime_to_timestamp(int hour,int minute,int second,struct tm *datetime)
{
	time_t timep;
	
	timep = datetime_to_timestamp(datetime);
	timep -= ((3600 * hour) + (60 * minute) + second);
	
	return timep;
}


int timestamp_to_localtime(int hour,int minute,int second,time_t timestamp,struct tm *datetime)
{
	time_t timep;
	
	timep = timestamp + ((3600 * hour) + (60 * minute) + second);
	timestamp_to_datetime(timep,datetime);
	
	return 0;
}


#endif


