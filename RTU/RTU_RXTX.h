/*********************************************************
  功能： Modbus RTU通讯的底层通讯实现
  描述： 对485端口的初始化以及Modbus通讯所需要的几个定时器的初始化。
  设计： azjiao
  版本： 0.1
  日期： 2018年10月08日
 *********************************************************/
#ifndef __MODBUS_RXTX_H
#define __MODBUS_RXTX_H

#include "bitBand.h"
#include "baseTimer.h"
//#include <stdio.h>

// 帧最大长度:字节数.Modbus规定一帧长度不超过256字节。
const u16  FRAME_MAXLEN      = 256;
const bool TXMODE            = false;  //发送模式
const bool RXMODE            = true;   //接收模式
const u8   FRAMEEND_TIMERRUN = 1; //帧结束监测器工作
const u8   TIMEOUT_TIMERRUN  = 2;  //超时监测器工作

#include "RTU_Config.h"
//------------------------------------------------------
//RS485端口初始化数据结构定义
typedef struct
{
    u32 unBaudRate;  //波特率
    u16 usDataBit;  //数据位字长
    u16 usStopBit;  //停止位
    u16 usParity;  //校验位：0无 1奇 2偶
} RS485Init_Struct;

//modbus通讯状态字
typedef  struct
{
    bool bMode;  //处于TXMODE:发送模式,还是RXMODE:接收状态.
    bool bReadEnb; //接收到的帧可读取标识。0：不可读取；1：可读取。
    bool bBusy;         //忙。对于接收，接收开始不一定忙。
    bool bErr;          //接收帧有错误.
    bool bTimeOut;  //应答超时。 可归并到usErrMsg中.
    u16  usErrMsg;     //通讯错误信息.0:无错;1:非本站信息(主站不用错误1);2:帧CRC错误;3:字节接收出错,4:接收超时。
    u32  unErrCount;  //校验失败计数。
}PortStatus_Struct;

//----------------------------------------------------------------------------------
//RS485端口类：用于初始化485端口。
class Port_RS485
{
    private:
        RS485Init_Struct RS485Init;  //端口参数。
        u8             ucPortNo = 2;  //USART编号，对于精英版是2,即USART2。
        void initPort(void);  //设置通讯端口硬件。
        void configCommParam(void);  //配置通讯端口通讯参数。

    public:
        Port_RS485(u32 unBR, u16 usDB, u16 usSB, u16 usPt){setPortParam(unBR, usDB, usSB, usPt);};
        ~Port_RS485(){};

        u32  getBaudRate() { return RS485Init.unBaudRate;}  //获取波特率.
        void setPortParam(u32 unBR, u16 usDB, u16 usSB, u16 usPt);  //修改端口初始化数据.
        void RS485_Init(void);  //设置485口的硬件和通讯特性参数。
        void RS485_TX(void) {PDout(7) = 1; }  //使PD7为1使能发送。
        void RS485_RX(void) {PDout(7) = 0; }  //使PD7为0使能接收。
};

//Modbus_RTU端口类。
class Port_RTU : Port_RS485
{
    private:
        //    u16 usT15_us;  //t1.5定时时间。
        u32 unT35_us;  //t3.5定时时间。
        u32 unTresponse_us;  //应答超时时间。
        u8  ucT15_35No;  //t1.5和t3.5所共用的定时器编号.由于t1.5和t3.5并不同时工作，可以设置使用同一个定时器。
        u8  ucTrespNo;  //超时定时器编号.

        BaseTimer T15_35;    //t1.5和t3.5共用定时器.
        BaseTimer Trespond;  //超时定时器.
        bool bSameTimer;    //t1.5_t3.5使用同一个定时器的标识：当ucT15_35和ucTrespNo一样时bSameTimer为true;
        u8 ucWhichTimer;            //目前是哪个定时器工作：FRAMEEND_TIMERRUN帧结束监测器还是FRAMEEND_TIMERRUN超时监测器？

    public:
        //以传入的参数初始化子类，并设置所用定时器属性，同时根据定时器编号决定是否是同一个定时器。
        Port_RTU(u32 unBR, u16 usDB, u16 usSB, u16 usPt);
        ~Port_RTU(){};

        //用Port_RTU私用属性初始化端口。初始化所用端口硬件和定时器。
        void portRTU_Init(void);
        //用给定参数初始化端口。
        void portRTU_Init(u32 unBR, u16 usDB, u16 usSB, u16 usPt)
        {
            setPortParam(unBR, usDB, usSB, usPt);  //使用子类接口初始化子类属性。
            portRTU_Init();
        };

        bool getSameTimer(){return bSameTimer;}
        //查询哪个定时器工作？
        u8 whichTimerRun() {return ucWhichTimer;};

        //重新复位并启动超时定时器。
        void timeRespTimeOut_Start(void)
        {
            Trespond.timer_ResetONOFF(bTIMERSTART);
            ucWhichTimer = TIMEOUT_TIMERRUN;
        }

        void timeRespTimeOut_Stop(void)
        {
            Trespond.timer_ResetONOFF(bTIMERSTOP);
        }
        //void timeFrameEnd_Start(void) {T15_35.timer_Init(bTimerStart); T15_35.timer_ResetONOFF(bTimerStart);ucWhichTimer = FRAMEEND_TIMERRUN;};
        void timeFrameEnd_Start(void)
        {
            T15_35.timer_ResetONOFF(bTIMERSTART);
            ucWhichTimer = FRAMEEND_TIMERRUN;
        }
        void timeFrameEnd_Stop(void)
        {
            T15_35.timer_ResetONOFF(bTIMERSTOP);
        }

        //公有化子类接口。
        using Port_RS485::RS485_TX ;
        using Port_RS485::RS485_RX ;
};

//Modbus_RTU端口数据收发控制类.
class RTU_DataCtrl : public Port_RTU
{
    private:
        //生成CRC校验字
        u16 CRC16Gen(u8* ucPtr, u16 usLen);
        //CRC16校验
        bool CRC16Check(u8* ucPtr, u16 usLen);
        //清空接收缓冲区.
        void resetRXBuffer(void){usRXIndex = 0;};
        //复位状态字准备接收数据帧
        void resetStatus4RX(void)
        {
            portStatus.bReadEnb = false;  //复位数据可读取标识。
            portStatus.bErr     = false;
            portStatus.bTimeOut = false;
            portStatus.bMode    = RXMODE;  //处于接收状态,以便在接收一帧后校验CRC.
            portStatus.bBusy    = false;  //设置系统状态进入空闲.
        };
        //复位状态字准备发送数据帧
        void resetStatus4TX(void)
        {
            portStatus.bReadEnb = false;  //复位数据可读取标识。
            portStatus.bErr     = false;
            portStatus.bTimeOut = false;
            portStatus.bMode    = TXMODE;  //处于发送状态.
            portStatus.bBusy    = true;  //设置系统状态进入空闲.
        };
    public:
        RTU_DataCtrl(u32 unBR = BAUDRATE, u16 usDB = DATABIT, u16 usSB = STOPBIT, u16 usPt = PARITY): \
            Port_RTU(unBR, usDB,usSB, usPt), \
            usRXIndex(0),usTXIndex(0) {};

        ~RTU_DataCtrl(){};

        //数据缓冲区
        u8 RXBuffer[FRAME_MAXLEN];  //接收缓冲区
        u16 usRXIndex;  //接收缓冲区当前索引
        u8 TXBuffer[FRAME_MAXLEN];  //发送缓冲区
        u16 usTXIndex;  //发送缓冲区当前索引
        PortStatus_Struct  portStatus;  //通讯状态字,外部中断等需要设置状态字,所以设置为public.

        //使用给定参数初始化端口，也用于无参数初始化(使用缺省参数).
        //主站和从站类使用该接口初始化通讯端口。
        void RTU_Init(u32 unBR = BAUDRATE, u16 usDB = DATABIT, u16 usSB = STOPBIT, u16 usPt = PARITY)
        {
            portRTU_Init(unBR, usDB, usSB, usPt);  //使用子类接口初始化端口.
        };
        u16  CRC16Gen(void);  //默认使用TXBuffer作为生成校验码的数据.
        bool CRC16Check(void);  //默认使用RXBuffer作为检查校验码是否正确的数据.
        //存储一个接收到的数据到接收缓冲区.
        void saveAData(u8 ucData)
        {
            if(usRXIndex < FRAME_MAXLEN)
            {
                RXBuffer[usRXIndex++] = ucData;
            }
        };

        //接收数据帧
        void ReceiveFrame(void);
        //发送数据帧
        void SendFrame(void);
};


#endif /* end of include guard: __MODBUS_RXTX_H */
