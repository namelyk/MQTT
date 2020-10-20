#include "app.h"
#include "sys_include.h" 

int system_ticks;
char app_enable = 0;

//#define BUSPRO_DEBUG
#ifdef BUSPRO_DEBUG
void buspro_debug(void)
{
	unsigned char buffer[10] = "123456789";
	
	app_commands_send(255,0x8888,255,255,buffer,5);
}
#endif


/***************************************************
				 app init
***************************************************/
void app_init(void)
{
	app_enable = 0;
#ifdef DEVICE_ENABLE		
#ifdef NETIP_GATEWAY_ENABLE
	netip_gateway_init();
#endif
	
	device_interface.init();
#endif
	
#ifdef DRIVER_ENABLE//eeprom datatime
	driver_init();
#endif
	
#ifdef GATEWAY_ENABLE	 //��
	gateway_interface.init();
#endif	

#ifdef IAP_ENABLE//��������
	iap_interface.init();
#endif
	
#ifdef FILTER_ENABLE//��
	filter_interface.init();
#endif
	
#ifdef ROUTE_ENABLE
	route_interface.init();
#endif	

#ifdef IOCTL_ENABLE//��
	ioctl_init();
#endif

#ifdef MODBUS_TCP_ENABLE//��
	MODBUS_TCP_INIT();
#endif

#ifdef IOT_ENABLE
	iot_config_interface.init();
#endif

}



/***************************************************
				 app handler
***************************************************/
void app_handler(void)
{
	static unsigned char app_1s_count = 0;
	static unsigned int system_10ticks = 0;
	
#ifdef MODBUS_TCP_ENABLE
	MODBUS_TCP_HANDLER();	
#endif
	
#ifdef NETCRYPT_ENABLE
	netcrypt_interface.handler((void*)NULL);
#endif
	//Զ�̷�����
#ifdef NETIP_GATEWAY_ENABLE		
	netip_gateway_handler();
#endif	
	if(system_ticks < 1) return;
	system_ticks = 0;
#ifdef IOT_ENABLE
	iot_config_interface.oneMs_handler(); // MQTT 1ms������ yk 2020-02-20��
#endif
	system_10ticks++;
	if (system_10ticks < 10) return;//10ms	
	system_10ticks = 0;	
	LED_UP_OFF();
	
	system_ticks = 0;
	app_enable = 1;
	
	RESET_WATCH_DOG();	 //�忴�Ź�
#if 0
	netcrypt_tenMs_handler(); //�������HDL-ON IP����  2020-07-01��
#endif
#ifdef DRIVER_ENABLE		
	driver_handler();
#endif
	
#ifdef DEVICE_ENABLE		
	device_interface.handler((void*)NULL);
#endif

#ifdef ROUTE_ENABLE
	route_interface.handler((void*)NULL);
#endif	

	//����������ʱ
#ifdef IAP_ENABLE
	iap_interface.handler((void *)NULL);
#endif

	app_1s_count++;
	if (app_1s_count < 100) return; //1s
	app_1s_count = 0;

#ifdef IOT_ENABLE
	//yk 2019-11-29�� ʱ��Ƭ���� 2020-05-27
//	iot_timestamp_auto_increment();
#endif
	
#ifdef BUSPRO_DEBUG
	buspro_debug();
#endif
	
#ifdef NETIP_ENABLE			
	netip_reset();
#endif
	


	
}





