#include "iot-config.h"
#include "driver.h"
#include "sys_include.h"
#include "timestamp.h"

#ifdef IOT_ENABLE
#include "mbedtls/base64.h"
#include "MQTTLinux.h"
#include "mqtt_client.h"
#include "netip.h"

#define HEAD_MIRACLE					"HDLMIRACLE"
/******************中心服务器IP and 端口号***************************/
//#define IOT_CONFIG_URL_ADDR		0x8010000	
//const char IOT_CONFIG_URL[64] __attribute__((at(IOT_CONFIG_URL_ADDR))) = "china.hdlcontrol.com\0";
//static char IOT_CONFIG_URL[64] = "china.hdlcontrol.com\0";
static char IOT_CONFIG_URL[64] = "tcpcenter.hdlcontrol.com\0";

#define IOT_CONFIG_PORT				26688
#define IS_LOCAL(A,B)				devices_is_localaddress(A,B)

/****************发布的主题 /Buspro/MAC(网关mac地址 16字节)/request***********/
char Sub_Topic_Name[100];//订阅主题
char Pub_Topic_Name[100];//发布主题
extern char HDL_MQTT_USE_MAC[17];  //用于mqtt
extern char MQTT_MAC_UUID[37];
/***************网关相关*****************/
extern struct netip_t netip_gateway;
extern unsigned char netip_rx_buffer[];
extern unsigned char netip_tx_buffer[];
extern unsigned char is_connect;
/***************时间片********************/
extern unsigned long ticks;
#define NTimeMS()			ticks

char fail_url[64] = {0}; //记录无效url，用于取消
char crc_buffer[3] = {0};
unsigned char Home_ID[37] = {0};
unsigned char Account[37] = {0};
unsigned char MQTT_Pname[21] = {0};
unsigned char MQTT_Gname[21] = {0};
unsigned char MQTT_user[9] = {0};

char key_buf[17] = {0};
unsigned char subcribe_time = 0; //两次订阅分开来，网络卡的情况下连续订阅失败率高
unsigned char fail_timestamp = 0;//用于
unsigned char clear_time = 0;

static char iot_reply_signal = 0;
//===================================================================
#pragma pack(1)
union Port
{
	char ch[2];
	unsigned short i;
};
union MQTT_IP
{
	char ch[4];
	unsigned int i;
};
union Timestamp
{
	char ch[4];
	unsigned int in;
};
struct iot_t
{
	int read_flag;
	int apply_flag;
	int apply_ok;
	int apply_enable;

	char enable;
	char status;
	
	union Timestamp timestamp;
	int sockfd;
	int sockfd_flag; //用于减少打开socket次数
	
	struct sockaddr_in ipaddr; 
	int timeout;	
	char clientId[64];
	char aeskey[16];
	
	char key[16];
	char url[64];
	union Port port;
	char username[64];
	char password[17];
	
	char aes_status;
	char monitor;  //用于开启调试模式
	
	char mqtt_message_flag;
	
	//yk 2020-02-20加
	struct hostent *hdl_host;  //全局变量 目的为了减少解析域名的次数
	//yk 2020-07-30加
	unsigned char subLen;
};
static struct iot_t iot_config = {0};
struct iot_t *iot;
unsigned char temp_buffer[1200];
unsigned char base_buffer[1200];
#pragma pack()
#define NC2(val) 								((((unsigned short)val&0xff) << 8) | ((unsigned short)val >> 8))
#define IOT_AES_DECRYPT(a,b,c,d)				netcrypt_aes_decrypt(a,b,c,d)
#define IOT_AES_ENCRYPT(a,b,c,d)			  netcrypt_aes_encrypt(a,b,c,d)
#define IOT_CALLBACK(A,B)								netip_commands_handler(101,(unsigned char *)A,B)
// ======================= config ======================================
/*****************************************************************************************************************************************************************************************************/
//监控模式 (yk 2020-02-20加)
void Iot_Debug_Monitor(char *message, int len)
{
	if(iot->monitor == 0) return;
	unsigned char subnet_buffer[3] = {0};
	devices_get_subnet(subnet_buffer);
	
	if(len < 67)//小包发送
	{
		app_commands_send(255,0x8888,255,255,(unsigned char *)message,len);
	}
	else
	{
		app_commands_send(255, 0x8888, 255, 255, (unsigned char *)"BigMessage", 10);
		netcrypt_long_send(0x8889, \
				subnet_buffer[0], subnet_buffer[1], 0xff, 0xff,  (unsigned char *)message, len);
	}
}
//设置和获取监控状态
void _set_monitor(unsigned char *buffer)//0 无效 1 有效
{
	if(buffer[0] != 0)
	{
		iot->monitor = 1;
	}
	else
	{
		iot->monitor = 0;
	}
	return;
}
unsigned char _get_monitor(unsigned char *buffer)
{
	if(buffer != NULL)
	{
		buffer[0] = iot->monitor;
	}
	return iot->monitor;
}
#define	AESKEY					"68df3a591b167268"
/************************************************************************
Name:		_callback
Function:	接收到中心服务器发回数据解析
Called By:	iot_request
return:		-1	failure
			1	success
*************************************************************************/
static int _callback(char *packet, int len)
{
	int rc = -1;
	char *p = NULL, *q = NULL, *result = NULL;
	unsigned char eeprom_buffer[HDL_MQTT_UUID_SIZE];
	unsigned char zero[64] = {0};
	unsigned char flag[2] = {0x01, 0x00};
	packet[len] = '\0';
	p = packet;
	
	q = strstr(p,"\r\n");
	if (q == NULL) return -1;
	*q++ = 0;
	*q++ = 0;
	
	memset(temp_buffer,0,sizeof(temp_buffer));
	memcpy(temp_buffer, packet, len);
	//memset(base_buffer,0,sizeof(base_buffer));
	
	//2020-05-20新增一个
	if(strlen(p) >= strlen("releaseBind")&&!strcmp(p,"releaseBind"))//网关已经被解绑了 但是账号还是之前的账号
	{
		iot->read_flag = 0;
		rc = -1;
		iot->status = 5; //进入一个暂停态
	}
	else if (strlen(p) >= strlen("applySuccess")&&!strcmp(p,"applySuccess"))// 申请UUID，和 登录KEY
	{
		p += 14;
		IOT_AES_DECRYPT((unsigned char *)iot->key,len-14,(unsigned char *)p,temp_buffer);
		p = (char *)temp_buffer;
		Iot_Debug_Monitor("applycallback",13);
		////Iot_Debug_Monitor(temp_buffer, strlen(temp_buffer));
		
		result = strtok(p, "\r\n");
		if(result == NULL || result[0] == '[') return -1;
		strcpy(key_buf, result);

		result = strtok(NULL, "\r\n");
		if(result == NULL) return -1;
		if((unsigned char)result[0] == 0xff && (unsigned char)result[1] == 0xff && (unsigned char)result[3] == 0xff)  //防止申请到0xfffffffffff
		{
			iot->read_flag = 0;
			rc = -1;
		}
		else
		{
			//2020-07-24改
			memset(MQTT_MAC_UUID, 0, 36);
			strcpy(MQTT_MAC_UUID,result);
			if(strlen(result) > 36) return -1;
			eeprom_read(HDL_MQTT_UUID_ADDRESS, eeprom_buffer, HDL_MQTT_UUID_SIZE);
			if(memcmp((char *)eeprom_buffer, MQTT_MAC_UUID, HDL_MQTT_UUID_SIZE))
			{
				eeprom_write(HDL_MQTT_UUID_ADDRESS ,(unsigned char *)MQTT_MAC_UUID, HDL_MQTT_UUID_SIZE);
			}
			rc = 1;
		}
		//yk 2020-03-11加 真正的homeID
		result = strtok(NULL, "\r\n");
		//2020-07-24改
		if(result == NULL) return -1;
		if(strlen(result) > 36) return 1;
		memcpy(Home_ID, result, strlen(result));	
	}
	else if (strlen(p) >= strlen("loginSuccess")&&!strcmp(p,"loginSuccess")) //登录返回 MQTT服务器的信息和通讯key
	{
		p += 14;
		IOT_AES_DECRYPT((unsigned char *)iot->key,len-14,(unsigned char *)p,temp_buffer);
		p = (char *)temp_buffer;
		Iot_Debug_Monitor("logincallback",13);
		////Iot_Debug_Monitor(temp_buffer, strlen(temp_buffer));
		//my_strtok(p, "\r\n", out1, out2);
		
		result = strtok(p, "\r\n");//protocol
		if(result == NULL)
		{
			return -1;
		}
		result = strtok(NULL, "\r\n");//domain
		if(result == NULL)
		{
			return -1;
		}
		
		result = strtok(NULL, "\r\n");//port
		if(result == NULL)
		{
			return -1;
		}
		iot->ipaddr.sin_port = atoi(result);
		
		result = strtok(NULL, "\r\n");//clientID
		if(result == NULL)
		{
			return -1;
		}
		memset(iot->clientId,0,sizeof(iot->clientId));
		strcpy(iot->clientId,result);
		
		result = strtok(NULL, "\r\n");//user
		if(result == NULL)
		{
			return -1;
		}
		memset(iot->username,0,sizeof(iot->username));
		strcpy(iot->username,result);
		
		result = strtok(NULL, "\r\n");//password
		if(result == NULL)
		{
			return -1;
		}
		memset(iot->password,0,sizeof(iot->password));
		strcpy(iot->password, result);
		
		result = strtok(NULL, "\r\n");//aesKey
		if(result == NULL)
		{
			return -1;
		}
		memset(iot->aeskey,0,sizeof(iot->aeskey));
		strcpy(iot->aeskey, result);
		
		result = strtok(NULL, "\r\n");//timestamp
		if(result == NULL)
		{
			return -1;
		}
		iot->timestamp.in = atoi(result);
		//Timestamp_to_datetime(iot->timestamp.in);
		
		//yk 2019-12-11加
		result = strtok(NULL, "\r\n");//ip
		if(result == NULL)
		{
			return -1;
		}
		iot->ipaddr.sin_addr.s_addr = ip2int(result);
//		//yk 2020-03-11加 真正的homeID
		result = strtok(NULL, "\r\n");
		if(result == NULL) return -1;
		//eeprom_write(MQTT_CONFIG_ADDRESS + 37, (unsigned char *)result, 36);
		memcpy(Home_ID, result, strlen(result));
//		yk 2020-04-28加 下次需要登录的域名
		result = strtok(NULL, "\r\n");
		if(result == NULL) return -1;
		if(strcmp(result, IOT_CONFIG_URL))
		{
			//iot_config_interface.set_coreurl((unsigned char *)result, strlen(result));
			memcpy(zero, result, strlen(result));
		#if 1
			//Flash_WriteURL(zero, 64);
			eeprom_write(IOT_CONFIGURE_URL_FLAG_ADDR, flag, 1);
			eeprom_write(IOT_CONFIGURE_URL_ADDR, zero, 64);
		#endif
		}
		rc = 1;
	}
	//yk 2010-01-06加 错误处理
	else if(strlen(p) >= strlen("fail")&&!strncmp(p,"fail", 4))			
	{
		p += 6;
		IOT_AES_DECRYPT((unsigned char *)AESKEY,len-6,(unsigned char *)p,temp_buffer);
		p = (char *)temp_buffer;
		iot->read_flag = 0;
		rc = -1;
	}
	else
	{
		iot->read_flag = 0;
		rc = -1;
	}
	return rc;
}

/************************************************************************
Name:		iot_request
Function:	接收到中心服务器发回数据解析
Called By:	_init
return:		-1	failure
			1	success
*************************************************************************/
//"68df3a59-b167-4b"//"g47c8-2f95-fd4f1"//"83060ff2-0751-4c"//"cf2d1e91-e2d0-4b"//"98912096-0307-4a"						//
#define GATEWAYNAME				"testGatewayName"
#define ACCOUNT					"5691958c-362d-482f-8fd2-5871c4cff206"
#define	HOMEID					"HomeId"
//#define DEBUG_MODE
static int iot_request(int s, char *aeskey, int ApplyforOrLogin)
{
	unsigned int len,olen;
	char *frame = NULL;
	unsigned char devices_remask[21] = {0};
	unsigned char devices_subnet[3] = {0};
	char subnetid_str[4] = {0};
	char deviceid_str[4] = {0};
	
	devices_get_remark(devices_remask);
	devices_get_subnet(devices_subnet);
	snprintf(subnetid_str, 4, "%d", devices_subnet[0]);
	snprintf(deviceid_str, 4, "%d", devices_subnet[1]);

	memset(netip_tx_buffer,0,1000);
	frame = (char *)netip_tx_buffer;
	strncpy(iot->key,(char *)aeskey,16); //公共key
	if(ApplyforOrLogin)//login
	{
		strcpy(frame, "0\r\n");
		strcpy(frame+3, "login\r\n");
	//	eeprom_read(HDL_MQTT_UUID_ADDRESS ,(unsigned char *)MQTT_MAC_UUID, HDL_MQTT_UUID_SIZE);
		strcat(frame, MQTT_MAC_UUID);
		strcat(frame, "\r\n");
		
		strcpy((char *)temp_buffer,HDL_MQTT_USE_MAC);
		strcat((char *)temp_buffer, "\r\n");
		strcat((char *)temp_buffer, (char *)MQTT_Pname);
		strcat((char *)temp_buffer, "\r\n");
		strcat((char *)temp_buffer, (char *)MQTT_Gname);
		strcat((char *)temp_buffer, "\r\n");
		//strcat((char *)temp_buffer, (char *)devices_remask);//这个参数改为备注
		strcat((char *)temp_buffer, (char *)MQTT_user);//这个参数改为备注
		strcat((char *)temp_buffer, "\r\n");
		//strcat((char *)temp_buffer,"\r\n\r\n"); //后面添加GATEWAYNAME
		//2019/11/19加
	#ifdef DEBUG_MODE
		strcat((char *)temp_buffer, (char *)HOMEID);
		strcat((char *)temp_buffer, "\r\n\r\n");
	#else
		strcat((char *)temp_buffer,(char *)devices_remask);  //2020-05-20更改为网关备注
		//strcat((char *)temp_buffer,"\r\n\r\n");
		strcat((char *)temp_buffer, "\r\n");
		strcat((char *)temp_buffer, "1");  			//1表示aes加密
		strcat((char *)temp_buffer, "\r\n");
		strcat((char *)temp_buffer, SOFTWRAE_REAL_VERSION);
		strcat((char *)temp_buffer, "\r\n");
		strcat((char *)temp_buffer, subnetid_str);  //子网id
		strcat((char *)temp_buffer, "\r\n");
		strcat((char *)temp_buffer, deviceid_str);  //设备id
		strcat((char *)temp_buffer, "\r\n\r\n");
	#endif
	}
	else//apply
	{
		strcpy(frame,"0\r\n");
		strcpy(frame+3,"apply\r\n");
		
		strcpy((char *)temp_buffer,HDL_MQTT_USE_MAC);
		strcat((char *)temp_buffer,"\r\n");
		strcat((char *)temp_buffer,(char *)MQTT_Pname);
		strcat((char *)temp_buffer,"\r\n");
		strcat((char *)temp_buffer,(char *)MQTT_Gname);
		strcat((char *)temp_buffer,"\r\n");
	
		strcat((char *)temp_buffer,(char *)MQTT_user);  //这个参数改为备注
		//strcat((char *)temp_buffer, (char *)devices_remask);
		strcat((char *)temp_buffer,"\r\n");
		//strcat((char *)temp_buffer,"\r\n\r\n"); //后面添加GATEWAYNAME，ACCOUNT和HOMEID
		//2019/11/19加
		#ifdef DEBUG_MODE
			strcat((char *)temp_buffer,(char *)HOMEID);
			strcat((char *)temp_buffer,"\r\n");
			strcat((char *)temp_buffer,(char *)ACCOUNT);
			strcat((char *)temp_buffer,"\r\n");
			strcat((char *)temp_buffer,(char *)HOMEID);
			strcat((char *)temp_buffer,"\r\n\r\n");
		#else
			strcat((char *)temp_buffer,(char *)devices_remask);  //2020-05-20更改为网关备注
			strcat((char *)temp_buffer,"\r\n");
			strcat((char *)temp_buffer,(char *)Account);
			strcat((char *)temp_buffer,"\r\n");
			strcat((char *)temp_buffer,(char *)Home_ID);//
			strcat((char *)temp_buffer,"\r\n\r\n");
		#endif
	}
	
	len = IOT_AES_ENCRYPT((unsigned char *)iot->key,strlen((char *)temp_buffer),temp_buffer,base_buffer);
	olen = strlen(frame);
	memcpy(&netip_tx_buffer[olen],base_buffer,len);
	len += olen;
	////Iot_Debug_Monitor("sendok", 6);	
	
	iot_reply_signal = 0;
	
	return linux_send(s, (unsigned char *)frame, len);
}

//时间片函数
int iot_timestamp(void)
{
	return iot->timestamp.in;
}
//yk 2019-11-29加 时间片自增
void iot_timestamp_auto_increment(void)
{
	iot->timestamp.in = iot->timestamp.in + 1;
}

/******************设置密码和获取密码 end line************************/
static unsigned char Iot_Topic_Common_Handle(int s,char *packet,int len)
{
	unsigned int olen = 0;
	unsigned char *frame_buffer = NULL;
	if(len < 26) return 0;
	memset(netip_rx_buffer, 0, 1200);
	if(iot->aes_status == 0 && iot->enable == 2) //这种情况不加密
	{
		memcpy(netip_rx_buffer, packet, len);
		olen = len;
	}
	else
	{
		//yk 2020-1-7加
		if(netip_rx_buffer[14] == 0xAA && netip_rx_buffer[15] == 0xAA)
		{
			return 0;
		}
		olen = IOT_AES_DECRYPT((unsigned char *)iot->aeskey, len, (unsigned char *)packet, (unsigned char *)netip_rx_buffer);//解密处理	
	}
	//解密失败
	if(netip_rx_buffer[14] != 0xAA || netip_rx_buffer[15] != 0xAA)
	{
		return 0;
	}
	
	
//	//数据处理，判断该消息解密后是否为Bus命令
//	olen = IS_BUS_COMMAND((unsigned char *)temp_buffer, len);
//	if(!olen) return 0;
	frame_buffer = netip_rx_buffer;
	//将数据发送到netip_commands_handler中进行转发处理 无关
	netip_commands_handler(101,(unsigned char *)&frame_buffer[16],olen-16);
	return 0;
}


/***********************************************************
Name:		_read_callback
Function:	MQTT回调函数
Parameter:	Topic:  接收主题
			packet:	解密前数据
			len:	数据长度
Return:		
***********************************************************/
static int _read_callback(char * Topic, int topic_len, char *packet, int len)
{
	//时间戳处理
	if (!strncmp(Topic,"/CloudTimestamp",15) && topic_len == 15)
	{
		char time_str[30] = {0};
		memcpy(time_str, packet, len);
		iot->timestamp.in = atoi(time_str);
	#ifdef DATETIME_ENABLE
		Timestamp_to_datetime(iot->timestamp.in);
	#endif
		return 0;
	}
	
	if(iot->enable == HDL_MQTT)
	{
		if(!strncmp(Topic + iot_config.subLen,"Common/CheckGateway",19) && topic_len == iot_config.subLen + 19) //yk改
		{
			if(iot->status != 3) return 0;
			char recall[100] = {0};
			snprintf(recall, 100, "%s/CheckGateway",Pub_Topic_Name);
			mqtt_client_publish(recall, "...", 0, Qos0);
			return 0;
		}
		else if (!strncmp(Topic + iot_config.subLen,"Common",6))
		{
			Iot_Topic_Common_Handle(101, packet, len);
		}
		else if(!strncmp(Topic + iot_config.subLen, "Push/AfreshLogin", 16) && topic_len == iot_config.subLen + 16)
		{
			iot->status = 4;  //收到该主题重连
			iot->apply_flag = 6;  //清空改标志位证明需要重新申请
		}
	}
	else if(iot->enable == STANDARD_MQTT)
	{
		if (strncmp(Topic,"/CloudTimestamp",15))
		{
			Iot_Topic_Common_Handle(101, packet, len);
		}
	}

	return 0;
}
/*********************************************************************************************************************************************************/
//消息头处理函数
static int Iot_Head_Handle(unsigned char *input)
{
	unsigned char MAC_Buf[10] = {0};
	memcpy((unsigned char *)(&input[0]),(unsigned char *)iot->timestamp.ch, 4);
	
	if(iot->enable == HDL_MQTT)
	{
		memcpy((unsigned char *)(&input[4]),(unsigned char *)HEAD_MIRACLE,10);
	}
	else if(iot->enable == STANDARD_MQTT)
	{
		devices_get_mac(MAC_Buf);
		memcpy((unsigned char *)(&input[4]),(unsigned char *)MAC_Buf,10);
	}
	return 0;
}
/*********************************************************************************************************************************************************/
//发送处理函数
int Iot_Send_Handle(unsigned char *input, int len)
{
	if(iot->status != 3) return 0;  //没连接上MQTT
	//发送数据	
	if(iot->aes_status == 0 && iot->enable == 2) //这种情况不加密
	{
#ifdef MQTT_CLIENT_ENABLE
		return mqtt_client_publish(Pub_Topic_Name, (char *)input, len, Qos0);//mqtt_client_publish
#else
		return 0;
#endif
	}
	else
	{
#ifdef MQTT_CLIENT_ENABLE
		len = IOT_AES_ENCRYPT((unsigned char *)iot->aeskey
				,len,(unsigned char *)input, (unsigned char *)base_buffer);
		
		return mqtt_client_publish(Pub_Topic_Name, (char *)base_buffer, len, Qos0);
#else
		return 0;
#endif	
	}
}

//  2020-05-21加 /BusGateWayToClient/MQTT_MAC_UUID/Push/Alarm
static int _push(unsigned int cmd,unsigned char src_subnet,\
					unsigned char src_device,unsigned char dest_subnet,\
					unsigned char dest_device,unsigned char *buffer,int len)
{
	int alen;
	unsigned int crc;
	unsigned int dev_type = netip_get_type();
	char Push_topic[100] = {0}; //  2020-05-21加 /BusGateWayToClient/MQTT_MAC_UUID/Push/Alarm
	snprintf(Push_topic, 100, "/BusGateWayToClient/%s/Push/Alarm", MQTT_MAC_UUID);
		
	if (len > 67) return 1;
/*消息头(ip + HEAD_MIRACLE) ip改成时间***************/  
	Iot_Head_Handle(temp_buffer);
/*Bus头*/
	temp_buffer[14] = 0xAA;
	temp_buffer[15] = 0xAA;
/*Bus数据处理*/
	temp_buffer[16] = len + 11; 
	temp_buffer[17] = src_subnet;  
	temp_buffer[18] = src_device;   
	temp_buffer[19] = (unsigned char)(dev_type >> 8);     
	temp_buffer[20] = (unsigned char)(dev_type & 0X00FF); 
	temp_buffer[21] = (unsigned char)(cmd >> 8);
	temp_buffer[22] = (unsigned char)(cmd & 0x00FF);
	temp_buffer[23] = dest_subnet;
	temp_buffer[24] = dest_device;

	memcpy((unsigned char *)&(temp_buffer[25]),buffer,len);

	crc = CRC16_XMODEM((unsigned char *)&temp_buffer[16],temp_buffer[16]-2);
	temp_buffer[len + 25] = (unsigned char)(crc >> 8);
	temp_buffer[len + 26] = (unsigned char)(crc);
//数据长度	
	alen = len + 27;
//发送数据
	if(iot->status != 3) return 0;  //没连接上MQTT
	//发送数据	
	if(iot->aes_status == 0 && iot->enable == 2) //这种情况不加密
	{
#ifdef MQTT_CLIENT_ENABLE
		return mqtt_client_publish(Push_topic, (char *)temp_buffer, alen, Qos0);//mqtt_client_publish
#else
		return 0;
#endif
	}
	else
	{
#ifdef MQTT_CLIENT_ENABLE
		alen = IOT_AES_ENCRYPT((unsigned char *)iot->aeskey
				,alen,(unsigned char *)temp_buffer, (unsigned char *)base_buffer);
		
		return mqtt_client_publish(Push_topic, (char *)base_buffer, alen, Qos0);
#else
		return 0;
#endif	
	}
}
/***********************************************************
Name:		_write
Function:	数据转发，将bus命令包添加数据头，协议头加密发送到MQTT服务器
Parameter:	packet:	Bus命令包
			len:	数据长度
Return:		
***********************************************************/
//#define Write_Debug //打开该宏时使用明文发送消息
static int _write(char *packet,int len)
{
	int alen;
/*消息头(时间戳 + HEAD_MIRACLE)***************/  
	Iot_Head_Handle(temp_buffer);
/*Bus头*/
	temp_buffer[14] = 0xAA;
	temp_buffer[15] = 0xAA;
/*消息复制*/
	memcpy((unsigned char *)(&temp_buffer[16]),(unsigned char *)packet,len);
	alen = len + 16;
	return Iot_Send_Handle(temp_buffer,alen);
	
}
/***********************************************************
Name:		_send
Function:	数据发布，将数据封包为bus命令，加密发布到MQTT服务器
Parameter:	packet:	加密前的数据
			len:	数据长度
Return:		
***********************************************************/
static int _send(unsigned int cmd,unsigned char src_subnet,\
					unsigned char src_device,unsigned char dest_subnet,\
					unsigned char dest_device,unsigned char *buffer,int len)
{
	int alen;
	unsigned int crc;
	unsigned int dev_type = netip_get_type();
	if (len > 67) return 1;
/*消息头(ip + HEAD_MIRACLE) ip改成时间***************/  
	Iot_Head_Handle(temp_buffer);
/*Bus头*/
	temp_buffer[14] = 0xAA;
	temp_buffer[15] = 0xAA;
/*Bus数据处理*/
	temp_buffer[16] = len + 11; 
	temp_buffer[17] = src_subnet;  
	temp_buffer[18] = src_device;   
	temp_buffer[19] = (unsigned char)(dev_type >> 8);     
	temp_buffer[20] = (unsigned char)(dev_type & 0X00FF); 
	temp_buffer[21] = (unsigned char)(cmd >> 8);
	temp_buffer[22] = (unsigned char)(cmd & 0x00FF);
	temp_buffer[23] = dest_subnet;
	temp_buffer[24] = dest_device;

	memcpy((unsigned char *)&(temp_buffer[25]),buffer,len);

	crc = CRC16_XMODEM((unsigned char *)&temp_buffer[16],temp_buffer[16]-2);
	temp_buffer[len + 25] = (unsigned char)(crc >> 8);
	temp_buffer[len + 26] = (unsigned char)(crc);
//数据长度	
	alen = len + 27;
//发送数据
	return Iot_Send_Handle(temp_buffer,alen);
}
/***********************************************************
Name:		_long_send
Function:	大包数据发布，将数据封包为bus命令，加密发布到MQTT服务器
Parameter:	packet:	加密前的数据
			len:	数据长度
Return:		
***********************************************************/
static int _long_send(unsigned int cmd,unsigned char src_subnet,\
					unsigned char src_device,unsigned char dest_subnet,\
					unsigned char dest_device,unsigned char *buffer,int len)
{
	int alen;
	unsigned int dev_type = netip_get_type();

	if (len > 1200) return 1;
/*消息头(时间戳 + HEAD_MIRACLE)***************/  
	Iot_Head_Handle(temp_buffer);
/*Bus头*/
	temp_buffer[14] = 0xAA;
	temp_buffer[15] = 0xAA;
/*Bus数据处理*/
	temp_buffer[16] = 0xFF; 
	temp_buffer[17] = src_subnet;  
	temp_buffer[18] = src_device;   
	temp_buffer[19] = (unsigned char)(dev_type >> 8);     
	temp_buffer[20] = (unsigned char)(dev_type & 0X00FF); 
	temp_buffer[21] = (unsigned char)(cmd >> 8);
	temp_buffer[22] = (unsigned char)(cmd & 0x00FF);
	temp_buffer[23] = dest_subnet;
	temp_buffer[24] = dest_device;

	temp_buffer[25] = (unsigned char)(len >> 8);
	temp_buffer[26] = (unsigned char)(len & 0x00FF);
	
	memcpy((unsigned char *)&(temp_buffer[27]),buffer,len);
//数据长度	
	alen = len + 27;
//发送数据
	return Iot_Send_Handle(temp_buffer,alen);
}

/***********设置和查询MQTT使能位*********************/
static int _get_status(void)
{
	iot = &iot_config;
	return (iot->status);
}

static void _set_status(char enable)
{
	static unsigned char mutex = 0; 
	if(mutex == 1) return;
	mutex = 1;
	
	iot = &iot_config;
	iot->enable = enable;
	memset(fail_url, 0, 64);
	if(iot->status > 1 && iot->status < 5) 
	{
		iot->status = 4;//重新连接
		iot->apply_flag = 6; //重新申请
		iot->apply_ok = 0;
		//iot->apply_flag = 0; //重新申请
	}
	else 
	{
		if(iot->enable == HDL_MQTT)
		{
			iot->apply_flag = 6; //重新申请
		}
		iot->status = 0;
		iot->apply_ok = 0;
	}
	mutex = 0;
}

static void _setAES(char *Buffer, int len)
{
	if(len > 17) return;
	if(Buffer[0] != 0) //开启AES加密模式
	{
		iot->aes_status = 1;
		if(iot->enable != STANDARD_MQTT) return;
		memcpy(iot->aeskey, &Buffer[1], 16);
	}
	else
	{
		iot->aes_status = 0;
	}
}
//获取AES使能及其密匙 
static void _getAES(char *Buffer, int len)
{
	if(iot->aes_status == 0)
	{
		Buffer[0] = 0;
	}
	else
	{
		Buffer[0] = 1;
	}
	memcpy(&Buffer[1], iot->aeskey, 16);
}

/*************************************************************************************************************************************************************************************************/
//设置主题 -->called bynetip_gateway_set_STANDAR_MQTT_TOPIC()
static void _setTopic(char *Buffer, int type)
{
	char Standard_Sub_Topic_Name[49];
	char Standard_Pub_Topic_Name[49];
	
	if(Buffer[0] == 1 && Buffer[1] == 1)
	{
		if(strncmp(Standard_Sub_Topic_Name, &Buffer[2], 48)) //如果和当前不同
		{
			mqtt_client_unsubscribe(Sub_Topic_Name);//取消之前的订阅
			memset(Standard_Sub_Topic_Name, 0, 49);
			memcpy(Standard_Sub_Topic_Name, &Buffer[2], 48);
			//mqtt_client_subscribe(Standard_Sub_Topic_Name);
		}
	}
	else if(Buffer[0] == 2 && Buffer[1] == 1)
	{
		memset(Standard_Pub_Topic_Name, 0, 49);
		memcpy(Standard_Pub_Topic_Name, &Buffer[2], 48);
	}
	else if(Buffer[0] == 1 && Buffer[1] == 0)
	{
		memset(Standard_Sub_Topic_Name, 0, 49);
		sprintf(Standard_Sub_Topic_Name, "/ClientToBus/%s/#", HDL_MQTT_USE_MAC);
		
		//mqtt_client_unsubscribe(Sub_Topic_Name);//取消之前的订阅
		memcpy(&Buffer[2], Standard_Sub_Topic_Name, 48);
		//mqtt_client_subscribe(Standard_Sub_Topic_Name);
	}
	else if(Buffer[0] == 2 && Buffer[1] == 0)
	{
		memset(Standard_Pub_Topic_Name, 0, 49);
		sprintf(Standard_Pub_Topic_Name, "/BusToClient/%s/Buspro", HDL_MQTT_USE_MAC);
		memcpy(&Buffer[2], Standard_Pub_Topic_Name, 48);
	}
	memcpy(Sub_Topic_Name, Standard_Sub_Topic_Name, 49);
	memcpy(Pub_Topic_Name, Standard_Pub_Topic_Name, 49);
}
/*************************************************************************************************************************************************************************************************/
//获取主题	-->called by mqtt_client_init()
static void _getTopic(void)
{
	unsigned char Buffer[50] = {0};
	int i = 0;
	char Standard_Sub_Topic_Name[49];
	char Standard_Pub_Topic_Name[49];
	char HDL_Sub_Topic_Name[100];
	char HDL_Pub_Topic_Name[100];	
/******************初始化订阅接口和发布接口***************************/
	if(iot->enable == HDL_MQTT)//HDL MQTT
	{
		memset(HDL_Pub_Topic_Name, 0, 100);
		sprintf(HDL_Pub_Topic_Name, "/BusGateWayToClient/%s/Common", MQTT_MAC_UUID);
		memset(HDL_Sub_Topic_Name, 0, 100);
		sprintf(HDL_Sub_Topic_Name, "/ClientToBusGateWay/%s/#", MQTT_MAC_UUID);
		
		iot_config.subLen = strlen(HDL_Sub_Topic_Name) - 1;
		memcpy(Pub_Topic_Name, HDL_Pub_Topic_Name, 100);
		memcpy(Sub_Topic_Name, HDL_Sub_Topic_Name, 100);
	}
	else if(iot->enable == STANDARD_MQTT)//Standard MQTT
	{
		Buffer[0] = 2; //2为发布
		netip_gateway_get_STANDAR_MQTT_TOPIC(Buffer);
		memcpy(Standard_Pub_Topic_Name,&Buffer[2],48);

		for(i = 0; i < 48; i++)
		{
			if((Standard_Pub_Topic_Name[i] > 0 && Standard_Pub_Topic_Name[i] < 33) || Standard_Pub_Topic_Name[i] > 127)
			{
				sprintf(Standard_Pub_Topic_Name, "/BusToClient/%s/Buspro", HDL_MQTT_USE_MAC);
				break;
			}
		}
		memset(Buffer, 0, 50);
		Buffer[0] = 1;//1为订阅
		netip_gateway_get_STANDAR_MQTT_TOPIC(Buffer);
		memcpy(Standard_Sub_Topic_Name,&Buffer[2],48);
		for(i = 0; i < 48; i++)
		{
			if( (Standard_Sub_Topic_Name[i] > 0 && Standard_Sub_Topic_Name[i] < 33) || Standard_Sub_Topic_Name[i] > 127)
			{
				sprintf(Standard_Sub_Topic_Name, "/ClientToBus/%s/#", HDL_MQTT_USE_MAC);
				break;
			}
		}
		memcpy(Pub_Topic_Name, Standard_Pub_Topic_Name, 49);
		memcpy(Sub_Topic_Name, Standard_Sub_Topic_Name, 49);
	}
}
/*************************************************************************************************************************************************************************************************/
#ifdef HDL_MQTT_ENABLE
//初始化HDL MQTT
static void host_Reset(void)
{
	iot->hdl_host = NULL;
}

static err_t Iot_Hdl_MQTT_low_recvfrom(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
	if (p != NULL)
	{
		tcp_recved(tpcb, p->tot_len);
		if(-1 != _callback((char *)p->payload, p->tot_len))
		{
			iot_reply_signal = 1;
		}
		pbuf_free(p);
	}
	else if (err == ERR_OK)
	{
		tcp_recved(tpcb, p->tot_len);
		return tcp_close(tpcb);
	}
	return ERR_OK;
}

static int Iot_Hdl_MQTT_Init(void)
{
	int rc = -1;
	unsigned long wwul1 = NTimeMS();
	struct in_addr ip_addr;
	char host_ipaddr[20] = {0};
//(1)DNX	
#if(1)	
	ip_addr = linux_gethostbyname((char *)IOT_CONFIG_URL);	//域名解析
	if(ip_addr.s_addr == 0)
	{
		memcpy(fail_url, IOT_CONFIG_URL, strlen(IOT_CONFIG_URL));
		Iot_Debug_Monitor("hdl_host_error",14);
		return -1;   //error
	}
	strcpy(host_ipaddr,(inet_ntoa(ip_addr)));
	
	//strcpy(host_ipaddr,"192.168.1.26");
	if(iot->apply_ok != 1) //如果申请不ok
	{
		iot->sockfd = linux_open(1,(host_ipaddr),IOT_CONFIG_PORT,1,0);
		if(iot->sockfd < 0)
		{
			memset(host_ipaddr,0,sizeof(host_ipaddr));
			host_Reset();
			system_reboot();
			Iot_Debug_Monitor("linux_open_error",16);
			return -1;
		}
		// tcp callback
		tcp_recv(netsock[iot->sockfd].tpcb, Iot_Hdl_MQTT_low_recvfrom);
	#else
	//(2)固定IP 114.215.173.87
	//	iot->sockfd = linux_open(1,"114.215.173.87",IOT_CONFIG_PORT,1,0);
	#endif
		//申请
		Iot_Debug_Monitor("apply", 5);
		if (iot_request(iot->sockfd, AESKEY, 0) != ERR_OK)//发送请求密码（获取MQTT服务器 aeskey port等等）
		{
			linux_close(iot->sockfd);	
			return -1;
		}
		rc = -1;
		wwul1 = NTimeMS();
		while (abs(NTimeMS() - wwul1) < 5000)
		{
			if (iot_reply_signal)
			{
				rc = 1;
				break;
			}
			linux_handler();//防止在while循环的时候，其它功能停止
		}
		
		linux_close(iot->sockfd);	
		if (iot_reply_signal == 0) 
		{
			return -1;
		}
		else if(iot_reply_signal == 1)
		{
			iot->apply_ok = 1;
		}
	}
	// connect
	iot->sockfd = linux_open(1,(host_ipaddr),IOT_CONFIG_PORT,1,0);
	if(iot->sockfd < 0)
	{
		memset(host_ipaddr,0,sizeof(host_ipaddr));
		host_Reset();
		system_reboot();
		Iot_Debug_Monitor("linux_open_error",16);
		return -1;
	}
	// tcp callback
	tcp_recv(netsock[iot->sockfd].tpcb, Iot_Hdl_MQTT_low_recvfrom);
	//登录
	Iot_Debug_Monitor("login", 5);
	if (iot_request(iot->sockfd, key_buf, 1) != ERR_OK)//发送请求密码（获取MQTT服务器 aeskey port等等）
	{
		linux_close(iot->sockfd);	
		return -1;
	}
	rc = -1;
	wwul1 = NTimeMS();
	while (abs(NTimeMS() - wwul1) < 5000)
	{
		if (iot_reply_signal)
		{
			iot_reply_signal = 0;
			iot->apply_ok = 0;
			rc = 1;
			break;
		}
		linux_handler();//防止在while循环的时候，其它功能停止
	}
	linux_close(iot->sockfd);	

	return rc;
}
#endif
	
/****************************************************************************************************************************************************************************************************/
//标准MQTT初始化函数
static int Iot_Standar_MQTT_Init(void)
{
	unsigned char Buffer[64] = {0};
	char host_ipaddr[20] = {0};
	static int fail_url_flag = -1;
	static unsigned long fail_url_clear = 0;
	struct in_addr ip_addr;
	netip_gateway_get_Standar_MQTT_URL_message(Buffer);//获取域名和端口号
	memcpy(iot->url, &Buffer[1], 60);
	if(!(iot->url[0] >= '0' && iot->url[0] <= '9'))
	{
		if((iot->url[0] < 0x30 || iot->url[0] > 0x7A)
			|| (iot->url[1] < 0x30 || iot->url[1] > 0x7A)
			|| (iot->url[2] < 0x30 || iot->url[2] > 0x7A)
		)
		{
			return -1;
		}
		if(!strcmp(iot->url, fail_url))
		{
			if(fail_url_flag != -1) 
			{
				if(abs(NTimeMS() - fail_url_clear) < 10000)//不为第一次并且相差30s内
				{
					return -1;//error
				}
				else	//如果超过五秒，先置为-1，后判断此次域名解析是否成功
				{
					memset(fail_url, 0, strlen(fail_url));
				}
			}
		}
		ip_addr = linux_gethostbyname((char *)iot->url);	//域名解析
		if(ip_addr.s_addr == 0)
		{
			fail_url_clear = NTimeMS(); //记录当前时间
			fail_url_flag = 0;
			memcpy(fail_url, iot->url, strlen(iot->url));
			Iot_Debug_Monitor("hdl_host_error",14);
			return -1;   //error
		}
		strcpy(host_ipaddr,(inet_ntoa(ip_addr)));
	}
	else
	{
		strcpy(host_ipaddr,iot->url);
	}
	iot->ipaddr.sin_addr.s_addr = inet_addr(host_ipaddr);//得到IP
	memcpy(iot->port.ch, &Buffer[61],2); //得到端口号字符串
	iot->ipaddr.sin_port = NC2(iot->port.i);//得到端口号
/*******************获取客户端ID用户名和密码***********************************/
	memset(Buffer, 0, 64);
	netip_gateway_get_Standar_MQTT_message(Buffer);
	
	memcpy(iot->clientId, Buffer, 32);//客户端ID
	memcpy(iot->username, &Buffer[32], 16);//用户名
	memcpy(iot->password, &Buffer[48], 16);//密码
	memset(Buffer, 0, 64);
/*******************获取AES加密模式***********************************/
	netip_gateway_get_STANDAR_MQTT_AES(Buffer);
	if(Buffer[0] != 0) //开启AES加密模式
	{
		iot->aes_status = 1;//通过该状态位来区分标准MQTT是否选择加密
		memcpy(iot->aeskey, &Buffer[1], 16);
	}
	else
	{
		iot->aes_status = 0; 
	}
	return 1;
}

int _set_coreurl(unsigned char *buffer, int buflen)
{
	int i = 0;
	int sum = 0;
	unsigned char zero[64] = {0};
	unsigned char flag[2] = {0x01, 0x00};
	if(buflen <= 1) return -1;
	if(buflen <= 64) return -1;
	for(i = 0; i < 64; i++)
	{
		sum += buffer[i];
	}
	sum = 0x100 - sum;
	if((unsigned char)(sum&0xff) != buffer[64])
	{
		return -1; 
	}
	memcpy(zero, buffer, 64);
	
	eeprom_write(IOT_CONFIGURE_URL_FLAG_ADDR, flag, 1);
	eeprom_write(IOT_CONFIGURE_URL_ADDR, zero, 64);
	if(iot->enable == HDL_MQTT)
	{
		iot->status = 4;//重新连接
		iot->apply_flag = 6; //重新申请
	}
	return 0;
}

unsigned char _get_coreurl(unsigned char *buffer)
{
	unsigned char len = 0;
	len = strlen(IOT_CONFIG_URL);
	if(len > 64) len = 64;
	memcpy(buffer, IOT_CONFIG_URL, 64);
	return 64;
}
/*************************************************************************************************************************************************************************************************/
//1ms处理函数(yk 2020-02-20加) 
static void _oneMs_handler(void)
{
	if(iot->status != 3) return;
	static unsigned char mutex = 0; 

	static unsigned int timeout = 0;
	static unsigned int keepalive_time = 0;
	static unsigned int mqtt_handler_time = 0;
	if(clear_time == 1)
	{
		mqtt_handler_time = 0;
		timeout = 0;
		keepalive_time = 0;
		fail_timestamp = 0;
		clear_time = 0;
	}
	if(mutex == 1) return; 
	mutex = 1;
	
	mqtt_handler_time++;
	if(mqtt_handler_time < 10) 
	{
		mutex = 0;
		return;
	}
	mqtt_handler_time = 0;

	timeout++;
	if(timeout < 50*2)
	{
		mutex = 0;
		return;  // 1s
	}
	timeout = 0;
	if(iot->status == 3)
	{
		if(get_tcp_recv_flag() != 1)
		{		
			fail_timestamp++;
			if(fail_timestamp >= 22)
			{
				iot->status = 4; //服务器断开
				fail_timestamp = 0;
			}
		}
		else
		{
			set_tcp_recv_flag(0);
			fail_timestamp = 0;
		}
	}

	//心跳 10s 发送一次
	keepalive_time++;
	if(keepalive_time < 10)
	{
		mutex = 0;
		return;  // 10s
	}
	keepalive_time = 0;	
	if(iot->status == 3)
	{
		mqtt_keepalive();
	}
	mutex = 0;
} 

static void _init(void)
{
	unsigned char MQTT_Buffer[80] = {0};
	unsigned char flag[2] = {0};
	iot = &iot_config;
	iot->monitor = 0;
	iot->sockfd_flag = 0;
	iot->apply_flag = 0;
	iot->apply_ok = 0;
	
	iot->mqtt_message_flag = 0;
	clear_time = 0;
	//获取组名 工程名 和 用户名
	netip_gateway_get_message(MQTT_Buffer, 0);
	//获取账号和住宅ID
	memset(MQTT_Buffer, 0, 80);
	netip_gateway_get_MQTT_message(MQTT_Buffer);
	memset(MQTT_Buffer, 0, 80);
	
	//读取当前是否已经申请到了MQTT登录信息
	eeprom_read(HDLMQTT_CONFIG_ADDR, temp_buffer, HDLMQTT_AES_SIZE + 1);
	if(!strncmp((char *)temp_buffer, "1", 1))
	{
		unsigned int current_pos = 0;
		union MQTT_IP ip;
		iot->mqtt_message_flag = 1;
		IOT_AES_DECRYPT((unsigned char *)AESKEY, HDLMQTT_AES_SIZE, &temp_buffer[1], base_buffer);
		
		memcpy(iot->clientId, base_buffer, HDLMQTT_CLIENTID_SIZE);
		current_pos += HDLMQTT_CLIENTID_SIZE;
		
		memcpy(iot->username, &base_buffer[current_pos], HDLMQTT_USERNAME_SIZE);
		current_pos += HDLMQTT_USERNAME_SIZE;
		
		memcpy(iot->password, &base_buffer[current_pos], HDLMQTT_PASSWORD_SIZE);
		current_pos += HDLMQTT_PASSWORD_SIZE;
		
		memcpy(iot->aeskey, &base_buffer[current_pos], HDLMQTT_AESKEY_SIZE);
		current_pos += HDLMQTT_AESKEY_SIZE;
		
		memcpy(ip.ch, &base_buffer[current_pos], HDLMQTT_IP_SIZE);
		iot->ipaddr.sin_addr.s_addr = ip.i;
		
		current_pos += HDLMQTT_IP_SIZE;	
		memcpy(iot->port.ch, &base_buffer[current_pos], HDLMQTT_PORT_SIZE);
		
		eeprom_read(HDL_MQTT_UUID_ADDRESS ,(unsigned char *)MQTT_MAC_UUID, HDL_MQTT_UUID_SIZE);
		iot->ipaddr.sin_port = iot->port.i;
	}
	memset(MQTT_Buffer, 0, 80);
	eeprom_read(IOT_CONFIGURE_URL_FLAG_ADDR, flag, 1);
	if(flag[0] == 0x01)
	{
		eeprom_read(IOT_CONFIGURE_URL_ADDR, MQTT_Buffer, 64);
		if(strncmp((char *)MQTT_Buffer, IOT_CONFIG_URL, 64))
		{
			iot->mqtt_message_flag = 0;//重新升级了
			memcpy(IOT_CONFIG_URL, MQTT_Buffer, 64);
		}
	}
}

/*****************************************************************************************************************************************************************************************************************
Name:		_handler
Function:	MQTT处理状态机 
			0:延时作用
			1:获取MQTT服务器基本信息 
			2:连接MQTT服务器
			3:确认发送心跳包后MQTT服务器是否有反溃,处理数据
			4:关闭MQTT服务器TCP连接
			5:重新连接
Called By:	main()	(@from main.c)
Parameter:	无
Return:		无
*****************************************************************************************************************************************************************************************************************/
static void _handler(void)
{
	static unsigned long wwul1 = 0;
	unsigned int debug_len = 0;
	static unsigned char mutex = 0; 
	if(mutex == 1) return;
	mutex = 1;
	
	switch (iot->status)
	{
		case 0:
			//获取组名用户名工程名和mac的CRC校验和
			if(iot->apply_flag >= 0 && iot->apply_flag < 6)
			{
				wwul1 = NTimeMS();
				while (abs(NTimeMS() - wwul1) < 1000) //2S
				{
					linux_handler();//防止在while循环的时候，其它功能停止
				}
				iot->status++;
				break;
			}
			iot->mqtt_message_flag = 0;
			iot->apply_flag = 0;
			wwul1 = NTimeMS();
			while (abs(NTimeMS() - wwul1) < 1000) //2S
			{
				linux_handler();//防止在while循环的时候，其它功能停止
			}
			iot->status++;
			break;
		
		case 1:
			if(iot->enable == STANDARD_MQTT)
			{
				if(1 == Iot_Standar_MQTT_Init())
				{
					iot->status++;
					mqtt_client_set(iot->clientId,(unsigned int)iot->ipaddr.sin_addr.s_addr,iot->ipaddr.sin_port,iot->username,iot->password);//设置基本信息
				}
			}
			else if (iot->enable == HDL_MQTT) 
			{	
				if(iot->mqtt_message_flag == 1 || (iot->apply_flag > 0 && iot->apply_flag != 6))
				{
					iot->status++;
					mqtt_client_set(iot->clientId,(unsigned int)iot->ipaddr.sin_addr.s_addr,iot->ipaddr.sin_port,iot->username,iot->password);//设置基本信息
					break;
				}
				//添加一个走过状态1的标志位,用于决定是否发送重新上线指令
				iot->apply_enable = 1;
				
				
				//获取MQTT服务器基本信息 
				#ifdef HDL_MQTT_ENABLE
					if (1 == Iot_Hdl_MQTT_Init())
					{			
						iot->apply_flag = 1;
						iot->status++;
						mqtt_client_set(iot->clientId,(unsigned int)iot->ipaddr.sin_addr.s_addr,iot->ipaddr.sin_port,iot->username,iot->password);//设置基本信息
					}
				#endif
			}
			break;
			
#ifdef MQTT_CLIENT_ENABLE		
		case 2:
			//连接MQTT服务器
			if(!mqtt_client_init())//yk 2020-01-16改
			{
				if (iot->enable == HDL_MQTT)
				{				
					if(mqtt_client_subscribe(Sub_Topic_Name) != 0)
					{
						iot->status = 4;
						break;
					}
					if(mqtt_client_subscribe("/CloudTimestamp") != 0)
					{
						iot->status = 4;
						break;
					}
					
					if(iot->apply_enable == 1)
					{
						char necessary_topic[100] = {0}; //连接成功后要发送一个空负载主题 /BusGateWayToClient/UUID值/NotifyGateWayInfoChange
						unsigned int current_pos = 0;
						snprintf(necessary_topic,100,"/BusGateWayToClient/%s/NotifyBusGateWayInfoChange",MQTT_MAC_UUID);
						mqtt_client_publish(necessary_topic, "...", 0, Qos0);
						memset(temp_buffer, 0, 1200);
						
						memcpy(&temp_buffer[current_pos], iot->clientId, 64);
						current_pos += 64;
						memcpy(&temp_buffer[current_pos], iot->username, 64);
						current_pos += 64;
						memcpy(&temp_buffer[current_pos], iot->password, 16);
						current_pos += 16;
						memcpy(&temp_buffer[current_pos], iot->aeskey, 16);
						current_pos += 16;
						memcpy(&temp_buffer[current_pos], (char *)&(iot->ipaddr.sin_addr.s_addr), 4);
						current_pos += 4;
						iot->port.i = iot->ipaddr.sin_port;
						memcpy(&temp_buffer[current_pos], iot->port.ch, 2);
						current_pos += 2;
						
						//当前current_pos为总长度
						debug_len = IOT_AES_ENCRYPT((unsigned char *)AESKEY, current_pos ,temp_buffer,base_buffer);
						memcpy(temp_buffer, "1", 1);
						memcpy(&temp_buffer[1], base_buffer, debug_len);
						eeprom_write(HDLMQTT_CONFIG_ADDR, temp_buffer, debug_len + 1);
						iot->mqtt_message_flag = 1;
						iot->apply_enable = 0;	
					#ifdef DATETIME_ENABLE
						Timestamp_to_datetime(iot->timestamp.in);	
					#endif
					}
					//subcribe_time = 5; //5s后订阅下一个主题
					iot->apply_flag = 1;
					fail_timestamp = 0;
					iot->status++;
					clear_time = 1;//清空所有时间
				}
				else
				{
					if(mqtt_client_subscribe(Sub_Topic_Name) != 0)
					{
						iot->status = 4;
						break;
					}
					if(mqtt_client_subscribe("/CloudTimestamp") != 0)
					{
						iot->status = 4;
						break;
					}
					clear_time = 1;
					iot->status++;				
				}
			}
			else
			{
				if (iot->enable == HDL_MQTT)
				{
					iot->apply_flag++;
				}
				iot->status = 4;
			}
			break;
			
		case 3:
			//接收数据处理
			if(!mqtt_client_handler()) //yk 2020-01-16改
			{
				iot->status++;
			}
			break;
			
		case 4:
			//关闭客户端
			iot->status = 0;
			iot->timeout = 0;
			mqtt_client_close();
			subcribe_time = 0; //清空
			break;
#endif	
		case 5:	
			
			break;
		
		default:
			iot->status = 0;
			break;
	}
	
	mutex = 0;
}

/************************************标准函数接口*************************************************/
const struct iot_config_t iot_config_interface =
{
	 _init, _handler, _get_status, _set_status,  _push, _write,  _send, _long_send, _read_callback,  _setAES, _getAES, _setTopic, _getTopic, _oneMs_handler, _set_monitor, _get_monitor, _set_coreurl, _get_coreurl
};


void iot_debug_status(int status)
{
	iot->status = status;
}	

void iot_debug_reapply(void)
{
	iot->status = 4;
	iot->apply_flag = 6;
}
/*****************************************************************************************************************************************************************************************************/
unsigned long ip2int(const char *ip)
{
	char temp_ip[16] = {0};
	int i,j=0;
	unsigned long ret = 0;
	unsigned char tem_char = 0; 
	unsigned char one_ip_str[4] = {0};
	if(ip == NULL)
	{
		return 0;
	}
	memcpy(temp_ip, ip, strlen(ip));
	for(i = 0; i <= strlen(ip); i++)
	{
		if(temp_ip[i] == '.' || temp_ip[i] == '\0')
		{
			tem_char = atoi((char *)one_ip_str);
			ret += tem_char<<(8*(i/4));
			memset(one_ip_str,0,3);
			j = 0;
		}
		else
		{
			one_ip_str[j] = temp_ip[i];
			j++;
		}
	}
	return ret;
}
/************************************************************************
Function:	test_read_somedata
desc:   
arking add 2020-02-12
*************************************************************************/
void test_read_somedata(unsigned char *data)
{
    data[0] = iot->status;
	  data[1] = iot->enable;
    memcpy(&data[2],iot->aeskey,16);	
}
#endif









