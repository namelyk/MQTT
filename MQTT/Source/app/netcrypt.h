#ifndef _NETCRYPT_H_
#define _NETCRYPT_H_
#include "mbedtls/aes.h"



extern int netcrypt_aes_encrypt(unsigned char *key, unsigned int length,unsigned char *input,unsigned char *output);
extern int netcrypt_aes_decrypt(unsigned char *key, unsigned int length,unsigned char *input,unsigned char *output);

extern void netcrypt_aes_test(void);


#define NETCRYPT_ENABLE
#ifdef NETCRYPT_ENABLE


extern void netcrypt_handler(void);
extern int netcrypt_send(unsigned int cmd,unsigned char src_subnet,\
					unsigned char src_device,unsigned char dest_subnet,\
					unsigned char dest_device,unsigned char *buffer,int len);
					
extern int netcrypt_long_send(unsigned int cmd,unsigned char src_subnet,\
												unsigned char src_device,unsigned char dest_subnet,\
												unsigned char dest_device,unsigned char *buffer,int len);
												
extern int netcrypt_multi_send(unsigned int cmd,unsigned char src_subnet,unsigned char src_device, \
											unsigned char dest_subnet,unsigned char dest_device, \
											unsigned char *buffer,unsigned char len);	
extern int netcrypt_allbroadcast_send(unsigned int cmd,unsigned char src_subnet,unsigned char src_device, \
											unsigned char dest_subnet,unsigned char dest_device, \
											unsigned char *buffer,unsigned char len);											
#if 0												
extern void netcrypt_tenMs_handler(void);
#endif										
extern const struct app_interface_t netcrypt_interface;


#endif
#endif

