/*********************************************************
  功能： Modbus-RTU从站通讯协议
  描述： 实现了从站通讯的几个功能码
  设计： azjiao
  版本： 0.1
  日期： 2018年10月10日
 *********************************************************/
#ifndef __RTU_SLAVE_H
#define __RTU_SLAVE_H

#include "RTU_RXTX.h"

//定义供通讯的服务器资源类型最大数量.
const u8 uc0xMaxLen = 32;  //内部位或物理coil/8,共计uc0xMaxLen*8个。
const u8 uc1xMaxLen = 32;  //输入离散量DI/8，共计uc1xMaxLen*8个。
const u8 uc3xMaxLen = 64;   //输入存储器AI
const u8 uc4xMaxLen = 64;   //保持寄存器HoldReg

//定义通讯资源结构
typedef struct
{
    u8  uc0xxxx[uc0xMaxLen];  //内部位或物理coil
    u8  uc0xIndex;
    u8  uc1xxxx[uc1xMaxLen];  //输入离散量DI
    u8  uc1xIndex;
    u16 us3xxxx[uc3xMaxLen];  //输入存储器AI
    u8  uc3xIndex;
    u16 us4xxxx[uc4xMaxLen];  //保持寄存器HoldReg
    u8  uc4xIndex;
} Source_Type;

//声明外部定义的公共RTU_DataCtrl实例
extern RTU_DataCtrl Neo_RTUPort;

//------------------------------------------------------------------------
//Modbut_RTU从站类.
class RTU_Slave
{
    private:
        u8  ucNodeAddr;  //站号.
        Source_Type Source;  //从站提供通讯的资源.
        RTU_DataCtrl* pRTUPort;  //从站使用的RTU_DataCtrl实例的指针。
        //定义从站使用的RTU_DataCtrl 实例的别名。
        #define RTU_PORT  (*(pRTUPort))   
        //从站支持的RTU协议功能码.
        enum
        {
            slaveFunc0x01 = 0x01,
            slaveFunc0x02 = 0x02,
            slaveFunc0x03 = 0x03,
            slaveFunc0x04 = 0x04,
            slaveFunc0x05 = 0x05,
            slaveFunc0x0F = 0x0F,
            slaveFunc0x10 = 0x10,
        } FunCode;
        //协议功能码处理函数。
        void slaveFunc_0x01(void);
        void slaveFunc_0x02(void);
        void slaveFunc_0x03(void);
        void slaveFunc_0x04(void);
        void slaveFunc_0x05(void);
        void slaveFunc_0x0F(void);
        void slaveFunc_0x10(void);
        void default_NonSupport(void);

    public:
        RTU_Slave(RTU_DataCtrl* ptr = &RTU_PORT_ALIAS, u8 ucAddr = NODEADDR);
        ~RTU_Slave(void) {pRTUPort = nullptr;};

        //从站初始化: 配置硬件并接收数据开始工作。
        void slave_Init(u32 unBR = BAUDRATE, u16 usDB = DATABIT, u16 usSB = STOPBIT, u16 usPt = PARITY)
        {
            RTU_PORT.RTU_Init(unBR, usDB ,usSB, usPt);
        };        

        //从站服务函数
        void slaveService(void);

        //资源的读写操作。(资源映射)
        //DQ数据的写入0xxxx
        //DQ数据必须以字节为单位写入，所写入的位置是字节索引编号。
        void write_SourceDQ(u8 ucData, u8 ucIndex)
        {
            if((ucIndex >= 0) && (ucIndex < uc0xMaxLen))
            {
                Source.uc0xxxx[ucIndex] = ucData;
            }
        };
        //DQ数据0xxxx的读取
        //一次读取指定位置的一个字节数据。
        u8 read_SourceDQ(u8 ucIndex)
        {
            if((ucIndex > 0) && (ucIndex < uc0xMaxLen))
            {
                return Source.uc0xxxx[ucIndex];
            }
        };
        //DI数据写入1xxxx
        void write_SourceDI(u8 ucData, u8 ucIndex);
        //DI数据1xxxx的读取
        u8 read_SourceDI(u8 ucIndex);
        //AI数据3x的写入3xxxx
        //AI是16位数据，以2字节为一个单位写入ucIndex指定的资源区。
        //映射时不改变usData本来的存储顺序。
        //FIXME:STM32F10x是小端模式。
        void write_SourceAI(u16 usData, u8 ucIndex)
        {
            if((ucIndex >= 0) && (ucIndex < uc3xMaxLen)) //无符号整数没必要和0比较，但逻辑思维上需要。可以去掉。
            {
                Source.us3xxxx[ucIndex] = usData;
            }
        };
        //从资源区3xxxx读取指定位置的AI数据
        u16 read_SourceAI(u8 ucIndex)
        {
            if((ucIndex >= 0) && (ucIndex < uc3xMaxLen))
            {
                return Source.us3xxxx[ucIndex];
            }
        };
        //HoldReg写入4xxxx
        //HoldReg是16位数据，以2字节为一个单位写入ucIndex指定的资源区。
        //映射时不改变usData本来的存储顺序。
        //FIXME:STM32F10x是小端模式
        void write_SourceHg(u16 usData, u8 ucIndex)
        {
            if((ucIndex >= 0) && (ucIndex < uc4xMaxLen))
            {
                Source.us4xxxx[ucIndex] = usData;
            }
        };
        //HoldReg数据4xxxx的读取
        u16 read_SourceHg(u8 ucIndex)
        {
            if((ucIndex >= 0) && (ucIndex < uc4xMaxLen))
            {
                return Source.us4xxxx[ucIndex];
            }
        };
};



#endif /* end of include guard: __RTU_SLAVE_H */
