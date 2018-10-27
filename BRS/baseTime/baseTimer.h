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

const bool bUNITMS = true;  //ms��λ
const bool bUNITUS = false;  //us��λ
const bool bTIMERSTART = true;  //��ʱ������
const bool bTIMERSTOP = false;  //��ʱ��ֹͣ

class BaseTimer
{
    private:
        u8  ucBaseTimerNo;  //������ʱ�����:6��7.
        u32 unTimePt; //��ʱֵ.
        bool bUnit;  //��ʱ��usTimePt�ĵ�λ��us��ms��
        u16 usPSC;  //������ʱ��Ԥ��Ƶֵ.
        u16 usARR;  //������ʱ���Զ���ת��ֵ.
        u8 ucPrePriority;  //��ռ���ȼ�
        u8 ucSubPriority;  //��Ӧ���ȼ�

        void setTimePt(u32 unPt, bool busOrms)  //busOrms=bUnitmsΪms,bUnitusΪus.
        {
            if(((busOrms == bUNITMS) && (unPt*10 < 65535)) || ((busOrms == bUNITUS) && (unPt/10 < 65535)))
            {                            
                usPSC = (busOrms == bUNITMS)? 7199: 719;
                //usPSC = 719;  //�̶�Ԥ��Ƶϵ��,һ����������Ϊ10us=0.01ms
                unTimePt = unPt;
                bUnit = busOrms;
                usARR = (u16)(busOrms == bUNITMS)? (unTimePt*10 - 1): (unTimePt/10 - 1);
            }
        }

    public:
        //��ɶ����Զ�ʱֵ�ĳ�ʼ����usPSC��usARR�Լ���ʱ����š���ʱֵ����λ��
        BaseTimer(u8 ucBTNo, u16 usPt, bool busOrms, u8 ucPre, u8 ucSub)
        {
            setProperty(ucBTNo, usPt, busOrms, ucPre, ucSub);
        };

        BaseTimer(void) {};
        ~BaseTimer(){};

        //���ö�ʱ������,����ʹ��ȱʡ���캯�����ɵĶ�������Խ������Գ�ʼ����
        void setProperty(u8 ucBTNo, u32 unPt, bool busOrms, u8 ucPre, u8 ucSub)
        {
            if(ucBTNo == 6 || ucBTNo == 7)
                ucBaseTimerNo = ucBTNo;
            ucPrePriority = ucPre;
            ucSubPriority = ucSub;
            setTimePt(unPt, busOrms);
        };
        //ʹ��ȱʡ���캯�����ɶ���ʱ��ʹ�øú���һ����ͣ��ʱ����
        //������Σ�������ʹ�øú�����ͣ��ʱ����
        //�����û��ʹ�øó�ʼ��������
    /*
        void timer_Init(u8 ucBTNo, u16 usPt, bool busOrms, bool bSwitch = bTimerStop)
        {
            setProperty(ucBTNo, usPt, busOrms);
            timer_Init(bSwitch);
        };
    */
        //ʹ�÷�ȱʡ�������ɶ�����Ѿ��ֲ�ʹ��setProperty()���ö�ʱ������ʱʹ�øú�����ͣ��ʱ����
        void timer_Init(bool bSwitch);
        void timer_ResetONOFF(bool bSwitch);  //��ʱ����λ������������ֹͣ.
};



#endif /* end of include guard: __BASETIMER_H */
