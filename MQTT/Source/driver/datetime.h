#ifndef _DATE_TIME_H_
#define _DATE_TIME_H_

#define DATETIME_ENABLE
#ifdef DATETIME_ENABLE
#include "time.h"

extern const struct driver_interface_t datetime_interface;
extern int Timestamp_to_datetime(time_t timestamp);
//yk 2020-05-27¼Ó
void set_datetime_enable(unsigned char enable);

#endif
#endif
