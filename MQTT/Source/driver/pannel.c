#include "driver.h"
#include "pannel.h"




#ifdef PANNEL_ENABLE

// ========================low driver ===================================
static void _setup(void)
{
  UART5_Configuration(9600,8,1,0);
}

static void _ioctl(int flags)
{
  if (flags)
	{
		RS485_IN_ENABLE();
	}
	else
	{
		RS485_IN_DISABLE();
	}
}

	
static void _enable(void)
{
  RS485_IN_TX_ISR_ENABLE();
}


static void _disable(void)
{
  RS485_IN_TX_ISR_DISABLE();
}

static void _send(unsigned char txbyte)
{
  RS485_IN_TX_SEND(txbyte);
}


static void _sendbuffer(unsigned char *buffer,int size)
{
  
}


static void _callback(unsigned char *buffer,int size)
{
    
}

struct iodriver_t pannel_interface =
{
  _setup,_ioctl,_enable,_disable,_send,_sendbuffer,_callback,
};



#endif


