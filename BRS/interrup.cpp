//中断服务函数必须以extern "C"{  }包围,防止编译器改变函数名为C++格式的长函数名.

#ifdef __cplusplus
    extern "C" {
#endif
#include "dev.h"
        
// SysTick定时器中断
//每1ms中断一次，每次使bPlus_ms翻转一次，使uTimer_ms减1。
//所产生的脉冲周期为2ms，每1ms翻转一次。
void SysTick_Handler(void)
{
    Timer.bPlus_ms = !Timer.bPlus_ms;
    if(Timer.uTimer_ms != 0U)
        Timer.uTimer_ms--;
}

#ifdef __cplusplus
}
#endif
