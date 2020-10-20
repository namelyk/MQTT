#include "driver.h"


#ifdef DRIVER_ENABLE	

/***************************************************
函数名称：	void driver_init(void)
输入参数：	
返 回 值:	
功能描述：	
***************************************************/
void driver_init(void)
{
	eeprom_init();
	
#ifdef OBJECTS_ENABLE
	objects_init();
#endif	
	
#ifdef SCENE_ENABLE	
	scene_interface.init();
#endif
	
	
#ifdef RS485_ENABLE
	rs485_interface.init();
#endif
	

	
#ifdef AIR_CONDITIONER_ENABLE	
	air_conditioner_interface.init();
#endif
#ifdef CURTAIN_ENABLE		
	curtain_interface.init();
#endif

#ifdef LED_ENABLE		
	led_interface.init();
#endif
	
#ifdef DIMMER_ENABLE
	dimmer_interface.init();
#endif
	
#ifdef RELAY_ENABLE	
	relay_interface.init();
#endif	


#ifdef TEMPERATURE_ENABLE
	temperature_interface.init();
#endif

#ifdef DATETIME_ENABLE
	datetime_interface.init();
#endif
}



/***************************************************
函数名称：	void driver_handler(void)
输入参数：	
返 回 值:	
功能描述：	
***************************************************/
void driver_handler(void)
{
	eeprom_handler();

#ifdef OBJECTS_ENABLE
	objects_handler();
#endif
	
#ifdef RS485_ENABLE
	rs485_interface.handler((void *)NULL);
#endif
	
#ifdef SCENE_ENABLE
	scene_interface.handler((void *)NULL);
#endif
	
	
#ifdef AIR_CONDITIONER_ENABLE	
	air_conditioner_interface.handler((void *)NULL);
#endif
	
#ifdef CURTAIN_ENABLE	
	curtain_interface.handler((void *)NULL);
#endif 
	
#ifdef LED_ENABLE	
	led_interface.handler((void *)NULL);
#endif
	
#ifdef DIMMER_ENABLE
	dimmer_interface.handler((void *)NULL);
#endif

#ifdef RELAY_ENABLE	
	relay_interface.handler((void *)NULL);
#endif


#ifdef TEMPERATURE_ENABLE
	temperature_interface.handler((void *)NULL);
#endif

#ifdef DATETIME_ENABLE
	datetime_interface.handler((void *)NULL);
#endif

}

#endif
