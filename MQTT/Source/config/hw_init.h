#ifndef _HW_INIT_H_
#define _HW_INIT_H_

#include "stm32f10x.h"
#include "stm32_eth.h"

//中断
#define DISABLE_INTERRUPT() 		__set_PRIMASK(1)
#define ENABLE_INTERRUPT() 			__set_PRIMASK(0)


//外部看门狗   		  
#define EXIT_DOG_PIN_HIGH()					{GPIOE->BSRR = GPIO_Pin_12;}
#define EXIT_DOG_PIN_LOW()					{GPIOE->BRR = GPIO_Pin_12;}


//LED 指示灯	 
#define LED_GREEN_ON()	 						{GPIOE->BRR = GPIO_Pin_14;}
#define LED_GREEN_OFF()	 						{GPIOE->BSRR = GPIO_Pin_14;}
		
#define LED_RED_ON()	 							{GPIOE->BRR = GPIO_Pin_15;}
#define LED_RED_OFF()	 							{GPIOE->BSRR = GPIO_Pin_15;}

//上面的指示灯
#define LED_UP_ON()									{GPIOE->BRR = GPIO_Pin_0;}
#define LED_UP_OFF()								{GPIOE->BSRR = GPIO_Pin_0;}

//#define LED_UP_FLASH()								{LED_UP_ON(); LED_UP_OFF();

#define LED_RED_TRIGER()			

//按键NET DEVICE ID
#define KEY_EVENT_VALUE()						(!GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_13))

// IO VALUE
#define SET_IO_INPUT()							{GPIOE->CRL = 0X88888888;}//GPIOE->CRL&=0X0FFFFFFF;GPIOE->CRL|=8<<28; 
#define SET_IO_OUTPUT()							{GPIOE->CRL = 0X33333333;}
#define GET_IO_VALUE()							(GPIOE->IDR & 0X00FF)
#define SET_IO_VALUE(X)							{GPIOE->ODR &= 0XFF00;GPIOE->ODR |= X;}


// SPI
#define SPI_CS_HIGH()								{GPIOA->BSRR = GPIO_Pin_4;}
#define SPI_CS_LOW()								{GPIOA->BRR = GPIO_Pin_4;}
#define SPI_SEND(X)									SPI1_SendByte(X)

// IIC
#define IIC_SCL_HIGH()							{GPIOB->BSRR = GPIO_Pin_6;}
#define IIC_SCL_LOW()								{GPIOB->BRR = GPIO_Pin_6;}
#define IIC_SDA_HIGH()							{GPIOB->BSRR = GPIO_Pin_7;}
#define IIC_SDA_LOW()								{GPIOB->BRR = GPIO_Pin_7;}

#define T_IIC_SCL_HIGH()							{GPIOD->BSRR = GPIO_Pin_6;}
#define T_IIC_SCL_LOW()								{GPIOD->BRR = GPIO_Pin_6;}
#define T_IIC_SDA_HIGH()							{GPIOD->BSRR = GPIO_Pin_5;}
#define T_IIC_SDA_LOW()								{GPIOD->BRR = GPIO_Pin_5;}



// 继电器
#define RELAY1_LOCK_HIGH()								{GPIOE->BSRR = GPIO_Pin_8;}
#define RELAY1_LOCK_LOW()									{GPIOE->BRR = GPIO_Pin_8;}
#define RELAY2_LOCK_HIGH()								{GPIOE->BSRR = GPIO_Pin_9;}
#define RELAY2_LOCK_LOW()									{GPIOE->BRR = GPIO_Pin_9;}
#define RELAY3_LOCK_HIGH()								{GPIOC->BSRR = GPIO_Pin_0;}
#define RELAY3_LOCK_LOW()									{GPIOC->BRR = GPIO_Pin_0;}

// led
#define RELAY4_LOCK_HIGH()								{GPIOE->BSRR = GPIO_Pin_10;}
#define RELAY4_LOCK_LOW()									{GPIOE->BRR = GPIO_Pin_10;}
#define RELAY5_LOCK_HIGH()								{GPIOE->BSRR = GPIO_Pin_11;}
#define RELAY5_LOCK_LOW()									{GPIOE->BRR = GPIO_Pin_11;}

// 干接点
#define KEY1_LOCK_HIGH()									{GPIOD->BSRR = GPIO_Pin_13;}
#define KEY1_LOCK_LOW()										{GPIOD->BRR = GPIO_Pin_13;}
#define KEY2_LOCK_HIGH()									{GPIOD->BSRR = GPIO_Pin_14;}
#define KEY2_LOCK_LOW()										{GPIOD->BRR = GPIO_Pin_14;}
#define KEY3_LOCK_HIGH()									{GPIOD->BSRR = GPIO_Pin_15;}
#define KEY3_LOCK_LOW()										{GPIOD->BRR = GPIO_Pin_15;}
#define KEY4_LOCK_HIGH()									{GPIOC->BSRR = GPIO_Pin_8;}
#define KEY4_LOCK_LOW()										{GPIOC->BRR = GPIO_Pin_8;}


//可控硅控制
#define DIMMER_SCR_1_HIGH()						{GPIOB->BSRR = GPIO_Pin_0;}
#define DIMMER_SCR_1_LOW()						{GPIOB->BRR = GPIO_Pin_0;}

#define DIMMER_SCR_2_HIGH()						{GPIOB->BSRR = GPIO_Pin_1;}
#define DIMMER_SCR_2_LOW()						{GPIOB->BRR = GPIO_Pin_1;}

#define DIMMER_SCR_3_HIGH()						{GPIOC->BSRR = GPIO_Pin_6;}
#define DIMMER_SCR_3_LOW()						{GPIOC->BRR = GPIO_Pin_6;}

#define DIMMER_SCR_4_HIGH()						{GPIOC->BSRR = GPIO_Pin_7;}
#define DIMMER_SCR_4_LOW()						{GPIOC->BRR = GPIO_Pin_7;}


// RS485 EXTI
#define RS485_EXTI_ENABLE()							{GPIOC->BSRR = GPIO_Pin_9;}
#define RS485_EXTI_DISABLE()						{GPIOC->BRR = GPIO_Pin_9;}
#define RS485_EXTI_TX_ISR_ENABLE()			USART_ITConfig(UART4, USART_IT_TC, ENABLE)
#define RS485_EXTI_TX_ISR_DISABLE()			USART_ITConfig(UART4, USART_IT_TC, DISABLE)
#define RS485_EXTI_TX_SEND(x)			 			USART_SendData(UART4, x)


// RS485 IN
#define RS485_IN_ENABLE()							{GPIOD->BSRR = GPIO_Pin_4;}
#define RS485_IN_DISABLE()						{GPIOD->BRR = GPIO_Pin_4;}
#define RS485_IN_TX_ISR_ENABLE()			USART_ITConfig(UART5, USART_IT_TC, ENABLE)
#define RS485_IN_TX_ISR_DISABLE()			USART_ITConfig(UART5, USART_IT_TC, DISABLE)
#define RS485_IN_TX_SEND(x)			 			USART_SendData(UART5, x)

//===========================BUSPRO=====================================
//BUSPRO使能       		
#define BUSPRO_TX_ENABLE()  	 			{GPIOA->BSRR = GPIO_Pin_12;}//GPIO_SetBits(GPIOA,GPIO_Pin_12)
#define BUSPRO_TX_DISABLE() 				{GPIOA->BRR = GPIO_Pin_12;}//GPIO_ResetBits(GPIOA,GPIO_Pin_12)


//串口发送中断使能
#define BUSPRO_TX_ISR_ENABLE()			USART_ITConfig(USART1, USART_IT_TC, ENABLE)
#define BUSPRO_TX_ISR_DISABLE()			USART_ITConfig(USART1, USART_IT_TC, DISABLE)
#define BUSPRO_TX_SEND(x)			 			USART_SendData(USART1, x)




/***************************************************
***************************************************/
extern void RCC_Configuration(void);
extern void hw_init(void);
extern void RESET_WATCH_DOG(void);

extern void USART_Configuration(void);
extern void USART_115200_Configuration(void);
extern void UART4_Configuration(int BaudRate,int WordLength,int stopbit,int Parity);
extern void UART5_Configuration(int BaudRate,int WordLength,int stopbit,int Parity);
extern unsigned char SPI1_SendByte(unsigned char value);

#endif

