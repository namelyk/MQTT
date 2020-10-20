#include "driver.h"
#include "datetime.h"
#include "timestamp.h"

#ifdef DATETIME_ENABLE
#include "sys_include.h"


#define DATETIME_SET				 app_commands_send

struct datetime_t
{
	unsigned char year;
	unsigned char month;
	unsigned char day;
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
	unsigned char week;
};


static unsigned char datetime_enable = 0;
static int datetime_timeout = 0;
struct datetime_t datetime;
unsigned long tm_timestamp = 0;

//=======================User define API===============================


unsigned char datetime_get_week(int year,unsigned char month,unsigned char day)
{
	unsigned char table_week[12] ={0,3,3,6,1,4,6,2,5,0,3,5};
	unsigned char yearH,yearL;
	unsigned char temp2;
	
	yearH = year/100;	
	yearL = year%100;
	
	/*2000*/
	if (yearH > 19)	 	
	{
		yearL += 100;
	}
	/*1900*/
	temp2 = yearL + yearL/4;
	temp2 = temp2%7; 
	temp2 = temp2 + day + table_week[month-1];
	if (yearL%4 == 0 && month < 3)	
	{
		temp2--;
	}	
	return(temp2%7);
}

void datetime_broadcast(void)//60s广播一次
{
	unsigned char info[10];
	struct tm tmp_datetime;
	timestamp_to_datetime(tm_timestamp, &tmp_datetime);
	datetime.year = tmp_datetime.tm_year - 2000;
	datetime.month = tmp_datetime.tm_mon;
	datetime.day = tmp_datetime.tm_mday;
	datetime.hour = tmp_datetime.tm_hour;
	datetime.minute = tmp_datetime.tm_min;
	datetime.second = tmp_datetime.tm_sec;
	
	datetime.week = datetime_get_week((int)(datetime.year+2000),datetime.month,datetime.day);
	info[0] = datetime.year;
	info[1] = datetime.month;
	info[2] = datetime.day;
	info[3] = datetime.hour;
	info[4] = datetime.minute;
	info[5] = datetime.second;
	info[6] = datetime.week;
	
	DATETIME_SET(255,0xDA44,0xff,0xff,info,7);
	
}


void datetime_going(void)
{

	datetime.second++;
	if (datetime.second > 59)
	{
		datetime.second = 0;
		datetime.minute++;
		if(datetime.minute > 59)
		{
			datetime.minute = 0;
			datetime.hour++;
			if (datetime.hour > 23)
			{
				datetime.hour = 0;
			}
		}
	}
}

	
int Timestamp_to_datetime(time_t timestamp)
{
	struct tm *p = NULL;
	struct datetime_t mydatetime = {0};
	unsigned long offset_datetime = 0;
	unsigned char info[7] = {0};
	int area_hour, area_minute;
	
	if(0 == netip_gateway_get_hour_minute_area(&area_hour, &area_minute))
	{
		p = NULL;
		return 0;
	}
	datetime_enable = 1;
	datetime_timeout = 0;
	
	if(area_hour >= 24) area_hour = 0;
	if(area_minute >= 60)	area_minute = 0;
	if(area_hour >= 12) //+
	{
		offset_datetime = (area_hour - 12) * 3600 + area_minute * 60 ;
		tm_timestamp = timestamp + offset_datetime;
	}
	else 
	{
		offset_datetime = (12 - area_hour) * 3600 + area_minute * 60 ;
		tm_timestamp = timestamp - offset_datetime;
	}
	return 0;
}
//yk 2020-05-27加
void set_datetime_enable(unsigned char enable)
{
	if(enable >= 1) 
		datetime_enable = 1;
	else 
		datetime_enable = 0;
}	


void datetime_get_localtime(unsigned char area_hour,unsigned char area_minute,unsigned char *buffer)
{
	unsigned char time_area;
	struct tm tm_datetime;
	unsigned long offset_datetime = 0;
	if(area_hour == 0xff) return;
		
	datetime_enable = 1;
	datetime_timeout = 0;
	
	//datetime_to_timestamp();
	datetime.year = buffer[0];
	datetime.month = buffer[1];
	datetime.day = buffer[2];

	datetime.hour = buffer[3];
	datetime.minute = buffer[4];
	datetime.second = buffer[5];
	
	tm_datetime.tm_year = datetime.year + 2000;
	tm_datetime.tm_mon = datetime.month;
	tm_datetime.tm_mday = datetime.day;
	tm_datetime.tm_hour = datetime.hour;
	tm_datetime.tm_min = datetime.minute;
	tm_datetime.tm_sec = datetime.second;
	
	tm_timestamp = datetime_to_timestamp(&tm_datetime);
	
	if(area_hour >= 24) area_hour = 0;
	if(area_minute >= 60)	area_minute = 0;


	if(area_hour >= 12) //+
	{
		offset_datetime = (area_hour - 12) * 3600 + area_minute * 60 ;
		tm_timestamp += offset_datetime;
	}
	else 
	{
		offset_datetime = (12 - area_hour) * 3600 + area_minute * 60 ;
		tm_timestamp -= offset_datetime;
	}
}

//=======================Standard define API===============================
static int _get(unsigned char *config,int len)
{
	config[0] = datetime.year;
	config[1] = datetime.month;
	config[2] = datetime.day;

	config[3] = datetime.hour;
	config[4] = datetime.minute;
	config[5] = datetime.second;
	config[6] = datetime.week;
	
	return 0;
}

static int _set(unsigned char *config,int len)
{
		return 0;
}

static void _init(void)
{
	datetime_enable = 0;
	datetime_timeout = 0;
}

static int _status(unsigned char *buffer,int len)
{
//	int index;
	
	return 0;
}

static int _read(unsigned char *buffer,int len)
{
		return 0;
}

static int _write(unsigned char *buffer,int len)
{
		return 0;
}

static int _handler(void *event)
{
		static char timeout = 0;
		static char counter = 0;
	
		if (datetime_enable == 0) 
			return 1;
		
		// 1s
		counter++;
		if (counter < 100) return 1;
		counter = 0;
		
		if (timeout)
		{
			timeout--;
			//datetime_going();
			tm_timestamp++; 
		}
		else
		{
			datetime_timeout++;
			if (datetime_timeout > 3)
			{
				datetime_enable = 0;
				datetime_timeout = 0;
			}
			timeout = 60;
			datetime_broadcast();
		}
		return 0;
}


//====================low interface=====================
const struct driver_interface_t datetime_interface =
{
	_get,_set,_init,_status,_read,_write,_handler
};


#endif
