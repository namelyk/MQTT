#ifndef _DEVICES_H_
#define _DEVICES_H_


#define DEVICE_ENABLE
#ifdef DEVICE_ENABLE



#define DEVICE_MCU_ENABLE

#pragma pack(1)

struct devices_t
{
	unsigned char subnet;
	unsigned char device;
	
	unsigned char MAC[8];
	unsigned char UID[16];
	
	unsigned char name[20];
};
#pragma pack()


extern struct devices_t *devices;

extern const struct app_interface_t device_interface;

//
extern int devices_get_remark(unsigned char *remark);
extern int devices_set_remark(unsigned char *remark);
extern int devices_get_mac(unsigned char *mac);
extern int devices_set_mac(unsigned char *mac);
extern int devices_get_subnet(unsigned char *buffer);
extern int devices_set_subnet(unsigned char *buffer);
extern int devices_check_set_subnet(unsigned char *buffer);
extern int devices_reset_config(unsigned char *buffer);
extern int devices_is_broadcast(unsigned char subnet,unsigned char device);
extern int devices_is_local_subnet(unsigned char subnet);
extern int devices_is_localaddress(unsigned char subnet,unsigned char device);
extern void devices_button_set_program(unsigned char value);
extern int devices_button_get_mode(void);


#endif
#endif
