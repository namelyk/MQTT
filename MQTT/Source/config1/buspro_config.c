#include "hw_init.h"
#include "stack.h"

#ifdef BUSPRO_ENABLE

static void interrupt_enable(void)
{
	BUSPRO_TX_ISR_ENABLE();
}

static void interrupt_disable(void)
{
	BUSPRO_TX_ISR_DISABLE();
}

static void ioctl_enable(void)
{
	BUSPRO_TX_ENABLE();
}

static void ioctl_disable(void)
{
	BUSPRO_TX_DISABLE();
}	

static void write(unsigned char write_byte)
{
	BUSPRO_TX_SEND(write_byte);
}

struct buspro_interface_t buspro_low_interface =
{ 
	interrupt_enable,interrupt_disable,ioctl_enable,ioctl_disable,write,
};
#endif
