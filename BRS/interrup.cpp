//�жϷ�����������extern "C"{  }��Χ,��ֹ�������ı亯����ΪC++��ʽ�ĳ�������.
#include "dev.h"

#ifdef __cplusplus
    extern "C" {
#endif

//TimerΪ��delay.cpp�ж����SysTickʹ�õĶ�ʱ���ṹ��        
extern SysTick_TimerType Timer;
        
// SysTick��ʱ���ж�
//ÿ1ms�ж�һ�Σ�ÿ��ʹbPlus_ms��תһ�Σ�ʹuTimer_ms��1��
//����������������Ϊ2ms��ÿ1ms��תһ�Ρ�
void SysTick_Handler(void)
{
    Timer.bPlus_ms = !Timer.bPlus_ms;
    //Timer.uTimer_ms���ڵȴ���ʱ����������õȴ���ʽ��ʱ��ò������á�
    if(Timer.uTimer_ms != 0U)
        Timer.uTimer_ms--;
}

#ifdef __cplusplus
}
#endif
