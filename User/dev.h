#ifndef __DEV_H
#define __DEV_H

        
#include "key.h"
#include "led.h"
#include "beep.h"
#include "delay.h"
#include "extix.h"
#include "bitBand.h"
#include "usart1.h"
//#include "baseTime.h"
#include "iwdg.h"

#ifdef __cplusplus
    extern "C" {
#endif
        
//GPIO_Init()库函数的二次封装。
//调用前必须使所用的GPIO外设时钟总线使能。
#ifdef __cplusplus
void My_GPIO_Init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIOMode_TypeDef GPIO_Mode, GPIOSpeed_TypeDef GPIO_Speed = GPIO_Speed_10MHz);
void My_USART_Init(USART_TypeDef* USARTx, u32 unBaudRate, u16 usWordLength, u16 usStopBits, u16 usParity, u16 usMode, u16 usHardwareFlowControl = USART_HardwareFlowControl_None);        
void My_NVIC_Init(u8 ucIRQChannel, u8 ucPrePriority, u8 ucSubPriority, FunctionalState NVIC_IRQChannelCmd);       
#else
void My_GPIO_Init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIOMode_TypeDef GPIO_Mode, GPIOSpeed_TypeDef GPIO_Speed);
void My_USART_Init(USART_TypeDef* USARTx, u32 unBaudRate, u16 usWordLength, u16 usStopBits, u16 usParity, u16 usMode, u16 usHardwareFlowControl);
#endif   

        

#ifdef __cplusplus
}
#endif

//对于输入模式的GPIO应用，频率是无用的，但C不支持默认参数，这里给出宏定义来实现默认参数，这样输入模式下可以不用写频率参数。
#define My_GPIO_Init_Macro(GPIOx, GPIO_Pin, GPIO_Mode)    My_GPIO_Init(GPIOx, GPIO_Pin, GPIO_Mode,  GPIO_Speed_10MHz)

#endif
