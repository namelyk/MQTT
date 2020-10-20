/***************************************************
ģ�����ƣ�  commands
����������	
�� �� ��:	Ѧ����(steven sit)  
��дʱ�䣺	2012-06-27
***************************************************/
#include "commands.h" 
#include "gateway.h"
#include "sys_include.h" 
#include "netcrypt.h"


const unsigned char success_flag = 0xf8;

#define NC2(X)			(((X & 0x00ff) << 8) | ((X & 0xff00) >>8))
#define IS_BROADCAST(A,B)			devices_is_broadcast(A,B)
#define IS_LOCAL(A,B)				devices_is_localaddress(A,B)
#define IS_PROG_MODE()				devices_button_get_mode()
#define IS_LOCAL_SUBNET(A)			devices_is_local_subnet(A)
#define BIG_BAO_SEND				netcrypt_long_send
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
	
	unsigned char payload[70];
};

struct long_commands_t
{
	unsigned char length;
	unsigned char src_subnet;
	unsigned char src_device;
	unsigned short device_type;
	unsigned short cmd;
	unsigned char dest_subnet;
	unsigned char dest_device;
	unsigned char payload[1200];
};
#define PAYLOAD_SIZE(X)			( X - 11 )

#pragma pack()



/***************************************************
�������ƣ�	 
���������	
�� �� ֵ:	
����������	 
***************************************************/
int app_commands_send(int s,unsigned int cmd,unsigned char subnet,\
					unsigned char device,unsigned char *buffer,unsigned char size)
{
	int res = 0;
	
	switch(s)
	{
		case 0:
			DISABLE_INTERRUPT();//�ر��ж�
			res = buspro_send(cmd,devices->subnet,devices->device,subnet,device,buffer,size);
			res = netcrypt_send(cmd,devices->subnet,devices->device,subnet,device,buffer,size);
			ENABLE_INTERRUPT();//���ж�
			break;

#ifdef NETIP_ENABLE		
		case 1:
#ifdef NETIP_GATEWAY_ENABLE	
			netip_gateway_send(cmd,subnet,device,buffer,size);
#endif
			res = netcrypt_send(cmd,devices->subnet,devices->device,subnet,device,buffer,size);
			if (cmd == READ_PROGRAM_REMARK_ACK
			|| cmd == WRITE_PROGRAM_REMARK_ACK)
			{	
				netcrypt_multi_send(cmd,devices->subnet,devices->device,subnet,device,buffer,size);
				netcrypt_allbroadcast_send(cmd,devices->subnet,devices->device,subnet,device,buffer,size);
			}
		
			break;
#endif
			
#ifdef NETIP_ENABLE	
		//Զ������
		case 100:
#ifdef NETIP_GATEWAY_ENABLE	
			netip_gateway_send(cmd,subnet,device,buffer,size);
#endif
			break;
#endif
		
		case 255:
			DISABLE_INTERRUPT();//�ر��ж�
			buspro_send(cmd,devices->subnet,devices->device,subnet,device,buffer,size);
			ENABLE_INTERRUPT();//���ж�
#ifdef NETIP_ENABLE		
			netcrypt_send(cmd,devices->subnet,devices->device,subnet,device,buffer,size);
			//netip_send(cmd,devices->subnet,devices->device,subnet,device,buffer,size);
		
			//netip_multi_send(cmd,devices->subnet,devices->device,subnet,device,buffer,size);
			// netip_allbroadcast_send(cmd,devices->subnet,devices->device,subnet,device,buffer,size);
#endif
			break;
	}
	
	return res;
}

#ifdef NETIP_ENABLE

/***************************************************
�������ƣ�   netip_commands_handler
���������	
�� �� ֵ:	 
����������	  
***************************************************/
void netip_commands_long_frame(unsigned char *buffer,int size)
{
	struct long_commands_t *frame;
	//yk 2019-12-30��
	unsigned char subnet_buffer[3] = {0};
	devices_get_subnet(subnet_buffer);
	
	int i,j;
	unsigned short cmd;
	int return_num = 0;
	
	frame = (struct long_commands_t *)buffer;
	cmd = NC2(frame->cmd);
	
	switch (cmd)
	{
#ifdef IAP_ENABLE
		case 0x15FF:
			iap_interface.write(frame->payload,size-9);
			
			break;
#endif
		//yk 2019-12-30��
		case MQTT_READ_MESSAGE_CMD:
			if (!IS_LOCAL(frame->dest_subnet,frame->dest_device))
				break;
		#ifdef IOT_ENABLE
			return_num = netip_gateway_get_MQTT_message(&frame->payload[2]);
			
			BIG_BAO_SEND(COMMAND_ACK(MQTT_READ_MESSAGE_CMD), \
				subnet_buffer[0], subnet_buffer[1], frame->src_subnet, frame->src_device, &frame->payload[2],return_num);
			if( netip_gateway_type() == MQTT_LINK_SERVER || netip_gateway_type() == STANDAR_MQTT_LINK_SERVER) //�������mqtt
			{
				#ifdef IOT_ENABLE	
					iot_config_interface.long_send(COMMAND_ACK(MQTT_READ_MESSAGE_CMD),subnet_buffer[0], subnet_buffer[1], frame->src_subnet, frame->src_device, &frame->payload[2],return_num);
				#endif
			}
			if( netip_gateway_type() == GATEWAY_LINK_SERVER || netip_gateway_type() == GATEWAY_LINK_DNS) //���������ͨԶ��
			{
				netip_gateway_longsend(COMMAND_ACK(MQTT_READ_MESSAGE_CMD), frame->src_subnet, frame->src_device, &frame->payload[2],return_num);
			}
		#endif
			break;
		//yk 2019-12-30��
		case MQTT_WRITE_MESSAGE_CMD:
			if (!IS_LOCAL(frame->dest_subnet,frame->dest_device))
				break;
		//yk 2020-05-29��
		#ifdef IOT_ENABLE
			return_num = netip_gateway_set_MQTT_message(&frame->payload[2]);
		#endif
	
			BIG_BAO_SEND(COMMAND_ACK(MQTT_WRITE_MESSAGE_CMD), \
				subnet_buffer[0], subnet_buffer[1], frame->src_subnet, frame->src_device, &frame->payload[2],return_num);
		#if 1
			if( netip_gateway_type() == MQTT_LINK_SERVER || netip_gateway_type() == STANDAR_MQTT_LINK_SERVER) //�������mqtt
			{
				#ifdef IOT_ENABLE	
					iot_config_interface.long_send(COMMAND_ACK(MQTT_WRITE_MESSAGE_CMD),subnet_buffer[0], subnet_buffer[1], frame->src_subnet, frame->src_device, &frame->payload[2],10);
				#endif
			}
			if( netip_gateway_type() == GATEWAY_LINK_SERVER || netip_gateway_type() == GATEWAY_LINK_DNS) //���������ͨԶ��
			{	
				netip_gateway_longsend(COMMAND_ACK(MQTT_WRITE_MESSAGE_CMD), frame->src_subnet, frame->src_device, &frame->payload[2],return_num);
			}
		#endif
			break;
		//д���ױ�� 
		case WRITE_PROGRAM_REMARK_CMD:
			if (!IS_LOCAL(frame->dest_subnet,frame->dest_device)) 
				break;
			
			if (!system_setting_set_program(&frame->payload[2],100))
			{
				app_commands_send(1,COMMAND_ACK(WRITE_PROGRAM_REMARK_CMD), \
						frame->src_subnet,frame->src_device,&frame->payload[2],0);
			}
			break;
		
	}
}

/***************************************************
�������ƣ�   netip_commands_handler
���������	
�� �� ֵ:	 
����������	  
***************************************************/
struct commands_t *frame;
void netip_commands_handler(int s,unsigned char *buffer, int size)
{
	unsigned short cmd;
	unsigned short device_type; //add
	frame = (struct commands_t *)buffer;
	cmd = NC2(frame->cmd);
	
	//�����ǰ�������� �������κ������ 2020-07-01
	if(system_reboot_get_enable())
	{
		return;
	}
	//��¼��ǰ����Դ
	if(cmd == UPDATA_APP_CMD)
	{
		if(s == 101) //MQTT
		{
			iap_set_update_source(MQTT_UPDATE);
		}
		else if(s == 100)
		{
			iap_set_update_source(REMOTE_UPDATE);
		}
		else
		{
			iap_set_update_source(LOCAL_UPDATE);
		}
	}
	
	if (s >= 100) // remote
	{
		if (!IS_LOCAL(frame->dest_subnet,frame->dest_device)) 
		{
		#ifdef NETIP_ENABLE
			netcrypt_interface.write(buffer, size);
		#endif
		}
		if (frame->length <= 79 && frame->length > 10)//С��
		{	
			if((frame-> dest_subnet == devices->subnet \
			|| frame->dest_device == 255 )\
			|| cmd == 0xF003 || cmd == 0xF005\
			|| cmd == 0xFE03 || cmd == 0xFE05)
			{
				app_commands_handler(100,buffer,buffer[0]);
			}
		}
		else if (frame->length == 255)//���
		{
			if (frame->dest_subnet == devices->subnet)//��SUBNET
			{	
				// long frame
				netip_commands_long_frame(buffer,size);
			}
		}
	}
	else//local (1)
	{
		device_type = NC2(frame->device_type);
		if(device_type == 0x8888)
		{
			//record_IP();//��¼IP
		}
		
		if (!IS_LOCAL(frame->dest_subnet,frame->dest_device)) 
		{
			#ifdef NETIP_GATEWAY_ENABLE
				netip_gateway_write(buffer,size);
			#endif
		}
		
		if (frame->length <= 79 && frame->length > 10)//С��
		{
			if((frame->dest_subnet == devices->subnet \
			|| frame->dest_device == 255 )\
			|| cmd == 0xF003 || cmd == 0xF005 \
			|| cmd == 0xFE03 || cmd == 0xFE05)
			{
				// cmd handler
				app_commands_handler(1,buffer,buffer[0]);
			}
		}
		else if (frame->length == 255)//���
		{
			if (frame->dest_subnet == devices->subnet)//��SUBNET
			{
				// long frame
				netip_commands_long_frame(buffer,size);
			}
		}
	}
}

#endif

/***************************************************
�������ƣ�	 
���������	
�� �� ֵ:	
����������	 
***************************************************/
#define SWITCH_ENABLE
void app_commands_handler(int s,unsigned char *buffer,unsigned char size)
{
	int cmd;
	unsigned char return_num;
	struct commands_t *frame = NULL;
	//yk 2019-12-30��
	unsigned char subnet_buffer[3] = {0};
	// frame struct
	frame = (struct commands_t *)buffer;
	cmd = NC2(frame->cmd);
	LED_UP_ON();
	
	if(IS_BROADCAST(frame->dest_subnet,frame->dest_device) && cmd == READ_DEVICE_REMARK_CMD) //�㲥000E
	{
		unsigned char Buffer[21];
		devices_get_remark(Buffer);
		app_commands_send(s,READ_DEVICE_REMARK_ACK, \
			frame->src_subnet,frame->src_device, Buffer,20);
	}
	
#if 1
	// check address
	#ifdef SWITCH_ENABLE
	if (!IS_LOCAL(frame->dest_subnet,frame->dest_device)) 
	{
		if (s)
		{
		#ifdef FILTER_ENABLE
			if (filter_interface.handler((void *)buffer))
		#endif
				if(IS_LOCAL_SUBNET(frame->dest_subnet) || IS_BROADCAST(frame->dest_subnet,frame->dest_device) || cmd== 0xF003 || cmd== 0xF005 || cmd==0xFE03 || cmd==0xFE05)//���ػ��߹㲥
				{
					DISABLE_INTERRUPT();//���ж�
					buspro_write(buffer,size);// yk 2019-12-31��
					ENABLE_INTERRUPT();//���ж�
				} 
		}
		else 
		{
	#ifdef NETIP_ENABLE
			netcrypt_interface.write(buffer, size);
	#ifdef NETIP_GATEWAY_ENABLE	
			netip_gateway_write(buffer,size);//ת��Զ��
	#endif
			
	#endif
		}
		if(frame->dest_subnet != 0xff && frame->dest_device != 0xff)
		{
			return;
		}
	}
#endif
	// commands handler
	switch (cmd)
	{
/*********************************************/
/*			   ������������					 */
/*********************************************/
#ifdef DEVICE_ENABLE
		//���豸��ע
		case READ_DEVICE_REMARK_CMD:
			if (IS_LOCAL(frame->dest_subnet,frame->dest_device))
										//|| IS_BROADCAST(frame->dest_subnet,frame->dest_device))
			{
				devices_get_remark(frame->payload);
				app_commands_send(s,READ_DEVICE_REMARK_ACK, \
					frame->src_subnet,frame->src_device,frame->payload,20);
			}
			break;

		//д�豸��ע
		case WRITE_DEVICE_REMARK_CMD:
			if (!IS_LOCAL(frame->dest_subnet, frame->dest_device)) 
				break;
			
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

		//������汾��
		case READ_SOFTWARE_VERSION_CMD:	
			if (!IS_LOCAL(frame->dest_subnet,frame->dest_device)) 
				break;
			
			app_commands_send(s,READ_SOFTWARE_VERSION_ACK, \
				frame->src_subnet,frame->src_device,(unsigned char*)SOFTWARE_VERSION,22);
			break;

		//д�豸MAC
		case WRITE_DEVICE_MAC_CMD:
			if (!IS_LOCAL(frame->dest_subnet,frame->dest_device)) 
				break;
			
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

	    //���豸��Ϣ
		case READ_DEVICE_BASIC_INFO_CMD:
			if (!IS_LOCAL(frame->dest_subnet,frame->dest_device)) 
				break;
			
			devices_get_mac(frame->payload);
			app_commands_send(s,READ_DEVICE_BASIC_INFO_ACK, \
				frame->src_subnet,frame->src_device,frame->payload,10);
			break;

		//д�豸��Ϣ��ַ
		case WRITE_DEVICE_BASIC_INFO_CMD: 
			if (!IS_LOCAL(frame->dest_subnet,frame->dest_device)) 
				break;
			
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


		//�������豸��ַ
		case READ_DEVICE_MODULE_ID_CMD: 
			if (!IS_PROG_MODE()) 
				break;
			
			devices_get_subnet(frame->payload);// yk 2019-12-30��
			app_commands_send(s,READ_DEVICE_MODULE_ID_ACK, \
			frame->src_subnet,frame->src_device,frame->payload,2);
			
			break;

		//����д�豸��ַ
		case WRITE_DEVICE_MODULE_ID_CMD: 
			if (!IS_PROG_MODE()) 
				break;
			
			if (!devices_set_subnet(frame->payload))
			{
				app_commands_send(s,WRITE_DEVICE_MODULE_ID_ACK, \
				frame->src_subnet,frame->src_device,frame->payload,10);
			}
			break;
#endif

#ifdef SYSTEM_SETTING_ENABLE			
		//�����ָ�
		case RESET_TARGET_INFO_CMD:
			if (!IS_LOCAL(frame->dest_subnet,frame->dest_device)) 
				break;
			
			if (!system_setting_reset_config(frame->payload))
			{
				app_commands_send(s,RESET_TARGET_INFO_ACK, \
					frame->src_subnet,frame->src_device,frame->payload,4);
				
				reset_restore_timeout();
			}
			break;
			
		//���ܶ�λ
		case SEARCH_DEVICE_POINT_CMD:
			if (!IS_LOCAL(frame->dest_subnet,frame->dest_device)) 
				break;
			
			if (!system_setting_set_position(frame->payload))
			{
				frame->payload[0] = 0xf8;
				app_commands_send(s,SEARCH_DEVICE_POINT_ACK, \
				frame->src_subnet,frame->src_device,frame->payload,1);
			}
			break;

		//�����ױ�� yk 2019-12-30��  2020��
		case READ_PROGRAM_REMARK_CMD:
		{	
			devices_get_subnet(subnet_buffer);
			unsigned char Buffer[90] = {0};
			if(PAYLOAD_SIZE(frame->length) >= 2)
			{
				memcpy(Buffer, frame->payload, PAYLOAD_SIZE(frame->length));
				return_num = system_setting_get_program(Buffer,frame->length);
				if(return_num == 0)
				{
					break;
				}	
				BIG_BAO_SEND(READ_PROGRAM_REMARK_ACK, \
					subnet_buffer[0], subnet_buffer[1], frame->src_subnet, frame->src_device, Buffer, return_num);
				//�����鲥
				if( netip_gateway_type() == MQTT_LINK_SERVER || netip_gateway_type() == STANDAR_MQTT_LINK_SERVER) //�������mqtt
				{
					#ifdef IOT_ENABLE	
						iot_config_interface.long_send(READ_PROGRAM_REMARK_ACK,subnet_buffer[0], subnet_buffer[1], frame->src_subnet, frame->src_device, Buffer, return_num);
					#endif
				}
				if( netip_gateway_type() == GATEWAY_LINK_SERVER || netip_gateway_type() == GATEWAY_LINK_DNS) //���������ͨԶ��
				{
					netip_gateway_longsend(READ_PROGRAM_REMARK_ACK, frame->src_subnet, frame->src_device, Buffer,return_num);
				}
			}
			break;
		}
		//д���ױ�� 
		case WRITE_PROGRAM_REMARK_CMD:
			if (!IS_LOCAL(frame->dest_subnet,frame->dest_device)) 
				break;
			
			if (!system_setting_set_program(frame->payload,PAYLOAD_SIZE(frame->length)))
			{
				app_commands_send(s,WRITE_PROGRAM_REMARK_ACK, \
				frame->src_subnet,frame->src_device,frame->payload,0);
			}
			break;
			
#endif
	

/*********************************************/
/*			  ��������						*/
/*********************************************/	
#ifdef IAP_ENABLE
		case UPDATA_APP_CMD:
		{
			//unsigned char Temp_Buffer[5] = {0};
			if (s)
			{
				if (!IS_LOCAL(frame->dest_subnet,frame->dest_device)) 
				break;
				//����ʱ��ձ�־λ yk 2019-12-11�ż�
			#if 0
				eeprom_read(UPDATE_FLAG_ADDRESS,(unsigned char *)Temp_Buffer,UPDATE_FLAG_SIZE);
				if(!strncmp((char *)Temp_Buffer,"HDL",3))
				{
					eeprom_write(UPDATE_FLAG_ADDRESS,(unsigned char *)"",UPDATE_FLAG_SIZE);
				}
			#endif
				iap_interface.set(frame->payload,1);
				app_commands_send(s,UPDATA_APP_ACK, \
					frame->src_subnet,frame->src_device,(unsigned char *)MCU_CHIP,20);
			}
		}
		break;
#endif

		
/*********************************************/
/*			  mcu type	configure			*/
/*********************************************/		

		case READ_DEVICE_MCU_TYPE_CMD:
			if (!IS_LOCAL(frame->dest_subnet,frame->dest_device)) 
				break;
			
			memcpy(&frame->payload[0],PCB_VERSION,10);
			memcpy(&frame->payload[10],MCU_CHIP,20);

			app_commands_send(s,0x3025/*READ_DEVICE_MCU_TYPE_ACK*/, \
					frame->src_subnet,frame->src_device,frame->payload,30);
			break;
		
#ifdef DEVICE_MCU_ENABLE		
		case READ_DEVICE_MCU_UID_CMD:
			if (!IS_LOCAL(frame->dest_subnet,frame->dest_device)) 
				break;
			
			serial_crypt(frame->payload,devices->UID);
			app_commands_send(s,READ_DEVICE_MCU_UID_ACK, \
					frame->src_subnet,frame->src_device,frame->payload,16);
	    break;
#endif

/*********************************************/
/*			  bandrate	configure							*/
/*********************************************/		
#ifdef BUSPRO_PARAMS_ENABLE		
		case BANDRATE_READ_CMD:
			if (!IS_LOCAL(frame->dest_subnet,frame->dest_device))
					break;
				buspro_get_baudrate(frame->payload);
				app_commands_send(s, BANDRATE_READ_ACK, \
						frame->src_subnet, frame->src_device, frame->payload, 1);
			break;
			
			case BANDRATE_WRITE_CMD:
				if (!IS_LOCAL(frame->dest_subnet,frame->dest_device))
					break;
				buspro_set_baudrate(frame->payload);
				frame->payload[0] = 0xF8;
				app_commands_send(s, BANDRATE_WRITE_ACK, \
						frame->src_subnet, frame->src_device, frame->payload, 1);
			break;
#endif

			
/*********************************************/
/*			  route	configure							*/
/*********************************************/		
#ifdef ROUTE_ENABLE		
		//���豸MAC
		case READ_DEVICE_MAC_ID_ACK:
			if (s == 0)
			{
				if (!IS_LOCAL(frame->dest_subnet,frame->dest_device))
					break;
				
				frame->payload[8] = 0;
				frame->payload[9] = frame->src_subnet;
				frame->payload[10] = frame->src_device;
				route_interface.set(frame->payload,11);
			}
			break;
		
		case AUTO_SET_ROUTE_DEVICE_CMD:
			if (!IS_LOCAL(frame->dest_subnet,frame->dest_device))
					break;
			frame->length = route_interface.write(frame->payload,PAYLOAD_SIZE(frame->length));
			app_commands_send(s,AUTO_SET_ROUTE_DEVICE_ACK, \
					frame->src_subnet,frame->src_device,frame->payload,frame->length);
			break;
#endif
		
/*********************************************/
/*			 datetime	configure							*/
/*********************************************/		
#ifdef DATETIME_ENABLE
		case READ_DATE_TIME_CMD:
			if (!IS_LOCAL(frame->dest_subnet,frame->dest_device))
					break;
			datetime_interface.get(frame->payload,7);
			app_commands_send(s,READ_DATE_TIME_ACK, \
					frame->src_subnet,frame->src_device,frame->payload,7);
			break;
#endif
		
/*********************************************/
/*			  filter configure				 */
/*********************************************/		
#ifdef FILTER_ENABLE
		case READ_FILTER_CONFIG_CMD:
			if (!IS_LOCAL(frame->dest_subnet,frame->dest_device))
					break;
			filter_interface.get(frame->payload,PAYLOAD_SIZE(frame->length));
			app_commands_send(s,READ_FILTER_CONFIG_ACK, \
					frame->src_subnet,frame->src_device,frame->payload,9);
			break;

		case WRITE_FILTER_CONFIG_CMD:
			if (!IS_LOCAL(frame->dest_subnet,frame->dest_device))
					break;
			if (filter_interface.set(frame->payload,PAYLOAD_SIZE(frame->length)))
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
		//���豸IP
		case READ_DEVICE_IPADDR_CMD:
			if (!IS_LOCAL(frame->dest_subnet,frame->dest_device))
					break;
				netip_gateway_get_ipaddress(frame->payload);
				app_commands_send(s,READ_DEVICE_IPADDR_ACK, \
						frame->src_subnet,frame->src_device,frame->payload,31);
			break;

     	//д�豸IP
		case WRITE_DEVICE_IPADDR_CMD:
			if (!IS_LOCAL(frame->dest_subnet,frame->dest_device))
					break;
				netip_gateway_set_ipaddress(frame->payload);
				frame->payload[0] = 0xf8;
				app_commands_send(s,WRITE_DEVICE_IPADDR_ACK, \
						frame->src_subnet,frame->src_device,frame->payload,1);
			break;
						
// ����������  ��MQTT��Ϣ�ô�С������ yk 2020-05-29��
		case DNS_NAME_READ_CMD:
			if (!IS_LOCAL(frame->dest_subnet,frame->dest_device))
					break;
			
			netip_gateway_get_domainname(frame->payload);
			app_commands_send(s,DNS_NAME_READ_ACK, \
					frame->src_subnet,frame->src_device,frame->payload,64);
			break;
			
		case DNS_NAME_WRITE_CMD:
			if (!IS_LOCAL(frame->dest_subnet,frame->dest_device))
					break;
			
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

#ifdef IOT_ENABLE			
		// д��������
		case WRITE_MODULE_SERVER_INFO_CMD:
		{
			if (!IS_LOCAL(frame->dest_subnet,frame->dest_device))
					break;
			
			app_commands_send(s,WRITE_MODULE_SERVER_INFO_ACK, \
					frame->src_subnet,frame->src_device,(unsigned char *)&success_flag,1);
			netip_gateway_set_message(frame->payload,PAYLOAD_SIZE(frame->length));
		}	
	    break;
		
		// ����������
		case READ_MODULE_SERVER_INFO_CMD:
			if (!IS_LOCAL(frame->dest_subnet,frame->dest_device))
					break;
			
			netip_gateway_get_message(frame->payload,PAYLOAD_SIZE(frame->length));
			app_commands_send(s,READ_MODULE_SERVER_INFO_ACK, \
					frame->src_subnet,frame->src_device,frame->payload,60);
			break;
		
		// д��������ַ
		case WRITE_MODULE_SERVER_IP_CMD:
			if (!IS_LOCAL(frame->dest_subnet,frame->dest_device))
					break;
			
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
			
		// ����������ַ
		case READ_MODULE_SERVER_IP_CMD:
			if (!IS_LOCAL(frame->dest_subnet,frame->dest_device))
					break;
			
			netip_gateway_get_server(frame->payload,PAYLOAD_SIZE(frame->length));
			app_commands_send(s,READ_MODULE_SERVER_IP_ACK, \
					frame->src_subnet,frame->src_device,frame->payload,15);
	    break;
#endif
		
		// д��ʷ��¼
		case WRITE_HISTORY_EVENTS_CMD:
			if (!IS_LOCAL(frame->dest_subnet,frame->dest_device))
					break;
			
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
		
		// ����ʷ��¼
		case READ_HISTORY_EVENTS_CMD:
			if (!IS_LOCAL(frame->dest_subnet,frame->dest_device))
					break;
			
			frame->length = netip_gateway_get_history(remote_ipaddr,remote_port,frame->payload,PAYLOAD_SIZE(frame->length));
			app_commands_send(s,READ_HISTORY_EVENTS_ACK, \
						frame->src_subnet,frame->src_device,frame->payload,frame->length);
	    break;
		
		
		// ��������
		case READ_LINK_GATEWAY_INFO_CMD://0x3019
			if (!IS_LOCAL(frame->dest_subnet,frame->dest_device))
					break;
			netip_gateway_remote_link_endpoint(frame->payload);
			app_commands_send(s,READ_LINK_GATEWAY_INFO_ACK, \
						frame->src_subnet,frame->src_device,frame->payload,40);
	    break;
		
		// �Ͽ�����
		case EXIT_LINK_GATEWAY_INFO_CMD://0x301B
			if (s == 0) break;
			if (!netip_gateway_type()) break;
		
			if (!IS_LOCAL(frame->dest_subnet,frame->dest_device))
					break;
			
			netip_gateway_remote_disable(remote_ipaddr,remote_port);
			frame->payload[0] = 0xf8;
			app_commands_send(s,EXIT_LINK_GATEWAY_INFO_ACK, \
					frame->src_subnet,frame->src_device,frame->payload,1);
	    break;
		
		// ������֤
		case MODULE_PASSWORD_LINK_CMD://0x300F
			if (s == 0) break;
			if (!netip_gateway_type()) break;
			//if (!IS_LOCAL(frame->dest_subnet,frame->dest_device))
			//		break;
			
			if (netip_gateway_password_module(remote_ipaddr,remote_port,\
				frame->src_subnet,frame->src_device,frame->payload,PAYLOAD_SIZE(frame->length)))
			{
				frame->payload[0] = 0xF5;
			}
			else
			{
				frame->payload[0] = 0xF8;
			}
			
			netip_gateway_dosend(remote_ipaddr,remote_port,MODULE_PASSWORD_LINK_ACK,\
						devices->subnet,devices->device,frame->src_subnet,frame->src_device,frame->payload,1);
			//netip_ptp_send(remote_ipaddr,remote_port,MODULE_PASSWORD_LINK_ACK,\
						devices->subnet,devices->device,frame->src_subnet,frame->src_device,frame->payload,1);
			
			app_commands_send(s,MODULE_PASSWORD_LINK_ACK, \
					frame->src_subnet,frame->src_device,frame->payload,1);
			
	    break;
		
		// �������
		case SOFTWARE_LINK_GATEWAY_CMD: 
			if (s == 0) break;
			if (!netip_gateway_type()) break;
		
			//if (!IS_LOCAL(frame->dest_subnet,frame->dest_device))
			//		break;
			
			if (netip_gateway_conneting_module(remote_ipaddr,remote_port,\
				frame->src_subnet,frame->src_device,frame->payload,PAYLOAD_SIZE(frame->length)))
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
			
		//����������ע����Ϣ
		case MODULE_REGISTER_SERVER_ACK: //0x300C
			if (s == 0) break;
			
			if (!netip_gateway_type()) break;
		
			if (!IS_LOCAL(frame->dest_subnet,frame->dest_device))
					break;
			
			netip_gateway_register_respons(frame->payload);
			break;
			
		case READ_REGISTER_LINK_GATEWAY_CMD:
			if (!IS_LOCAL(frame->dest_subnet,frame->dest_device))
					break;
			
			netip_gateway_register_error_report(frame->payload);
			app_commands_send(s,READ_REGISTER_LINK_GATEWAY_ACK, \
					frame->src_subnet,frame->src_device,frame->payload,8);
			break;
#endif
		
		
/*********************************************/
/*			   ���Ʋ�������					 */
/*********************************************/
		
/*********************************************/
/*			   ΢��					 										*/
/*********************************************/
#ifdef WECHAT_ENABLE
			
			// ΢��	����״̬
			case WECHAT_SEND_INFO_ACK:
				wechat_send_message_reply(frame->payload[0],frame->payload[1]);
				break;
			
			// ΢��	UTF8 ��Ϣ�͵�������
			case WECHAT_SEND_UTF8_CMD:
				wechat_send_utf8(frame->src_subnet,frame->src_device,frame->payload,PAYLOAD_SIZE(frame->length));
				frame->payload[0] = 0xf8;
				app_commands_send(s,WECHAT_SEND_UTF8_ACK, \
					frame->src_subnet,frame->src_device,frame->payload,1);
				break;	
				
			case WECHAT_SEND_UTF8_ACK:
				wechat_send_utf8_reply(frame->payload,PAYLOAD_SIZE(frame->length));
				break;
			
		  //����ͨ�ÿ���
			case SET_UNIVERSAL_CMD:
				wechat_send_message(frame->src_subnet,frame->src_device,frame->payload[0],frame->payload[1]); //F006
				if (frame->payload[1] == 0xff)
				{
					frame->payload[1] = 1;
				}
				else
				{
					frame->payload[1] = 0;
				}
				app_commands_send(s, SET_UNIVERSAL_ACK, \
					frame->src_subnet, frame->src_device, frame->payload, 2);
				break;
#endif
				
#ifdef IOT_ENABLE
//��׼MQTT��ȡ�ͻ���ID �û��� ����				
			case STANDAR_MQTT_READ_MESSAGE_CMD:
				if (!IS_LOCAL(frame->dest_subnet,frame->dest_device))
					break;
				
				return_num = netip_gateway_get_Standar_MQTT_message(&frame->payload[0]);
				app_commands_send(s, COMMAND_ACK(STANDAR_MQTT_READ_MESSAGE_CMD), \
						frame->src_subnet,frame->src_device, frame->payload, return_num);
				break;
			
			case STANDAR_MQTT_WRITE_MESSAGE_CMD:
				if (!IS_LOCAL(frame->dest_subnet,frame->dest_device))
					break;

				return_num = netip_gateway_set_Standar_MQTT_message(&frame->payload[0]);
				app_commands_send(s, COMMAND_ACK(STANDAR_MQTT_WRITE_MESSAGE_CMD), \
						frame->src_subnet,frame->src_device, frame->payload, return_num);
				break;
//��׼MQTT��ȡURL 			
			case STANDAR_MQTT_Read_URL_MESSAGE_CMD:
				if (!IS_LOCAL(frame->dest_subnet,frame->dest_device))
					break;
				return_num = netip_gateway_get_Standar_MQTT_URL_message(&frame->payload[0]);
				app_commands_send(s, COMMAND_ACK(STANDAR_MQTT_Read_URL_MESSAGE_CMD), \
						frame->src_subnet,frame->src_device, frame->payload, return_num);
				
				break;
			
			case STANDAR_MQTT_WRITE_URL_MESSAGE_CMD:
				if (!IS_LOCAL(frame->dest_subnet,frame->dest_device))
					break;
				return_num = netip_gateway_set_Standar_MQTT_URL_message(&frame->payload[0]);
				app_commands_send(s, COMMAND_ACK(STANDAR_MQTT_WRITE_URL_MESSAGE_CMD), \
						frame->src_subnet,frame->src_device, frame->payload, return_num);
				break;
//��׼MQTT��AES����
			case STANDAR_MQTT_READ_AES_CMD:
				if (!IS_LOCAL(frame->dest_subnet,frame->dest_device))
					break;
				return_num = netip_gateway_get_STANDAR_MQTT_AES(&frame->payload[0]);
				app_commands_send(s, COMMAND_ACK(STANDAR_MQTT_READ_AES_CMD), \
						frame->src_subnet,frame->src_device, frame->payload, return_num);
				
				break;
			
			case STANDAR_MQTT_WRITE_AES_CMD:
				if (!IS_LOCAL(frame->dest_subnet,frame->dest_device))
					break;
				return_num = netip_gateway_set_STANDAR_MQTT_AES(&frame->payload[0]);
				app_commands_send(s, COMMAND_ACK(STANDAR_MQTT_WRITE_AES_CMD), \
						frame->src_subnet,frame->src_device, frame->payload, return_num);
				break;
//��׼MQTT��topic
			case STANDAR_MQTT_READ_TOPIC_CMD:
				if (!IS_LOCAL(frame->dest_subnet,frame->dest_device))
					break;
				return_num = netip_gateway_get_STANDAR_MQTT_TOPIC(&frame->payload[0]);
				app_commands_send(s, COMMAND_ACK(STANDAR_MQTT_READ_TOPIC_CMD), \
						frame->src_subnet,frame->src_device, frame->payload, return_num);
				
				break;
			
			case STANDAR_MQTT_WRITE_TOPIC_CMD:
				if (!IS_LOCAL(frame->dest_subnet,frame->dest_device))
					break;
				return_num = netip_gateway_set_STANDAR_MQTT_TOPIC(&frame->payload[0]);
				app_commands_send(s, COMMAND_ACK(STANDAR_MQTT_WRITE_TOPIC_CMD), \
						frame->src_subnet,frame->src_device, frame->payload, return_num);
				break;
//��ȡuuid
			case MQTT_GET_UUID_CMD:
				if (!IS_LOCAL(frame->dest_subnet,frame->dest_device))
					break;
				//eeprom_read(HDL_MQTT_UUID_ADDRESS ,(unsigned char *)frame->payload, HDL_MQTT_UUID_SIZE);
				extern char MQTT_MAC_UUID[];
				strcpy(frame->payload,MQTT_MAC_UUID);
				frame->payload[HDL_MQTT_UUID_SIZE] = 0xff;
				test_read_somedata(&frame->payload[HDL_MQTT_UUID_SIZE+1]);
				
				app_commands_send(s, COMMAND_ACK(MQTT_GET_UUID_CMD), \
						frame->src_subnet,frame->src_device, frame->payload, HDL_MQTT_UUID_SIZE+1+2+16);
				break;
//���ؼ���  yk 2020-01-10��
			case LOCAL_GET_AESKEY_CMD:
				if (!IS_LOCAL(frame->dest_subnet,frame->dest_device) && (frame->dest_subnet != 0xff && frame->dest_device != 0xff))
					break;
//				//����û�а��첻�ܻ�ȡ����
//				if (devices_button_get_mode() != 1)
//				{
//					break;
//				}
				return_num = netcrypt_interface.get(frame->payload, PAYLOAD_SIZE(frame->length));
				app_commands_send(s, COMMAND_ACK(LOCAL_GET_AESKEY_CMD), \
						frame->src_subnet,frame->src_device, frame->payload, return_num);
				break;
			
			case LOCAL_SET_AESKEY_CMD:
			{
				unsigned char Buffer[20];
				if (!IS_LOCAL(frame->dest_subnet,frame->dest_device) && (frame->dest_subnet != 0xff && frame->dest_device != 0xff))
					break;
				
//				//����û�а��첻���޸�����
//				if (devices_button_get_mode() != 1)
//				{
//					break;
//				}
				if(PAYLOAD_SIZE(frame->length) < 17)
				{
					frame->payload[20] = 0xf5;
				}
				else
				{
					frame->payload[20] = 0xf8;
					memcpy(Buffer, frame->payload, 17);
				}
				//�Ȼ���Ϣ����ֹ���ʹ������ݻ�ȥ
				app_commands_send(s, COMMAND_ACK(LOCAL_SET_AESKEY_CMD), \
						frame->src_subnet,frame->src_device, &frame->payload[20], 1);
				if(frame->payload[20] == 0xf8)
				{
					return_num = netcrypt_interface.set(Buffer, 17);
				}
				break;
			}

			//yk 2020-02-20��
			case MQTT_SET_MONITOR_CMD:
				if (!IS_LOCAL(frame->dest_subnet,frame->dest_device))
					break;
			
				iot_config_interface.set_monitor(frame->payload);
				frame->payload[0] = 0xF8;
				app_commands_send(s, COMMAND_ACK(MQTT_SET_MONITOR_CMD), \
						frame->src_subnet,frame->src_device, frame->payload, 1);
				break;
			
			case MQTT_GET_MONITOR_CMD:
				if (!IS_LOCAL(frame->dest_subnet,frame->dest_device))
					break;
				iot_config_interface.get_monitor(frame->payload);
				app_commands_send(s, COMMAND_ACK(MQTT_GET_MONITOR_CMD), \
						frame->src_subnet,frame->src_device, frame->payload, 1);
				break;
			
			//yk 2020-04-15��
			case MQTT_SET_CORE_URL_CMD:
				if (!IS_LOCAL(frame->dest_subnet,frame->dest_device))
					break;
				if(-1 == iot_config_interface.set_coreurl(frame->payload, PAYLOAD_SIZE(frame->length)))
				{
					frame->payload[0] = 0xF5;
				}
				else
					frame->payload[0] = 0xF8;
				app_commands_send(s, COMMAND_ACK(MQTT_SET_CORE_URL_CMD), \
						frame->src_subnet,frame->src_device, frame->payload, 1);
				break;
			
			case MQTT_GET_CORE_URL_CMD:
				if (!IS_LOCAL(frame->dest_subnet,frame->dest_device))
					break;
				return_num = iot_config_interface.get_coreurl(frame->payload);
				app_commands_send(s, COMMAND_ACK(MQTT_GET_CORE_URL_CMD), \
						frame->src_subnet,frame->src_device, frame->payload, return_num);
				break;
#endif
		default: 
			break;
	}
#endif

}



