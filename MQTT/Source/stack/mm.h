#ifndef _MM_H_
#define _MM_H_


//#define SYS_MEM_ENABLE

#ifdef SYS_MEM_ENABLE
extern int sys_mpercent(void);
extern int sys_mremain(void);
extern int sys_mfree(void);
extern unsigned char *sys_malloc(int size);
#endif

extern void sys_memset(unsigned char *mem,unsigned char set,int size);
extern void sys_memcpy(unsigned char *dest,unsigned char *src,int size);
extern int sys_memcmp(unsigned char *dest,unsigned char *src,int size);


#endif
