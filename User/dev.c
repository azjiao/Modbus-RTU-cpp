/*
功能： 对库函数的二次封装。
描述： 封装GPIO_Init()函数为My_GPIO_Init()函数，初始化化结构放在函数内来实现，只需传入初始化数据不需要传入初始化结构。使外部调用初始化变得简单。
设计： azjiao
版本： 0.1
日期： 2018年09月06日
*/
#include <stm32f10x.h>
#include "dev.h"

//GPIO_Init()库函数的二次封装。
//调用前必须使所用的GPIO外设时钟总线使能。
void My_GPIO_Init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIOMode_TypeDef GPIO_Mode, GPIOSpeed_TypeDef GPIO_Speed)
{
    GPIO_InitTypeDef GPIO_InitStruct; //声明初始化数据结构。
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed;
    GPIO_Init(GPIOx, &GPIO_InitStruct);
}

//USART_Init()库函数的二次封装。
//调用前必须使所用的USARTx外设时钟总线使能。
void My_USART_Init(USART_TypeDef* USARTx, u32 unBaudRate, u16 usWordLength, u16 usStopBits, u16 usParity, u16 usMode, u16 usHardwareFlowControl)
{
    USART_InitTypeDef  USART_InitStruct;  //声明初始化数据结构。
    
    USART_InitStruct.USART_HardwareFlowControl = usHardwareFlowControl;
    USART_InitStruct.USART_BaudRate = unBaudRate;
    USART_InitStruct.USART_Mode = usMode;
    USART_InitStruct.USART_WordLength = usWordLength;
    USART_InitStruct.USART_StopBits = usStopBits; 
    USART_InitStruct.USART_Parity = usParity;
    
    USART_Init(USART2, &USART_InitStruct);
}

//NVIC_Init()库函数的二次封装。
void My_NVIC_Init(u8 ucIRQChannel, u8 ucPrePriority, u8 ucSubPriority, FunctionalState NVIC_IRQChannelCmd)
{
    NVIC_InitTypeDef  NVIC_InitStruct;  //声明初始化数据结构。
    
    //配置中断优先级
    NVIC_InitStruct.NVIC_IRQChannel = ucIRQChannel;  //待配置优先级的中断.
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = ucPrePriority;  //抢占优先级
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = ucSubPriority;  //响应优先级
    NVIC_InitStruct.NVIC_IRQChannelCmd = NVIC_IRQChannelCmd;  //使能或失能.
    NVIC_Init(&NVIC_InitStruct);
}
