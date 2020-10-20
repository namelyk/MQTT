#include "linux.h"
#include "sys_include.h"

#ifdef LINUX_ENABLE



/***************************************************
				 linux_handler
***************************************************/
void linux_init(void)
{
	
}


void linux_handler(void)
{
		RESET_WATCH_DOG();	 //清看门狗

#ifdef NETCRYPT_ENABLE	
	netcrypt_handler();
#endif
	
#ifdef BUSPRO_ENABLE	
		buspro_handler();
#endif
		
#ifdef NETIP_ENABLE			
		netip_handler();
#endif
	
	app_handler();//处理
}

#endif


