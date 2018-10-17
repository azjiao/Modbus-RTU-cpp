#include <stm32f10x.h>
//#include <core_cm3.h>
#include "delay.h"

// ��1ms����SysTick.
void delay_Init(void)
{
    // SysTick���á�
    if (SysTick_Config(RELOAD_1MS))
    {
        while(1);
    }
}




// ms��delay.
void delay_ms(u32 utime_ms)
{

    Timer.uTimer_ms = utime_ms;
    // ��ѯ��ʱ�Ƿ񵽣������������ѯ��
    while( Timer.uTimer_ms != 0U )
        ;
}

// s��delay.
void delay_s(u32 utime_s)
{
    // ��ʱʱ��Ϊutime_s����1000ms��
    for (int i = 0; i < utime_s; ++i)
    {
        delay_ms(1000U);
    }
}

// ��ʱ��ͨ��ʱ��
// ��bEnbΪtrueʱ��ʼ��ʱ����ʱ��λΪ1ms��
// ��bEnbΪfalseʱ��λ��ʱ����
// ����ʱ��������û�и�λ��ʱ����ʱ����ǰ����ֵuEt���ֲ��䡣
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
        //ÿ�μ�⵽����(ÿ1ms��תһ��)�ͼ�1.
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

//ʱ���ۼ���
//��bEnbΪtrueʱ��ʱ��ʼ��ʱ�䵥λΪms��
//��bEnbΪfalseʱ��λ��ʱ����
//ʱ���ۼ�ֵ���Ϊ32Ϊ�޷���������Ϊ4294967295ms��Լ47�졣
//���ۼ�ʱ��Խ��ʱ�Զ���λΪ0.
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
        //timer->bQΪtrue��ζ���ۼ��ڽ��С�
        timer->bQ = true;
    }

    return timer->uEt;
}

