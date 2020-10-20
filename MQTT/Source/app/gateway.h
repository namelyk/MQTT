#ifndef _GATEWAY_H_
#define _GATEWAY_H_


#define NETIP_GATEWAY_ENABLE
#ifdef NETIP_GATEWAY_ENABLE

#define GATEWAY_LINK_NULL         0
#define GATEWAY_LINK_REMOTE       1
#define GATEWAY_LINK_SERVER       2
#define GATEWAY_LINK_DNS		  3
#define MQTT_LINK_SERVER          4
#define STANDAR_MQTT_LINK_SERVER  5
#define GATEWAY_LINK_MAX_NUM      6

extern struct netip_t netip_gateway;


extern void netip_gateway_init(void);
extern int netip_gateway_type(void);
extern int netip_gateway_send(unsigned int cmd,unsigned char subnet,\
					unsigned char device,unsigned char *buffer,unsigned char size);
extern int netip_gateway_longsend(unsigned int cmd,unsigned char subnet,\
					unsigned char device,unsigned char *buffer,unsigned char size);
extern int netip_gateway_send_special(unsigned int cmd,unsigned char src_subnet,\
					unsigned char src_device, unsigned char subnet,\
					unsigned char device,unsigned char *buffer,unsigned char size);
extern int netip_gateway_write(unsigned char *buffer,int len);

extern void netip_gateway_reset_ipaddress(void);
extern int netip_gateway_get_ipaddress(unsigned char *buffer);
extern int netip_gateway_set_ipaddress(unsigned char *buffer);
extern int netip_gateway_get_domainname(unsigned char *buffer);
extern int netip_gateway_set_domainname(unsigned char *buffer);
extern int netip_gateway_get_message(unsigned char *buffer,int len);
extern int netip_gateway_set_message(unsigned char *buffer,int len);
extern int netip_gateway_get_server(unsigned char *buffer,int len);
extern int netip_gateway_set_server(unsigned char *buffer,int len);

extern int netip_gateway_get_history(unsigned long ipaddr,unsigned short port,unsigned char *buffer,int len);
extern int netip_gateway_set_history(unsigned long ipaddr,unsigned short port,unsigned char *buffer,int len);

extern int netip_gateway_remote_link_endpoint(unsigned char *buffer);
extern void netip_gateway_remote_disable(unsigned long ipaddr,unsigned short port);
extern void netip_gateway_remote_link(unsigned long ipaddr,unsigned short port);
extern int netip_gateway_password_module(unsigned long ipaddr,unsigned short port,\
							unsigned char subnet,unsigned char device,unsigned char *buffer,int len);
extern int netip_gateway_conneting_module(unsigned long ipaddr,unsigned short port,\
					unsigned char subnet,unsigned char device,unsigned char *buffer,int len);

extern void netip_gateway_connect_client(void);
extern void netip_gateway_connect_client_response(unsigned long ipaddr,unsigned short port);

extern void netip_gateway_register_error_report(unsigned char *report);
extern void netip_gateway_register_request(void);
extern void netip_gateway_register_respons(unsigned char *buffer);
extern void netip_gateway_handler(void);

extern void netip_gateway_get_passwd(unsigned char *);
extern int netip_gateway_get_MQTT_message(unsigned char *buffer);
extern int netip_gateway_set_MQTT_message(unsigned char *buffer);

extern int netip_gateway_get_Standar_MQTT_message(unsigned char *buffer);
extern int netip_gateway_set_Standar_MQTT_message(unsigned char *buffer);

extern int netip_gateway_get_Standar_MQTT_URL_message(unsigned char *buffer);
extern int netip_gateway_set_Standar_MQTT_URL_message(unsigned char *buffer);

extern int netip_gateway_dowrite(unsigned int ipaddr,unsigned short port,\
												unsigned char *packet,int len);
extern int easy_netip_gateway_set_ipaddress(unsigned char *buffer);

extern int netip_gateway_get_STANDAR_MQTT_AES(unsigned char *buffer);
extern int netip_gateway_set_STANDAR_MQTT_AES(unsigned char *buffer);

extern int netip_gateway_get_STANDAR_MQTT_TOPIC(unsigned char *buffer);
extern int netip_gateway_set_STANDAR_MQTT_TOPIC(unsigned char *buffer);

extern unsigned short netip_gateway_get_message_CRC(char *crc);
extern void netip_MQTT_Set_Status(void);

extern int netip_gateway_get_hour_minute_area(int *hour, int *minute);

extern int netip_gateway_dosend(unsigned int ipaddr,unsigned short port,\
									unsigned int cmd,unsigned char src_subnet,unsigned char src_device, \
											unsigned char dest_subnet,unsigned char dest_device, \
											unsigned char *buffer,unsigned char len);

#endif
#endif

