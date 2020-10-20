#ifndef _CLIENT_H_
#define _CLIENT_H_

//#define CLIENT_ENABLE
#ifdef CLIENT_ENABLE

extern int client_set(unsigned int ipaddr,unsigned short port);
extern void client_init(void);
extern int client_write(unsigned char *buffer,int len);
extern int client_handler(void);



#endif
#endif

