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
//#include <stdio.h>   //test

const bool bUnitms = true;  //ms单位
const bool bUnitus = false;  //us单位
const bool bTimerStart = true;  //定时器启动
const bool bTimerStop = false;  //定时器停止

class BaseTimer
{
    private:
        u8  ucBaseTimerNo;  //基本定时器编号:6或7.
        u16 usARR;  //基本定时器自动重转载值.
        u16 usPSC;  //基本定时器预分频值.
        u16 usTimePt; //定时值.
    //test:
        void setBTNo(u8 ucBTNo) {if(ucBTNo == 6 || ucBTNo == 7) ucBaseTimerNo = ucBTNo;}
        void setPsc(u16 usPsc) {usPSC = usPsc;}  //test
        void setTimePt(u16 usPt, bool busOrms)  //busOrms=bUnitms为ms,bUnitus为us.
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
        
        //设置定时器属性,用于使用缺省构造函数生成的对象。
        void setProperty(u8 ucBTNo, u16 usPt, bool busOrms)
        {
            setBTNo(ucBTNo);
            setTimePt(usPt, busOrms);                        
        };
        //使用缺省构造函数生成对象时，使用该函数一次启停定时器。
        //无论如何，均可以使用该函数启停定时器。
        void timer_Init(u8 ucBTNo, u16 usPt, bool busOrms, bool bSwitch = bTimerStop);
        //使用非缺省构造生成对象或已经分步使用setProperty()设置定时器属性时使用该函数启停定时器。
        void timer_Init(bool bSwitch);
        void timer_ResetONOFF(bool bSwitch);  //定时器复位并重新启动或停止.                  
};




#endif /* end of include guard: __BASETIMER_H */
