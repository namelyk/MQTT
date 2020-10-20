#ifndef MQTTAPP_H_ 
#define MQTTAPP_H_ 
 
void mqttAppInit(); 
void mqttAppConnect(); 
void mqttAppSend(); 
void mqttAppHandle(); 
void mqttAppDisconnect(); 
void mqttAppPublish(char *topic, char *data,int len); 
 
#endif /* MQTTAPP_H_ */
