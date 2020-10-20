#include "app.h"
#include "sys_include.h" 

int system_ticks;


extern void do_tcp_test(void);

extern int app_commands_send(int s,unsigned int cmd,unsigned char subnet,\
					unsigned char device,unsigned char *buffer,unsigned char size);

void netip_debug(void)
{
	static char timeout;
	char buffer[64];
	uint32_t temp;
	
	do_tcp_test();
	
	
	timeout++;
	if (timeout < 10) return;
	timeout = 0;
	
	
	
	//app_commands_send(1,0x8888,255,255,(unsigned char *)buffer,16);
}



/***************************************************
				 app init
***************************************************/
void app_init(void)
{
#ifdef DRIVER_ENABLE	
	driver_init();
#endif
	
	
#ifdef HOTEL_ENABLE
	hotel_init();
#endif	
	
	
#ifdef DEVICE_ENABLE		
	device_interface.init();
#endif
	
#ifdef GATEWAY_ENABLE	
	gateway_interface.init();
#endif	

#ifdef IAP_ENABLE
	iap_interface.init();
#endif
	
#ifdef FILTER_ENABLE
	filter_interface.init();
#endif
	
#ifdef ROUTE_ENABLE
	route_interface.init();
#endif	

}


/***************************************************
				 app handler
***************************************************/
void app_handler(void)
{
	static unsigned char app_1s_count = 0;
	static unsigned char app_10s_count = 0;
	
 	if (system_ticks < 10) return;//10ms
	system_ticks = 0;

	RESET_WATCH_DOG();	 //清看门狗
	
	
#ifdef DRIVER_ENABLE		
	driver_handler();
#endif
	
	
	//在线升级超时
#ifdef IAP_ENABLE
	iap_interface.handler((void *)NULL);
#endif

	

	app_1s_count++;
	if (app_1s_count < 100) return; //1s
	app_1s_count = 0;
	
	
	
	//远程服务器
#ifdef NETIP_GATEWAY_ENABLE		
	netip_gateway_handler();
#endif


	//远程连接用户处理
#ifdef REMOTE_ENABLE
	remote_interface.handler((void *)NULL);
#endif

	LED_GREEN_OFF();
	LED_RED_OFF();
	
	
#ifdef NETIP_ENABLE	
	if (!KEY_EVENT_VALUE())
	{
		netip_reset();
	}
#endif
	
	
	
	
	netip_debug();
}





