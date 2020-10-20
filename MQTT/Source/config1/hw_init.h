#ifndef _HW_INIT_H_
#define _HW_INIT_H_

#include "stm32f10x.h"

//中断
#define DISABLE_INTERRUPT() 		__set_PRIMASK(1)
#define ENABLE_INTERRUPT() 			__set_PRIMASK(0)


//外部看门狗   		  
#define EXIT_DOG_PIN_HIGH()					{GPIOE->BSRR = GPIO_Pin_12;}
#define EXIT_DOG_PIN_LOW()					{GPIOE->BRR = GPIO_Pin_12;}


//LED 指示灯	 
#define LED_GREEN_ON()	 						GPIO_ResetBits(GPIOE,GPIO_Pin_14)
#define LED_GREEN_OFF()	 						GPIO_SetBits(GPIOE,GPIO_Pin_14)
		
#define LED_RED_ON()	 							GPIO_ResetBits(GPIOE,GPIO_Pin_15)
#define LED_RED_OFF()	 							GPIO_SetBits(GPIOE,GPIO_Pin_15)
#define LED_RED_TRIGER()			



//网络LED指示 	
#define WORK_LED_ON()	 		
#define WORK_LED_OFF()	 	


//按键NET DEVICE ID
#define KEY_EVENT_VALUE()						GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_13)


//===========================BUSPRO=====================================
//BUSPRO使能       		
#define BUSPRO_TX_ENABLE()  	 			GPIO_SetBits(GPIOA,GPIO_Pin_12)
#define BUSPRO_TX_DISABLE() 				GPIO_ResetBits(GPIOA,GPIO_Pin_12)


//串口发送中断使能
#define BUSPRO_TX_ISR_ENABLE()			USART_ITConfig(USART1, USART_IT_TC, ENABLE)
#define BUSPRO_TX_ISR_DISABLE()			USART_ITConfig(USART1, USART_IT_TC, DISABLE)
#define BUSPRO_TX_SEND(x)			 			USART_SendData(USART1, x)




/***************************************************
***************************************************/
extern void RCC_Configuration(void);
extern void hw_init(void);
extern void RESET_WATCH_DOG(void);



#endif

