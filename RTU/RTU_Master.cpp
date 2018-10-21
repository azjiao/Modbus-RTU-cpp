#include <stdio.h>
#include "usart1.h"
#include "RTU_Master.h"

//RTU_Master 构造函数。
RTU_Master::RTU_Master(RTU_DataCtrl* ptr)
{
    //masterStatus.bBusy = false;  //noused
    masterStatus.bDone = false;
    masterStatus.bErr = false;
    masterStatus.bTimeOut = false;
    masterStatus.usErrMsg = 0;
    masterStatus.unErrCount = 0;
    pRTUPort = ptr;
}

//向数据缓冲区写数据
//ucPtrSource是数据源字节指针，usLen是待写入的数据量。
void RTU_Master::write2Buffer(u8* ucPtrSource, u16 usLen)
{
    int i = 0;
    for(i = 0; (i < usLen) && (i < usUser_BufMaxLen); i++)
    {
        User_DataBuffer.ucData[i] = *(ucPtrSource + i);
    }
    User_DataBuffer.usIndex = i;
}

//从数据缓冲区读数据
//usLen是需要读出的数据字节量，ucPtrDes是保存的目的字节指针。
//必须保证目的空间已分配。
void RTU_Master::read4Buffer(u8* ucPtrDes, u16 usLen )
{
    int i = 0;
    for(i = 0; (i < usLen) && (i < usUser_BufMaxLen); i++)
    {
        *(ucPtrDes + i) = User_DataBuffer.ucData[i];
    }
}

//向数据缓冲区写入16位数据(short int、float...)：小端模式数据源。
//大小端都是指传入的数据源数据存放方式而言,对于User_Databuffer来说是按照Modbus-RTU协议要求的大端模式来存放的，以便按顺序先传送高字节。
//小端模式是STM32F10x采用的数据存储方式。
void RTU_Master::write16bitData_LMode(u16* ptrSource, u16 usNum)
{
    int i = 0;
    for(i = 0; (i < usNum) && (i < (usUser_BufMaxLen >> 1)); i++)
    {
        User_DataBuffer.ucData[2*i] = (*(ptrSource + i) & 0xFF00) >> 8;  //低地址存放高字节, 源数据的高字节在高地址.
        User_DataBuffer.ucData[2*i + 1] = (*(ptrSource + i)) & 0xFF;  //高地址存放低字节
        //other.
        //*((u16*)User_DataBuffer.ucData + i) = ((*(ptrSource + i)) & 0xFF << 8) | ((*(ptrSource + i) & 0xFF00) >> 8);
        //-----------------------------------------(          低字节         )--------(       高字节          )
    }
}

//向数据缓冲区写入16位数据(short int、float...)：大端模式数据源。
//大小端都是指传入的数据源数据存放方式而言,对于User_Databuffer来说是按照Modbus-RTU协议要求的大端模式来存放的，以便按顺序先传送高字节。
//大端模式是各类PLC等控制单元采用的数据存储方式。
//这个函数主要用于从各类采用大端模式的设备采集得来的数据的通讯处理。
void RTU_Master::write16bitData_BMode(u16* ptrSource, u16 usNum)
{
    for(int i = 0; (i < usNum) && (i < (usUser_BufMaxLen >> 1)); i++)
    {
        //为了避免函数调用的开销，这里不调用write2Buffer()函数。
        User_DataBuffer.ucData[2*i] = *(ptrSource + i) & 0xFF;  //低地址存放高字节,源数据高字节在低地址。
        User_DataBuffer.ucData[2*i + 1] = (*(ptrSource + i) & 0xFF00) >> 8;  //高地址存放低字节
        //other.
        //*((u16*)User_DataBuffer.ucData + i) = ((*(ptrSource + i) & 0xFF00)) | (*(ptrSource + i) & 0xFF);
        //-------------------------------------(          低字节         )-----(       高字节          )
    }
}

//从数据缓冲区读出16位数据(short int、float...)：小端模式数据源。
//大小端都是指传入的数据源数据存放方式而言,对于User_Databuffer来说是按照Modbus-RTU协议要求的大端模式来存放的。
//小端模式是STM32F10x采用的数据存储方式。
//本函数按照小端模式来读取User_DataBuffer中的数据，读回来存放在以小端模式表示的ptrDes数据中。
//如果不按照Modbus-RTU协议传送数据，则传送回来的数据会自动把低字节放在高地址，则系统需要按照小端模式来读取才正确。
//所以，本函数只在协议滥用的时候才使用。
void RTU_Master::read16bitData_LMode(u16* ptrDes, u16 usNum)
{
    for(int i = 0; (i < usNum) && (i < (usUser_BufMaxLen >> 1)); i++)
    {
        *(ptrDes + i) = User_DataBuffer.ucData[2*i + 1] << 8 | ((u16)User_DataBuffer.ucData[2*i]);
        //other.
        //*(ptrDes + i) = *((u16*)User_DataBuffer.ucData + i);
    }
}

//从数据缓冲区读出16位数据(short int、float...)：大端模式数据源。
//大小端都是指传入的数据源数据存放方式而言,对于User_Databuffer来说是按照Modbus-RTU协议要求的大端模式来存放的。
//大端模式是各类PLC等控制单元采用的数据存储方式
//本函数按照大端模式读取User_DataBuffer中的数据，读回来存放在以小端模式表示的ptrDes数据中。
//如果按照Modbus-RTU协议传送数据，则传送回来的数据会自动把低字节放在高地址，则系统需要按照大端模式来读取才正确。
void RTU_Master::read16bitData_BMode(u16* ptrDes, u16 usNum)
{
    for(int i = 0; (i < usNum) && (i < (usUser_BufMaxLen >> 1)); i++)
    {
        *(ptrDes + i) = User_DataBuffer.ucData[2*i] << 8 | ((u16)User_DataBuffer.ucData[2*i + 1]);
    }
}

//向数据缓冲区写入32位数据(浮点数float....):小端模式数据源。
//Modbus-RTU本身并不直接支持32位多字节数据，需要组合才能使用。
//所以，32位数据并不是严格按照大端模式来传输的，而是把数据分成2个16位数据，在每个16位数据中按照大端来传输。
//至于这两个16位数据的顺序则没有要求，需要根据需要进行转换：主从方必须一致才能够正确读取。
//这里的方案是采用：源数据字节序列是M->L:A B C D,写入的字节序列M->L:D C B A。也即纯大端模式存放。
void RTU_Master::write32bitData_LMode(u32* ptrSource, u16 usNum)
{
    int i;
    for(i = 0; (i < usNum) && (i < (usUser_BufMaxLen >> 2)); i++)
    {
        User_DataBuffer.ucData[4*i]     = ((*(ptrSource + i)) & 0xFF000000) >> 24;  //低地址存放高字节, 源数据的高字节在高地址.
        User_DataBuffer.ucData[4*i + 1] = ((*(ptrSource + i)) & 0x00FF0000) >> 16;  //高地址存放低字节
        User_DataBuffer.ucData[4*i + 2] = ((*(ptrSource + i)) & 0xFF00) >> 8;       //低地址存放高字节, 源数据的高字节在高地址.
        User_DataBuffer.ucData[4*i + 3] = (*(ptrSource + i)) & 0xFF;                //高地址存放低字节
    }
}

//向数据缓冲区写入32位数据(浮点数float....):大端模式数据源。
void RTU_Master::write32bitData_BMode(u32* ptrSource, u16 usNum)
{
    int i;
    for(i = 0; (i < usNum) && (i < (usUser_BufMaxLen >> 2)); i++)
    {
        User_DataBuffer.ucData[4*i + 3] = ((*(ptrSource + i)) & 0xFF000000) >> 24;  //低地址存放高字节, 源数据的高字节在高地址.
        User_DataBuffer.ucData[4*i + 2] = ((*(ptrSource + i)) & 0x00FF0000) >> 16;  //高地址存放低字节
        User_DataBuffer.ucData[4*i + 1] = ((*(ptrSource + i)) & 0xFF00) >> 8;       //低地址存放高字节, 源数据的高字节在高地址.
        User_DataBuffer.ucData[4*i]     = (*(ptrSource + i)) & 0xFF;                //高地址存放低字节
    }
}

//从数据缓冲区读出32位数据：大端模式。
void RTU_Master::read32bitData_BMode(u32* ptrDes, u16 usNum)
{
    for(int i = 0; (i < usNum) && (i < (usUser_BufMaxLen >> 2)); i++)
    {
        *(ptrDes + i) = User_DataBuffer.ucData[4*i] << 24 | ((u32)User_DataBuffer.ucData[4*i + 1]) << 16 |
            (u32)User_DataBuffer.ucData[4*i + 2] << 8 | (u32)User_DataBuffer.ucData[4*i + 3];
    }
}

void RTU_Master::printBuff(void)
{
    int i;
    printf("缓冲区的数据是：\r\n");
    for(i = 0; i < 10; i++)
    {
        printf("0x%x\t", User_DataBuffer.ucData[i]);
    }
    printf("\r\n");
}

//Modbus主站通讯控制
//ucNodeAddr:从站地址
//bMode_RW:模式0读或1写
//usDataAddr:所需从站数据地址,即元件基址，含元件类型。
//usNum:数据byte长度,可能是读取的元件数量，也可能是写入的元件数量。
//ucPtr:从站回传数据缓冲区或待发送的赋值数据。
void RTU_Master::master(u8 ucNodeAddr, bool bMode_RW, u16 usDataAddr, u16 usNum)
{
    //masterStatus.bBusy = RTU_PORT.portStatus.bBusy;

    //判断元件类型:HoldReg,标识为4xxxx.
    if(usDataAddr >= 40000 && usDataAddr < 50000)
    {
        //写操作：连续写多个HoldReg,Func=0x10
        if(bMode_RW == bWrite)
        {
            masterFunc_0x10(ucNodeAddr, usDataAddr, usNum);
        }
        //读操作：连续读多个HoldReg,Func=0x03
        else{
            masterFunc_0x03(ucNodeAddr, usDataAddr, usNum);
        }
    }

    //判断元件类型：AI，标识为3xxxx.
    if(usDataAddr >= 30000 && usDataAddr < 40000)
    {
        //3xxxx只支持读操作
        if(bMode_RW == bRead)
        {
            masterFunc_0x04(ucNodeAddr, usDataAddr, usNum);
        }
    }

    //判断元件类型：DI，标识为1xxxx.
    if(usDataAddr >= 10000 && usDataAddr < 20000)
    {
        //1xxxx只支持读操作
        if(bMode_RW == bRead)
        {
            masterFunc_0x02(ucNodeAddr, usDataAddr, usNum);
        }
    }

    //判断元件类型：DQ，标识为0xxxx.
    //if(usDataAddr >= (u16)0 && usDataAddr < 9999)
    if(usDataAddr < 9999)  //无符号整数和0比较无意义。
    {
        //强制DQ
        if(bMode_RW == bWrite)
        {
            masterFunc_0x0F(ucNodeAddr, usDataAddr, usNum);
        }
        //读DQ
        else{
            masterFunc_0x01(ucNodeAddr, usDataAddr, usNum);
        }
    }
}

//--------------------------------function code:0x10---------------------------------------------------
//Function:0x10
//写多个连续的保持寄存器。
void RTU_Master::masterFunc_0x10(u8 ucNodeAddr, u16 usDataAddr, u16 usNum)
{
    bool bUnCode = false;

    masterStatus.bDone = false;
    //元件基址 4xxxx
    u16 Addr = usDataAddr - 40000;

    //发送数据打包为数据帧。
    enCode_0x10(ucNodeAddr, Addr, usNum);
    //发送
    RTU_PORT.SendFrame();
       
    //发送后转入接收。
    RTU_PORT.ReceiveFrame();
    //应答超时监测使能。
    RTU_PORT.timeRespTimeOut_Start();
 

    while((!RTU_PORT.portStatus.bReadEnb) && !RTU_PORT.portStatus.bErr);

    if(RTU_PORT.portStatus.bReadEnb)
        bUnCode = unCode_0x10(ucNodeAddr);

    judge("F0x10", bUnCode);
}


//编码0x10
//打包后的结果存放在TX_Struct中。
//ucPtr指向赋值字节数组。
bool RTU_Master::enCode_0x10(u8 ucNodeAddr, u16 usDataAddr, u16 usNum)
{
    u16 CRC16;
    u16 j;
    if(usNum > 123) return false;  //元件数量不能大于123.

    //发送缓冲区清零
    RTU_PORT.usTXIndex = 0;
    //从站地址
    RTU_PORT.TXBuffer[0] = ucNodeAddr;
    RTU_PORT.TXBuffer[0] = ucNodeAddr;
    //功能码
    RTU_PORT.TXBuffer[1] = 0x10;
    //元件基址
    RTU_PORT.TXBuffer[2] = usDataAddr >> 8;
    RTU_PORT.TXBuffer[3] = usDataAddr;
    //元件数量
    RTU_PORT.TXBuffer[4] = usNum >> 8; //高字节
    RTU_PORT.TXBuffer[5] = usNum;  //低字节
    //数据长度:usNum*2
    RTU_PORT.TXBuffer[6] = usNum << 1;
    //数据域,长度取决于数量usNum.
    for(int i = 0; i < RTU_PORT.TXBuffer[6]; i++)
    {
        RTU_PORT.TXBuffer[7 + i] = *(User_DataBuffer.ucData + i);
    }

    //数据CRC生成
    //j是CRC存放开始单元,低字节在前。
    j = RTU_PORT.TXBuffer[6] + 7;
    RTU_PORT.usTXIndex = j;
    //生成的CRC16高字节在前。
    CRC16 = RTU_PORT.CRC16Gen();
    //添加CRC16,低字节在前
    RTU_PORT.TXBuffer[j] = CRC16;
    RTU_PORT.TXBuffer[j+1] = CRC16 >> 8;
    //帧长度字节数。
    RTU_PORT.usTXIndex = j + 2;
    return true;
}

//接收数据解析0x10
//ucNodeAddr是从站站号。
bool RTU_Master::unCode_0x10(u8 ucNodeAddr)
{

    //如果接收到的功能码高位不为1，则接收正常。
    if(!(RTU_PORT.RXBuffer[1] & 0x80))
    {
        //省略比较返回的其他数据：功能码、元件基址、元件数量。
        //从站号正确
        if(RTU_PORT.RXBuffer[0] == ucNodeAddr)
            return true;
        else
            return false;
    }
    //如果从站没有返回则会超时.
    //如果从站异常。
    else
    {
        return false;
    }
}

//--------------------------------function code:0x03---------------------------------------------------
//Function:0x03
//读多个连续的保持寄存器。
void RTU_Master::masterFunc_0x03(u8 ucNodeAddr, u16 usDataAddr, u16 usNum)
{
    bool bUnCode = false;
    masterStatus.bDone = false;
    //元件基址:4xxxx
    u16 Addr = usDataAddr - 40000;

    //发送数据打包为数据帧。
    enCode_0x03(ucNodeAddr, Addr, usNum);
    //发送
    RTU_PORT.SendFrame();
        
    //发送后转入接收。
    RTU_PORT.ReceiveFrame();
    //应答超时监测使能。
    RTU_PORT.timeRespTimeOut_Start();

    //等待接收：转入接收不一定通讯接口忙，所以还必须或上是否可读。
    while((!RTU_PORT.portStatus.bReadEnb) && !RTU_PORT.portStatus.bErr);

    if(RTU_PORT.portStatus.bReadEnb)
        bUnCode = unCode_0x03(ucNodeAddr);

    judge("F0x03", bUnCode);
}

//Function:0x03
//编码0x03
//打包后的结果存放在TX_Struct中。
bool RTU_Master::enCode_0x03(u8 ucNodeAddr, u16 usDataAddr, u16 usNum)
{
    u16 CRC16;

    if(usNum > 125) return false; //待读取的连续元件数量不能超过125个。
    //发送缓冲区清零
    RTU_PORT.usTXIndex = 0;
    //从站地址
    RTU_PORT.TXBuffer[0] = ucNodeAddr;
    //功能码
    RTU_PORT.TXBuffer[1] = 0x03;
    //元件基址
    RTU_PORT.TXBuffer[2] = usDataAddr >> 8;
    RTU_PORT.TXBuffer[3] = usDataAddr;
    //元件数量
    RTU_PORT.TXBuffer[4] = usNum >> 8; //高字节
    RTU_PORT.TXBuffer[5] = usNum;  //低字节

    //数据CRC生成
    //生成的CRC16高字节在前。
    RTU_PORT.usTXIndex = 6;
    CRC16 = RTU_PORT.CRC16Gen();
    //添加CRC16,低字节在前
    RTU_PORT.TXBuffer[6] = CRC16;
    RTU_PORT.TXBuffer[7] = CRC16 >> 8;
    //帧长度字节数。
    RTU_PORT.usTXIndex = 8;
    return true;
}

//接收数据解析0x03
//Mb_Addr是从站站号。
//ucPtr用于存放读取的HoldReg值。
bool RTU_Master::unCode_0x03(u8 ucNodeAddr)
{
    //如果接收到的功能码高位不为1，则接收正常。
    if(!(RTU_PORT.RXBuffer[1] & 0x80))
    {
        //从站号正确且功能码正确
        if((RTU_PORT.RXBuffer[0] == ucNodeAddr) && (RTU_PORT.RXBuffer[1] == 0x03))
        {
            //对接收到的HoldReg值进行转储。值从索引3开始。
            u8 usNum = RTU_PORT.RXBuffer[2];
            for(int i = 0; i < usNum; i++)
            {
                *(User_DataBuffer.ucData + i) = RTU_PORT.RXBuffer[3 + i];
            }
            return true;
        }
        else
            return false;
    }
    //如果从站异常
    else
    {
        return false;
    }
}

//--------------------------------function code:0x04---------------------------------------------------
//Function:0x04
//读AI:输入存储器
void RTU_Master::masterFunc_0x04(u8 ucNodeAddr, u16 usDataAddr, u16 usNum)
{
    bool bUnCode = false;
    masterStatus.bDone = false;
    //元件基址:3xxxx
    u16 Addr = usDataAddr - 30000;

    //发送数据打包为数据帧。
    enCode_0x04(ucNodeAddr, Addr, usNum);
    //发送
    RTU_PORT.SendFrame();
    //发送后转入接收。
    RTU_PORT.ReceiveFrame();
    //应答超时监测使能。
    RTU_PORT.timeRespTimeOut_Start();

    //等待接收
    while((!RTU_PORT.portStatus.bReadEnb) && !RTU_PORT.portStatus.bErr);

    if(RTU_PORT.portStatus.bReadEnb)
        bUnCode = unCode_0x04(ucNodeAddr);

    judge("F0x04", bUnCode);
}

//编码0x04
//打包后的结果存放在TX_Struct中。
bool RTU_Master::enCode_0x04(u8 ucNodeAddr, u16 usDataAddr, u16 usNum)
{
    u16 CRC16;

    if(usNum > 125) return false; //待读取的连续元件数量不能超过125个。
    //发送缓冲区清零
    RTU_PORT.usTXIndex = 0;
    //从站地址
    RTU_PORT.TXBuffer[0] = ucNodeAddr;
    //功能码
    RTU_PORT.TXBuffer[1] = 0x04;
    //元件基址
    RTU_PORT.TXBuffer[2] = usDataAddr >> 8;
    RTU_PORT.TXBuffer[3] = usDataAddr;
    //元件数量
    RTU_PORT.TXBuffer[4] = usNum >> 8; //高字节
    RTU_PORT.TXBuffer[5] = usNum;  //低字节

    //数据CRC生成
    //生成的CRC16高字节在前。
    RTU_PORT.usTXIndex = 6;
    CRC16 = RTU_PORT.CRC16Gen();
    //添加CRC16,低字节在前
    RTU_PORT.TXBuffer[6] = CRC16;
    RTU_PORT.TXBuffer[7] = CRC16 >> 8;
    //帧长度字节数。
    RTU_PORT.usTXIndex = 8;
    return true;
}

//接收数据解析0x04
//ucPtr用于存放读取的HoldReg值。
bool RTU_Master::unCode_0x04(u8 ucNodeAddr)
{
    //如果接收到的功能码高位不为1，则接收正常。
    if(!(RTU_PORT.RXBuffer[1] & 0x80))
    {
        //从站号正确且功能码正确
        if((RTU_PORT.RXBuffer[0] == ucNodeAddr) && (RTU_PORT.RXBuffer[1] == 0x04))
        {
            //对接收到的AI值进行转储。值从索引3开始。
            u8 usNum = RTU_PORT.RXBuffer[2];
            for(int i = 0; i < usNum; i++)
            {
                *(User_DataBuffer.ucData + i) = RTU_PORT.RXBuffer[3 + i];
            }
            return true;
        }
        else
            return false;
    }
    //如果从站异常
    else{
        return false;
    }
}


/*------------------------------------------------------------*/
//--------------------------------function code:0x02---------------------------------------------------
//Function:0x02
//读多个连续的输入离散量DI
void RTU_Master::masterFunc_0x02(u8 ucNodeAddr, u16 usDataAddr, u16 usNum)
{
    bool bUnCode = false;
    masterStatus.bDone = false;
    //元件基址:1xxxx
    u16 Addr = usDataAddr - 10000;

    //发送数据打包为数据帧。
    enCode_0x02(ucNodeAddr, Addr, usNum);
    //发送
    RTU_PORT.SendFrame();
    //发送后转入接收。
    RTU_PORT.ReceiveFrame();
    //应答超时监测使能。
    RTU_PORT.timeRespTimeOut_Start();

    //等待接收转入
    while((!RTU_PORT.portStatus.bReadEnb) && !RTU_PORT.portStatus.bTimeOut);

    if(RTU_PORT.portStatus.bReadEnb)
        bUnCode = unCode_0x02(ucNodeAddr);

    judge("F0x02", bUnCode);
}

//编码0x02
bool RTU_Master::enCode_0x02(u8 ucNodeAddr, u16 usDataAddr, u16 usNum)
{
    u16 CRC16;

    if(usNum > 2000) return false; //待读取的连续元件数量不能超过2000个。
    //发送缓冲区清零
    RTU_PORT.usTXIndex = 0;
    //从站地址
    RTU_PORT.TXBuffer[0] = ucNodeAddr;
    //功能码
    RTU_PORT.TXBuffer[1] = 0x02;
    //元件基址
    RTU_PORT.TXBuffer[2] = usDataAddr >> 8;
    RTU_PORT.TXBuffer[3] = usDataAddr;
    //元件数量
    RTU_PORT.TXBuffer[4] = usNum >> 8; //高字节
    RTU_PORT.TXBuffer[5] = usNum;  //低字节

    //数据CRC生成
    //生成的CRC16高字节在前。
    RTU_PORT.usTXIndex = 6;
    CRC16 = RTU_PORT.CRC16Gen();
    //添加CRC16,低字节在前
    RTU_PORT.TXBuffer[6] = CRC16;
    RTU_PORT.TXBuffer[7] = CRC16 >> 8;
    //帧长度字节数。
    RTU_PORT.usTXIndex = 8;
    return true;
}

//接收数据解析0x02
//ucPtr用于存放读取的DI值。
bool RTU_Master::unCode_0x02(u8 ucNodeAddr)
{
    //如果接收到的功能码高位不为1，则接收正常。
    if(!(RTU_PORT.RXBuffer[1] & 0x80))
    {
        //从站号正确且功能码正确
        if((RTU_PORT.RXBuffer[0] == ucNodeAddr) && (RTU_PORT.RXBuffer[1] == 0x02))
        {
            //对接收到的DI值进行转储。值从索引3开始。
            u8 usNum = RTU_PORT.RXBuffer[2];
            for(int i = 0; i < usNum; i++)
            {
                *(User_DataBuffer.ucData + i) = RTU_PORT.RXBuffer[3 + i];
            }
            return true;
        }
        else
            return false;
    }
    //如果从站异常
    else{
        return false;
    }
}

//--------------------------------function code:0x0F---------------------------------------------------
//Function:0x0F
//写多个连续的输出离散量DQ
void RTU_Master::masterFunc_0x0F(u8 ucNodeAddr, u16 usDataAddr, u16 usNum)
{
    bool bUnCode = false;

    masterStatus.bDone = false;
    //元件基址 0xxxx
    u16 Addr = usDataAddr - 0;

    //发送数据打包为数据帧。
    enCode_0x0F(ucNodeAddr, Addr, usNum);
    //发送
    RTU_PORT.SendFrame();
    //发送后转入接收。
    RTU_PORT.ReceiveFrame();

    //应答超时监测使能。
    RTU_PORT.timeRespTimeOut_Start();

    //等待接收
    while((!RTU_PORT.portStatus.bReadEnb) && !RTU_PORT.portStatus.bTimeOut);

    if(RTU_PORT.portStatus.bReadEnb)
        bUnCode = unCode_0x0F(ucNodeAddr);

    judge("F0x0F", bUnCode);
}

//编码
bool RTU_Master::enCode_0x0F(u8 ucNodeAddr, u16 usDataAddr, u16 usNum)
{
    u16 CRC16;
    u16 j;
    if(usNum > 1976) return false;  //元件数量不能大于1976.

    //发送缓冲区清零
    RTU_PORT.usTXIndex = 0;
    //从站地址
    RTU_PORT.TXBuffer[0] = ucNodeAddr;
    //功能码
    RTU_PORT.TXBuffer[1] = 0x0F;
    //元件基址
    RTU_PORT.TXBuffer[2] = usDataAddr >> 8;
    RTU_PORT.TXBuffer[3] = usDataAddr;
    //元件数量
    RTU_PORT.TXBuffer[4] = usNum >> 8; //高字节
    RTU_PORT.TXBuffer[5] = usNum;  //低字节
    //数据长度:(usNum%8)?usNum/8+1:usNum/8;
    RTU_PORT.TXBuffer[6] = (usNum%8)? usNum/8+1: usNum/8;
    //数据域,长度取决于数量usNum.
    for(int i = 0; i < RTU_PORT.TXBuffer[6]; i++)
    {
        RTU_PORT.TXBuffer[7 + i] = *(User_DataBuffer.ucData + i);
    }

    //数据CRC生成
    //j是CRC存放开始单元,低字节在前。
    j = RTU_PORT.TXBuffer[6] + 7;
    //生成的CRC16高字节在前。
    RTU_PORT.usTXIndex = j;
    CRC16 = RTU_PORT.CRC16Gen();
    //添加CRC16,低字节在前
    RTU_PORT.TXBuffer[j] = CRC16;
    RTU_PORT.TXBuffer[j+1] = CRC16 >> 8;
    //帧长度字节数。
    RTU_PORT.usTXIndex = j + 2;
    return true;
}

//接收数据解析
//ucPtr用于存放读取的DI值。
bool RTU_Master::unCode_0x0F(u8 ucNodeAddr)
{
    //如果接收到的功能码高位不为1，则接收正常。
    if(!(RTU_PORT.RXBuffer[1] & 0x80))
    {
        //省略比较返回的其他数据：功能码、元件基址、元件数量。
        //从站号正确
        if(RTU_PORT.RXBuffer[0] == ucNodeAddr)
            return true;
        else
            return false;
    }
    //如果从站没有返回则会超时.
    //如果从站异常。
    else{
        return false;
    }
}

//--------------------------------function code:0x01---------------------------------------------------
//Function:0x01
//读取多个连续输出离散量DQ
void RTU_Master::masterFunc_0x01(u8 ucNodeAddr, u16 usDataAddr, u16 usNum)
{
    bool bUnCode = false;

    masterStatus.bDone = false;
    //元件基址:0xxxx
    u16 Addr = usDataAddr - 00000;

    //发送数据打包为数据帧。
    enCode_0x01(ucNodeAddr, Addr, usNum);
    //发送
    RTU_PORT.SendFrame();
    //发送后转入接收。
    RTU_PORT.ReceiveFrame();
    //应答超时监测使能。
    RTU_PORT.timeRespTimeOut_Start();

    //等待接收
    while((!RTU_PORT.portStatus.bReadEnb) && !RTU_PORT.portStatus.bTimeOut);

    if(RTU_PORT.portStatus.bReadEnb)
        bUnCode = unCode_0x01(ucNodeAddr);

    judge("F0x01", bUnCode);
}

//编码
bool RTU_Master::enCode_0x01(u8 ucNodeAddr, u16 usDataAddr, u16 usNum)
{
    u16 CRC16;

    if(usNum > 2000) return false; //待读取的连续元件数量不能超过2000个。
    //发送缓冲区清零
    RTU_PORT.usTXIndex = 0;
    //从站地址
    RTU_PORT.TXBuffer[0] = ucNodeAddr;
    //功能码
    RTU_PORT.TXBuffer[1] = 0x01;
    //元件基址
    RTU_PORT.TXBuffer[2] = usDataAddr >> 8;
    RTU_PORT.TXBuffer[3] = usDataAddr;
    //元件数量
    RTU_PORT.TXBuffer[4] = usNum >> 8; //高字节
    RTU_PORT.TXBuffer[5] = usNum;  //低字节

    //数据CRC生成
    //生成的CRC16高字节在前。
    RTU_PORT.usTXIndex = 6;
    CRC16 = RTU_PORT.CRC16Gen();
    //添加CRC16,低字节在前
    RTU_PORT.TXBuffer[6] = CRC16;
    RTU_PORT.TXBuffer[7] = CRC16 >> 8;
    //帧长度字节数。
    RTU_PORT.usTXIndex = 8;
    return true;
}

//接收数据解析
bool RTU_Master::unCode_0x01(u8 ucNodeAddr)
{
    //如果接收到的功能码高位不为1，则接收正常。
    if(!(RTU_PORT.RXBuffer[1] & 0x80))
    {
        //从站号正确且功能码正确
        if((RTU_PORT.RXBuffer[0] == ucNodeAddr) && (RTU_PORT.RXBuffer[1] == 0x01))
        {
            //对接收到的DI值进行转储。值从索引3开始。
            u8 usNum = RTU_PORT.RXBuffer[2];
            for(int i = 0; i < usNum; i++)
            {
                *(User_DataBuffer.ucData + i) = RTU_PORT.RXBuffer[3 + i];
            }
            return true;
        }
        else
            return false;
    }
    //如果从站异常
    else{
        return false;
    }
}


//接收数据帧判断。
void RTU_Master::judge(char* ptrFunCode, bool bUnCode)
{
    if(RTU_PORT.portStatus.bReadEnb)
    {
        //如果返回数据不符。
        if(!bUnCode)
        {
            masterStatus.bErr = true;
            masterStatus.usErrMsg = 1;
            printf("%s返回帧含出错信息或不是所需从站返回帧 \n\n\n\n", ptrFunCode);
            Usart_SendFrame(USART1, User_DataBuffer.ucData, User_DataBuffer.usIndex);
            printf("\n");
        }
        //返回正确。
        else
        {
            //置位完成。写操作不需要转储返回的其他数据。
            masterStatus.bDone = true;
            masterStatus.bErr = false;
        }
    }
    //如果出错：CRC校验失败。
    else if(RTU_PORT.portStatus.bErr && RTU_PORT.portStatus.usErr == 2)
    {
        masterStatus.bErr = true;
        masterStatus.usErrMsg = 2;
        printf("%s返回帧CRC16校验失败！\n", ptrFunCode);
        //打印帧数据
        for(int i = 0; i < RTU_PORT.usRXIndex; i++)
            printf("0x%x ", RTU_PORT.RXBuffer[i]);
        printf("\r\n");
    }
    //如果应答超时则不解析,处理下个事务（重发或进行下一帧发送）。
    else if(RTU_PORT.portStatus.bTimeOut)
    {
        masterStatus.bErr = true;
        masterStatus.usErrMsg = 4;
        RTU_PORT.portStatus.unErrCount++;//通讯错误次数统计。
        printf("%s接收超时!\n", ptrFunCode);
    }
}

