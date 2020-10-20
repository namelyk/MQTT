#ifndef _DEVICES_H_
#define _DEVICES_H_


#define DEVICE_ENABLE
#ifdef DEVICE_ENABLE

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

#endif
#endif
