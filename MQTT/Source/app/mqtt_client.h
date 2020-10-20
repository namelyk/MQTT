#ifndef _MQTT_CLIENT_H_
#define _MQTT_CLIENT_H_


#define MQTT_CLIENT_ENABLE
#ifdef MQTT_CLIENT_ENABLE
#include "MQTTClient.h"


extern int mqtt_client_set(char *clientid,unsigned int ipaddr,int port,char *username,char *password);
extern int mqtt_client_subscribe(const char* topicName);
extern int mqtt_client_unsubscribe(const char* topicName);
extern int mqtt_client_publish(const char* topicName, char* pubmsg,int msgsize, unsigned char send_level); //2019-11-1添加一个等级

extern int mqtt_client_init(void);
extern int mqtt_client_close(void);
extern int mqtt_client_handler(void);
extern void mqtt_client_test(void);
extern int mqtt_keepalive(void);



#endif
#endif

