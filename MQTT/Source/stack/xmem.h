#ifndef _XMEM_H_
#define _XMEM_H_


#define XMEM_ENABLE
#ifdef XMEM_ENABLE

#pragma pack(1)


struct xmem_frame_t
{
   unsigned short size;
   unsigned char payload[1024];
};



struct xmem_pool_t
{
	unsigned short size;
	unsigned short lenght;
	
	unsigned short count;
	unsigned short index;
	unsigned short point;
	unsigned short compare;
	
  unsigned char retry;
	unsigned char ok;
  unsigned char status;
  unsigned char timeout;
  
  unsigned char *frame;
};



struct iodriver_t
{
  void (*setup)(void);
  void (*ioctl)(int flags);
  void (*enable)(void);
  void (*disable)(void);
  void (*send)(unsigned char);
  void (*sendbuffer)(unsigned char *,int);
  void (*callback)(unsigned char *,int);
};


struct xmem_t
{
	char xmode;
	char loopback;
	int tx_timeout;
	int rx_timeout;
	
	struct xmem_frame_t *crx;
	struct xmem_frame_t *ctx;
	struct xmem_frame_t *hrx;
	struct xmem_frame_t *htx;
	
	struct xmem_pool_t *rx;
	struct xmem_pool_t *tx;
	
	struct iodriver_t *lowdriver;
};

#pragma pack()



//============================api=========================================
void xmem_init(struct xmem_t *xmem,
							struct xmem_pool_t *txpool,unsigned char *txframe,int txnum,int txsize,
							struct xmem_pool_t *rxpool,unsigned char *rxframe,int rxnum,int rxsize);						
void xmem_register(struct xmem_t *xmem,struct iodriver_t *iodriver);
void xmem_ticks(struct xmem_t *xmem);
void xmem_status(struct xmem_t *xmem);
void xmem_send(struct xmem_t *xmem);
void xmem_receive(struct xmem_t *xmem,unsigned char input);
int xmem_write(struct xmem_t *xmem,unsigned char *buffer,int size);
void xmem_handler(struct xmem_t *xmem);

#endif
#endif
