#ifndef _BUSPRO_H_
#define _BUSPRO_H_


#define BUSPRO_ENABLE


//======================================================================
struct buspro_interface_t
{
	void (*interrupt_enable)(void);
	void (*interrupt_disable)(void);
	void (*ioctl_enable)(void);
	void (*ioctl_disable)(void);
	void (*write)(unsigned char);
};

#ifdef BUSPRO_ENABLE
//======================================================================
extern int buspro_set_type(unsigned int type,unsigned char *encode,\
					void (*callback)(int ,unsigned char*,unsigned char));
extern unsigned int buspro_get_type(void);

extern void buspro_init(void);
extern void buspro_receive(unsigned char recvbyte);
extern void buspro_sendto(void);
extern void buspro_ticks(void);
extern void buspro_status(void);
extern void buspro_handler(void);
extern int buspro_send(unsigned int cmd,unsigned char src_subnet,unsigned char src_device, \
											unsigned char dest_subnet,unsigned char dest_device, \
											unsigned char *buffer,unsigned char len);
extern int buspro_write(unsigned char *buffer,unsigned char len);

#endif

#endif
