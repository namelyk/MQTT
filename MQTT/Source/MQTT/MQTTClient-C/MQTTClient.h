#ifndef __MQTTCLIENT_H
#define __MQTTCLIENT_H


#include "MQTTLinux.h"
extern int MYMQTTConnect(Network *ifnet, unsigned short heartbeat, char *client_ID, char *username, char *password);
extern int MYMQTTPublish(Network *ifnet, unsigned char *topic, int topic_len, unsigned char *message, int msg_len);
extern int MYMQTTSubcribe(Network *ifnet, unsigned char *topic, int topic_len);
extern int MYMQTTKeepalive(Network *ifnet);
extern int MYMQTTHandler(unsigned char *ReadBuffer, int ReadBufferLen);


//暂时用宏来指定
#define Linux_write(ifnet, tx_buffer, len, timeout_ms)   linux_write(ifnet, tx_buffer, len, timeout_ms)
#define Read_callback(topic, topic_len, buffer, len)		iot_config_interface.read_callback(topic, topic_len, buffer, len)					


enum Qos{
	Qos0 = 0,
	Qos1,Qos2
};
#endif

