
#include "rs485.h"
#include "driver.h"


#ifdef RS485_ENABLE


#define RS485_PORT_SIZE											1

#define RS485_TX_TIMEOUT										1
#define RS485_RX_TIMEOUT										5

#define RS485_RX_SIZE												15
#define RS485_TX_SIZE												15

#define RS485_FRAME_MAX_SIZE                (82)
#define RS485_TX_BUFFER_SIZE								(RS485_FRAME_MAX_SIZE * RS485_RX_SIZE)
#define RS485_RX_BUFFER_SIZE								(RS485_FRAME_MAX_SIZE * RS485_RX_SIZE)


static unsigned char rs485_tx_tasks_frame[RS485_PORT_SIZE][RS485_TX_BUFFER_SIZE];
static unsigned char rs485_rx_tasks_frame[RS485_PORT_SIZE][RS485_RX_BUFFER_SIZE];
static struct xmem_pool_t rs485_tx_tasks_pool[RS485_PORT_SIZE];
static struct xmem_pool_t rs485_rx_tasks_pool[RS485_PORT_SIZE];
struct xmem_t rs485_tasks_struct[RS485_PORT_SIZE];

//=======================User define API===============================


//=======================Standard define API===============================

static int _get(unsigned char *config,int len)
{
	
	return 0;
}


static int _set(unsigned char *config,int len)
{
	
	return 0;
}




//=============================================================
static void _init(void)
{
	int i;
	
	// init 
	for (i = 0; i < RS485_PORT_SIZE; i++)
	{
		rs485_tasks_struct[i].loopback = 1;
		rs485_tasks_struct[i].xmode = 0;
		rs485_tasks_struct[i].tx_timeout = RS485_TX_TIMEOUT;
		rs485_tasks_struct[i].rx_timeout = RS485_RX_TIMEOUT;
		xmem_init(&rs485_tasks_struct[i],&rs485_tx_tasks_pool[i],
							&rs485_tx_tasks_frame[i][0],RS485_TX_SIZE,RS485_FRAME_MAX_SIZE,
							&rs485_rx_tasks_pool[i],
							&rs485_rx_tasks_frame[i][0],RS485_RX_SIZE,RS485_FRAME_MAX_SIZE);
	}
	
//	RS485_PORT1_REGISTER(&pannel_interface);

}



static int _status(unsigned char *buffer,int len)
{
	
	return 0;
}

static int _read(unsigned char *buffer,int len)
{
	
	return 0;
}


static int _write(unsigned char *buffer,int len)
{
	
	return 0;
}


static int _handler(void *event)
{
	

	return 0;
}

//====================low interface=====================
const struct driver_interface_t rs485_interface =
{
	_get,_set,_init,_status,_read,_write,_handler
};

#endif


