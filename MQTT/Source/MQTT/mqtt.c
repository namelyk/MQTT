
#include "mqtt.h"   
#include <stdint.h>   
#include <string.h>   
   
   
extern volatile uint32_t msTime;   

extern char netip_tx_buffer[];

err_t recv_callback(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{   
    uint8_t *mqttData;   
   
    Mqtt *xmqtt = arg;   
   
    //uint8_t strLen = p->tot_len - 4;   
    //char out[strLen + 1];   
    /* Check if status is ok and data is arrived. */   
    if (err == ERR_OK && p != NULL) {   
        /* Inform TCP that we have taken the data. */   

        tcp_recved(pcb, p->tot_len);   
        mqttData = (uint8_t*)(p->payload);   
 
        uint8_t *topic = mqttData + 2 + 2;//ok   
        uint16_t topicLen = (mqttData[2] << 8) | mqttData[3];   
        uint8_t *data = &mqttData[2+2+topicLen];   
        uint32_t dataLen = p->tot_len - (2 + 2 + topicLen);   
   
        switch(mqttData[0] & 0xF0)   
        {   
        case MQTT_MSGT_PINGRESP:   
          //  UARTprintf("PingResp\n");   
            break;   
   
        case MQTT_MSGT_PUBLISH:
            xmqtt->msgReceivedCallback(xmqtt, topic, topicLen, data, dataLen);   
            break;   
   
        case MQTT_MSGT_CONACK:   
            xmqtt->connected = 1;   
            break;   
   
        default:   
          //  UARTprintf("default:\n");  
					break;
        }   
   
        //   
    }   
    else   
    {   
        /* No data arrived */   
        tcp_close(pcb);   
    }   
    pbuf_free(p);   
    return ERR_OK;   
}   
   
   
   
/* Accept an incomming call on the registered port */   
err_t accept_callback(void *arg, struct tcp_pcb *npcb, err_t err) {   
   
    //LWIP_UNUSED_ARG(arg);   
    Mqtt *xmqtt = arg;   
   
    //this->connected = 1;   
   
    /* Subscribe a receive callback function */   
    tcp_recv(npcb, recv_callback);   
   
    /* Don't panic! Everything is fine. */   
    return ERR_OK;   
}   
   
void mqttDisconnectForced(Mqtt *xmqtt)   
{   
    if(!xmqtt->connected)   
        return;   

    //tcp_close(this->pcb);   
    tcp_abort(xmqtt->pcb);   
   
    xmqtt->connected = 0;   
}   
   
   
void mqttInit(Mqtt *xmqtt, struct ip_addr serverIp, int port, msgReceived fn, char *devId){   
    xmqtt->msgReceivedCallback = fn;   
    //this->pcb = tcp_new();   
    xmqtt->server = serverIp;   
    xmqtt->port = port;   
    xmqtt->connected = 0;   
    xmqtt->deviceId = devId;   
    xmqtt->autoConnect = 0;   
   
}   
   
   
static void   
conn_err(void *arg, err_t err)   
{    
  LWIP_UNUSED_ARG(err);   
   
}   
   
static err_t   
http_poll(void *arg, struct tcp_pcb *pcb)   
{   
  //struct http_state *hs;   
   
  //hs = arg;   
   
  //LWIP_DEBUGF(HTTPD_DEBUG, ("http_poll 0x%08x\n", pcb));   
   
  /*  printf("Polll\n");*/   
   
  /*  
  if ((hs == NULL) && (pcb->state == ESTABLISHED)) {  
  
    tcp_abort(pcb);  
    return ERR_ABRT;  
  } else {  
    ++hs->retries;  
    if (hs->retries == 4) {  
      tcp_abort(pcb);  
      return ERR_ABRT;  
    }  
  
    // If this connection has a file open, try to send some more data. If  
     // it has not yet received a GET request, don't do this since it will  
    // cause the connection to close immediately.  
    if(hs && (hs->handle)) {  
      send_data(pcb, hs);  
    }  
  }*/   
    Mqtt *xmqtt = arg;   
   
    if(!xmqtt->connected && xmqtt->pollAbortCounter == 4)   
    {   
        //UARTprintf("http_poll(ABORT!)\n");   
        tcp_abort(pcb);   
        return ERR_ABRT;   
    }   
   
    xmqtt->pollAbortCounter++;   
   
    //UARTprintf("http_poll()\n");   
   
  return ERR_OK;   
}   
   
uint8_t mqttConnect(Mqtt *xmqtt) {   
   
    if(xmqtt->connected)   
        return 1;   
   
    xmqtt->pollAbortCounter = 0;   
   
    xmqtt->pcb = tcp_new();   
    err_t err= tcp_connect(xmqtt->pcb, &(xmqtt->server), 1883, accept_callback);   
   
    if(err != ERR_OK){   
       // UARTprintf("Connection Error : %d\r\n",err);   
        mqttDisconnectForced(xmqtt);   
        return 1;   
    } else {   
      //  UARTprintf("Connection sucessed..\r\n");   
    }   
   
    //Delay_ms(10);   
   
    tcp_arg(xmqtt->pcb, xmqtt);   
   
    tcp_err(xmqtt->pcb, conn_err);   
    tcp_poll(xmqtt->pcb, http_poll, 4);   
    tcp_accept(xmqtt->pcb, &accept_callback);   
   
   
    //device_poll();   
   
    // variable header   
    uint8_t var_header[] = {0x00,0x06,0x4d,0x51,0x49,0x73,0x64,0x70,0x03,0x02,0x00,KEEPALIVE/1000,0x00,strlen(xmqtt->deviceId)};   
   
    // fixed header: 2 bytes, big endian   
    uint8_t fixed_header[] = {MQTTCONNECT,12+strlen(xmqtt->deviceId)+2};   
   
		char *packet = &netip_tx_buffer[0];
  //  char packet[sizeof(fixed_header)+sizeof(var_header)+strlen(xmqtt->deviceId)];   
   
    // Clear memory   
    //memset(packet,0,1500);   
   
    // Copy fixed header   
    memcpy(packet,fixed_header,sizeof(fixed_header));   
    // Copy variable header   
    memcpy(packet+sizeof(fixed_header),var_header,sizeof(var_header));   
    // Copy device name   
    memcpy(packet+sizeof(fixed_header)+sizeof(var_header),xmqtt->deviceId,strlen(xmqtt->deviceId));   
   
    //Send MQTT identification message to broker.   
    err = tcp_write(xmqtt->pcb, (void *)packet, sizeof(packet), 1);   
   
    if (err == ERR_OK) {   
        tcp_output(xmqtt->pcb);   
        xmqtt->lastActivity = msTime;   
        //this->connected = 1;   
       // UARTprintf("Identificaiton message sended correctlly...\n");   
        return 0;   
    } else {   
       // UARTprintf("Failed to send the identification message to broker...\n");   
       // UARTprintf("Error is: %d\n",err);   
        mqttDisconnectForced(xmqtt);   
        return 1;   
    }   
   
    return 0;   
}   
   
   
uint8_t mqttPublish(Mqtt *xmqtt,char* pub_topic, char* msg,int len) 
{   
   
    if(!xmqtt->connected)   
    {   
        //UARTprintf("Publish err:no this->connected");   
        return 1;   
    }   
   
    uint8_t var_header_pub[strlen(pub_topic)+3];   
    strcpy((char *)&var_header_pub[2], pub_topic);   
    var_header_pub[0] = 0;   
    var_header_pub[1] = strlen(pub_topic);   
    var_header_pub[sizeof(var_header_pub)-1] = 0;   
   
    uint8_t fixed_header_pub[] = {MQTTPUBLISH,sizeof(var_header_pub)+strlen(msg)};   
   
    uint8_t packet_pub[sizeof(fixed_header_pub)+sizeof(var_header_pub)+strlen(msg)];   
    memset(packet_pub,0,sizeof(packet_pub));   
    memcpy(packet_pub,fixed_header_pub,sizeof(fixed_header_pub));   
    memcpy(packet_pub+sizeof(fixed_header_pub),var_header_pub,sizeof(var_header_pub));   
    memcpy(packet_pub+sizeof(fixed_header_pub)+sizeof(var_header_pub),msg,strlen(msg));   
   
    //Publish message   
    err_t err = tcp_write(xmqtt->pcb, (void *)packet_pub, sizeof(packet_pub), 1); //TCP_WRITE_FLAG_MORE   
   
    if (err == ERR_OK) {   
        tcp_output(xmqtt->pcb);   
       // UARTprintf("Publish: %s ...\r\n", msg);   
    } else {   
        //UARTprintf("Failed to publish...\r\n");   
       // UARTprintf("Error is: %d\r\n",err);   
        mqttDisconnectForced(xmqtt);   
        return 1;   
    }   
    //printf("\r\n");   
    //device_poll();   
    return 0;   
}   
   
   
static void   
close_conn(struct tcp_pcb *pcb)   
{   
  err_t err;   
  //LWIP_DEBUGF(HTTPD_DEBUG, ("Closing connection 0x%08x\n", pcb));   
   
  tcp_arg(pcb, NULL);   
  tcp_sent(pcb, NULL);   
  tcp_recv(pcb, NULL);   
   
  err = tcp_close(pcb);   
  if(err != ERR_OK)   
  {   
      //LWIP_DEBUGF(HTTPD_DEBUG, ("Error %d closing 0x%08x\n", err, pcb));   
  }   
}   
   
   
uint8_t mqttDisconnect(Mqtt *xmqtt) {   
   
    if(!xmqtt->connected)   
        return 1;   
   
    uint8_t packet_224[] = {2,2,4};   
    tcp_write(xmqtt->pcb, (void *)packet_224, sizeof(packet_224), 1);   
    tcp_write(xmqtt->pcb, (void *)(0), sizeof((0)), 1);   
    //socket->send((char*)224,sizeof((char*)224));   
    //socket->send((uint8_t)0,sizeof((uint8_t)0));   
    tcp_close(xmqtt->pcb);   
    //this->lastActivity = timer.read_ms();   
    xmqtt->connected = 0;   
   
    return 0;   
}   
   
uint8_t mqttSubscribe(Mqtt *xmqtt, char* topic) {   
   
    if (!xmqtt->connected)   
        return -1;   
   
        uint8_t var_header_topic[] = {0,10};   
        uint8_t fixed_header_topic[] = {MQTTSUBSCRIBE,sizeof(var_header_topic)+strlen(topic)+3};   
   
        // utf topic   
        uint8_t utf_topic[strlen(topic)+3];   
        strcpy((char *)&utf_topic[2], topic);   
   
        utf_topic[0] = 0;   
        utf_topic[1] = strlen(topic);   
        utf_topic[sizeof(utf_topic)-1] = 0;   
   
        char packet_topic[sizeof(var_header_topic)+sizeof(fixed_header_topic)+strlen(topic)+3];   
        memset(packet_topic,0,sizeof(packet_topic));   
        memcpy(packet_topic,fixed_header_topic,sizeof(fixed_header_topic));   
        memcpy(packet_topic+sizeof(fixed_header_topic),var_header_topic,sizeof(var_header_topic));   
        memcpy(packet_topic+sizeof(fixed_header_topic)+sizeof(var_header_topic),utf_topic,sizeof(utf_topic));   
   
        //Send message   
        err_t err = tcp_write(xmqtt->pcb, (void *)packet_topic, sizeof(packet_topic), 1); //TCP_WRITE_FLAG_MORE   
        if (err == ERR_OK) {   
            tcp_output(xmqtt->pcb);   
           // UARTprintf("Subscribe sucessfull to: %s...\r\n", topic);   
        } else {   
            //UARTprintf("Failed to subscribe to: %s...\r\n", topic);   
            //UARTprintf("Error is: %d\r\n",err);   
            mqttDisconnectForced(xmqtt);   
            return 1;   
        }   
       // UARTprintf("\r\n");   
        //device_poll();   
   
        return 0;   
   
}   
   
   
   
int mqttPing(Mqtt *xmqtt)   
{   
    MqttFixedHeader pingReq;   
   
    if(!xmqtt->connected)   
        return -1;   
   
    pingReq.header = MQTT_PINGREQ_HEADER;   
    pingReq.remainingLength = 0x00;   
   
    //Publish message   
    err_t err = tcp_write(xmqtt->pcb, (void *)&pingReq, sizeof(pingReq), 1); //TCP_WRITE_FLAG_MORE   
   
    if (err == ERR_OK) {   
        tcp_output(xmqtt->pcb);   
        //UARTprintf("Pinreq...\r\n");   
    } else {   
       // UARTprintf("Failed to send PingReq...\r\n");   
        //UARTprintf("Error is: %d\r\n",err);   
        mqttDisconnectForced(xmqtt);   
        return -1;   
    }   
   
   
    return 1;   
}   
   
uint8_t mqttLive(Mqtt *xmqtt) {   
   
   
    uint32_t t = msTime;   
    if (t - xmqtt->lastActivity > (KEEPALIVE - 2000)) {   
   
        if (xmqtt->connected) {   
           // UARTprintf("Sending keep-alive\n");   
            mqttPing(xmqtt);   
        } else if(xmqtt->autoConnect){   
            mqttConnect(xmqtt);   
        }   
   
        xmqtt->lastActivity = t;   
    }   
   
    return 0;   
}   
   
/*  
int PubSub_mbed::connected() {  
int rc = 0;(int)socket.connected();  
if (!rc) {  
socket.close();  
}  
return rc;  
}  
*/ 