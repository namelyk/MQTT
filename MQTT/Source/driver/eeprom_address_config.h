#ifndef _EEPROM_ADDRESS_CONFIG_H_
#define _EEPROM_ADDRESS_CONFIG_H_

#define K																			(1024)
#define M																			(K * K)


#define DATA_BASIC_ADDRESS										(M)


// subnet device ID
#define EEPROM_DEVICE_BASIC_ADDR							(0)//2byte

// MAC
#define EEPROM_MAC_BASIC_ADDR									(EEPROM_DEVICE_BASIC_ADDR + 2)//8byte 

// remark
#define EEPROM_REMARK_BASIC_ADDR							(EEPROM_MAC_BASIC_ADDR + 8)//20byte			10开始


// ip config
#define IPADDR_CONFIG_BASIC_ADDRESS						(EEPROM_REMARK_BASIC_ADDR+20)//50BYTE	//30开始		

// project name
#define PROJECT_CONFIG_BASIC_ADDRESS					50	//(DOMAINNAME_CONFIG_BASIC_ADDRESS + 65)// 60byte	

// server config
#define SERVER_CONFIG_BASIC_ADDRESS						(PROJECT_CONFIG_BASIC_ADDRESS + 60)//15byte  110

#define DNS_SERVER_IPADDR_ADDR							180
#define DOMAINNAME_CONFIG_BASIC_ADDRESS					(DNS_SERVER_IPADDR_ADDR + 9)	//(IPADDR_CONFIG_BASIC_ADDRESS + 50)// 65byte

#define FILTER_CONFIG_BASIC_ADDRESS						300
//#define HISTORY_CONFIG_BASIC_ADDRESS					2*K


// route table
#define ROUTE_TABLE_BASIC_ADDR								(SERVER_CONFIG_BASIC_ADDRESS + K)//64 * 16 = 1K  110 + 1K

// filter config
#define FILTER_CONFIG_BASIC_ADDRESS2						(ROUTE_TABLE_BASIC_ADDR + K)// 1K

// history
#define HISTORY_CONFIG_BASIC_ADDRESS					(FILTER_CONFIG_BASIC_ADDRESS2 + 2*K)//2Kbyte

#define MQTT_CONFIG_ADDRESS				(HISTORY_CONFIG_BASIC_ADDRESS + 2*K)
#define MQTT_CONFIG_SIZE				73
#define MQTT_CONFIG_END					(MQTT_CONFIG_ADDRESS + MQTT_CONFIG_SIZE)

#define STANDAR_MQTT_CONFIG_ADDRESS 	MQTT_CONFIG_END
#define STANDAR_MQTT_CONFIG_SIZE		64
#define STANDAR_MQTT_CLIENT_ID_ADDR		STANDAR_MQTT_CONFIG_ADDRESS 
#define STANDAR_MQTT_CLIENT_ID_SIZE		32
#define STANDAR_MQTT_USER_NAME_ADDR		(STANDAR_MQTT_CLIENT_ID_ADDR + STANDAR_MQTT_CLIENT_ID_SIZE)
#define STANDAR_MQTT_USER_NAME_SIZE		16
#define STANDAR_MQTT_KEY_ADDR			(STANDAR_MQTT_USER_NAME_ADDR + STANDAR_MQTT_USER_NAME_SIZE)
#define STANDAR_MQTT_KEY_SIZE			16
#define STANDAR_MQTT_CONFIG_END			(STANDAR_MQTT_CONFIG_ADDRESS + STANDAR_MQTT_CONFIG_SIZE)

#define STANDAR_MQTT_CONFIG_URL_ADDRESS		STANDAR_MQTT_CONFIG_END
#define STANDAR_MQTT_CONFIG_URL_SIZE		63
#define STANDAR_MQTT_CONFIG_URL_END			(STANDAR_MQTT_CONFIG_URL_ADDRESS + STANDAR_MQTT_CONFIG_URL_SIZE)

#define STANDAR_MQTT_CONFIG_AES_ADDRESS    	STANDAR_MQTT_CONFIG_URL_END
#define STANDAR_MQTT_CONFIG_AES_SIZE		17
#define STANDAR_MQTT_CONFIG_AES_END			(STANDAR_MQTT_CONFIG_AES_ADDRESS + STANDAR_MQTT_CONFIG_AES_SIZE)

#define STANDAR_MQTT_CONFIG_PUBTOPIC_ADDRESS 	STANDAR_MQTT_CONFIG_AES_END
#define STANDAR_MQTT_CONFIG_PUBTOPIC_SIZE		50
#define STANDAR_MQTT_CONFIG_PUBTOPIC_END		(STANDAR_MQTT_CONFIG_PUBTOPIC_ADDRESS + STANDAR_MQTT_CONFIG_PUBTOPIC_SIZE)

#define STANDAR_MQTT_CONFIG_SUBTOPIC_ADDRESS 	STANDAR_MQTT_CONFIG_PUBTOPIC_END
#define STANDAR_MQTT_CONFIG_SUBTOPIC_SIZE		50
#define STANDAR_MQTT_CONFIG_SUBTOPIC_END		(STANDAR_MQTT_CONFIG_SUBTOPIC_ADDRESS + STANDAR_MQTT_CONFIG_SUBTOPIC_SIZE)


#define HDL_MQTT_KEY_FLAG_ADDRESS		STANDAR_MQTT_CONFIG_SUBTOPIC_END  //为Buspro则不修改
#define HDL_MQTT_KEY_FLAG_SIZE			6
#define HDL_MQTT_KEY_FLAG_END			(HDL_MQTT_KEY_FLAG_ADDRESS + HDL_MQTT_KEY_FLAG_SIZE)

#define HDL_MQTT_CRC_ADDRESS	HDL_MQTT_KEY_FLAG_END
#define HDL_MQTT_CRC_SIZE		2
#define HDL_MQTT_CRC_END		(HDL_MQTT_CRC_ADDRESS + HDL_MQTT_CRC_SIZE)

#define HDL_MQTT_KEY_ADDRESS		HDL_MQTT_CRC_END
#define HDL_MQTT_KEY_SIZE			16
#define HDL_MQTT_KEY_END			(HDL_MQTT_KEY_ADDRESS + HDL_MQTT_KEY_SIZE)

#define HDL_MQTT_UUID_ADDRESS		HDL_MQTT_KEY_END
#define HDL_MQTT_UUID_SIZE			36
#define HDL_MQTT_UUID_END			(HDL_MQTT_UUID_ADDRESS + HDL_MQTT_UUID_SIZE)

//yk 2019-12-11加 
#define UPDATE_FLAG_ADDRESS			HDL_MQTT_UUID_END
#define UPDATE_FLAG_SIZE			3
#define UPDATE_FLAG_END				(UPDATE_FLAG_ADDRESS + UPDATE_FLAG_SIZE)

#define NETCRYPT_KEY_ADDR			UPDATE_FLAG_END
#define NETCRYPT_KEY_SIZE			17
#define NETCRYPT_KEY_END			(NETCRYPT_KEY_ADDR + NETCRYPT_KEY_SIZE)

//yk 2020-04-27加 保存HDL MQTT信息
#define HDLMQTT_CONFIG_ADDR			NETCRYPT_KEY_END

#define HDLMQTT_CONFIGFLAG_ADDR		HDLMQTT_CONFIG_ADDR
#define HDLMQTT_CONFIGFLAG_SIZE		1
#define HDLMQTT_CONFIGFLAG_END		(HDLMQTT_CONFIGFLAG_ADDR + HDLMQTT_CONFIGFLAG_SIZE)

#define HDLMQTT_CONFIGMESSAGE_ADDR	HDLMQTT_CONFIGFLAG_END

#define HDLMQTT_CLIENTID_ADDR		HDLMQTT_CONFIG_ADDR
#define HDLMQTT_CLIENTID_SIZE		(64)
#define HDLMQTT_CLIENTID_END		(HDLMQTT_CLIENTID_ADDR + HDLMQTT_CLIENTID_SIZE)

#define HDLMQTT_USERNAME_ADDR		HDLMQTT_CLIENTID_END
#define HDLMQTT_USERNAME_SIZE		(64)
#define HDLMQTT_USERNAME_END		(HDLMQTT_CLIENTID_END + HDLMQTT_USERNAME_SIZE)

#define HDLMQTT_PASSWORD_ADDR		HDLMQTT_USERNAME_END
#define HDLMQTT_PASSWORD_SIZE		(16)
#define HDLMQTT_PASSWORD_END		(HDLMQTT_PASSWORD_ADDR + HDLMQTT_PASSWORD_SIZE)

#define HDLMQTT_AESKEY_ADDR			HDLMQTT_PASSWORD_END
#define HDLMQTT_AESKEY_SIZE			(16)
#define HDLMQTT_AESKEY_END			(HDLMQTT_AESKEY_ADDR + HDLMQTT_AESKEY_SIZE)

#define HDLMQTT_IP_ADDR				HDLMQTT_AESKEY_END
#define HDLMQTT_IP_SIZE				(4)
#define HDLMQTT_IP_END				(HDLMQTT_IP_ADDR + HDLMQTT_IP_SIZE)

#define HDLMQTT_PORT_ADDR			HDLMQTT_IP_END
#define HDLMQTT_PORT_SIZE			(2)
#define HDLMQTT_PORT_END			(HDLMQTT_PORT_ADDR + HDLMQTT_PORT_SIZE)

#define HDLMQTT_AES_SIZE			(176)
#define HDLMQTT_CONFIG_END			(HDLMQTT_CONFIG_ADDR + 200)



#define IOT_CONFIGURE_URL_FLAG_ADDR     HDLMQTT_CONFIG_END
#define IOT_CONFIGURE_URL_FLAG_SIZE		1
#define IOT_CONFIGURE_URL_FLAG_END		(IOT_CONFIGURE_URL_FLAG_ADDR + IOT_CONFIGURE_URL_FLAG_SIZE)	

#define IOT_CONFIGURE_URL_ADDR			IOT_CONFIGURE_URL_FLAG_END
#define IOT_CONFIGURE_URL_SIZE			64 
#define IOT_CONFIGURE_URL_END			(IOT_CONFIGURE_URL_ADDR + IOT_CONFIGURE_URL_SIZE) 


//// scene
//#define SCENE_EVENT_BASIC_ADDRESS							(DATA_BASIC_ADDRESS)//1M byte  512 * 99 * 2 = 1M byte
//#define SCENE_CONFIG_BASIC_ADDRESS						(SCENE_EVENT_BASIC_ADDRESS + M)// 512 * 24 = 12K byte

//// logic
//#define LOGIC_EVENT_BASIC_ADDRESS							(SCENE_CONFIG_BASIC_ADDRESS + 12*K)//512 * 80 = 40K byte
//#define LOGIC_CONFIG_BASIC_ADDRESS						(LOGIC_EVENT_BASIC_ADDRESS + 40*K)// 512 * 24 = 12K byte

//// dry
//#define DRY_EVENT_BASIC_ADDRESS								(LOGIC_CONFIG_BASIC_ADDRESS + 12*K)// 512 * 6 * 10 = 30K byte
//#define DRY_CONFIG_BASIC_ADDRESS							(DRY_EVENT_BASIC_ADDRESS + 30*K)// 512 * 24 = 12K byte

//// scheduler
//#define SCHEDULER_EVENT_BASIC_ADDRESS					(DRY_CONFIG_BASIC_ADDRESS + 12*K)// 512 * 32 = 16K byte
//#define SCHEDULER_CONFIG_BASIC_ADDRESS				(SCHEDULER_EVENT_BASIC_ADDRESS + 16*K)// 512 * 32 = 16K byte

//// air conditioner
//#define AIR_CONDITIONER_CONFIG_BASIC_ADDRESS	(SCHEDULER_CONFIG_BASIC_ADDRESS + 16*K)// 64 * 32 = 2K byte

//// curtain
//#define CURTAIN_CONFIG_BASIC_ADDRESS					(AIR_CONDITIONER_CONFIG_BASIC_ADDRESS + 2*K)// 64 * 32 = 2K byte

//// dimmer
//#define DIMMER_CONFIG_BASIC_ADDRESS					  (CURTAIN_CONFIG_BASIC_ADDRESS + 2*K)// 1024 * 32 = 32K byte

//// led
//#define LED_CONFIG_BASIC_ADDRESS					    (DIMMER_CONFIG_BASIC_ADDRESS + 32*K)// 256 * 32 = 8K byte

//// relay
//#define RELAY_CONFIG_BASIC_ADDRESS						(LED_CONFIG_BASIC_ADDRESS + 8*K)// 1024 * 32 = 32K byte

//// IR
//#define IR_CONFIG_BASIC_ADDRESS								(RELAY_CONFIG_BASIC_ADDRESS + 32*K)// 64 * 32 = 2K byte
//#define IR_EVENT_BASIC_ADDRESS								(IR_CONFIG_BASIC_ADDRESS + 2*K)// 64 * 512 = 32K byte




#endif
