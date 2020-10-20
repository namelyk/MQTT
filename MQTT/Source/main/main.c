/***************************************************
ģ�����ƣ�  Demo
����������	
�� �� ��:	  Ѧ����(steven sit)  
��дʱ�䣺	2012-05-10
***************************************************/
#include "sys_include.h" 
#include "stm32_eth.h"


/***************************************************
�������ƣ�	int main(void)
���������	
�� �� ֵ:	
����������	
***************************************************/
int main(void)
{
	DISABLE_INTERRUPT(); //�ر��ж�
#ifdef NETIP_ENABLE	
	RCC_Configuration();
#endif
	
	hw_init(); //ϵͳ��ʼ��
	
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
	
	ENABLE_INTERRUPT();//���ж�
	while(1)
	{
		app_handler();//���� init������
#ifdef BUSPRO_ENABLE	
		buspro_handler(); //������
#endif
		
#ifdef NETIP_ENABLE		
#ifdef IOT_ENABLE	
		IOT_HANDLER();
#endif
		
		netip_handler();
#endif
		
		
		
	}
}





