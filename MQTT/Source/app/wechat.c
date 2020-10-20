#include "wechat.h"
#include "sys_include.h"



#ifdef WECHAT_ENABLE

#define WECHAT_SEND(cmd,src_subnet,src_device, subnet, device, buffer,size)	  netip_gateway_send_special(cmd, src_subnet, src_device, subnet, device, buffer, size)



//=======================User define API===============================


int wechat_send_message(unsigned char subnet,unsigned char device, unsigned char channel,unsigned char status)
{
	unsigned char buff[4];
	
	buff[0] = channel;
	buff[1] = status;
	
	return WECHAT_SEND(WECHAT_SEND_INFO_CMD, subnet, device, 251, 251, buff, 2);
}

int wechat_send_message_reply(unsigned char channel,unsigned char status)
{
	
	return 0;
}


int wechat_send_utf8(unsigned char subnet,unsigned char device, unsigned char *utf8,unsigned char size)
{
	
	return WECHAT_SEND(WECHAT_SEND_UTF8_CMD,subnet,device,251,251,utf8,size);
}

int wechat_send_utf8_reply(unsigned char *reply,unsigned char size)
{
	
	return 0;
}



#endif
