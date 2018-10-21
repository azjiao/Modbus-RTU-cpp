/*********************************************************
���ܣ� baseTimer��ʵ��
������ STM32F10x������������ʱ��:TIM6��TIM7��
��ƣ� azjiao
�汾�� 0.1
���ڣ� 2018��10��09��
*********************************************************/
#include "baseTimer.h"
#include "dev.h"
#include <stdio.h>

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

//busOrms���ó�false:us��ʱ����true:ms��ʱ
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
    //��ʶ��һ�ν��ж�ʱ����ʼ����
    static bool bFirstInit = true;
    TIM_TimeBaseInitTypeDef  TIM_InitStrut; //��ʱ����ʼ�������ṹ��
    
    TIM_TypeDef * TIMx = (ucBaseTimerNo==6)? TIM6: TIM7;
    u32 RCC_APB1Periph_TIMx = (ucBaseTimerNo==6)? RCC_APB1Periph_TIM6: RCC_APB1Periph_TIM7;
    //�ض϶�ʱ��ʱ�����ߣ�ԭ���ǵ��ظ�����TIM_TimeBaseInit(TIMx, &TIM_InitStrut)ʱ����������жϡ�
    if(bFirstInit == false)
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIMx, DISABLE);
        TIM_SetCounter(TIMx, 0); //��λ���ϼ�������ǰֵΪ0.
    }
    
    //��ʱ��ʱ��ʹ��.
    if(bFirstInit == true)
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIMx, ENABLE);      
    }    

    TIM_InitStrut.TIM_Period = usARR; // �Զ�װ�ؼĴ���ֵ.
    TIM_InitStrut.TIM_Prescaler = usPSC; //��ʱ��ʱ��Ԥ��Ƶֵ����Ϊ������ʱ���á� 
    TIM_InitStrut.TIM_CounterMode = TIM_CounterMode_Up;    //���ϼ���ģʽ��
    // TIMx�ǻ�����ʱ����ֻ��ʹ�����ϼ���ģʽ���������á�
    TIM_TimeBaseInit(TIMx, &TIM_InitStrut);  
    
    //ʹ��TIMx�жϣ������жϸ��¡�
    TIM_ITConfig(TIMx, TIM_IT_Update, ENABLE);
    TIM_ClearITPendingBit(TIMx, TIM_IT_Update); //����жϸ��±�ʶ.   
    
    if(bFirstInit == false)   
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIMx, ENABLE); 
    }
    
    //�����ж����ȼ�.    
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
    bFirstInit = false;   
}

//ucBTNo�Ƕ�ʱ�����,usPt�Ƕ�ʱֵ,busOrms��us����ms��λѡ��,bStart�����Ƿ�����������ʱ��.
void BaseTimer::timer_Init(u8 ucBTNo, u16 usPt, bool busOrms, bool bSwitch)
{    
    setProperty(ucBTNo, usPt, busOrms);
    timer_Init(bSwitch);    
}

//��ʱ����λ�����¿�ʼ��ʱ.
void BaseTimer::timer_ResetONOFF(bool bSwitch)
{
    TIM_TypeDef * TIMx = (ucBaseTimerNo==6)? TIM6: TIM7;        
    
    TIM_ClearITPendingBit(TIMx, TIM_IT_Update); //�����ʱ���жϸ��±�ʶ.
    TIM_SetCounter(TIMx, 0); //��λ���ϼ�������ǰֵΪ0.
   // TIM_SetAutoreload(TIMx, usARR); //��ֵ�ڳ�ʼ��ʱ�Ѿ����á�
    if(bSwitch == bTimerStart)
        TIM_Cmd(TIMx, ENABLE); //ʹ�ܶ�ʱ����ʼ��ʱ.
    else
        TIM_Cmd(TIMx, DISABLE); //ʧ���ܶ�ʱ��ֹͣ��ʱ.   
}



