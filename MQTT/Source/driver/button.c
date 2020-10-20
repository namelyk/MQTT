#include "driver.h"
#include "button.h"


#ifdef BUTTON_ENABLE

#define BUTTON_MAX_SIZE					(4)



//=============================================================
struct button_t
{
	unsigned char enable:1;
	unsigned char update:1;
	unsigned char level;
	
};

static int button_index = 0;
static struct button_t button_struct[BUTTON_MAX_SIZE];

//=====================================================================

#define BUTTON_GET(A,B,C)		



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
const struct driver_interface_t button_interface =
{
	_get,_set,_init,_status,_read,_write,_handler
};

#endif


