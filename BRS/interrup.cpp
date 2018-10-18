//中断服务函数必须以extern "C"{  }包围,防止编译器改变函数名为C++格式的长函数名.
#include "dev.h"

#ifdef __cplusplus
    extern "C" {
#endif

//Timer为在delay.cpp中定义的SysTick使用的定时器结构。        
extern SysTick_TimerType Timer;
        
// SysTick定时器中断
//每1ms中断一次，每次使bPlus_ms翻转一次，使uTimer_ms减1。
//所产生的脉冲周期为2ms，每1ms翻转一次。
void SysTick_Handler(void)
{
    Timer.bPlus_ms = !Timer.bPlus_ms;
    //Timer.uTimer_ms用于等待延时，如果不采用等待方式延时则该参数无用。
    if(Timer.uTimer_ms != 0U)
        Timer.uTimer_ms--;
}

#ifdef __cplusplus
}
#endif
