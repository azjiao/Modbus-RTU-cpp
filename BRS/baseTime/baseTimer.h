/*********************************************************
���ܣ� baseTimer��ʵ��
������ STM32F10x������������ʱ��:TIM6��TIM7��
       ϵͳʱ��Ϊ72MHz����ʱ������ʱ��ԴҲ��72MHz��
       ����ʱ��λΪusʱ�����ö�ʱ����ʱ��Ƶ��Ϊ100kHz,ÿ����10us�����趨PSC=719.
       ����ʱ��λΪmsʱ�����ö�ʱ����ʱ��Ƶ��Ϊ10kHz,ÿ����0.1ms�����趨PSC=7199.
��ƣ� azjiao
�汾�� 0.1
���ڣ� 2018��10��09��
*********************************************************/
#ifndef __BASETIMER_H
#define __BASETIMER_H

#include <stm32f10x.h>
//#include <stdio.h>   //test

const bool bUnitms = true;  //ms��λ
const bool bUnitus = false;  //us��λ
const bool bTimerStart = true;  //��ʱ������
const bool bTimerStop = false;  //��ʱ��ֹͣ

class BaseTimer
{
    private:
        u8  ucBaseTimerNo;  //������ʱ�����:6��7.
        u16 usARR;  //������ʱ���Զ���ת��ֵ.
        u16 usPSC;  //������ʱ��Ԥ��Ƶֵ.
        u16 usTimePt; //��ʱֵ.
    //test:
        void setBTNo(u8 ucBTNo) {if(ucBTNo == 6 || ucBTNo == 7) ucBaseTimerNo = ucBTNo;}
        void setPsc(u16 usPsc) {usPSC = usPsc;}  //test
        void setTimePt(u16 usPt, bool busOrms)  //busOrms=bUnitmsΪms,bUnitusΪus.
        {                        
            usPSC = (busOrms == bUnitms)? 7199: 719;
            usTimePt = usPt;
            usARR = (busOrms == bUnitms)? (usTimePt*10 - 1): (usTimePt/10 - 1);
        }
                
    public:
        //BaseTimer(u8 ucBTNo, u16 usArr, u16 usPsc);
        BaseTimer(u8 ucBTNo, u16 usPt, bool busOrms);
        BaseTimer(void){};  
        ~BaseTimer(){};
        
        //���ö�ʱ������,����ʹ��ȱʡ���캯�����ɵĶ���
        void setProperty(u8 ucBTNo, u16 usPt, bool busOrms)
        {
            setBTNo(ucBTNo);
            setTimePt(usPt, busOrms);                        
        };
        //ʹ��ȱʡ���캯�����ɶ���ʱ��ʹ�øú���һ����ͣ��ʱ����
        //������Σ�������ʹ�øú�����ͣ��ʱ����
        void timer_Init(u8 ucBTNo, u16 usPt, bool busOrms, bool bSwitch = bTimerStop);
        //ʹ�÷�ȱʡ�������ɶ�����Ѿ��ֲ�ʹ��setProperty()���ö�ʱ������ʱʹ�øú�����ͣ��ʱ����
        void timer_Init(bool bSwitch);
        void timer_ResetONOFF(bool bSwitch);  //��ʱ����λ������������ֹͣ.                  
};




#endif /* end of include guard: __BASETIMER_H */
