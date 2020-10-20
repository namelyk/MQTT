/***************************************************
模块名称：  hw init
功能描述：	
设 计 者:	  薛柯利(steven sit)  
编写时间：	2012-05-14
***************************************************/
#include "hw_init.h"

#define WATCH_DOG_EN

/***************************************************
函数名称：	GPIO_Configuration
输入参数：	
返 回 值:	
功能描述：	
***************************************************/
void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB\
						| RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD\
						| RCC_APB2Periph_GPIOE ,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	   
			
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 |GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_SetBits(GPIOA,GPIO_Pin_3);
	GPIO_ResetBits(GPIOA,GPIO_Pin_12);
	GPIO_Init(GPIOA, &GPIO_InitStructure);	

	

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_ResetBits(GPIOD,GPIO_Pin_4);
	GPIO_Init(GPIOD, &GPIO_InitStructure);	

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_SetBits(GPIOE,GPIO_Pin_0);
	GPIO_Init(GPIOE, &GPIO_InitStructure);	

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|\
								GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|\
								GPIO_Pin_12|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_SetBits(GPIOE,GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_14|GPIO_Pin_15);
	GPIO_Init(GPIOE, &GPIO_InitStructure);	


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10| GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11|GPIO_Pin_5;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2| GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_13;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10|GPIO_Pin_12;;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
}

/***************************************************
函数名称：	void NVIC_Configuration(void)
输入参数：	
返 回 值:	
功能描述：	
***************************************************/
void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure); 
	
}

/***************************************************
函数名称： EXTI_Configuration
输入参数：	
返 回 值:	
功能描述：	
***************************************************/
void EXTI_Configuration(void)
{

	EXTI_InitTypeDef EXTI_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource11);

	EXTI_InitStructure.EXTI_Line =  EXTI_Line11;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure); 

	EXTI_GenerateSWInterrupt(EXTI_Line11);  
}

/***************************************************
函数名称：	SysTick_Configuration
输入参数：	
返 回 值:	
功能描述：	定时1ms中断
***************************************************/
void SysTick_Configuration(void)
{
	SysTick_Config(SystemFrequency / 1000);
}
/***************************************************
函数名称：	USART_Configuration
输入参数：	
返 回 值:	
功能描述：	初始化串口硬件设备，启用中断。
***************************************************/
void USART_Configuration(void)
{
	USART_InitTypeDef USART_InitStructure;
  
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	USART_DeInit(USART1);
	
	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_9b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_Even;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART1, &USART_InitStructure);
	USART_GetITStatus(USART1, USART_IT_TC); 
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART1, ENABLE);   
}


/***************************************************
函数名称：	WDT_Configuration
输入参数：	
返 回 值:	
功能描述：	
***************************************************/
void WDT_Configuration(void)
{
#ifdef WATCH_DOG_EN
	if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET)
		RCC_ClearFlag();
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	IWDG_SetPrescaler(IWDG_Prescaler_256); //?6.4 MS
 	IWDG_SetReload(300); //6.4 * 300 = 1920 MS 
	IWDG_ReloadCounter(); //?1920 MS ??????? ???????.
	IWDG_Enable();
#endif
}


/***************************************************
函数名称：	RESET_WATCH_DOG
输入参数：	
返 回 值:	
功能描述：	
***************************************************/
void RESET_WATCH_DOG(void)
{

	EXIT_DOG_PIN_HIGH();
#ifdef WATCH_DOG_EN	
	IWDG_ReloadCounter(); 
#endif	
 	EXIT_DOG_PIN_LOW();

}

/***************************************************
函数名称：	hw_init
输入参数：	
返 回 值:	
功能描述：	初始化硬件
***************************************************/
void hw_init(void)
{
	
	GPIO_Configuration();

	USART_Configuration();

	SysTick_Configuration();

	EXTI_Configuration();
	
	NVIC_Configuration();
	
	WDT_Configuration();
	
}



