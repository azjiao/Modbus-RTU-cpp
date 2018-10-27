/*********************************************************
功能： baseTimer的实现
描述： STM32F10x有两个基本定时器:TIM6和TIM7。
       系统时钟为72MHz，定时器计数时钟源也是72MHz。
       当定时单位为us时，设置定时计数时钟频率为100kHz,每周期10us，即设定PSC=719.
       当定时单位为ms时，设置定时计数时钟频率为10kHz,每周期0.1ms，即设定PSC=7199.
设计： azjiao
版本： 0.1
日期： 2018年10月09日
*********************************************************/
#ifndef __BASETIMER_H
#define __BASETIMER_H

#include <stm32f10x.h>

const bool bUNITMS = true;  //ms单位
const bool bUNITUS = false;  //us单位
const bool bTIMERSTART = true;  //定时器启动
const bool bTIMERSTOP = false;  //定时器停止

class BaseTimer
{
    private:
        u8  ucBaseTimerNo;  //基本定时器编号:6或7.
        u32 unTimePt; //定时值.
        bool bUnit;  //定时器usTimePt的单位：us或ms。
        u16 usPSC;  //基本定时器预分频值.
        u16 usARR;  //基本定时器自动重转载值.
        u8 ucPrePriority;  //抢占优先级
        u8 ucSubPriority;  //响应优先级

        void setTimePt(u32 unPt, bool busOrms)  //busOrms=bUnitms为ms,bUnitus为us.
        {
            if(((busOrms == bUNITMS) && (unPt*10 < 65535)) || ((busOrms == bUNITUS) && (unPt/10 < 65535)))
            {                            
                usPSC = (busOrms == bUNITMS)? 7199: 719;
                //usPSC = 719;  //固定预分频系数,一个计数周期为10us=0.01ms
                unTimePt = unPt;
                bUnit = busOrms;
                usARR = (u16)(busOrms == bUNITMS)? (unTimePt*10 - 1): (unTimePt/10 - 1);
            }
        }

    public:
        //完成对属性定时值的初始化：usPSC、usARR以及定时器编号、定时值、单位。
        BaseTimer(u8 ucBTNo, u16 usPt, bool busOrms, u8 ucPre, u8 ucSub)
        {
            setProperty(ucBTNo, usPt, busOrms, ucPre, ucSub);
        };

        BaseTimer(void) {};
        ~BaseTimer(){};

        //设置定时器属性,用于使用缺省构造函数生成的对象的属性进行属性初始化。
        void setProperty(u8 ucBTNo, u32 unPt, bool busOrms, u8 ucPre, u8 ucSub)
        {
            if(ucBTNo == 6 || ucBTNo == 7)
                ucBaseTimerNo = ucBTNo;
            ucPrePriority = ucPre;
            ucSubPriority = ucSub;
            setTimePt(unPt, busOrms);
        };
        //使用缺省构造函数生成对象时，使用该函数一次启停定时器。
        //无论如何，均可以使用该函数启停定时器。
        //本设计没有使用该初始化函数。
    /*
        void timer_Init(u8 ucBTNo, u16 usPt, bool busOrms, bool bSwitch = bTimerStop)
        {
            setProperty(ucBTNo, usPt, busOrms);
            timer_Init(bSwitch);
        };
    */
        //使用非缺省构造生成对象或已经分步使用setProperty()设置定时器属性时使用该函数启停定时器。
        void timer_Init(bool bSwitch);
        void timer_ResetONOFF(bool bSwitch);  //定时器复位并重新启动或停止.
};



#endif /* end of include guard: __BASETIMER_H */
