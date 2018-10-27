/*********************************************************
���ܣ� baseTimer��ʵ��
������ STM32F10x������������ʱ��:TIM6��TIM7��
��ƣ� azjiao
�汾�� 0.1
���ڣ� 2018��10��09��
*********************************************************/
#include "baseTimer.h"
#include "dev.h"

void BaseTimer::timer_Init(bool bSwitch)
{
    TIM_TypeDef * TIMx = (ucBaseTimerNo==6)? TIM6: TIM7;       

    TIM_TimeBaseInitTypeDef  TIM_InitStrut; //��ʱ����ʼ�������ṹ��                
    u32 RCC_APB1Periph_TIMx = (ucBaseTimerNo==6)? RCC_APB1Periph_TIM6: RCC_APB1Periph_TIM7;
            
    //��ʱ������ʱ��ʹ��.       
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIMx, ENABLE);      
     
    TIM_InitStrut.TIM_Period = usARR; // �Զ�װ�ؼĴ���ֵ.
    TIM_InitStrut.TIM_Prescaler = usPSC; //��ʱ��ʱ��Ԥ��Ƶֵ����Ϊ������ʱ���á� 
    TIM_InitStrut.TIM_CounterMode = TIM_CounterMode_Up;    //���ϼ���ģʽ��
    // TIMx�ǻ�����ʱ����ֻ��ʹ�����ϼ���ģʽ���������á�
    TIM_TimeBaseInit(TIMx, &TIM_InitStrut);  
    //��������жϱ�ʶUIF,�ñ�ʶ��TIM_TimeBaseInit()�����е�UG�����Զ����á�
    TIM_ClearITPendingBit(TIMx, TIM_IT_Update);
    
    //�����ڶ�ʱ�������ж�ֻ���ɼ��������������
    TIM_UpdateRequestConfig(TIMx, TIM_UpdateSource_Regular);
    
    //ʹ��TIMx�жϣ������жϸ��¡�
    TIM_ITConfig(TIMx, TIM_IT_Update, ENABLE);
       
    //�����ж����ȼ�. 
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


//��ʱ����λ�����¿�ʼ��ʱ.
void BaseTimer::timer_ResetONOFF(bool bSwitch)
{
    TIM_TypeDef * TIMx = (ucBaseTimerNo==6)? TIM6: TIM7;        
    
    TIM_ClearITPendingBit(TIMx, TIM_IT_Update); //�����ʱ���жϸ��±�ʶ.

    TIM_SetAutoreload(TIMx, usARR); 
    TIM_SetCounter(TIMx, 0); //��λ���ϼ�������ǰֵΪ0.
    if(bSwitch == bTIMERSTART)
        TIM_Cmd(TIMx, ENABLE); //ʹ�ܶ�ʱ����ʼ��ʱ.
    else
        TIM_Cmd(TIMx, DISABLE); //ʧ���ܶ�ʱ��ֹͣ��ʱ.   
}



