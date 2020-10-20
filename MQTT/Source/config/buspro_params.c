#include "buspro_params.h"
#include "sys_include.h"

#ifdef BUSPRO_PARAMS_ENABLE

static unsigned char buspro_baudrate = 3;
int buspro_get_baudrate(unsigned char *baudrate)
{
	*baudrate = buspro_baudrate;
	return 0;
}


int buspro_set_baudrate(unsigned char *baudrate)
{
	buspro_baudrate = *baudrate;
	
	if (buspro_baudrate == 8)
	{
		USART_115200_Configuration();
	}
	else
	{
		buspro_baudrate = 3;
		USART_Configuration();
	}
	return 0;
}


#endif


