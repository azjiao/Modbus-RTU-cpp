/*********************************************************
功能： Modbus RTU通讯的底层通讯实现
描述： 对485端口的初始化以及Modbus通讯所需要的几个定时器的初始化。
设计： azjiao
版本： 0.1
日期： 2018年10月08日
*********************************************************/
#include <stdio.h>
#include "dev.h"
#include "RTU_RXTX.h"

//公共RTU_DataCtrl 实例
RTU_DataCtrl Neo_RTUPort;

//Port_RS485构造函数：给端口初始化数据赋值。
Port_RS485::Port_RS485(u32 unBR, u16 usDB, u16 usSB, u16 usPt)
{
    setPortParam(unBR, usDB, usSB, usPt);
}

//修改端口号.
void Port_RS485::setPortNo(u8 ucPNo)
{
    ucPortNo = ucPNo;
}

//修改端口初始化数据.
void Port_RS485::setPortParam(u32 unBR, u16 usDB, u16 usSB, u16 usPt)
{
    RS485Init.unBaudRate = unBR;  //波特率

    switch(usDB)  //数据位
    {
        case 8:
            RS485Init.usDataBit = USART_WordLength_8b;
            break;
        case 9:
            RS485Init.usDataBit = USART_WordLength_9b;
            break;
    }

    switch(usSB)  //停止位
    {
        case 1:
            RS485Init.usStopBit = USART_StopBits_1;
            break;
        case 2:
            RS485Init.usStopBit = USART_StopBits_2;
            break;
        default:  //一般是1个停止位。
            RS485Init.usStopBit = USART_StopBits_1;
    }

    switch(usPt)  //校验位
    {
        case 0:
            RS485Init.usParity = USART_Parity_No;  //无校验
            break;
        case 1:
            RS485Init.usParity = USART_Parity_Odd; //奇校验
            break;
        case 2:
            RS485Init.usParity = USART_Parity_Even;  //偶校验
            break;
    }
}

//设置通讯端口硬件。
void Port_RS485::initPort(void)
{
    //精英板的485口是USART2口,只针对开发版实现。
    //USART2复用PA2和PA3作为收发口。(USART2是全双工口)
    //RS485外围芯片的收发使能端使用PD7控制。(RS485需要半双工模式)
    if (ucPortNo == 2)
    {
        //使能GPIOA和GPIOD总线时钟。
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOD, ENABLE);
        //USART2时钟使能
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

        //PA2和PA3复用配置
        //TX复用了PA2口,作复用推挽输出。
        My_GPIO_Init(GPIOA, GPIO_Pin_2, GPIO_Mode_AF_PP, GPIO_Speed_50MHz);
        //RX复用了PA3口，作浮空输入(频率参数做输入时无用)。
        My_GPIO_Init(GPIOA, GPIO_Pin_3, GPIO_Mode_IN_FLOATING);
        RCC_APB1PeriphResetCmd(RCC_APB1Periph_USART2,ENABLE);//复位串口2
        RCC_APB1PeriphResetCmd(RCC_APB1Periph_USART2,DISABLE);//停止复位

        //PD7配置，用于控制RS485电平转换芯片收发使能。配置为推挽输出.
        My_GPIO_Init(GPIOD, GPIO_Pin_7, GPIO_Mode_Out_PP, GPIO_Speed_50MHz);
    }
}

//配置通讯端口通讯参数。
void Port_RS485::configCommParam(void)
{
    if (ucPortNo == 2)
    {
        //设置485口通讯的特性参数。
        My_USART_Init(USART2, RS485Init.unBaudRate, RS485Init.usDataBit, RS485Init.usStopBit, RS485Init.usParity, USART_Mode_Rx | USART_Mode_Tx);

        //配置USART2中断优先级
        My_NVIC_Init(USART2_IRQn, 2, 2, ENABLE);
        //使能串口2接收中断
        USART_ITConfig(USART2, USART_IT_RXNE,ENABLE);
        //使能串口2
        USART_Cmd(USART2, ENABLE);
    }
}

//485端口初始化
void Port_RS485::RS485_Init(void)
{
    initPort();  //设置通讯端口硬件。
    configCommParam();  //485端口初始化.
}

//---------------------------------------------------------------------------
//Port_RTU构造函数
Port_RTU::Port_RTU(u32 unBR, u16 usDB, u16 usSB, u16 usPt) : \
            Port_RS485(unBR, usDB, usSB, usPt), \
            ucT15_35No(6), ucTrespNo(7)
{
    float fOneByteTime;  //一个字节发送的时间(us)。
    u32 unBaudRate = getBaudRate();
    //根据波特率计算一个字节发送的时间。
    if(unBaudRate <= 19200)
        fOneByteTime = (11/(float)unBaudRate) * 1000000;  //一个字节发送的时间(us)。
    else
        fOneByteTime = 1750; //只有波特率大于19200时才使用固定定时。

    usT15_us = 1.5 * fOneByteTime;  //t1.5定时时间us.
    usT35_us = 3.5 * fOneByteTime;  //t3.5定时时间us.
    usTresponse_ms = usTimeOut;  //超时定时值(500)ms.
    T15_35.setProperty(ucT15_35No, usT35_us, bUnitus);    //以t3.5工作，取消了字节流监测。
    Trespond.setProperty(ucTrespNo, usTresponse_ms, bUnitms);
}

//静态成员定时器的初始化
//使用缺省构造函数，没有初始化定时器属性。
BaseTimer Port_RTU::T15_35;
BaseTimer Port_RTU::Trespond;

//RTU端口通讯初始化.
void Port_RTU::portRTU_Init(void)
{
    //端口硬件初始化以及通讯特性参数设置.
    RS485_Init();

    //设置T15_35,本设计不监测字节流，只监测帧结束和应答超时。所以T15_35定时器以t3.5方式工作,(第四参数缺省)暂不启动定时器.
 //   T15_35.Timer_Init(ucT15_35No, usT35_us, bUnitus);
    //FixME:由于已经在构造函数中初始化过定时器属性，可以使用如下初始化定时器。(简化初始化，只在一处进行属性设置，避免无谓的错误。)
    T15_35.timer_Init(bTimerStop);
    //设置Trespond,(第四参数缺省)暂不启动定时器.
 //   Trespond.Timer_Init(ucTrespNo, usTresponse_ms, bUnitms);
    //FixME:由于已经在构造函数中初始化过定时器属性，可以使用如下初始化定时器。
    Trespond.timer_Init(bTimerStop);

    //默认为接收模式
    RS485_RX();
}

//------------------------------------------------------------------------------------------------
//RTU_DataCtrl构造函数
RTU_DataCtrl::RTU_DataCtrl(u32 unBR, u16 usDB, u16 usSB, u16 usPt) : \
                   Port_RTU(unBR, usDB,usSB, usPt)
{
    //清空数据缓冲区.
    usRXIndex = usTXIndex = 0;
}


//CRC16校验码生成
//待校验数据为发送缓冲区数据.
//返回原生16位CRC，高字节在前。
u16 RTU_DataCtrl::CRC16Gen(u8* ucPtr, u16 usLen)
{
    u16 CRC16 = 0xFFFF;  //CRC预置值
    for(u16 i = 0; i < usLen; i++)  //依次处理所有数据。
    {
        CRC16 ^= *(ucPtr + i);   //异或数据

        for(u8 bit = 0; bit < 8; bit++)
        {
            //如果待右移出位为1，则右移后异或多项式。
            if(CRC16 & 0x0001)
            {
                CRC16 >>= 1;
                CRC16 ^= 0xA001;  //校验多项式，CRC16_Modbus标准。
            }
            //否则只作右移一位。
            else{
                CRC16 >>= 1;
            }
        }
    }
    return (CRC16);
}

//使用接收缓冲区数据作为生成校验码的数据,这是默认的方式.
u16 RTU_DataCtrl::CRC16Gen(void)
{
    return CRC16Gen(TXBuffer, usTXIndex);
}

//CRC16校验,如果校验正确返回true,否则返回false.
bool RTU_DataCtrl::CRC16Check(u8* ucPtr, u16 usLen)
{
    //使用重新把含CRC的数据再校验，如果生成的CRC为0x00则正确，否则不正确。
    if(CRC16Gen(ucPtr, usLen) == (u16)0)
        return true;
    else
        return false;
}

//CRC16校验
//待校验数据为接收缓冲区数据,这是默认的方式.
bool RTU_DataCtrl::CRC16Check(void)
{
    return CRC16Check(RXBuffer, usRXIndex);
}


//接收数据帧
void RTU_DataCtrl::ReceiveFrame(void)
{
    //清空接收缓冲区.
    resetRXBuffer();
    //复位状态字用于接收.
    resetStatus4RX();
    //设置端口处于接收使能状态.
    RS485_RX();
}

//发送数据帧
void RTU_DataCtrl::SendFrame(void)
{
    //复位状态字用于发送.
    resetStatus4TX();
    //设置端口处于发送使能状态.
    RS485_TX();

    for(u16 i = 0; i < usTXIndex; i++)
    {
        USART_SendData(USART2,TXBuffer[i]);
        LED0_ON;
        while(USART_GetFlagStatus(USART2,USART_FLAG_TXE) != SET);
    }
    //等待全部连续数据发送完毕。
    while(USART_GetFlagStatus(USART2, USART_FLAG_TC) != SET);
    //帧间延时，开启t3.5。
    //延时结束会使bBusy复位。
    //T15_35.timer_ResetONOFF(bTimerStart);
    timeFrameEnd_Start();
    //等待帧间隔结束。
    while(portStatus.bBusy);
    LED0_OFF;
}



