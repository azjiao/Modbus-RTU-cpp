#include <stm32f10x.h>
//#include <core_cm3.h>
#include "delay.h"

// 以1ms设置SysTick.
void delay_Init(void)
{
    // SysTick配置。
    if (SysTick_Config(RELOAD_1MS))
    {
        while(1);
    }
}




// ms级delay.
void delay_ms(u32 utime_ms)
{

    Timer.uTimer_ms = utime_ms;
    // 查询延时是否到，不到则继续查询。
    while( Timer.uTimer_ms != 0U )
        ;
}

// s级delay.
void delay_s(u32 utime_s)
{
    // 延时时间为utime_s倍的1000ms。
    for (int i = 0; i < utime_s; ++i)
    {
        delay_ms(1000U);
    }
}

// 延时接通定时器
// 当bEnb为true时开始定时，定时单位为1ms。
// 当bEnb为false时复位定时器。
// 当定时到达后如果没有复位定时器则定时器当前计数值uEt保持不变。
bool TimeON(bool bEnb, u32 uPt, TimerType *timer)
{
    if(!bEnb){
        timer->uEt = 0U;
        timer->bTemp = false;
        timer->bQ = false;
        return false;
    }
    else{
        //if((timer->uEt < uPt) && ((Timer.bPlus_ms) & (Timer.bPlus_ms ^ timer->bTemp)))
        //每次检测到边沿(每1ms翻转一次)就加1.
        if((timer->uEt < uPt) && (Timer.bPlus_ms ^ timer->bTemp))
            timer->uEt++;

        timer->bTemp = Timer.bPlus_ms;

        if((timer->uEt >= uPt)){
            timer->bQ = true;
            return true;
        }
        else{
            timer->bQ = false;
            return false;
        }
    }
}

//时间累计器
//当bEnb为true时计时开始，时间单位为ms。
//当bEnb为false时复位计时器。
//时间累计值最大为32为无符号整数，为4294967295ms，约47天。
//当累计时间越限时自动复位为0.
uint16_t TimeACC(bool bEnb, TimerType *timer)
{
    if(!bEnb)
    {
        timer->uEt = 0U;
        timer->bTemp = false;
        timer->bQ = false;
    }
    else{
        if(Timer.bPlus_ms ^ timer->bTemp)
            timer->uEt++;

        timer->bTemp = Timer.bPlus_ms;
        //timer->bQ为true意味着累计在进行。
        timer->bQ = true;
    }

    return timer->uEt;
}

