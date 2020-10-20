
#ifndef _COMMANDS_H_
#define _COMMANDS_H_


#define COMMAND_ACK(A) 				(A + 1)

// MCU
//#define READ_DEVICE_MCU_TYPE_CMD        0X3024
//#define READ_DEVICE_MCU_TYPE_ACK        0X3025

//��������
#define UPDATA_APP_CMD					0xF81A
#define UPDATA_APP_ACK					0xF81B

//���豸��ע
#define READ_DEVICE_REMARK_CMD 			0x000E
#define READ_DEVICE_REMARK_ACK 			0x000F

//д�豸��ע
#define WRITE_DEVICE_REMARK_CMD		 	0x0010
#define WRITE_DEVICE_REMARK_ACK			0x0011

//������汾
#define READ_SOFTWARE_VERSION_CMD  		0xEFFD
#define READ_SOFTWARE_VERSION_ACK		0xEFFE

//д�豸��Ϣ MAC
#define WRITE_DEVICE_MAC_CMD 			0xF001
#define WRITE_DEVICE_MAC_ACK 			0xF002

//���豸��ϢMAC ADDR TYPE
#define READ_DEVICE_BASIC_INFO_CMD 		0xF003
#define READ_DEVICE_BASIC_INFO_ACK 		0xF004

//д�豸��Ϣ��ַ
#define WRITE_DEVICE_BASIC_INFO_CMD 	0xF005
#define WRITE_DEVICE_BASIC_INFO_ACK 	0xF006


//�������豸��ַ
#define READ_DEVICE_MODULE_ID_CMD 		0xE5F5
#define READ_DEVICE_MODULE_ID_ACK 		0xE5F6


//����д�豸��ַ
#define WRITE_DEVICE_MODULE_ID_CMD		0xE5F7
#define WRITE_DEVICE_MODULE_ID_ACK		0xE5F8

#define READ_DEVICE_IPADDR_CMD           0xF037
#define READ_DEVICE_IPADDR_ACK           0xF038

#define WRITE_DEVICE_IPADDR_CMD          0xF039
#define WRITE_DEVICE_IPADDR_ACK          0xF03a


//����
#define READ_FILTER_CONFIG_CMD						0X1D42
#define READ_FILTER_CONFIG_ACK						0X1D43

#define WRITE_FILTER_CONFIG_CMD						0X1D44
#define WRITE_FILTER_CONFIG_ACK						0X1D45


// read MCU 
#define READ_DEVICE_MCU_TYPE_CMD				0x3024
#define READ_DEVICE_MCU_TYPE_ACK				0x3025

#define READ_DEVICE_MCU_UID_CMD					0x1366
#define READ_DEVICE_MCU_UID_ACK					0x1367

/***********ģ�� - ������************************************************/
#define WRITE_MODULE_SERVER_INFO_CMD        0X3003
#define WRITE_MODULE_SERVER_INFO_ACK        0X3004

#define READ_MODULE_SERVER_INFO_CMD         0X3007
#define READ_MODULE_SERVER_INFO_ACK         0X3008


#define WRITE_MODULE_SERVER_IP_CMD          0X3005
#define WRITE_MODULE_SERVER_IP_ACK          0X3006

#define READ_MODULE_SERVER_IP_CMD           0X3009
#define READ_MODULE_SERVER_IP_ACK           0X300A


#define MODULE_REGISTER_SERVER_CMD          0X300B
#define MODULE_REGISTER_SERVER_ACK          0X300C


#define MODULE_PASSWORD_LINK_CMD            0X300F
#define MODULE_PASSWORD_LINK_ACK            0X3010

//#define MODULE_CHK_REMOTE_INFO_CMD        0X300D
//#define MODULE_CHK_REMOTE_INFO_ACK        0X300E

//д��¼
#define WRITE_HISTORY_EVENTS_CMD            0X3015
#define WRITE_HISTORY_EVENTS_ACK            0X3016

//����¼
#define READ_HISTORY_EVENTS_CMD         		0x3017
#define READ_HISTORY_EVENTS_ACK         		0x3018

//����¼
#define READ_LINK_GATEWAY_INFO_CMD           0x3019
#define READ_LINK_GATEWAY_INFO_ACK           0x301A

//�˳�����
#define EXIT_LINK_GATEWAY_INFO_CMD           0x301B
#define EXIT_LINK_GATEWAY_INFO_ACK           0x301C


//���������������
#define SOFTWARE_LINK_GATEWAY_CMD           0x301E
#define SOFTWARE_LINK_GATEWAY_ACK           0x301D



//
#define READ_MCU_ID_CMD						0x1366
#define READ_MCU_ID_ACK						0x1367

#define READ_REGISTER_LINK_GATEWAY_CMD		0x1368
#define READ_REGISTER_LINK_GATEWAY_ACK		0x1369


//
#define SOFTWAVE_MASTER_LINK_GATEWAY_CMD	0x136A
#define SOFTWAVE_MASTER_LINK_GATEWAY_ACK	0x136B

#define GATEWAY_LINK_SOFTWAVE_CMD			0x136C
#define GATEWAY_LINK_SOFTWAVE_ACK			0x136D




//�㲥ʱ��
#define BROADCAST_DATETIME_CMD						0xDA44

#define READ_DATE_TIME_CMD							0xDA00
#define READ_DATE_TIME_ACK							0xDA01



/***************config**********************/
// system config
#define READ_DEVICE_AREA_CONFIG_CMD		0x0004
#define READ_DEVICE_AREA_CONFIG_ACK		0x0005

#define WRITE_DEVICE_AREA_CONFIG_CMD	0x0006
#define WRITE_DEVICE_AREA_CONFIG_ACK	0x0007

#define READ_DEVICE_SCENE_CONFIG_CMD	0x0000
#define READ_DEVICE_SCENE_CONFIG_ACK	0x0001

#define WRITE_DEVICE_SCENE_CONFIG_CMD	0x0008
#define WRITE_DEVICE_SCENE_CONFIG_ACK	0x0009


#define READ_DEVICE_AREA_REMARK_CMD		0xF00A
#define READ_DEVICE_AREA_REMARK_ACK		0xF00B

#define WRITE_DEVICE_AREA_REMARK_CMD	0xF00C
#define WRITE_DEVICE_AREA_REMARK_ACK	0xF00D

// channel
#define READ_CHANNEL_REMARK_CMD			0xF00E
#define READ_CHANNEL_REMARK_ACK			0xF00F

#define WRITE_CHANNEL_REMARK_CMD		0xF010
#define WRITE_CHANNEL_REMARK_ACK		0xF011

#define READ_CHANNEL_TYPE_CMD			0xF012
#define READ_CHANNEL_TYPE_ACK			0xF013

#define WRITE_CHANNEL_TYPE_CMD			0xF014
#define WRITE_CHANNEL_TYPE_ACK			0xF015

// channel
#define READ_SET_LIMIT_LEVEL_CMD		0xF016
#define READ_SET_LIMIT_LEVEL_ACK		0xF017

#define WRITE_SET_LIMIT_LEVEL_CMD		0xF018
#define WRITE_SET_LIMIT_LEVEL_ACK		0xF019

//scene remark
#define READ_SCENE_REMARK_CMD			0xF024
#define READ_SCENE_REMARK_ACK			0xF025

#define WRITE_SCENE_REMARK_CMD			0xF026
#define WRITE_SCENE_REMARK_ACK			0xF027



//scene power down
#define READ_SCENE_TYPE_POWER_DOWN_CMD		0xF051
#define READ_SCENE_TYPE_POWER_DOWN_ACK		0xF052

#define WRITE_SCENE_TYPE_POWER_DOWN_CMD		0xF053
#define WRITE_SCENE_TYPE_POWER_DOWN_ACK		0xF054

//scene power down
#define READ_SCENE_ID_POWER_ON_CMD		0xF055
#define READ_SCENE_ID_POWER_ON_ACK		0xF056

#define WRITE_SCENE_ID_POWER_ON_CMD		0xF057
#define WRITE_SCENE_ID_POWER_ON_ACK		0xF058

//scene remark
#define READ_SCENE_REMARK_CMD			0xF024
#define READ_SCENE_REMARK_ACK			0xF025

#define WRITE_SCENE_REMARK_CMD			0xF026
#define WRITE_SCENE_REMARK_ACK			0xF027



//dali online
#define READ_DALI_ONLINE_CMD			0xA008
#define READ_DALI_ONLINE_ACK			0xA009

//#define READ_DALI_ON_LINE_CMD			0xA00A
//#define READ_DALI_ON_LINE_CMD			0xA00A


#define SET_START_DALI_FLASH_CMD		0xA014
#define SET_START_DALI_FLASH_ACK		0xA015

#define SET_STOP_DALI_FLASH_CMD			0xA016
#define SET_STOP_DALI_FLASH_ACK			0xA017


//dali addr init 
#define SET_INIT_DALI_ADDR_CMD			0xA006
#define SET_INIT_DALI_ADDR_ACK			0xA007

#define SET_DALI_ADDR_CONFIG_ACK		0xA022
#define SET_DALI_ADDR_OK_ACK			0xA003

//dali addr modify
#define SET_MODIFY_DALI_ADDR_CMD		0xA020
#define SET_MODIFY_DALI_ADDR_ACK		0xA021

#define DALI_UPDATE_CONFIG_CMD			0X1448
#define DALI_UPDATE_CONFIG_ACK			0X1449

// dali output test
#define DALI_OUTPUT_CONTROL_CMD			0XF074
#define DALI_OUTPUT_CONTROL_ACK			0XF075



// dali dimmer curve
#define DALI_DIMMER_CURVE_CMD			0X14FE
// 1byte 0 write curve config  1 read curve config
// 2byte 0 : curve line 
//		 1 : curve log10
// 		 2 : curve X2
//		 3 : curve sqrt
//       4 : curve sin
//       5 : curve cos
//       6 : curve user 
#define DALI_DIMMER_CURVE_ACK			0X14FF
// 1byte 0 write curve config  1 read curve config
// 2byte 0 : curve line 
//		 1 : curve log10
// 		 2 : curve X2
//		 3 : curve sqrt
//       4 : curve sin
//       5 : curve cos
//       6 : curve user 


#define  Read_Dimer_Curve_Data           0xE600
//  1byte (1) curve order
//  2byte (1~2) data order
#define  Read_Dimer_Curve_Data_ack       0xE601
//  1byte (1)  curve order
//  2byte (1~2)  data order
//  3~54 byte   (if data order==1, 50 bytes; if data order==2, 51 bytes)
#define  Write_Dimer_Curve_Data          0xE602
//  1byte (1)  curve order
//  2byte (1~2)  data order
//  3~54 byte  (if data order==1, 50 bytes; if data order==2, 51 bytes)
#define  Write_Dimer_Curve_Data_ack      0xE603
//  1byte (1)  curve order
//  2byte (1~2)  data order


// temperature cmd
#define TEMPERATURE_BROADCAST_CMD		0xE3E5


/***************��·����**********************/
//���õ�·����	single
#define SET_SINGLE_CMD					0x0031
#define SET_SINGLE_ACK					0x0032

#define SET_SINGLE_REACK				0x1310


//��״̬��·����
#define READ_SINGLE_CMD					0x0033
#define READ_SINGLE_ACK					0x0034

#define READ_SINGLE_REACK				0x1311

//������·ʵ��״̬
#define READ_REAL_SINGLE_CMD			0x0038
#define READ_REAL_SINGLE_ACK			0x0039

#define READ_REAL_SINGLE_REACK			0x1312

//��ϵ�������
#define SINGLE_DIMMER_CMD				0x180A

//dali read dimmer level
#define READ_DALI_LEVEL_CMD				0XE4F4
#define READ_DALI_LEVEL_ACK				0XE4F5

/*********************************************/



/***************��������**********************/
//���ó���		scene
#define SET_SCENE_CMD					0x0002
#define SET_SCENE_ACK					0x0003

#define SET_SCENE_REACK					0x1313

//������״̬
#define READ_SCENE_CMD					0x000C
#define READ_SCENE_ACK					0x000D

#define READ_SCENE_REACK				0x1314

//������״̬�㲥
#define BROADCAST_SCENE_CMD				0xEFFF
#define BROADCAST_SCENE_ACK				0xEEFF

#define BROADCAST_SCENE_REACK			0x1315



#define SET_DIM_SCENE_CMD				0XF03C

#define SET_DIM_SCENE_ACK				0x132B

#define SAVE_DIM_SCENE_CMD				0XF03D
#define SAVE_DIM_SCENE_ACK				0XF03E

#define SAVE_DIM_SCENE_REACK			0x132C


//��ϵ�������
#define SCENE_DIMMER_CMD				0x180B

//��ϵ�������
#define SAVE_SCENE_DIMMER_CMD			0x180C

/*********************************************/



/***************���п���**********************/
//��������  	sequence 
#define SET_SEQUENCE_CMD				0x001A
#define SET_SEQUENCE_ACK				0x001B

#define SET_SEQUENCE_REACK				0x1316

//������״̬
#define READ_SEQUENCE_CMD				0xE014
#define READ_SEQUENCE_ACK				0xE015

#define READ_SEQUENCE_REACK				0x1317

//����״̬�㲥
#define BROADCAST_SEQUENCE_CMD			0xF036
#define BROADCAST_SEQUENCE_ACK			0xF037

#define BROADCAST_SEQUENCE_REACK		0x1318

/*********************************************/



/***************��ʱ����**********************/
//���ö�ʱ������ timer
#define SET_TIMER_CMD					0xF116
#define SET_TIMER_ACK					0xF117

#define SET_TIMER_REACK					0x1319

//����ʱ������״̬
#define READ_TIMER_CMD					0xF112
#define READ_TIMER_ACK					0xF113

#define READ_TIMER_REACK				0x131A

//��ʱ��״̬�㲥
#define BROADCAST_TIMER_CMD				0xF12F
#define BROADCAST_TIMER_ACK				0xF130

#define BROADCAST_TIMER_REACK			0x131B

/*********************************************/



/**************ͨ�ÿ��ؿ���*******************/
//����ͨ�ÿ���  Universal
#define SET_UNIVERSAL_CMD				0xE01C
#define SET_UNIVERSAL_ACK				0xE01D

#define SET_UNIVERSAL_REACK				0xE022

//��ͨ�ÿ���״̬
#define READ_UNIVERSAL_CMD				0xE018
#define READ_UNIVERSAL_ACK				0xE019

#define READ_UNIVERSAL_REACK			0x131C

//ͨ�ÿ���״̬�㲥
#define BROADCAST_UNIVERSAL_CMD			0xE017
#define BROADCAST_UNIVERSAL_ACK			0xE016

#define BROADCAST_UNIVERSAL_REACK		0x131D

/*********************************************/




/**************�������ؿ���*******************/
//���ô�������  curtains
#define SET_CURTAINS_CMD				0xE3E0
#define SET_CURTAINS_ACK				0xE3E1

#define SET_CURTAINS_REACK				0x131E

//����������״̬
#define READ_CURTAINS_CMD				0xE3E2
#define READ_CURTAINS_ACK				0xE3E3

#define READ_CURTAINS_REACK				0x131F

//��������״̬�㲥
#define BROADCAST_CURTAINS_CMD			0xE3E4
#define BROADCAST_CURTAINS_ACK			0xE3E5

#define BROADCAST_CURTAINS_REACK		0x1320

/*********************************************/




/****************�������********************/
//���ú��⿪��   IR
#define SET_IR_CMD						0xE3D0
#define SET_IR_ACK						0xE3D1

#define SET_IR_REACK					0x1321

//�����⿪��״̬
#define READ_IR_CMD						0xE3D2
#define READ_IR_ACK						0xE3D3

#define READ_IR_REACK					0x1322

/*********************************************/



/****************GPRS����********************/
//����GPRS����   GPRS
#define SET_GPRS_CMD					0xE3D4
#define SET_GPRS_ACK					0xE3D5

#define SET_GPRS_REACK					0x1323

//��GPRS����״̬
#define READ_GPRS_CMD					0xE3D6
#define READ_GPRS_ACK					0xE3D7

#define READ_GPRS_REACK					0x1324

/*********************************************/



/****************������********************/
//������忪��   pannel
#define SET_PANNEL_CMD					0xE3D8
#define SET_PANNEL_ACK					0xE3D9

#define SET_PANNEL_REACK				0x1325

//����忪��״̬
#define READ_PANNEL_CMD					0xE3DA
#define READ_PANNEL_ACK					0xE3DB

#define READ_PANNEL_REACK				0x1326

/*********************************************/



/****************��������********************/
//������������   security
#define SET_SECURITY_CMD				0x0104
#define SET_SECURITY_ACK				0x0105

#define SET_SECURITY_REACK				0x1327

//����������״̬
#define READ_SECURITY_CMD				0x011E
#define READ_SECURITY_ACK				0x011F

#define READ_SECURITY_REACK				0x1328

/*********************************************/




/****************���ֿ���********************/
//������������   music
#define SET_MUSIC_CMD					0x0218
#define SET_MUSIC_ACK					0x0219

#define SET_MUSIC_REACK					0x1329

//����������״̬
#define READ_MUSIC_CMD					0x021A
#define READ_MUSIC_ACK					0x021B

#define READ_MUSIC_REACK				0x132A

/*********************************************/
//�û��Զ���������

/****************��������********************/

//ʹ��
#define READ_SWITCH_ENABLE_CMD		 	0x0128
#define READ_SWITCH_ENABLE_ACK			0x0129

#define SET_SWITCH_ENABLE_CMD			0x012A
#define SET_SWITCH_ENABLE_ACK			0x012B

#define READ_SWITCH_REENABLE_CMD		0x0158
#define READ_SWITCH_REENABLE_ACK		0x0159

#define SET_SWITCH_REENABLE_CMD			0x015A
#define SET_SWITCH_REENABLE_ACK			0x015B


//����
#define READ_SWITCH_TYPE_CMD			0xD205
#define READ_SWITCH_TYPE_ACK			0xD206

#define SET_SWITCH_TYPE_CMD				0xD207
#define SET_SWITCH_TYPE_ACK				0xD208

//ģʽ
#define READ_SWITCH_MODE_CMD			0xD230
#define READ_SWITCH_MODE_ACK			0xD231

#define SET_SWITCH_MODE_CMD				0xD232
#define SET_SWITCH_MODE_ACK				0xD233

//��ʱ
#define READ_SWITCH_DELAY_CMD			0xD218
#define READ_SWITCH_DELAY_ACK			0xD219

#define SET_SWITCH_DELAY_CMD			0xD20C
#define SET_SWITCH_DELAY_ACK			0xD20D

//���Ᵽ��
#define READ_DIMMER_SAVE_CMD			0xE134
#define READ_DIMMER_SAVE_ACK			0xE135

#define SET_DIMMER_SAVE_CMD				0xE136
#define SET_DIMMER_SAVE_ACK				0xE137

//���ر�ע
#define READ_SWITCH_RMK_CMD				0xD210
#define READ_SWITCH_RMK_ACK				0xD211

#define SET_SWITCH_RMK_CMD				0xD220
#define SET_SWITCH_RMK_ACK				0xD221


//�������ֵ
#define READ_DIMMER_LEVEL_CMD			0xE0E0
#define READ_DIMMER_LEVEL_ACK			0xE0E1

#define SET_DIMMER_LEVEL_CMD			0xE0E2
#define SET_DIMMER_LEVEL_ACK			0xE0E3


//Ŀ����Ϣ
#define READ_TARGET_INFO_CMD			0xD21C
#define READ_TARGET_INFO_ACK			0xD21D

#define SET_TARGET_INFO_CMD				0xD21E
#define SET_TARGET_INFO_ACK				0xD21F




//������Ϣ
#define READ_SECURITY_INFO_CMD			0x15DA
#define READ_SECURITY_INFO_ACK			0x15DB

#define SET_SECURITY_INFO_CMD			0x15DC
#define SET_SECURITY_INFO_ACK			0x15DD

//������ע
//#define SET_SECURITY_RMK_CMD			0x15D0
//#define SET_SECURITY_RMK_ACK			0x15D1

/*********************************************/



/******************���Ͱ���*******************/
//����
#define READ_SENSOR_STATUS_RECMD		0x15D0
#define READ_SENSOR_STATUS_REACK		0x15D1

#define READ_SENSOR_CHANNEL_CMD			0X15E2
#define READ_SENSOR_CHANNEL_ACK			0X15E3

#define READ_SENSOR_STATUS_CMD			0X15CE
#define READ_SENSOR_STATUS_ACK			0X15CF


/*********************************************/



/******************��е����*******************/
//�������
#define READ_POWERON_MACHINE_CMD		0x1350
#define READ_POWERON_MACHINE_ACK		0x1351

#define SET_POWERON_MACHINE_CMD			0X1352
#define SET_POWERON_MACHINE_ACK			0X1353

//��е���ع���
/*
#define READ_ASSOCIATION_MACHINE_CMD	0x1354
#define READ_ASSOCIATION_MACHINE_ACK	0x1355

#define SET_ASSOCIATION_MACHINE_CMD		0X1356
#define SET_ASSOCIATION_MACHINE_ACK		0X1357
*/
/*********************************************/





/*********************************************/


//���ÿ���Ŀ��
#define READ_CTRL_INFO_CMD				0XE000
#define READ_CTRL_INFO_ACK				0XE001

#define WRITE_CTRL_INFO_CMD				0XE002
#define WRITE_CTRL_INFO_ACK				0XE003


//==================΢��===================
#define WECHAT_SEND_INFO_CMD			0XF600
#define WECHAT_SEND_INFO_ACK			0XF601


#define WECHAT_SEND_UTF8_CMD			0XF602
#define WECHAT_SEND_UTF8_ACK			0XF603


//=================DNS NAME=================
#define DNS_NAME_READ_CMD			  	0X3020
#define DNS_NAME_READ_ACK			  	0X3021

#define DNS_NAME_WRITE_CMD			  0X3022
#define DNS_NAME_WRITE_ACK			 	0X3023



//=================������=================
#define BANDRATE_READ_CMD			  	0XE41C
#define BANDRATE_READ_ACK			  	0XE41D

#define BANDRATE_WRITE_CMD			  0XE41E
#define BANDRATE_WRITE_ACK			 	0XE41F


/*********************************************/

//ע���Զ�����ID
#define READ_DEVICE_MAC_ID_CMD	  		0X13F0
#define READ_DEVICE_MAC_ID_ACK	  		0X13F1
#define WRITE_DEIVE_SUBNET_CMD	  		0X13F2
#define WRITE_DEIVE_SUBNET_ACK	  		0X13F3

#define AUTO_SET_ROUTE_DEVICE_CMD			0X13F4
#define AUTO_SET_ROUTE_DEVICE_ACK			0X13F5

/*********************************************/

//�����ָ�
#define RESET_TARGET_INFO_CMD			0x3000
#define RESET_TARGET_INFO_ACK			0x3001

//�޸ļ�¼
#define WRITE_RECORD_CMD				0x3015
#define WRITE_RECORD_ACK				0x3016


//���ܶ�λ
#define SEARCH_DEVICE_POINT_CMD			0xE442
#define SEARCH_DEVICE_POINT_ACK			0xE443

//��ģ���ź�
#define READ_ANALOG_DATA_CMD		 	0xE440
#define READ_ANALOG_DATA_ACK		 	0xE441

#define READ_ANALOG_FLAG_CMD		 	0xE444
#define READ_ANALOG_FLAG_ACK		 	0xE445

#define WRITE_ANALOG_FLAG_CMD		 	0xE446
#define WRITE_ANALOG_FLAG_ACK		 	0xE447

//���ױ��
#define READ_PROGRAM_REMARK_CMD			0xE54A
#define READ_PROGRAM_REMARK_ACK			0xE54B

#define WRITE_PROGRAM_REMARK_CMD			0xE54C
#define WRITE_PROGRAM_REMARK_ACK			0xE54D

#define READ_DEVICE_STATUS_CMD			0xE44A
#define READ_DEVICE_STATUS_ACK			0xE44B

#define WRTIE_DEVICE_INFO_CMD			0xE44C
#define WRTIE_DEVICE_INFO_ACK			0xE44D


//�������
#define WRTIE_DEVICE_WORK_CMD			0xE44E
#define WRTIE_DEVICE_WORK_ACK			0xE44F

#define WRTIE_DEVICE_TICK_CMD			0xE450
#define WRTIE_DEVICE_TICK_ACK			0xE451



//������ģʽ����
//#define WRITE_DEVICE_SERVER_MODE_CMD	0xE452
//#define WRITE_DEVICE_SERVER_MODE_ACK	0xE453

//#define READ_DEVICE_SERVER_MODE_CMD		0xE454
//#define READ_DEVICE_SERVER_MODE_ACK		0xE455
 

//�ɽӵ� ������ģʽ�´�������
#define SENSOR_SERVER_MODE_CTRL_CMD		0xE456	
#define SENSOR_SERVER_MODE_CTRL_ACK		0xE457

//HDL MQTT����
#define MQTT_READ_MESSAGE_CMD			0x3020
#define MQTT_WRITE_MESSAGE_CMD			0x3022

#define MQTT_GET_UUID_CMD  				0x303A

//STANDAR MQTT����
#define STANDAR_MQTT_READ_MESSAGE_CMD   	0x302E
#define STANDAR_MQTT_WRITE_MESSAGE_CMD		0x3030

#define STANDAR_MQTT_Read_URL_MESSAGE_CMD   0x302A
#define STANDAR_MQTT_WRITE_URL_MESSAGE_CMD   0x302C

#define STANDAR_MQTT_READ_AES_CMD  			0x3032
#define STANDAR_MQTT_WRITE_AES_CMD			0x3034

#define STANDAR_MQTT_READ_TOPIC_CMD			0x3036
#define STANDAR_MQTT_WRITE_TOPIC_CMD		0x3038

#define LOCAL_GET_AESKEY_CMD				0x303C
#define LOCAL_SET_AESKEY_CMD				0x303E

#define MQTT_SET_MONITOR_CMD				0x3040
#define MQTT_GET_MONITOR_CMD				0x3042

#define MQTT_GET_CORE_URL_CMD				0x3044
#define MQTT_SET_CORE_URL_CMD				0x3046

//====================================================
// app cmd
extern int app_commands_send(int s,unsigned int cmd,unsigned char subnet,\
						unsigned char device,unsigned char *buffer,unsigned char size);
extern void app_commands_handler(int s,unsigned char *buffer,unsigned char size);
extern void netip_commands_handler(int s,unsigned char *buffer, int size);


#endif







