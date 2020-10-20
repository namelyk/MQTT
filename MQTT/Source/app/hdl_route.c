/***************************************************
模块名称：  route
功能描述：	
设 计 者:	薛柯利(steven sit)  
编写时间：	2015-08-01
***************************************************/

#include "sys_include.h"
#include "hdl_route.h"


#ifdef ROUTE_ENABLE

#define ROUTE_STATE_IDLE        				0
#define ROUTE_STATE_SEND        				1 
#define ROUTE_STATE_WAIT        				2
#define ROUTE_STATE_RECV        				3   
#define ROUTE_STATE_SAVE        				4 



#define ROUTE_MAX_SIZE          				1
#define ROUTE_TABLE_NUM         				64


#define SOCKET_LINE_NO									128



//=============================================
typedef struct
{
  uint8_t flag:1;
  uint8_t update:7;
  
  uint8_t old_net;
  uint8_t old_dev;
  
  uint8_t new_net;
  uint8_t new_dev;
  
  uint8_t type;
  
  uint8_t mac[8];
  
}RouteTable_t;


typedef struct
{
  uint8_t enable:1;
	uint8_t getlist:1;
  uint8_t ready:6;
  
  uint8_t rank[4];
  
  RouteTable_t route[ROUTE_TABLE_NUM]; 
   
}Route_t;






//========================================================
uint32_t route_tick_count = 0;
static uint32_t route_tick_backup = 0;

uint32_t route_update_eeprom = 0;
uint8_t route_update_eeprom_save = 0;
static uint8_t route_subnet;

static uint8_t route_state = ROUTE_STATE_IDLE;
static uint8_t route_index = 0;
static uint8_t route_timeout = 0;
static uint8_t route_update_timeout = 0;
static Route_t _route[ROUTE_MAX_SIZE];

static void route_table_update(uint8_t no);
static void route_eeprom_update(void);





/***************************************************
函数名称：   system_socket_send
输入参数：	
返 回 值:	 
功能描述：	  
***************************************************/
void system_socket_send(int s,unsigned int cmd,unsigned char subnet,unsigned char device,\
											unsigned char *buffer,unsigned char size)
{
	switch (s)
	{
		case 0:
			buspro_send(cmd,devices->subnet,devices->device,subnet,device,buffer,size);
			break;
		
		case SOCKET_LINE_NO:
			netcrypt_send(cmd,devices->subnet,devices->device,subnet,device,buffer,size);
			//netip_send(cmd,devices->subnet,devices->device,subnet,device,buffer,size);// 2020-05-26改
			break;
	}
}

/***************************************************
函数名称：   route_enable
输入参数：	
返 回 值:	 
功能描述：	  
***************************************************/
static void route_ready(uint8_t no,uint8_t *rank)
{
  uint32_t tmp;
  
  tmp = route_tick_count;
  if (no >= ROUTE_MAX_SIZE) return; 
  _route[no].ready = 1;
  
  rank[0] = rank[0]^(uint8_t)(tmp);
  rank[1] = rank[1]^(uint8_t)(tmp>>8);
  rank[2] = rank[2]^(uint8_t)(tmp>>16);
  rank[3] = rank[3]^(uint8_t)(tmp>>24);
  
  _route[no].rank[0] = rank[0];
  _route[no].rank[1] = rank[1];
  _route[no].rank[2] = rank[2];
  _route[no].rank[3] = rank[3];
}

/***************************************************
函数名称：   route_enable
输入参数：	
返 回 值:	 
功能描述：	  
***************************************************/
static uint8_t route_enable(uint8_t no,uint8_t *rank)
{
  if (no >= ROUTE_MAX_SIZE) return 1; 
  if (_route[no].ready == 0) return 1;
  if (_route[no].rank[0] != rank[0] ||
      _route[no].rank[1] != rank[1] ||
      _route[no].rank[2] != rank[2] ||
      _route[no].rank[3] != rank[3]) return 1;
  
  _route[no].ready = 0;
  _route[no].enable = 1;
	_route[no].getlist = 0;
  return 0;
}
/***************************************************
函数名称：   route_getlist
输入参数：	
返 回 值:	 
功能描述：	  
***************************************************/
static uint8_t route_getlist(uint8_t no,uint8_t *rank)
{
  if (no >= ROUTE_MAX_SIZE) return 1; 
  if (_route[no].ready == 0) return 1;
  if (_route[no].rank[0] != rank[0] ||
      _route[no].rank[1] != rank[1] ||
      _route[no].rank[2] != rank[2] ||
      _route[no].rank[3] != rank[3]) return 1;
  
  _route[no].ready = 0;
  _route[no].enable = 1;
	_route[no].getlist = 1;
  return 0;
}

/***************************************************
函数名称：   route_disable
输入参数：	
返 回 值:	 
功能描述：	  
***************************************************/
static uint8_t route_disable(uint8_t no)
{
  if (no >= ROUTE_MAX_SIZE) return 1;
  _route[no].enable = 0;
  return 0;
}

/***************************************************
函数名称：   route_register
输入参数：	
返 回 值:	 
功能描述：	  
***************************************************/
static uint8_t route_register(uint8_t no,uint8_t subnet,uint8_t device,uint8_t *mac)
{
  uint8_t i;
  if (no >= ROUTE_MAX_SIZE) return 1;
  route_subnet = no + devices->subnet;
        
  //检测是否存在
  for(i = 0; i < ROUTE_TABLE_NUM; i++)
  {
    if (_route[no].route[i].flag == 1)
    {
      if (_route[no].route[i].new_net != route_subnet)
      {
        _route[no].route[i].flag = 0;
        continue;
      }
      
      if (!memcpy(_route[no].route[i].mac,mac,8))
			{
				if (subnet == route_subnet)
				{
					return 0;
				}
				else
				{
					_route[no].route[i].flag = 0;
				}
			}
    }
	}
  
	
  //检测空闲路由表  
  for(i = 0; i < ROUTE_TABLE_NUM; i++)
  {
    if (_route[no].route[i].flag == 0)
    {
        memcpy(_route[no].route[i].mac,mac,8);
        _route[no].route[i].type = mac[8];
        _route[no].route[i].flag = 1;
        _route[no].route[i].update = 1;
			
				if (_route[no].getlist == 1)
				{
					_route[no].route[i].new_net = subnet;
					_route[no].route[i].new_dev = device;
					
					_route[no].route[i].old_net = subnet;
					_route[no].route[i].old_dev = device;
				}
				else
				{
					_route[no].route[i].new_net = route_subnet;
					_route[no].route[i].new_dev = i+1;
					
					_route[no].route[i].old_net = subnet;
					_route[no].route[i].old_dev = device;
				}
        return 0;
    }
  }
  return 1;
}
/***************************************************
函数名称：   route_unregister
输入参数：	
返 回 值:	 
功能描述：	  
***************************************************/
static uint8_t route_unregister(uint8_t no,uint8_t *mac)
{
  uint8_t i;
  if (no >= ROUTE_MAX_SIZE) return 1;
  
  for(i = 0; i < ROUTE_TABLE_NUM; i++)
  {
    if (_route[no].route[i].flag)
    {
      if (!memcmp(_route[no].route[i].mac,mac,8))
      {
        _route[no].route[i].flag = 0;
        _route[no].route[i].update = 0;
        _route[no].route[i].new_net = 0;
        _route[no].route[i].new_dev = 0;
        
        _route[no].route[i].old_net = 0;
        _route[no].route[i].old_dev = 0;
        
        //保存状态
        route_eeprom_update(); 
        return 0;
      }
    }
  }
  return 1;
}

/***************************************************
函数名称：   route_clear
输入参数：	
返 回 值:	 
功能描述：	  
***************************************************/
static uint8_t route_clear(uint8_t no)
{
  uint8_t i;
  if (no >= ROUTE_MAX_SIZE) return 1;
  if (_route[no].ready != 1) return 1;
  _route[no].ready = 0;
  
  for(i = 0; i < ROUTE_TABLE_NUM; i++)
  {
    memset(&_route[no].route[i],0,sizeof(RouteTable_t));
  }
	
  //保存状态
	route_table_update(no);
	
  //保存状态
//  route_eeprom_update();
  return 0;
}

/***************************************************
函数名称：   route_size
输入参数：	
返 回 值:	 
功能描述：	  
***************************************************/
static uint8_t route_size(uint8_t no)
{
  uint8_t i;
  uint8_t num = 0;
  
  for(i = 0; i < ROUTE_TABLE_NUM; i++)
  {
    if (_route[no].route[i].flag == 1)
    {
      num++;
    }
  }
  return (num);
}
/***************************************************
函数名称：   route_status
输入参数：	
返 回 值:	 
功能描述：	  
***************************************************/
static uint8_t route_status(void)
{
  uint8_t i;
  
  for(i = 0; i < ROUTE_MAX_SIZE; i++)
  {
    if (_route[i].enable == 1)
      return 1;
  }
  
  return (route_state);
}
/***************************************************
函数名称：   route_start
输入参数：	
返 回 值:	 
功能描述：	  
***************************************************/
static void route_start(void)
{
  uint8_t i;
  
  for(i = 0; i < ROUTE_MAX_SIZE; i++)
  {
    _route[i].enable = 1;
  }
  
  route_tick_backup = route_tick_count;
  _route[0].enable = 0;
  route_state = ROUTE_STATE_SEND;
  route_update_timeout = 0;   
  route_index = 0;  
}

/***************************************************
函数名称：   route_table_update
输入参数：	
返 回 值:	 
功能描述：	  
***************************************************/
static void route_table_init(uint8_t no)
{
  int i,j;
  uint32_t flash_addr;
	uint8_t buffer[K];

  flash_addr = ROUTE_TABLE_BASIC_ADDR + (uint32_t)no * K;
  eeprom_read(flash_addr,buffer,K);
	j = 0;
  for(i = 0; i < ROUTE_TABLE_NUM; i++)
  {
    if (buffer[j] == 0x01)
    {
			_route[no].route[i].update = 0;
      _route[no].route[i].flag = buffer[j];
      _route[no].route[i].old_net = buffer[j+1];
      _route[no].route[i].old_dev = buffer[j+2];
      _route[no].route[i].new_net = buffer[j+3];
      _route[no].route[i].new_dev = buffer[j+4];
      _route[no].route[i].type = buffer[j+5];
      memcpy(_route[no].route[i].mac,&buffer[j+6],8);
    }

    j += 16;
  } 
}

/***************************************************
函数名称：   route_table_update
输入参数：	
返 回 值:	 
功能描述：	  
***************************************************/
static void route_table_update(uint8_t no)
{
  int i,j;
  uint32_t flash_addr;
	uint8_t buffer[K];
	
  j = 0;
  flash_addr = ROUTE_TABLE_BASIC_ADDR + (uint32_t)no * K;
  for(i = 0; i < ROUTE_TABLE_NUM; i++)
  {
    buffer[j] = _route[no].route[i].flag;
    buffer[j+1] = _route[no].route[i].old_net;
    buffer[j+2] = _route[no].route[i].old_dev;
    buffer[j+3] = _route[no].route[i].new_net;
    buffer[j+4] = _route[no].route[i].new_dev;
    buffer[j+5] = _route[no].route[i].type;
    memcpy(&buffer[j+6],_route[no].route[i].mac,8);
    j += 16;
  }
	eeprom_write(flash_addr,buffer,K);
}

/***************************************************
函数名称：   route_read_table
输入参数：	
返 回 值:	 
功能描述：	  
***************************************************/
static uint8_t route_read_table(uint8_t no,uint8_t index,uint8_t *table)
{
  if (no >= ROUTE_MAX_SIZE) return 1;
  if (index >= ROUTE_TABLE_NUM) return 1;
  
  table[0] = _route[no].route[index].flag;
  table[1] = _route[no].route[index].new_net;
  table[2] = _route[no].route[index].new_dev;
  memcpy(&table[3],_route[no].route[index].mac,8);
  table[11] = _route[no].route[index].type;  
  return 0;
}
/***************************************************
函数名称：   route_write_table
输入参数：	
返 回 值:	 
功能描述：	  
***************************************************/
static uint8_t route_write_table(uint8_t no,uint8_t index,uint8_t *table)
{
  if (route_register(no,table[0],table[1],&table[3])) return 1;
  
  //保存状态
  route_eeprom_update();  
  return 0;
}
/***************************************************
函数名称：   route_modify_table
输入参数：	
返 回 值:	 
功能描述：	  
***************************************************/
static uint8_t route_modify_table(uint8_t no,uint8_t index,uint8_t *table)
{
  if (no >= ROUTE_MAX_SIZE) return 1;
  if (index >= ROUTE_TABLE_NUM) return 1;
  
  _route[no].route[index].flag = 1;
  _route[no].route[index].new_net = table[1];
  _route[no].route[index].new_dev = table[2];
  memcpy(_route[no].route[index].mac,&table[3],8);
  _route[no].route[index].type = table[11];  
  //保存状态
  route_eeprom_update();  
  return 0;
}
/***************************************************
函数名称：   route_delete_table
输入参数：	
返 回 值:	 
功能描述：	  
***************************************************/
static uint8_t route_delete_table(uint8_t no,uint8_t index)
{
  if (no >= ROUTE_MAX_SIZE) return 1;
  if (index >= ROUTE_TABLE_NUM) return 1;
  
  _route[no].route[index].flag = 0;
  
  //保存状态
  route_eeprom_update();  
  return 0;
}

/***************************************************
函数名称：   route_eeprom_update
输入参数：	
返 回 值:	 
功能描述：	  
***************************************************/
static void route_eeprom_update(void)
{
  if (route_state == ROUTE_STATE_IDLE)
  {
    route_update_eeprom_save = 1;
    route_update_eeprom = 2000;
    route_state = ROUTE_STATE_SAVE; 
  }
}

/***************************************************
函数名称：   route_update_cmd
输入参数：	
返 回 值:	 
功能描述：	  
***************************************************/
static void route_update_cmd(uint8_t no,RouteTable_t *route)
{
  uint8_t cmd_data[10];
  
  memcpy(cmd_data,route->mac,8);
  cmd_data[8] = route->new_net;
  cmd_data[9] = route->new_dev;
  
  system_socket_send(no,0X13F2,\
          route->old_net,route->old_dev,cmd_data,10);
}


/***************************************************
函数名称：   route_check_cmd
输入参数：	
返 回 值:	 
功能描述：	  
***************************************************/
static void route_check_cmd(uint8_t no)
{
  uint8_t cmd_data[4];
  
  cmd_data[0] = (uint8_t)(route_tick_backup >> 24);
  cmd_data[1] = (uint8_t)(route_tick_backup >> 16);
  cmd_data[2] = (uint8_t)(route_tick_backup >> 8);
  cmd_data[3] = (uint8_t)(route_tick_backup >> 0);
  
  system_socket_send(no,0X13F0,\
                        0xff,0xff,cmd_data,4);
}
/***************************************************
函数名称：   route_link_cmd
输入参数：	
返 回 值:	 
功能描述：   device->pc	  
***************************************************/
static void route_link_cmd(uint8_t no) 
{
  uint8_t cmd_data[4];
  
  cmd_data[0] = 0x03;  
  system_socket_send(SOCKET_LINE_NO+no,0X13F5,\
                              0xff,0xff,cmd_data,1);
  
}
/***************************************************
函数名称：   route_complete_cmd
输入参数：	
返 回 值:	 
功能描述：   device->pc		  
***************************************************/
static void route_complete_cmd(uint8_t no)
{
  uint8_t cmd_data[4];
  
  cmd_data[0] = 0x04;
  cmd_data[1] = route_size(no);
  cmd_data[2] = 1;
  cmd_data[3] = cmd_data[1];
  system_socket_send(SOCKET_LINE_NO+no,0X13F5,\
                              0xff,0xff,cmd_data,4); 
}

/***************************************************
函数名称：   route_init
输入参数：	
返 回 值:	 
功能描述：	  
***************************************************/
static void route_init(void)
{
  uint8_t i;
  
  route_subnet = devices->subnet;
  
  for (i = 0; i < ROUTE_MAX_SIZE; i++)
	{
    route_table_init(i);
	}   
}

/***************************************************
函数名称：   route_handler
输入参数：	
返 回 值:	 
功能描述：	  
***************************************************/
static void route_handler(void) //10ms
{ 
  uint8_t i;
  
  switch (route_state)
  {
    //检测更新
    case ROUTE_STATE_IDLE:
      route_index++;
      if (route_index >= ROUTE_MAX_SIZE)
        route_index = 0;
      if (_route[route_index].enable == 1)
      {
        route_tick_backup = route_tick_count;
        _route[route_index].enable = 0;
        route_state = ROUTE_STATE_SEND;
        route_update_timeout = 0;
        route_update_eeprom_save = 0;
      }
      break;
    
    //发送命令
    case ROUTE_STATE_SEND:
      //发送查找命令
      route_check_cmd(route_index);
      route_timeout = 0;
      route_state = ROUTE_STATE_WAIT;
      break;
     
    //等待
    case ROUTE_STATE_WAIT:
      route_timeout++;
      if (route_timeout > 200)//2s
      {
        route_timeout = 0;
        route_state = ROUTE_STATE_RECV;
      }
      break;
      
    //接收命令
    case ROUTE_STATE_RECV:
      for(i = 0; i < ROUTE_TABLE_NUM; i++)
      {
        if (_route[route_index].route[i].flag == 1
        && _route[route_index].route[i].update == 1)
        {
          _route[route_index].route[i].update = 0;
          route_update_timeout = 0;
          
          route_update_eeprom_save = 1;
          
          //发送设置
          route_update_cmd(route_index,&(_route[route_index].route[i]));
          break;
        }
      }
      
      //
      route_update_timeout++;
      if (route_update_timeout > 20)
      {
        //发送完毕
        route_complete_cmd(route_index);
        route_update_timeout = 0;
        route_update_eeprom = 0;
        route_state = ROUTE_STATE_SAVE;    
      }
      else 
      {
        route_link_cmd(route_index);//send - >pc
        route_state = ROUTE_STATE_SEND;
      }
      break;
    
    case ROUTE_STATE_SAVE:
      //保存数据
      if(route_update_eeprom == 0)
      {
        if (route_update_eeprom_save)
        {
          route_table_update(route_index);
          route_update_eeprom_save = 0;
        }
        route_state = ROUTE_STATE_IDLE; 
      }
      else
      {
        route_update_eeprom--;
      }
      break;

    default:
      route_state = ROUTE_STATE_IDLE;
      break;
  }
}


/***************************************************
函数名称：   route_event_handler
输入参数：	
返 回 值:	 
功能描述：	  
***************************************************/
int route_events(unsigned char *buffer,int len)
{
	if (buffer[0] == 1)//开始
	{
		route_ready(0,&buffer[1]);
		return 5;
	}
	else if (buffer[0] == 2)//确定
	{
		if (route_enable(0,&buffer[1]))
			buffer[0] = 0xf5;
		else
			buffer[0] = buffer[1];
		
		return 1;
	}
	else if (buffer[0] == 0x11)//读取设备列表开始
	{
		route_ready(0,&buffer[1]);
		return 5;
	}
	else if (buffer[0] == 0x12)//确定
	{
		if (route_getlist(0,&buffer[1]))
			buffer[0] = 0xf5;
		return 1;	
	}
	else if (buffer[0] == 0x81)////删除
	{
		route_ready(0,&buffer[1]);
		return 5;
	}
	else if (buffer[0] == 0x82)////确定
	{
			if (route_clear(0))
				buffer[0] = 0xf5;	
			return 1;
	}
	else if (buffer[0] == 0xa1)//读取路由表
	{
		if(route_read_table(0,buffer[1],&buffer[2]))
		{
			buffer[0] = 0xf5;
			return 5;
		}
		return 14;
	}
	else if (buffer[0] == 0xa2)//增加路由表
	{
		if (route_write_table(0,0,&buffer[1]))
			buffer[0] = 0xf5;
		else
			buffer[0] = 0xf8;
		return 1;		
	}
	else if (buffer[0] == 0xa3)//修改路由表
	{
		if (route_modify_table(0,buffer[1],&buffer[2]))
			buffer[0] = 0xf5;
		else
			buffer[0] = 0xf8;
			
		return 1;
	}
	else if (buffer[0] == 0xa4)//删掉
	{		
		if (route_delete_table(0,buffer[1]))
			buffer[0] = 0xf5;
		else
			buffer[0] = 0xf8;
		return 1;
	}
	else
	{
		buffer[0] = 0xf5;
		return 1;
	}
}



//=======================Standard define API===============================
static int _get(unsigned char *config,int len)
{
	
	return 0;
}


static int _set(unsigned char *config,int len)
{
	
	route_register(config[8],config[9],config[10],&config[0]);
	
	return 0;
}

static void _init(void)
{
	route_init();
	
}

static int _status(unsigned char *buffer,int len)
{
	
	return 0;
}


static int _read(unsigned char *buffer,int len)
{
	
	return 0;
}


static int _write(unsigned char *buffer,int len)
{
	return route_events(buffer,len);
}


static int _handler(void *event)
{
	
	route_handler();
	
	return 0;
}

//====================app interface=====================
const struct app_interface_t route_interface =
{
	_get, _set,_init, _status,_read, _write, _handler
};


#endif
