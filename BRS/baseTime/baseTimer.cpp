/*********************************************************
功能： baseTimer的实现
描述： STM32F10x有两个基本定时器:TIM6和TIM7。
设计： azjiao
版本： 0.1
日期： 2018年10月09日
*********************************************************/
#include "baseTimer.h"
#include "dev.h"

//BaseTimer::BaseTimer(u8 ucBTNo, u16 usArr, u16 usPsc)
//{
//    if(ucBTNo == 6 || ucBTNo == 7)
//    {
//        ucBaseTimerNo = ucBTNo;
//        usARR = usArr;
//        usPSC = usPsc;
//    }
//    
//}

//busOrms设置成false:us定时还是true:ms定时
BaseTimer::BaseTimer(u8 ucBTNo, u16 usPt, bool busOrms)
{
    if(ucBTNo == 6 || ucBTNo == 7)
    {
        ucBaseTimerNo = ucBTNo; 
        setTimePt(usPt, busOrms);                   
    }
}

void BaseTimer::timer_Init(bool bSwitch)
{
    TIM_TimeBaseInitTypeDef  TIM_InitStrut; //定时器初始化参数结构。
    
    TIM_TypeDef * TIMx = (ucBaseTimerNo==6)? TIM6: TIM7;
    u32 RCC_APB1Periph_TIMx = (ucBaseTimerNo==6)? RCC_APB1Periph_TIM6: RCC_APB1Periph_TIM7;
  
    //定时器时钟使能.
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIMx, ENABLE);  
    
    TIM_InitStrut.TIM_Period = usARR; // 自动装载寄存器值.
    TIM_InitStrut.TIM_Prescaler = usPSC; //定时器时钟预分频值，作为计数器时钟用。    
    // TIMx是基本定时器，只能使用向上计数模式，不用设置。
    TIM_TimeBaseInit(TIMx, &TIM_InitStrut);
    
    //使能TIMx中断，允许中断更新。
    TIM_ITConfig(TIMx, TIM_IT_Update, ENABLE);
    TIM_ClearITPendingBit(TIMx, TIM_IT_Update); //清除中断更新标识.
    
    //设置中断优先级.    
    if(ucBaseTimerNo == 6)
    {             
        My_NVIC_Init(TIM6_IRQn, 0, 3, ENABLE);
    }
    else if(ucBaseTimerNo == 7)
    {                
        My_NVIC_Init(TIM7_IRQn, 0, 4, ENABLE);
    }    
    
    if(bSwitch == bTimerStart)
    {
        TIM_Cmd(TIMx,ENABLE);
    }
    else
    {
        TIM_Cmd(TIMx, DISABLE);
    }    
}

//ucBTNo是定时器编号,usPt是定时值,busOrms是us还是ms单位选择,bStart决定是否立即启动定时器.
void BaseTimer::timer_Init(u8 ucBTNo, u16 usPt, bool busOrms, bool bSwitch)
{    
    setProperty(ucBTNo, usPt, busOrms);
    timer_Init(bSwitch);    
}

//定时器复位并重新开始定时.
void BaseTimer::timer_ResetONOFF(bool bSwitch)
{
    TIM_TypeDef * TIMx = (ucBaseTimerNo==6)? TIM6: TIM7;        
    
    TIM_ClearITPendingBit(TIMx, TIM_IT_Update); //清除定时器中断更新标识.
    TIM_SetCounter(TIMx, 0); //复位向上计数器当前值为0.
    TIM_SetAutoreload(TIMx, usARR);
    if(bSwitch == bTimerStart)
        TIM_Cmd(TIMx, ENABLE); //使能定时器开始定时.
    else
        TIM_Cmd(TIMx, DISABLE); //失能能定时器停止定时.
    
}



