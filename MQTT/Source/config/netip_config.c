#include "hw_init.h"
#include "stack.h"


#ifdef NETIP_ENABLE

static void interrupt_enable(void)
{
	ENABLE_INTERRUPT();
}

static void interrupt_disable(void)
{
	DISABLE_INTERRUPT();
}

void netip_wdt_reset(void)
{
	RESET_WATCH_DOG();
}


struct netip_interface_t netip_low_interface =
{
	interrupt_enable,interrupt_disable,netip_wdt_reset
};

#endif


