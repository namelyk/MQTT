
#include "rs232.h"
#include "driver.h"


#ifdef RS232_ENABLE

extern void USART1_Configuration(unsigned char baudrate,unsigned char databit,unsigned char stopbit,unsigned char parity);


#define RS232_PORT_SIZE											1

#define RS232_TX_TIMEOUT										1
#define RS232_RX_TIMEOUT										5


#define RS232_RX_SIZE												15
#define RS232_TX_SIZE												15

#define RS232_FRAME_MAX_SIZE                (82)
#define RS232_TX_BUFFER_SIZE								(RS232_FRAME_MAX_SIZE * RS232_RX_SIZE)
#define RS232_RX_BUFFER_SIZE								(RS232_FRAME_MAX_SIZE * RS232_RX_SIZE)




static unsigned char rs232_tx_tasks_frame[RS232_PORT_SIZE][RS232_TX_BUFFER_SIZE];
static unsigned char rs232_rx_tasks_frame[RS232_PORT_SIZE][RS232_RX_BUFFER_SIZE];
static struct xmem_pool_t rs232_tx_tasks_pool[RS232_PORT_SIZE];
static struct xmem_pool_t rs232_rx_tasks_pool[RS232_PORT_SIZE];
struct xmem_t rs232_tasks_struct[RS232_PORT_SIZE];



//=======================User define API===============================


//=======================Standard define API===============================

static int _get(unsigned char *config,int len)
{
	// ²¨ÌØÂÊ
	eeprom_read(EEPROM_SERIAL_BASIC_ADDR,config,5);
	
	if (config[0] == 1)
	{
		USART1_Configuration(config[1],config[2],config[3],config[4]);
	}
	return 0;
}


static int _set(unsigned char *config,int len)
{
	eeprom_write(EEPROM_SERIAL_BASIC_ADDR,config,5);
	
	if (config[0] == 1)
	{
		USART1_Configuration(config[1],config[2],config[3],config[4]);
	}
	return 0;
}




//=============================================================
static void _init(void)
{
	int i;
	unsigned char buf[5];
	
	// init 
	for (i = 0; i < RS232_PORT_SIZE; i++)
	{
		rs232_tasks_struct[i].loopback = 0;
		rs232_tasks_struct[i].xmode = 0;
		rs232_tasks_struct[i].tx_timeout = RS232_TX_TIMEOUT;
		rs232_tasks_struct[i].rx_timeout = RS232_RX_TIMEOUT;
		xmem_init(&rs232_tasks_struct[i],&rs232_tx_tasks_pool[i],
							&rs232_tx_tasks_frame[i][0],RS232_TX_SIZE,RS232_FRAME_MAX_SIZE,
							&rs232_rx_tasks_pool[i],
							&rs232_rx_tasks_frame[i][0],RS232_RX_SIZE,RS232_FRAME_MAX_SIZE);
	}
	// 
	RS232_PORT1_REGISTER((struct iodriver_t *)&usb_serial_interface);
	
	
	_get(buf,5);

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
	RS232_PORT1_SEND(buffer,len);
	return 0;
}


static int _handler(void *event)
{
	
	return 0;
}

//====================low interface=====================
const struct driver_interface_t rs232_interface =
{
	_get,_set,_init,_status,_read,_write,_handler
};

#endif


