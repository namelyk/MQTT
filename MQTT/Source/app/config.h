#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "buspro_params.h"

//device type
//#define DEVICE_TYPE						9600
//#define DEVICE_ENCODE					"HDL-2580A59A"
#define DEVICE_TYPE						4605
#define DEVICE_ENCODE					"HDL-11FDEC9B"

//#define DEVICE_TYPE						4611
//#define DEVICE_ENCODE					"HDL-120311F0"

//software version
#define SOFTWARE_VERSION			"HDL_V05.24U_2020/10/19"//9 02更改子网掩码的问题 10-14更改中心服务器反馈错误处理机制，和获取密码数组越界问题

#define SOFTWRAE_REAL_VERSION		"V05.24U" //MQTT用  旧云端版本
#define MCU_CHIP					"STM32F107VCT6"
#define PCB_VERSION						"0892D"

#endif
