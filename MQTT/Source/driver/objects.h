#ifndef _OBJECTS_H_
#define _OBJECTS_H_


#define OBJECTS_ENABLE
#ifdef OBJECTS_ENABLE

extern void objects_init(void);
extern int objects_request(int cmd,unsigned char subnet,unsigned char device,unsigned char *buffer,unsigned char len);
extern int objects_response(int cmd,unsigned char subnet,unsigned char device,unsigned char *buffer,unsigned char len);
extern void objects_handler(void);


#endif
#endif

