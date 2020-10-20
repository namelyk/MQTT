#ifndef _IOT_CONFIG_H_
#define _IOT_CONFIG_H_

#define IOT_ENABLE
#ifdef IOT_ENABLE

#define IOT_HANDLER()			iot_config_interface.handler()





#define HDL_MQTT_ENABLE
enum MQTT_TYPE 
{
	HDL_MQTT = 1,
	STANDARD_MQTT
};

// api
extern int iot_timestamp(void);
extern void iot_close(void);
extern void iot_debug_status(int status);
extern void Iot_Debug_Monitor(char *message, int len);

#pragma pack(1)
struct iot_config_t
{
	void (*init)(void);
	void (*handler)(void);
	int  (*get_status)(void);
	void (*set_status)(char);
	int (*push)(unsigned int cmd,unsigned char src_subnet,\
					unsigned char src_device,unsigned char dest_subnet,\
					unsigned char dest_device,unsigned char *buffer,int len); //2020-05-21加
	
	int  (*write)(char *packet,int len);
	
	int  (*send)(unsigned int cmd,unsigned char src_subnet,unsigned char src_device, \
					unsigned char dest_subnet,unsigned char dest_device, \
					unsigned char *buffer,int len);
	int  (*long_send)(unsigned int cmd,unsigned char src_subnet,unsigned char src_device, \
					unsigned char dest_subnet,unsigned char dest_device, \
					unsigned char *buffer,int len);
	int  (*read_callback)(char *Topic, int Topic_len, char *packet,int len);//_read_callback
	
	void (*setAES_enable)(char *Buffer, int len);
	void (*getAES)(char *Buffer, int len); //yk 2019-12-30加
	void (*settopic)(char *Buffer, int type);
	void (*getTopic)(void);
	void (*oneMs_handler)(void);	
	void (*set_monitor)(unsigned char *buffer);//0 无效 1 有效		
	unsigned char (*get_monitor)(unsigned char *buffer);
	int (*set_coreurl)(unsigned char *buffer, int len);					
	unsigned char (*get_coreurl)(unsigned char *buffer);
};
#pragma pack()

extern const struct iot_config_t iot_config_interface;
extern struct iot_t *iot;
//yk 2019-11-29加 时间片自增
extern void iot_timestamp_auto_increment(void);
extern int Iot_Send_Handle(unsigned char *input, int len);	
extern unsigned long ip2int(const char *ip);

extern void iot_debug_reapply(void);
#endif

void test_read_somedata(unsigned char *data);
#endif
