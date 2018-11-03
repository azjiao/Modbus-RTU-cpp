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
        My_NVIC_Init(USART2_IRQn, 1, 1, ENABLE);
        //使能串口2接收中断
        USART_ITConfig(USART2, USART_IT_RXNE,ENABLE);
        //使能串口2空闲中断
        //USART_ITConfig(USART2, USART_IT_IDLE,ENABLE);
        
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
//ucT15_35No 和ucTrespNo使用同一个定时器TIM6,不可变。
Port_RTU::Port_RTU(u32 unBR, u16 usDB, u16 usSB, u16 usPt) : \
        Port_RS485(unBR, usDB, usSB, usPt), \
        ucT15_35No(6), ucTrespNo(6)
{
    float fOneByteTime;  //一个字节发送的时间(us)。
    u32 unBaudRate = getBaudRate();
    //根据波特率计算一个字节发送的时间。
    if(unBaudRate <= 19200)
        fOneByteTime = (11/(float)unBaudRate) * 1000000;  //一个字节发送的时间(us)。
    else
        fOneByteTime = 1750; //只有波特率大于19200时才使用固定定时。

    //usT15_us = 1.5 * fOneByteTime;  //t1.5定时时间us.
    unT35_us = 3.5 * fOneByteTime;  //t3.5定时时间us.
    unTresponse_us = TIMEOUTVAL * 1000;  //超时定时值(500)ms.

    T15_35.setProperty(ucT15_35No, unT35_us, bUNITUS, 0, 3);    //以t3.5工作，取消了字节流监测。
    //帧结束和接收超时检测使用同一个定时器,只对Trespond属性进行初始化，设为同一优先级。
    Trespond.setProperty(ucTrespNo, unTresponse_us, bUNITUS, 0, 3); //超时监测和帧结束监测使用同一个定时器。
}

//静态成员定时器的初始化
//使用缺省构造函数，没有初始化定时器属性。
//BaseTimer Port_RTU::T15_35;
//BaseTimer Port_RTU::Trespond;

//RTU端口通讯初始化.
void Port_RTU::portRTU_Init(void)
{
    //端口硬件初始化以及通讯特性参数设置.
    RS485_Init();

    //设置T15_35,本设计不监测字节流，只监测帧结束和应答超时。所以T15_35定时器以t3.5方式工作,(第四参数缺省)暂不启动定时器.
    T15_35.timer_Init(bTIMERSTOP);
    //使用同一个定时器硬件，则不用重复初始化Trespond。
    //默认为接收模式
    RS485_RX();
}

//------------------------------------------------------------------------------------------------

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
    timeFrameEnd_Start();
    //等待帧间隔结束。
    while(portStatus.bBusy);
    LED0_OFF;
}


//RS485口的DMA接收配置:由于需要使用接收缓冲区RXBuffer做DMA的目的，所以放在本类中而不是子类。
void RTU_DataCtrl::portReceiveDMA_Config(void)
{
    DMA_InitTypeDef DMA_InitStructure;
    //使能DMA1总线。
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    USART_DMACmd(USART2,USART_DMAReq_Rx,ENABLE);   //使能串口2 DMA接收
    
    DMA_DeInit(DMA1_Channel6);   //将DMA2的通道6寄存器重设为缺省值  串口2的接收对应的是DMA1通道6
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART2->DR;  //DMA外设ADC基地址
    //DMA_InitStructure.DMA_MemoryBaseAddr = (u32)RXBuffer;  //DMA内存基地址
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)RXBuffer;  //DMA内存基地址
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //数据传输方向，从外设读取发送到内存
    DMA_InitStructure.DMA_BufferSize = FRAME_MAXLEN;  //DMA通道的缓存的大小
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //外设地址寄存器不变
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //内存地址寄存器递增
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //外设数据宽度为8位
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //存储器数据宽度为8位
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  //工作在正常缓存模式
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMA通道拥有中优先级 
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMA通道x没有设置为内存到内存传输
    DMA_Init(DMA1_Channel6, &DMA_InitStructure);  //根据DMA_InitStruct中指定的参数初始化DMA的通道:DMA1的通道6，即为USART2_RX。
            
    DMA_Cmd(DMA1_Channel6, ENABLE);  //正式驱动DMA传输
}

