/*********************************************************
  功能： Modbus-RTU主站通讯协议
  描述： 实现了从站通讯的几个功能码
  设计： azjiao
  版本： 0.1
  日期： 2018年10月12日
 *********************************************************/
#ifndef __RTU_MASTER_H
#define __RTU_MASTER_H

#include "RTU_RXTX.h"

//用户数据缓冲区最大长度字节数。
const u16 usUser_BufMaxLen = (u16)256;

const bool bRead = false;
const bool bWrite = true;

typedef  struct
{
    bool bDone;       //主站一次通讯完成，数据可读取。1:回传的数据可以读取。
    bool bBusy;       //忙。对于接收，接收开始不一定忙。
    bool bErr;        //接收帧有错误.
    bool bTimeOut;    //应答超时。 可归并到usErr中.这是当bErr为true且usErrMsg=4的等价bool量。
    u16  usErrMsg;    //通讯错误信息.0:无错;1:非本站信息(主站不用错误1);2:帧CRC错误;3:字节接收出错,4:接收超时。
    u32  unErrCount;  //失败计数。
}ModbusStatus_Struct;

class RTU_Master
{
    private:
        //待发送到从站的数据以及从从站读取的数据。
        //这个数据缓冲区存放的是用户数据，不是数据帧数据。即存放的是原始待发送数据以及解读出来的从站数据。
        //按照Modbus-RTU协议，数据的存放按照高地址字节存放数据低字节的方式，也就是大端模式。
        struct
        {
            u8 ucData[usUser_BufMaxLen];
            u16 usIndex;
        }User_DataBuffer;

        //主站使用的RTU_DataCtrl 实例的指针
        RTU_DataCtrl* pRTUPort;
        //定义主站使用的RTU_DataCtrl 实例的别名
        #define RTU_PORT  (*(pRTUPort))

        //主站支持的RTU协议功能码。
        enum
        {
            masterFunc0x01 = 0x01,
            masterFunc0x02 = 0x02,
            masterFunc0x03 = 0x03,
            masterFunc0x04 = 0x04,
            masterFunc0x0F = 0x0F,
            masterFunc0x10 = 0x10,
        } FunCode;
        //协议功能码处理函数。
        void masterFunc_0x01(u8 ucNodeAddr, u16 usDataAddr, u16 usNum);
        void masterFunc_0x02(u8 ucNodeAddr, u16 usDataAddr, u16 usNum);
        void masterFunc_0x03(u8 ucNodeAddr, u16 usDataAddr, u16 usNum);
        void masterFunc_0x04(u8 ucNodeAddr, u16 usDataAddr, u16 usNum);
        void masterFunc_0x0F(u8 ucNodeAddr, u16 usDataAddr, u16 usNum);
        void masterFunc_0x10(u8 ucNodeAddr, u16 usDataAddr, u16 usNum);
        //各功能码数据帧的编码与解码函数。
        bool enCode_0x01(u8 ucNodeAddr, u16 usDataAddr, u16 usNum);
        bool unCode_0x01(u8 ucNodeAddr);
        bool enCode_0x02(u8 ucNodeAddr, u16 usDataAddr, u16 usNum);
        bool unCode_0x02(u8 ucNodeAddr);
        bool enCode_0x03(u8 ucNodeAddr, u16 usDataAddr, u16 usNum);
        bool unCode_0x03(u8 ucNodeAddr);
        bool enCode_0x04(u8 ucNodeAddr, u16 usDataAddr, u16 usNum);
        bool unCode_0x04(u8 ucNodeAddr);
        bool enCode_0x0F(u8 ucNodeAddr, u16 usDataAddr, u16 usNum);
        bool unCode_0x0F(u8 ucNodeAddr);
        bool enCode_0x10(u8 ucNodeAddr, u16 usDataAddr, u16 usNum);
        bool unCode_0x10(u8 ucNodeAddr);



    public:
        RTU_Master(RTU_DataCtrl* ptr = &RTU_PORT_ALIAS);
        ~RTU_Master(void){pRTUPort = nullptr; };

        ModbusStatus_Struct masterStatus;  //主站状态字。
         //主站初始化: 配置硬件并接收数据开始工作。
        void master_Init(u32 unBR = BAUDRATE, u16 usDB = DATABIT, u16 usSB = STOPBIT, u16 usPt = PARITY)
        {
            RTU_PORT.RTU_Init(unBR, usDB ,usSB, usPt);
        };
        //向数据缓冲区写入8位数据
        void write2Buffer(u8* ucPtrSource, u16 usLen);
        //从数据缓冲区读出8位数据
        void read4Buffer(u8* ucPtrDes, u16 usLen);

        //向数据缓冲区写入16位数据(short int、float...)：小端模式数据源。
        void write16bitData_LMode(u16* ptrSource, u16 usNum);
        //向数据缓冲区写入16位数据(short int、float...)：大端模式数据源。
        void write16bitData_BMode(u16* ptrSource, u16 usNum);

        //从数据缓冲区读出16位数据(short int、float...)：小端模式数据源。
        //只有在滥用RTU协议时才根据需要来读取的不常用方式。
        void read16bitData_LMode(u16* ptrDes, u16 usNum);

        //从数据缓冲区读出16位数据(short int、float...)：大端模式数据源。
        //这是按照RTU协议应该采用的读取方式。
        void read16bitData_BMode(u16* ptrDes, u16 usNum);

        //向数据缓冲区写入32位数据(浮点数float....):小端模式数据源。
        //Modbus-RTU本身并不直接支持32位多字节数据，需要组合才能使用。
        //所以，32位数据并不是严格按照大端模式来传输的，而是把数据分成2个16位数据，在每个16位数据中按照大端来传输。
        //至于这两个16位数据的顺序则没有要求，需要根据需要进行转换：主从方必须一致才能够正确读取。
        void write32bitData_LMode(u32* ptrSource, u16 usNum);
        //向数据缓冲区写入32位数据(浮点数float....):大端模式数据源。
        void write32bitData_BMode(u32* ptrSource, u16 usNum);
        //从数据缓冲区读出32位数据：大端模式。
        void read32bitData_BMode(u32* ptrDes, u16 usNum);
        //Modbus主站通讯控制
        void master(u8 ucNodeAddr, bool bRWMode, u16 usData_Addr, u16 usNum);
        //test
        void printBuff(void);

};


#endif /* end of include guard: __RTU_MASTER_H */
