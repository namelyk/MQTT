#ifndef _CRYPT_H_
#define _CRYPT_H_

extern int remote_crypt(unsigned char *key,unsigned char key_size,unsigned char *encode,unsigned char en_size);
extern int ascii_crypt(unsigned int key,unsigned char *encode);
extern int serial_crypt(unsigned char *key,unsigned char *encode);

#endif
