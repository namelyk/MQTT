#ifndef _WECHAT_H_
#define _WECHAT_H_


#define WECHAT_ENABLE
#ifdef WECHAT_ENABLE

extern int wechat_send_message(unsigned char subnet,unsigned char device, unsigned char channel,unsigned char status);
extern int wechat_send_message_reply(unsigned char channel,unsigned char status);
extern int wechat_send_utf8(unsigned char subnet,unsigned char device, unsigned char *utf8,unsigned char size);
extern int wechat_send_utf8_reply(unsigned char *reply,unsigned char size);

#endif
#endif
