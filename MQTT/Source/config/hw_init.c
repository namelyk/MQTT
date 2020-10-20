/***************************************************
模块名称：  hw init
功能描述：	
设 计 者:	  薛柯利(steven sit)  
编写时间：	2012-05-14
***************************************************/
#include "hw_init.h"

#define WATCH_DOG_EN ///need restore

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

// OUT	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 |GPIO_Pin_4 |GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_SetBits(GPIOA,GPIO_Pin_3);
	GPIO_SetBits(GPIOA,GPIO_Pin_4);
	GPIO_ResetBits(GPIOA,GPIO_Pin_12);
	GPIO_Init(GPIOA, &GPIO_InitStructure);	

//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_6|GPIO_Pin_7;
//	GPIO_Init(GPIOB, &GPIO_InitStructure);

//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8\
//								|GPIO_Pin_9;
//	GPIO_Init(GPIOC, &GPIO_InitStructure);
//	GPIO_ResetBits(GPIOC,GPIO_Pin_9);
//	
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4| GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_13|GPIO_Pin_14;
//	GPIO_ResetBits(GPIOD,GPIO_Pin_4);
//	GPIO_Init(GPIOD, &GPIO_InitStructure);	

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_14|GPIO_Pin_15;

	GPIO_SetBits(GPIOE, GPIO_Pin_0|GPIO_Pin_12|GPIO_Pin_14|GPIO_Pin_15);
	GPIO_Init(GPIOE, &GPIO_InitStructure);	


//IN
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10| GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
//	GPIO_Init(GPIOB, &GPIO_InitStructure);

//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11|GPIO_Pin_5;
//	GPIO_Init(GPIOC, &GPIO_InitStructure);

//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
//	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_Init(GPIOE, &GPIO_InitStructure);



//AF
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
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
//	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn ;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&NVIC_InitStructure);
//	
//	NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn ;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure); 
	
//	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&NVIC_InitStructure); 
//	
//	NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&NVIC_InitStructure); 
	
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
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO, ENABLE);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource11);
	EXTI_InitStructure.EXTI_Line =  EXTI_Line11;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure); 
	EXTI_GenerateSWInterrupt(EXTI_Line11);  

#if 1	
//	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource14);
//	EXTI_InitStructure.EXTI_Line =  EXTI_Line14;
//	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
//	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  
//	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//	EXTI_Init(&EXTI_InitStructure); 
//	EXTI_GenerateSWInterrupt(EXTI_Line14); 
//	
//	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource0);
//	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource1);
//	EXTI_InitStructure.EXTI_Line =  EXTI_Line0;
//	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
//	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  
//	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//	EXTI_Init(&EXTI_InitStructure); 
//	EXTI_GenerateSWInterrupt(EXTI_Line0);  
//	
//	EXTI_InitStructure.EXTI_Line =  EXTI_Line1;
//	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
//	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  
//	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//	EXTI_Init(&EXTI_InitStructure); 
//	EXTI_GenerateSWInterrupt(EXTI_Line1);  
#endif
}

/***************************************************
函数名称：	SysTick_Configuration
输入参数：	
返 回 值:	
功能描述：	定时1ms中断
***************************************************/
void SysTick_Configuration(void)
{

	if (SysTick_Config(SystemFrequency / 1000))
	{
		while(1);
	}
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


void USART_115200_Configuration(void)
{
	USART_InitTypeDef USART_InitStructure;
  
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	USART_DeInit(USART1);
	
	USART_InitStructure.USART_BaudRate = 115200;
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
函数名称：	UART4_Configuration
输入参数：	
返 回 值:	
功能描述：	
***************************************************/
void UART4_Configuration(int BaudRate,int WordLength,int stopbit,int Parity)
{
	USART_InitTypeDef USART_InitStructure;
  
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);

	USART_DeInit(UART4);
	
	switch (BaudRate)
	{
		case 1200:	USART_InitStructure.USART_BaudRate = 1200;break;
		case 2400:	USART_InitStructure.USART_BaudRate = 2400;break;	
		case 4800:	USART_InitStructure.USART_BaudRate = 4800;break;	
		case 9600:	USART_InitStructure.USART_BaudRate = 9600;break;	
		case 19200:	USART_InitStructure.USART_BaudRate = 19200;break;	
		case 38400:	USART_InitStructure.USART_BaudRate = 38400;break;	
		case 56000:	USART_InitStructure.USART_BaudRate = 56000;break;	
		case 57600:	USART_InitStructure.USART_BaudRate = 57600;break;	
		case 115200:	USART_InitStructure.USART_BaudRate = 115200;break;	
		case 250000:	USART_InitStructure.USART_BaudRate = 250000;break;	
		default:
			USART_InitStructure.USART_BaudRate = 9600;break;	
	}
	
	if (WordLength == 9)
		USART_InitStructure.USART_WordLength = USART_WordLength_9b;
	else
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	
	switch (stopbit)
	{
		case 0:	USART_InitStructure.USART_StopBits = USART_StopBits_0_5;break;
		case 1:	USART_InitStructure.USART_StopBits = USART_StopBits_1;break;
		case 2:	USART_InitStructure.USART_StopBits = USART_StopBits_2;break;
		case 3:	USART_InitStructure.USART_StopBits = USART_StopBits_1_5;break;
		default:
			USART_InitStructure.USART_StopBits = USART_StopBits_1;break;
	}

	switch (Parity)
	{
		case 0:	USART_InitStructure.USART_Parity = USART_Parity_No;break;
		case 1:	USART_InitStructure.USART_Parity = USART_Parity_Even;break;
		case 2:	USART_InitStructure.USART_Parity = USART_Parity_Odd;break;
		default:
			USART_InitStructure.USART_Parity = USART_Parity_No;break;
	}
	
	
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(UART4, &USART_InitStructure);
	USART_GetITStatus(UART4, USART_IT_TC); 
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
	USART_Cmd(UART4, ENABLE);   
}
/***************************************************
函数名称：	UART5_Configuration
输入参数：	
返 回 值:	
功能描述：	
***************************************************/
void UART5_Configuration(int BaudRate,int WordLength,int stopbit,int Parity)
{
	USART_InitTypeDef USART_InitStructure;
  
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);

	USART_DeInit(UART5);
	
	switch (BaudRate)
	{
		case 1200:	USART_InitStructure.USART_BaudRate = 1200;break;
		case 2400:	USART_InitStructure.USART_BaudRate = 2400;break;	
		case 4800:	USART_InitStructure.USART_BaudRate = 4800;break;	
		case 9600:	USART_InitStructure.USART_BaudRate = 9600;break;	
		case 19200:	USART_InitStructure.USART_BaudRate = 19200;break;	
		case 38400:	USART_InitStructure.USART_BaudRate = 38400;break;	
		case 56000:	USART_InitStructure.USART_BaudRate = 56000;break;	
		case 57600:	USART_InitStructure.USART_BaudRate = 57600;break;	
		case 115200:	USART_InitStructure.USART_BaudRate = 115200;break;	
		case 250000:	USART_InitStructure.USART_BaudRate = 250000;break;	
		default:
			USART_InitStructure.USART_BaudRate = 9600;break;	
	}
	
	if (WordLength == 9)
		USART_InitStructure.USART_WordLength = USART_WordLength_9b;
	else
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	
	switch (stopbit)
	{
		case 0:	USART_InitStructure.USART_StopBits = USART_StopBits_0_5;break;
		case 1:	USART_InitStructure.USART_StopBits = USART_StopBits_1;break;
		case 2:	USART_InitStructure.USART_StopBits = USART_StopBits_2;break;
		case 3:	USART_InitStructure.USART_StopBits = USART_StopBits_1_5;break;
		default:
			USART_InitStructure.USART_StopBits = USART_StopBits_1;break;
	}

	switch (Parity)
	{
		case 0:	USART_InitStructure.USART_Parity = USART_Parity_No;break;
		case 1:	USART_InitStructure.USART_Parity = USART_Parity_Even;break;
		case 2:	USART_InitStructure.USART_Parity = USART_Parity_Odd;break;
		default:
			USART_InitStructure.USART_Parity = USART_Parity_No;break;
	}
	
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(UART5, &USART_InitStructure);
	USART_GetITStatus(UART5, USART_IT_TC); 
	USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);
	USART_Cmd(UART5, ENABLE);   
}
/***************************************************
函数名称：	SPI1_Configuration
输入参数：	
返 回 值:	
功能描述：	
***************************************************/
void SPI1_Configuration(void)
{
	
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;


	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 | RCC_APB2Periph_GPIOA , ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA, GPIO_Pin_4);
	
	

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;	
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;	

	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;		
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;	
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;	
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	
	SPI_InitStructure.SPI_CRCPolynomial = 7;			
	SPI_Init(SPI1, &SPI_InitStructure);
	
	SPI_Cmd(SPI1, ENABLE);
	
	GPIO_SetBits(GPIOA, GPIO_Pin_4);								

}

/***************************************************
函数名称：	SPI1_SendByte
输入参数：	
返 回 值:	
功能描述：	
***************************************************/

unsigned char SPI1_SendByte(unsigned char value)
{
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
	
	SPI_I2S_SendData(SPI1, value);
	
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

	return SPI_I2S_ReceiveData(SPI1);
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
#ifdef WATCH_DOG_EN
	EXIT_DOG_PIN_HIGH();
	
	IWDG_ReloadCounter(); 
	
 	EXIT_DOG_PIN_LOW();
#endif
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

	EXTI_Configuration();
	
	NVIC_Configuration();
	
//	SPI1_Configuration();
	
	WDT_Configuration();
	SysTick_Configuration();
}



