/***************************************************
模块名称：  commands
功能描述：	
设 计 者:	薛柯利(steven sit)  
编写时间：	2012-06-27
***************************************************/
#include "commands.h" 
#include "sys_include.h" 




//
#define NC2(X)			(((X & 0x00ff) << 8) | ((X & 0xff00) >>8))




/***************************************************
			commands
***************************************************/
#pragma pack(1)
struct commands_t
{
	unsigned char length;
	unsigned char src_subnet;
	unsigned char src_device;
	unsigned short device_type;
	unsigned short cmd;
	unsigned char dest_subnet;
	unsigned char dest_device;
	unsigned char payload[67];
	
};
#define PAYLOAD_SIZE(X)			(X-11)
#pragma pack()



unsigned char reply_buffer[67];



/***************************************************
函数名称：	 
输入参数：	
返 回 值:	
功能描述：	 
***************************************************/
int app_commands_send(int s,unsigned int cmd,unsigned char subnet,\
					unsigned char device,unsigned char *buffer,unsigned char size)
{
	int res = 0;
	
	switch(s)
	{
#ifdef BUSPRO_ENABLE
		case 0:
			res = buspro_send(cmd,devices->subnet,devices->device,subnet,device,buffer,size);
			break;
#endif
		
#ifdef NETIP_ENABLE		
		case 1:
			res = netip_send(cmd,devices->subnet,devices->device,subnet,device,buffer,size);
			break;
		
		case 255:
			res = netip_allbroadcast_send(cmd,devices->subnet,devices->device,subnet,device,buffer,size);
			break;
#endif
		
	}
	return res;
}

#ifdef NETIP_ENABLE

/***************************************************
函数名称：   netip_commands_handler
输入参数：	
返 回 值:	 
功能描述：	  
***************************************************/
void netip_commands_long_frame(unsigned char *buffer,int size)
{
	
}

/***************************************************
函数名称：   netip_commands_handler
输入参数：	
返 回 值:	 
功能描述：	  
***************************************************/
void netip_commands_handler(int s,unsigned char *buffer, int size)
{
	struct commands_t *frame;
	unsigned short cmd;
	
	//REMOTE_IP = remote_ipaddr;
	//remote_port	= NC2(remote_port);
	
	frame = (struct commands_t *)buffer;
	cmd = NC2(frame->cmd);
	
	if (s) // remote
	{
		if (cmd == 0xE54A /*READ_PROGRAM_REMARK_CMD*/
			|| cmd == 0xE54C /*WRITE_PROGRAM_REMARK_CMD*/
			|| cmd == 0x1400 /*READ_DEVICE_KEY_STATUS_CMD*/
			|| cmd == 0X300C /*MODULE_REGISTER_SERVER_ACK*/
			|| cmd == 0X300F /*MODULE_PASSWORD_LINK_CMD */
			|| cmd == 0x301E /*SOFTWARE_LINK_GATEWAY_CMD*/)
		{
			;
		}
		else
		{
			// remote link status
			#ifdef REMOTE_ENABLE	
	     if (!remote_interface->status(buffer,size))
	     {
	        return;
	     }
			#endif
		}
			
			
		if (frame->length < 79 && frame->length > 10)//小包
		{
			#ifdef GATEWAY_ENABLE	
			if(frame->dest_subnet != devices->subnet)
			{
				if (gateway_interface.status(buffer,size))
				{
					netip_write((char *)buffer,size);
				}
			}
			#endif
				
			if((frame->dest_subnet == devices->subnet \
			|| frame->dest_device == 255) \
			|| cmd == 0xF003 || cmd == 0xF005\
			|| cmd == 0xFE03 || cmd == 0xFE05)
			{
				// cmd handler
				app_commands_handler(1,buffer,buffer[0]);
			}
		}
		else if (frame->length == 255)//大包
		{
			if (frame->dest_subnet == devices->subnet)//本SUBNET
			{
				// long frame
				netip_commands_long_frame(buffer,size);
			}
			else
			{
				#ifdef GATEWAY_ENABLE	
				if (gateway_interface.status(buffer,size))
				{
					netip_write((char *)buffer,size);
				}
				#endif
			}
		}
	}
	else//local
	{
		if (frame->length < 79 && frame->length > 10)//小包
		{
			
			#ifdef GATEWAY_ENABLE	
			if(frame->dest_subnet != devices->subnet)// send server
			{	
				gateway_interface.write(buffer,size);
			}
			#endif
				
			if((frame->dest_subnet == devices->subnet \
			|| frame->dest_device == 255) \
			|| cmd == 0xF003 || cmd == 0xF005\
			|| cmd == 0xFE03 || cmd == 0xFE05)
			{
				// cmd handler
				app_commands_handler(1,buffer,buffer[0]);
			}
		}
		else if (frame->length == 255)//大包
		{
			if (frame->dest_subnet == devices->subnet)//本SUBNET
			{
				// long frame
				netip_commands_long_frame(buffer,size);
			}
			else
			{
				#ifdef GATEWAY_ENABLE	
				gateway_interface.write(buffer,size);
				#endif
			}
		}
	}
}

#endif

/***************************************************
函数名称：	 
输入参数：	
返 回 值:	
功能描述：	 
***************************************************/
void app_commands_handler(int s,unsigned char *buffer,unsigned char size)
{
	struct commands_t *frame;
	int cmd;
	
	frame = (struct commands_t *)buffer;
	cmd = NC2(frame->cmd);
	
	if (s)
	{
	;//	buspro_write(buffer,size);
	}
	else
	{
		netip_write(buffer,size);
	}
	
#if 1
	// check address
	
	// commands handler
	switch (cmd)
	{
/*********************************************/
/*			   基本操作命令					 */
/*********************************************/
#ifdef DEVICE_ENABLE
		//读设备备注
		case READ_DEVICE_REMARK_CMD:
			devices_get_remark(frame->payload);
			app_commands_send(s,READ_DEVICE_REMARK_ACK, \
				frame->src_subnet,frame->src_device,frame->payload,20);
			break;

		//写设备备注
		case WRITE_DEVICE_REMARK_CMD:
			if (devices_set_remark(frame->payload))
			{
				frame->payload[0] = 0xf5;
			}
			else
			{
				frame->payload[0] = 0xf8;
			}
			app_commands_send(s,WRITE_DEVICE_REMARK_ACK, \
				frame->src_subnet,frame->src_device,frame->payload,1);
			break;

		//读软件版本号
		case READ_SOFTWARE_VERSION_CMD:	
			app_commands_send(s,READ_SOFTWARE_VERSION_ACK, \
				frame->src_subnet,frame->src_device,(unsigned char*)SOFTWARE_VERSION,22);
			break;

		//写设备MAC
		case WRITE_DEVICE_MAC_CMD:	 
			if (devices_set_mac(frame->payload))
			{
				frame->payload[0] = 0xf5;
			}
			else
			{
				frame->payload[0] = 0xf8;
			}
			app_commands_send(s,WRITE_DEVICE_MAC_ACK, \
				frame->src_subnet,frame->src_device,frame->payload,1);
			break;	

	    //读设备信息
		case READ_DEVICE_BASIC_INFO_CMD: 
			devices_get_mac(frame->payload);
		
			app_commands_send(s,READ_DEVICE_BASIC_INFO_ACK, \
				frame->src_subnet,frame->src_device,frame->payload,10);
			break;

		//写设备信息地址
		case WRITE_DEVICE_BASIC_INFO_CMD: 
			if (devices_check_set_subnet(frame->payload))
			{
				frame->payload[0] = 0xf5;
			}
			else
			{
				frame->payload[0] = 0xf8;
			}
			app_commands_send(s,WRITE_DEVICE_BASIC_INFO_ACK, \
				frame->src_subnet,frame->src_device,frame->payload,10);
			break;


		//按键读设备地址
		case READ_DEVICE_MODULE_ID_CMD: 
			if (!devices_get_subnet(frame->payload))
			{
				app_commands_send(s,READ_DEVICE_MODULE_ID_ACK, \
				frame->src_subnet,frame->src_device,frame->payload,2);
			}
			break;

		//按键写设备地址
		case WRITE_DEVICE_MODULE_ID_CMD: 
			if (!devices_set_subnet(frame->payload))
			{
				app_commands_send(s,WRITE_DEVICE_MODULE_ID_ACK, \
				frame->src_subnet,frame->src_device,frame->payload,10);
			}
			break;
			
			//出厂恢复
		case RESET_TARGET_INFO_CMD:
			if (!devices_reset_config(frame->payload))
			{
				app_commands_send(s,RESET_TARGET_INFO_ACK, \
				frame->src_subnet,frame->src_device,frame->payload,4);
			}
			break;
			
			//智能定位
		case SEARCH_DEVICE_POINT_CMD:
			if (!devices_setting_position(frame->payload))
			{
				frame->payload[0] = 0xf8;
				app_commands_send(s,SEARCH_DEVICE_POINT_ACK, \
				frame->src_subnet,frame->src_device,frame->payload,1);
			}
			break;

		//读简易编程
		case READ_PROGRAM_REMARK_CMD:
			if (!devices_get_program(frame->payload,PAYLOAD_SIZE(frame->length)))
			{
				app_commands_send(s,READ_PROGRAM_REMARK_ACK, \
				frame->src_subnet,frame->src_device,frame->payload,53);
				app_commands_send(255,READ_PROGRAM_REMARK_ACK, \
				frame->src_subnet,frame->src_device,frame->payload,53);
			}
			break;
			
		//写简易编程
		case WRITE_PROGRAM_REMARK_CMD:
			if (!devices_set_program(frame->payload,PAYLOAD_SIZE(frame->length)))
			{
				app_commands_send(s,WRITE_PROGRAM_REMARK_ACK, \
				frame->src_subnet,frame->src_device,frame->payload,0);
			}
			break;
			
#endif
	

/*********************************************/
/*			  在线升级						*/
/*********************************************/	
#ifdef IAP_ENABLE
		case UPDATA_APP_CMD:
			if (s)
			{
				iap_program_request();
				app_commands_send(s,UPDATA_APP_ACK, \
					frame->src_subnet,frame->src_device,(unsigned char *)MCU_CHIP,20);
			}
			break;
#endif

		
/*********************************************/
/*			  mcu type	configure							*/
/*********************************************/		
#ifdef DEVICE_MCU_ENABLE
		case READ_DEVICE_MCU_TYPE_CMD:
			memcpy(&frame->payload[0],PCB_VERSION,10);
			memcpy(&frame->payload[10],MCU_CHIP,20);

			app_commands_send(s,READ_DEVICE_MCU_TYPE_ACK, \
					frame->src_subnet,frame->src_device,frame->payload,30);
			break;
		
		
		case READ_DEVICE_MCU_UID_CMD:
			serial_crypt(frame->payload,MCU_UID);
			app_commands_send(s,READ_DEVICE_MCU_UID_ACK, \
					frame->src_subnet,frame->src_device,frame->payload,16);
	    break;
#endif


			
/*********************************************/
/*			  bandrate	configure							*/
/*********************************************/		
#ifdef BANDRATE_ENABLE		
		case BANDRATE_READ_CMD:
			buspro_get_bandrate(frame->payload);
			app_commands_send(s,BANDRATE_READ_ACK, \
					frame->src_subnet,frame->src_device,frame->payload,1);
			break;
			
			case BANDRATE_WRITE_CMD:
			buspro_set_bandrate(frame->payload);
			app_commands_send(s,BANDRATE_WRITE_ACK, \
					frame->src_subnet,frame->src_device,frame->payload,1);
			break;
#endif

			
/*********************************************/
/*			  route	configure							*/
/*********************************************/		
#ifdef ROUTE_ENABLE		
		//读设备MAC
		case READ_DEVICE_MAC_ID_ACK:
			if (s == 0)
			route_register(0,frame->src_subnet,frame->src_device,frame->payload);
			break;
		
		case AUTO_SET_ROUTE_DEVICE_CMD:
			frame->length = route_events(frame->payload);
			app_commands_send(s,AUTO_SET_ROUTE_DEVICE_ACK, \
					frame->src_subnet,frame->src_device,frame->payload,frame->length);
			break;
#endif
		
/*********************************************/
/*			 datetime	configure							*/
/*********************************************/		
#ifdef DATETIME_ENABLE
		case READ_DATE_TIME_CMD:
			datetime_interface.read(frame->payload,7);
			app_commands_send(s,READ_DATE_TIME_ACK, \
					frame->src_subnet,frame->src_device,frame->payload,7);
			break;
		
		
#endif
		
/*********************************************/
/*			  filter configure							*/
/*********************************************/		
#ifdef FILTER_ENABLE
		case READ_FILTER_CONFIG_CMD:
			filter_interface.get(frame->payload,frame->length);
			app_commands_send(s,READ_FILTER_CONFIG_ACK, \
					frame->src_subnet,frame->src_device,frame->payload,7);
			break;

		case WRITE_FILTER_CONFIG_CMD:
			if (filter_interface.set(frame->payload,frame->lenght))
			{
				frame->payload[0] = 0xf5;
			}
			else
			{
				frame->payload[0] = 0xf8;
			}
			app_commands_send(s,WRITE_FILTER_CONFIG_ACK, \
					frame->src_subnet,frame->src_device,frame->payload,1);
			break;
#endif
		
/*********************************************/
/*			  remote server	configure							*/
/*********************************************/		
#ifdef NETIP_GATEWAY_ENABLE
		//读设备IP
		case READ_DEVICE_IPADDR_CMD:
			netip_gateway_get_ipaddress(frame->payload);
			app_commands_send(s,READ_DEVICE_IPADDR_ACK, \
					frame->src_subnet,frame->src_device,frame->payload,31);
			break;

     	//写设备IP
		case WRITE_DEVICE_IPADDR_CMD:
			if(netip_gateway_get_ipaddress(frame->payload))
			{
				frame->payload[0] = 0xf5;
			}
			else
			{
				frame->payload[0] = 0xf8;
			}	
			app_commands_send(s,WRITE_DEVICE_IPADDR_ACK, \
					frame->src_subnet,frame->src_device,frame->payload,1);
			break;
						
		// 服务器名字
		case DNS_NAME_READ_CMD:
			netip_gateway_get_domainname(frame->payload);
			app_commands_send(s,DNS_NAME_READ_ACK, \
					frame->src_subnet,frame->src_device,frame->payload,64);
			break;
			
		case DNS_NAME_WRITE_CMD:
			if(netip_gateway_set_domainname(frame->payload))
			{
				frame->payload[0] = 0xf5;
			}
			else
			{
				frame->payload[0] = 0xf8;
			}	
			app_commands_send(s,DNS_NAME_WRITE_ACK, \
					frame->src_subnet,frame->src_device,frame->payload,1);
			break;

			
		// 写工程名称
		case WRITE_MODULE_SERVER_INFO_CMD:
			if (netip_gateway_set_message(frame->payload,PAYLOAD_SIZE(frame->length)))
			{
				frame->payload[0] = 0xf5;
			}
			else
			{
				frame->payload[0] = 0xf8;
			}
			app_commands_send(s,WRITE_MODULE_SERVER_INFO_ACK, \
					frame->src_subnet,frame->src_device,frame->payload,1);
	    break;
		
		// 读工程名称
		case READ_MODULE_SERVER_INFO_CMD:
			netip_gateway_get_message(frame->payload,PAYLOAD_SIZE(frame->length));
			app_commands_send(s,READ_MODULE_SERVER_INFO_ACK, \
					frame->src_subnet,frame->src_device,frame->payload,60);
			break;
		
		// 写服务器地址
		case WRITE_MODULE_SERVER_IP_CMD:
			if (netip_gateway_set_server(frame->payload,PAYLOAD_SIZE(frame->length)))
			{
				frame->payload[0] = 0xf5;
			}
			else
			{
				frame->payload[0] = 0xf8;
			}
			app_commands_send(s,WRITE_MODULE_SERVER_IP_ACK, \
					frame->src_subnet,frame->src_device,frame->payload,1);
	    break;
			
		// 读服务器地址
		case READ_MODULE_SERVER_IP_CMD:
			netip_gateway_get_server(frame->payload,PAYLOAD_SIZE(frame->length));
			app_commands_send(s,READ_MODULE_SERVER_IP_ACK, \
					frame->src_subnet,frame->src_device,frame->payload,15);
	    break;
		
		// 写历史记录
		case WRITE_HISTORY_EVENTS_CMD:
			if (s == 0) break;
			if (netip_gateway_set_history(remote_ipaddr,remote_port,frame->payload,PAYLOAD_SIZE(frame->length)))
			{
				frame->payload[0] = 0xf5;
			}
			else
			{
				frame->payload[0] = 0xf8;
			}
			app_commands_send(s,WRITE_HISTORY_EVENTS_ACK, \
					frame->src_subnet,frame->src_device,frame->payload,1);
	    break;
		
		// 读历史记录
		case READ_HISTORY_EVENTS_CMD:
			if (s == 0) break;
			frame->length = netip_gateway_get_history(remote_ipaddr,remote_port,frame->payload,PAYLOAD_SIZE(frame->length));
			app_commands_send(s,READ_HISTORY_EVENTS_ACK, \
					frame->src_subnet,frame->src_device,frame->payload,frame->length);
	    break;
		
		
		// 读连接数
		case READ_LINK_GATEWAY_INFO_CMD:
			if (s == 0) break;
		
			netip_gateway_remote_link_endpoint(frame->payload);
			app_commands_send(s,READ_LINK_GATEWAY_INFO_ACK, \
					frame->src_subnet,frame->src_device,frame->payload,40);
	    break;
		
		// 断开连接
		case EXIT_LINK_GATEWAY_INFO_CMD:
			if (s == 0) break;
		
			netip_gateway_remote_disable(remote_ipaddr,remote_port);
			frame->payload[0] = 0xf8;
			app_commands_send(s,EXIT_LINK_GATEWAY_INFO_ACK, \
					frame->src_subnet,frame->src_device,frame->payload,1);
	    break;
		
		// 密码认证
		case MODULE_PASSWORD_LINK_CMD:
			if (s == 0) break;
			if (netip_gateway_password_module(remote_ipaddr,remote_port,frame->payload,PAYLOAD_SIZE(frame->length)))
			{
				frame->payload[0] = 0xF5;
			}
			else
			{
				frame->payload[0] = 0xF8;
			}
			app_commands_send(s,MODULE_PASSWORD_LINK_ACK, \
					frame->src_subnet,frame->src_device,frame->payload,1);
	    break;
		
		// 软件连接
		case SOFTWARE_LINK_GATEWAY_CMD: 
			if (s == 0) break;
			if (netip_gateway_conneting_module(remote_ipaddr,remote_port,frame->payload,PAYLOAD_SIZE(frame->length)))
			{
				frame->payload[0] = 0xF5;
			}
			else
			{
				frame->payload[0] = 0xF8;
			}
			app_commands_send(s,SOFTWARE_LINK_GATEWAY_ACK, \
					frame->src_subnet,frame->src_device,frame->payload,1);
      break;
#endif
		
		
/*********************************************/
/*			   控制操作命令					 */
/*********************************************/
		
/*********************************************/
/*			   微信					 										*/
/*********************************************/
#ifdef WECHAT_ENABLE
			
			// 微信	开关状态
			case WECHAT_SEND_INFO_ACK:
				wechat_send_message_reply(frame->payload[0],frame->payload[1]);
				break;
			
			// 微信	UTF8 信息送到服务器
			case WECHAT_SEND_UTF8_CMD:
				wechat_send_utf8(frame->src_subnet,frame->src_device,frame->payload,PAYLOAD_SIZE(frame->length));
				frame->payload[0] = 0xf8;
				app_commands_send(s,WECHAT_SEND_UTF8_ACK, \
					frame->src_subnet,frame->src_device,frame->payload,1);
				break;	
				
			case WECHAT_SEND_UTF8_ACK:
				wechat_send_utf8_reply(frame->payload,PAYLOAD_SIZE(frame->length));
				break;
			
		  //设置通用开关
			case SET_UNIVERSAL_CMD:
				wechat_send_message(frame->src_subnet,frame->src_device,frame->payload[0],frame->payload[1]);
				if (frame->payload[1] == 0xff)
				{
					frame->payload[1] = 1;
				}
				else
				{
					frame->payload[1] = 0;
				}
				app_commands_send(s,SET_UNIVERSAL_ACK, \
					frame->src_subnet,frame->src_device,frame->payload,1);
				break;
#endif
			
				
		

				
				

		default: 
			break;
	}
#endif
	
}



