#ifndef _RS485_H_
#define _RS485_H_

//#define RS485_ENABLE
#ifdef RS485_ENABLE
#include "xmem.h"








extern struct xmem_t rs485_tasks_struct[];


// ==================API=============================================
#define RS485_PORT1_REGISTER(a)   xmem_register(&rs485_tasks_struct[0],a)
#define RS485_PORT2_REGISTER(a)   xmem_register(&rs485_tasks_struct[1],a)
#define RS485_PORT3_REGISTER(a)   xmem_register(&rs485_tasks_struct[2],a)
#define RS485_PORT4_REGISTER(a)  	xmem_register(&rs485_tasks_struct[3],a)
#define RS485_PORT5_REGISTER(a)   xmem_register(&rs485_tasks_struct[4],a)

#define RS485_PORT1_TICKS()   xmem_ticks(&rs485_tasks_struct[0])
#define RS485_PORT2_TICKS()   xmem_ticks(&rs485_tasks_struct[1])
#define RS485_PORT3_TICKS()   xmem_ticks(&rs485_tasks_struct[2])
#define RS485_PORT4_TICKS()   xmem_ticks(&rs485_tasks_struct[3])
#define RS485_PORT5_TICKS()   xmem_ticks(&rs485_tasks_struct[4])


#define RS485_PORT1_STATUS()   xmem_status(&rs485_tasks_struct[0])
#define RS485_PORT2_STATUS()   xmem_status(&rs485_tasks_struct[1])
#define RS485_PORT3_STATUS()   xmem_status(&rs485_tasks_struct[2])
#define RS485_PORT4_STATUS()   xmem_status(&rs485_tasks_struct[3])
#define RS485_PORT5_STATUS()   xmem_status(&rs485_tasks_struct[4])

#define RS485_PORT1_SENDTO()   xmem_send(&rs485_tasks_struct[0])
#define RS485_PORT2_SENDTO()   xmem_send(&rs485_tasks_struct[1])
#define RS485_PORT3_SENDTO()   xmem_send(&rs485_tasks_struct[2])
#define RS485_PORT4_SENDTO()   xmem_send(&rs485_tasks_struct[3])
#define RS485_PORT5_SENDTO()   xmem_send(&rs485_tasks_struct[4])

#define RS485_PORT1_RECEIVE(a)   xmem_receive(&rs485_tasks_struct[0],a)
#define RS485_PORT2_RECEIVE(a)   xmem_receive(&rs485_tasks_struct[1],a)
#define RS485_PORT3_RECEIVE(a)   xmem_receive(&rs485_tasks_struct[2],a)
#define RS485_PORT4_RECEIVE(a)   xmem_receive(&rs485_tasks_struct[3],a)
#define RS485_PORT5_RECEIVE(a)   xmem_receive(&rs485_tasks_struct[4],a)

#define RS485_PORT1_SEND(a,b)   xmem_write(&rs485_tasks_struct[0],a,b)
#define RS485_PORT2_SEND(a,b)   xmem_write(&rs485_tasks_struct[1],a,b)
#define RS485_PORT3_SEND(a,b)   xmem_write(&rs485_tasks_struct[2],a,b)
#define RS485_PORT4_SEND(a,b)   xmem_write(&rs485_tasks_struct[3],a,b)
#define RS485_PORT5_SEND(a,b)   xmem_write(&rs485_tasks_struct[4],a,b)


#define RS485_PORT1_HANDLER()   xmem_handler(&rs485_tasks_struct[0])
#define RS485_PORT2_HANDLER()   xmem_handler(&rs485_tasks_struct[1])
#define RS485_PORT3_HANDLER()   xmem_handler(&rs485_tasks_struct[2])
#define RS485_PORT4_HANDLER()   xmem_handler(&rs485_tasks_struct[3])
#define RS485_PORT5_HANDLER()   xmem_handler(&rs485_tasks_struct[4])

//
extern const struct driver_interface_t rs485_interface;


#endif
#endif



