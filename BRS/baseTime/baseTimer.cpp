/*********************************************************
功能： baseTimer的实现
描述： STM32F10x有两个基本定时器:TIM6和TIM7。
设计： azjiao
版本： 0.1
日期： 2018年10月09日
*********************************************************/
#include "baseTimer.h"
#include "dev.h"

void BaseTimer::timer_Init(bool bSwitch)
{
    TIM_TypeDef * TIMx = (ucBaseTimerNo==6)? TIM6: TIM7;       

    TIM_TimeBaseInitTypeDef  TIM_InitStrut; //定时器初始化参数结构。                
    u32 RCC_APB1Periph_TIMx = (ucBaseTimerNo==6)? RCC_APB1Periph_TIM6: RCC_APB1Periph_TIM7;
            
    //定时器总线时钟使能.       
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIMx, ENABLE);      
     
    TIM_InitStrut.TIM_Period = usARR; // 自动装载寄存器值.
    TIM_InitStrut.TIM_Prescaler = usPSC; //定时器时钟预分频值，作为计数器时钟用。 
    TIM_InitStrut.TIM_CounterMode = TIM_CounterMode_Up;    //向上计数模式。
    // TIMx是基本定时器，只能使用向上计数模式，不用设置。
    TIM_TimeBaseInit(TIMx, &TIM_InitStrut);  
    //清除更新中断标识UIF,该标识由TIM_TimeBaseInit()函数中的UG设置自动设置。
    TIM_ClearITPendingBit(TIMx, TIM_IT_Update);
    
    //设置在定时器更新中断只能由计数器溢出产生。
    TIM_UpdateRequestConfig(TIMx, TIM_UpdateSource_Regular);
    
    //使能TIMx中断，允许中断更新。
    TIM_ITConfig(TIMx, TIM_IT_Update, ENABLE);
       
    //设置中断优先级. 
    int TIMx_IRQn = (TIMx == TIM6? TIM6_IRQn: TIM7_IRQn);
    My_NVIC_Init(TIMx_IRQn, ucPrePriority, ucSubPriority, ENABLE);   
    
    if(bSwitch == bTIMERSTART)
    {
        TIM_Cmd(TIMx,ENABLE);
    }
    else
    {
        TIM_Cmd(TIMx, DISABLE);
    }                     
}


//定时器复位并重新开始定时.
void BaseTimer::timer_ResetONOFF(bool bSwitch)
{
    TIM_TypeDef * TIMx = (ucBaseTimerNo==6)? TIM6: TIM7;        
    
    TIM_ClearITPendingBit(TIMx, TIM_IT_Update); //清除定时器中断更新标识.

    TIM_SetAutoreload(TIMx, usARR); 
    TIM_SetCounter(TIMx, 0); //复位向上计数器当前值为0.
    if(bSwitch == bTIMERSTART)
        TIM_Cmd(TIMx, ENABLE); //使能定时器开始定时.
    else
        TIM_Cmd(TIMx, DISABLE); //失能能定时器停止定时.   
}



