/*******************************************************************************
									MQTT协议栈(只支持Qos0)
 *******************************************************************************/
#include "MQTTClient.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "iot-config.h"
#include <errno.h>
#include "MQTTLinux.h"
/***************时间片********************/
extern unsigned long ticks;
#define NTimeMS()			ticks
enum{
	
	FAILURE = -1,
	SUCCESS_OK = 0,
};
const unsigned char keepalive_buffer[2] = {0xc0,0x00};//心跳包
//int read_point;
extern unsigned char netip_tx_buffer[];
extern unsigned char netip_rx_buffer[];


unsigned char *tx_buffer = NULL;
unsigned char *rx_buffer = NULL;
int MYMQTTPublish(Network *ifnet, unsigned char *topic, int topic_len, unsigned char *message, int msg_len)
{
	int rc;
	int msg_start;
	int all_len = 2 + topic_len + msg_len;
	//可以选择去掉  稳定的话 限制发送MQTT的包
//	static unsigned long wwul1 = 0;
//	static int packet_num = 0;
//	if (abs(NTimeMS() - wwul1) < 200) 
//	{
//		packet_num++;
//		if(packet_num > 11)
//		{
//			return SUCCESS_OK;
//		}
//	}
//	else
//	{
//		packet_num = 0;
//		wwul1 = NTimeMS();
//	}
	//memset(netip_tx_buffer, 0, 1500);
	netip_tx_buffer[0] = 0x30;
	if(2 + topic_len + msg_len <= 127)
	{
		netip_tx_buffer[1] = all_len;
		msg_start = 2;
	}
	else
	{
		netip_tx_buffer[1] = all_len%128 + 128;
		netip_tx_buffer[2] = all_len/128;
		msg_start = 3;
	}
	
	netip_tx_buffer[msg_start] = topic_len>>8;
	netip_tx_buffer[msg_start + 1] = topic_len&0xff;
	memcpy(&netip_tx_buffer[msg_start + 2], topic, topic_len);
	memcpy(&netip_tx_buffer[msg_start + 2 + topic_len], message, msg_len);
	
	rc = Linux_write(ifnet, netip_tx_buffer, msg_start + all_len, 0/*TimerLeftMS(timer)*/);
	if(rc < 0)
	{
		return FAILURE;
	}
	return SUCCESS_OK;
}

extern Network ifnet;
extern char mqtt_recv_flag;
int MYMQTTConnect(Network *ifnet, unsigned short heartbeat, char *client_ID, char *username, char *password)
{
	int current_len = 0, rc = FAILURE;
	int client_len = strlen(client_ID);
	int user_len = strlen(username);
	int password_len = strlen(password);
	int all_len = 10 + 6 + client_len + user_len + password_len; 
	unsigned char msg_start = 0;
	unsigned long wwul1;

	//memset(netip_tx_buffer, 0, 1500);
	netip_tx_buffer[0] = 0x10;
	if(all_len <= 127)
	{
		netip_tx_buffer[1] = all_len;
		msg_start = 2;
	}
	else
	{
		netip_tx_buffer[1] = all_len%128 + 128;
		netip_tx_buffer[2] = all_len/128;
		msg_start = 3;
	}
	
//程序 版本 7字节
	netip_tx_buffer[msg_start] = 0x00;
	netip_tx_buffer[msg_start+1] = 0x04;//程序名字长度
	memcpy(&netip_tx_buffer[msg_start+2], "MQTT", 4);
	netip_tx_buffer[msg_start+6] = 0x04;//版本4
//连接标识符 心跳 3字节
	netip_tx_buffer[msg_start+7] = 0xc2; //连接标志位
	netip_tx_buffer[msg_start+8] = heartbeat>>8;
	netip_tx_buffer[msg_start+9] = heartbeat&0xff; //心跳时间
//client ID (2 + client_len)
	netip_tx_buffer[msg_start+10] = client_len>>8;
	netip_tx_buffer[msg_start+11] = client_len&0xff;
	memcpy(&netip_tx_buffer[msg_start+12], client_ID, client_len);
//user Name	(2 + user_len)
	current_len = msg_start + 12 + client_len;
	netip_tx_buffer[current_len] = user_len>>8;
	netip_tx_buffer[current_len + 1] = user_len&0xff;
	memcpy(&netip_tx_buffer[current_len + 2], username, user_len);
//password (2 + password_len)
	current_len = current_len + 2 + user_len;
	netip_tx_buffer[current_len] = password_len>>8;
	netip_tx_buffer[current_len + 1] = password_len&0xff;
	memcpy(&netip_tx_buffer[current_len + 2], password, password_len);

	rc = Linux_write(ifnet, netip_tx_buffer, msg_start + all_len, 0/*TimerLeftMS(timer)*/);
	if(rc < 0)
	{
		return FAILURE;
	}
	
	wwul1 = NTimeMS();
	mqtt_recv_flag = 0;
	while (abs(NTimeMS() - wwul1) < 6000)
	{
		if (mqtt_recv_flag)
			break;
		linux_handler();//防止在while循环的时候，其它功能停止
	}
	if (mqtt_recv_flag)
		return SUCCESS_OK;
	else
		return FAILURE;
}

unsigned int subcribe_number = 1;
int MYMQTTSubcribe(Network *ifnet, unsigned char *topic, int topic_len)
{
	int rc = FAILURE;
	int all_len = topic_len + 5; 
	unsigned char msg_start = 0;
	unsigned long wwul1;

	//memset(netip_tx_buffer, 0, 1500);
	netip_tx_buffer[0] = 0x82;
	if(all_len <= 127)
	{
		netip_tx_buffer[1] = all_len;
		msg_start = 2;
	}
	else
	{
		netip_tx_buffer[1] = all_len%128 + 128;
		netip_tx_buffer[2] = all_len/128;
		msg_start = 3;
	}
//
	netip_tx_buffer[msg_start] = subcribe_number>>8;
	netip_tx_buffer[msg_start+1] = subcribe_number&0xff;
	netip_tx_buffer[msg_start+2] = topic_len>>8;
	netip_tx_buffer[msg_start+3] = topic_len&0xff;
	memcpy(&netip_tx_buffer[msg_start+4], topic, topic_len);
	netip_tx_buffer[msg_start+4+topic_len] = 0; //等级0
	subcribe_number++;
	rc = Linux_write(ifnet, netip_tx_buffer, msg_start + all_len, 0/*TimerLeftMS(timer)*/);
	if(rc < 0)
	{
		return FAILURE;
	}
	
	wwul1 = NTimeMS();
	mqtt_recv_flag = 0;
	while (abs(NTimeMS() - wwul1) < 6000)
	{
		if (mqtt_recv_flag)
			break;
		linux_handler();//防止在while循环的时候，其它功能停止
	}
	if (mqtt_recv_flag)
		return SUCCESS_OK;
	else
		return FAILURE;
}

int MYMQTTKeepalive(Network *ifnet)
{
	Linux_write(ifnet, (unsigned char *)keepalive_buffer, 2, 0);
	return 0;
}

int MYMQTTHandler(unsigned char *ReadBuffer, int ReadBufferLen)
{	
	int current_position = 0;
#ifdef IOT_ENABLE
	if(iot_config_interface.get_status()!= 3) 
	{
		return 0;
	}
#endif
	int all_len = 0;
	int topic_len;
	int packet_len;
	while(1)
	{
		if(current_position + 5 >= ReadBufferLen) 
		{
			return 0;
		}
		unsigned char *pReadBuffer = &ReadBuffer[current_position];
		unsigned char mqtt_start_position;
		if(pReadBuffer[0] == 0x30)  //发布
		{
			if(pReadBuffer[1] <= 127) //包数量低于128
			{
				all_len = pReadBuffer[1];
				topic_len = pReadBuffer[3]; //包小于128 主题长度不可能超过128		
				mqtt_start_position = 2;
			}
			else
			{
				all_len = pReadBuffer[1] + (pReadBuffer[2]-1)*128;
				topic_len = pReadBuffer[3]*256 + pReadBuffer[4];
				mqtt_start_position = 3;
			}
			//剩余长度不够的问题
			if((all_len + mqtt_start_position) > (ReadBufferLen - current_position))
			{
				return 0;
			}
#ifdef IOT_ENABLE			
			packet_len = all_len - topic_len - 2;
			if(iot_config_interface.get_status()!= 3) 
			{
				return 0;
			}
			Read_callback((char *)&pReadBuffer[2 + mqtt_start_position], topic_len, (char *)&pReadBuffer[2 + mqtt_start_position + topic_len], packet_len);
			current_position += all_len + mqtt_start_position;
			if(current_position + 5 >= ReadBufferLen) 
			{
				return 0;
			}
#endif
		}
		else
		{
			int i = 1;
			while( i < ReadBufferLen - current_position)
			{
				if(pReadBuffer[i] == 0x30)
				{
					break;
				}
				i++;
			}
			current_position += i;
		}
	}
}

//int MYMQTTInit(unsigned char *tx_buf, int tx_len, unsigned char *rx_buf, int rx_len)
//{
//	tx_buffer = tx_buf;
//	rx_buffer = rx_buf;
//}

