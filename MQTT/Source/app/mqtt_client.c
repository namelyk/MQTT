#include "mqtt_client.h"
#include "gateway.h"
#include <stdio.h>
#include <string.h>
#include "iot-config.h"


/***************时间片********************/
extern unsigned long ticks;
#define NTimeMS()			ticks
#ifdef MQTT_CLIENT_ENABLE

unsigned char ReadBuffer[1600] = {0};
unsigned char Readflag = 0;
unsigned int ReadBufferLen;
int current_position = 0;
int RC_Point;

extern char HDL_MQTT_USE_MAC[17];//@from gateway.c 16Byte的ANSI MAC地址
extern char Sub_Topic_Name[100];//订阅主题
extern char Pub_Topic_Name[100];//发布主题
#pragma pack(1)
struct opts_struct
{
	char* clientid;
	unsigned char qos;
	char* username;
	char* password;
	unsigned int ipaddr;
	int port;

	int showtopics;
};

//struct opts_struct opts;
struct opts_struct opts =
{
	"BusproGateway_busprogatewayId1",
	0, 
	"busprogatewayId1", 
	"123456", 
	129/*inet_addr("192.168.1.26")*/,
	1883, 
	0
};
#pragma pack()
Network ifnet;
extern char netip_tx_buffer[];
extern char netip_rx_buffer[];
char mqtt_recv_flag = 0;
/******************************************************************
Name:		mqtt_client_set
Function:	用于配置MQTT的基本信息，客户端id 用户名 密码等等
Called By: iot_config_interface.handler() (@from iot-config.c)
Other:		clientid port ipaddr 和 qos有用到 其他没用
*******************************************************************/
int mqtt_client_set(char *clientid,unsigned int ipaddr,int port,char *username,char *password)
{
	opts.clientid = clientid;
	opts.username = username;
	opts.password = password;
	opts.port = port;
	opts.ipaddr = ipaddr;
	opts.qos = 0;
	opts.showtopics = 0;
	
	return 0;	
}
/*************订阅和取消订阅�************************/
int mqtt_client_subscribe(const char* topicName)
{
	return MYMQTTSubcribe(&ifnet, (unsigned char *)topicName, strlen(topicName));
}

int mqtt_client_unsubscribe(const char* topicName)
{
	return 1;
}

/******************************************************************
Name:		mqtt_client_publish
Function:	重要的一个函数，用于发布消息到MQTT服务器
Called By: 	iot_config_interface.write() (@from iot-config.c) iot_config_interface.send() ……
Other:		nothing
*******************************************************************/
int mqtt_client_publish(const char* topicName, char* pubmsg,int msgsize, unsigned char send_level) //2019-11-1添加一个等级
{
	return MYMQTTPublish(&ifnet, (unsigned char *)topicName, strlen(topicName), (unsigned char *)pubmsg, msgsize);
}

// mqtt recvfrom callback
static err_t mqtt_client_recvfrom(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    if (p != NULL)
    {
		set_tcp_recv_flag(1);
		mqtt_recv_flag = 1;
		tcp_recved(tpcb, p->tot_len);  
		if (p->len < 1500)
		{
			memcpy(ReadBuffer,p->payload,p->len);
			if(ReadBuffer[0] == 0x20 && ReadBuffer[1] == 0x02 && ReadBuffer[2] == 0x00 && ReadBuffer[3] == 5)
			{
			#ifdef IOT_ENABLE
				iot_debug_reapply(); //重新申请
			#endif
			}
			
			RC_Point = p->len;
		}
		pbuf_free(p);
    }
    else if (err == ERR_OK)
    {
        tcp_recved(tpcb, p->tot_len);
	#ifdef IOT_ENABLE
		iot_debug_status(4);
	#endif
        return tcp_close(tpcb);
    }
    return ERR_OK;
}

/******************************************************************
Name:		mqtt_client_init
Function:	重要的一个函数，用于发布消息到MQTT服务器
Called By: 	iot_config_interface.write() (@from iot-config.c) iot_config_interface.send() ……
Other:		nothing
*******************************************************************/
int mqtt_client_init(void)
{
	int rc = -1;
#ifdef IOT_ENABLE
	iot_config_interface.getTopic();
#endif
	/*初始化网络*/
	NetworkInit(&ifnet);
	rc = NetworkConnect(&ifnet, opts.ipaddr, opts.port);
	if (rc < 0)return -1;
	tcp_recv(netsock[rc].tpcb, mqtt_client_recvfrom);
	rc = MYMQTTConnect(&ifnet, 60, opts.clientid, opts.username, opts.password);
	return rc;
}

/********关闭客户端*********/
extern int subcribe_number;
int mqtt_client_close(void)
{
	NetworkDisconnect(&ifnet);
	subcribe_number = 1;
	return 0;
}

int mqtt_keepalive(void)
{
	return MYMQTTKeepalive(&ifnet);
}

/*******接收数据处理****/
int mqtt_client_handler(void)
{
	if(RC_Point >= 15)
	{
		set_tcp_recv_flag(1);
		ReadBufferLen = RC_Point;
		MYMQTTHandler(ReadBuffer, ReadBufferLen);
		RC_Point = 0;
	}
	else if(RC_Point > 0) 
	{
		set_tcp_recv_flag(1);
		RC_Point = 0;
	}
	return 1;
}
#endif


