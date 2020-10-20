/***************************************************
模块名称：  Demo
功能描述：	
设 计 者:	  薛柯利(steven sit)  
编写时间：	2012-05-10
***************************************************/
#include "sys_include.h" 
#include "stm32_eth.h"


/***************************************************
函数名称：	int main(void)
输入参数：	
返 回 值:	
功能描述：	
***************************************************/
int main(void)
{
	DISABLE_INTERRUPT(); //关闭中断
#ifdef NETIP_ENABLE	
	RCC_Configuration();
#endif
	
	hw_init(); //系统初始化
	
	app_init();

#ifdef BUSPRO_ENABLE	
	buspro_set_type(DEVICE_TYPE,\
		(unsigned char *)DEVICE_ENCODE,&app_commands_handler);
	buspro_init();
#endif
	
#ifdef NETIP_ENABLE	
	netip_set_type(DEVICE_TYPE,\
		(unsigned char *)DEVICE_ENCODE,&netip_commands_handler);
	netip_init();
#endif
	
	ENABLE_INTERRUPT();//打开中断
	while(1)
	{
		app_handler();//处理 init在里面
#ifdef BUSPRO_ENABLE	
		buspro_handler(); //任务处理
#endif
		
#ifdef NETIP_ENABLE		
#ifdef IOT_ENABLE	
		IOT_HANDLER();
#endif
		
		netip_handler();
#endif
		
		
		
	}
}





