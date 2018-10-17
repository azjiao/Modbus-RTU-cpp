/*********************************************************
功能： Modbus-RTU从站通讯协议
描述： 实现了从站通讯的几个功能码
设计： azjiao
版本： 0.1
日期： 2018年10月10日
*********************************************************/
#include "RTU_Slave.h"

//RTU_Slave构造函数
RTU_Slave::RTU_Slave(RTU_DataCtrl* ptr, u8 ucAddr) : ucNodeAddr(ucAddr)
{
    pRTUPort = ptr;
};

//从站服务函数
void RTU_Slave::slaveService(void)
{
    //如果接收到了数据帧.
    if(RTU_PORT.modbusStatus.bReadEnb)
    {
        //如果无错则解析
        //当帧可读时其实bErr为false，可以取消判断。
        if(!RTU_PORT.modbusStatus.bErr)
        {
            //首先判断从站地址是否相符。
            //站地址不符则放弃,重启接收。
            if(RTU_PORT.RXBuffer[0] != ucNodeAddr)
            {
                RTU_PORT.ReceiveFrame();
                return;
            }

            //提取功能码做判断
            switch(RTU_PORT.RXBuffer[1])
            {
                case slaveFunc0x01:slaveFunc_0x01();  //读多个DQ_0xxxx
                          break;
                case slaveFunc0x0F:slaveFunc_0x0F();  //写多个DQ_0xxxx
                          break;
                case slaveFunc0x05:slaveFunc_0x05();  //写单个DQ_0xxxx
                          break;
                case slaveFunc0x02:slaveFunc_0x02();  //读多个DI_1xxxx
                          break;
                case slaveFunc0x04:slaveFunc_0x04();  //读多个AI_3xxxx
                          break;
                case slaveFunc0x03:slaveFunc_0x03();  //读多个HoldReg_4xxxx
                          break;
                case slaveFunc0x10:slaveFunc_0x10();  //写多个HoldReg_4xxxx
                          break;
                default:  default_NonSupport();  //不支持的功能处理。

            }
            //结束后转入接收。
            RTU_PORT.ReceiveFrame();
        }
    }
}

//Function:0x01
//读取多个连续的DQ，0xxxx
//执行函数时从站地址已经相符。
void RTU_Slave::slaveFunc_0x01(void)
{
    uint16_t u16Num;  //元件数量
    uint16_t u16DataAddr;  //元件基址
    uint16_t u16CRC;
    uint16_t u16ByteIndex;  //基址所在字节索引，从0开始的字节索引。
    uint8_t u8BitIndex;  //基址所在字节的开始位索引，从0开始的位索引。

    //获取元件基址及数量。
    //元件基址在第2、3字节，高字节在前。
    u16DataAddr = (((uint16_t)RTU_PORT.RXBuffer[2]) << 8) | RTU_PORT.RXBuffer[3];
    //数量在第4、5字节,高字节在前
    u16Num = (((uint16_t)RTU_PORT.RXBuffer[4]) << 8) | RTU_PORT.RXBuffer[5];

    //装配数据:
    //从站地址
    RTU_PORT.TXBuffer[0] = ucNodeAddr;
    //功能码
    RTU_PORT.TXBuffer[1] = 0x01;

    //判断元件数量是否合理。
    //不能超过2000(0x07D0)个DQ请求。
    if((u16Num >= 0x0001) && (u16Num <= 0x07D0))
    {
        //判断元件基址和数量是否合适。
        //从站单元地址从0开始。(可以修改为1开始，可视情况定。)
        if((u16DataAddr + u16Num) <= (uc0xMaxLen * 8))
        {
            uint8_t u8ByteNum;  //所需字节数量

            //打包数据帧
            //响应的数据字节数量
            u8ByteNum = (u16Num%8)? (u16Num/8+1):u16Num/8;
            RTU_PORT.TXBuffer[2] = u8ByteNum;

            //定位u16DataAddr所在字节索引。
            u16ByteIndex = u16DataAddr/8;
            //定位所在字节的开始位索引。
            u8BitIndex = u16DataAddr%8;

            uint16_t u16DQ_Index = u16ByteIndex;  //DQ字节开始索引。
            uint8_t u8DQBit_Index = u8BitIndex;  //DQ字节位索引初始值。
            uint8_t u8DQMask = 0x01 << u8DQBit_Index;  //DQ字节初始掩码.
            uint16_t u16Tx_Index = 0;  //发送字节索引初始值。
            uint8_t u8TxBit_Index = 0; //发送字节位索引初始值.
            uint8_t u8TxMask = 0x01;  //TX字节初始掩码。

            //把需要响应的数据值赋值给TXBuffer[3]开始的单元。
            for(uint16_t i = 0; i < u16Num; i++)
            {
                if(u8DQBit_Index >= 8)
                {
                    u16DQ_Index++;
                    u8DQBit_Index = 0;
                    u8DQMask = 0x01;
                }

                if(u8TxBit_Index >= 8)
                {
                    u16Tx_Index++;
                    u8TxBit_Index = 0;
                    u8TxMask = 0x01;
                }

                if(Source.uc0xxxx[u16DQ_Index] & u8DQMask)
                    RTU_PORT.TXBuffer[u16Tx_Index + 3] |= u8TxMask;
                else
                    RTU_PORT.TXBuffer[u16Tx_Index + 3] &= (u8TxMask ^ 0xFF);
                u8DQMask <<= 1;
                u8TxMask <<= 1;

                u8DQBit_Index++;
                u8TxBit_Index++;
            }
            //最后一个TX数据字节高位填充0
            for(; u8TxBit_Index < 8; u8TxBit_Index++)
            {
                RTU_PORT.TXBuffer[u16Tx_Index + 3] &= (u8TxMask ^ 0xFF);
                u8TxMask <<= 1;
            }

            RTU_PORT.usTXIndex = u8ByteNum + 3; //j为CRC所在单元。

        }
        else{
            //产生异常02：地址非法
            RTU_PORT.TXBuffer[1] |= 0x80;
            RTU_PORT.TXBuffer[2] = 0x02;
            RTU_PORT.usTXIndex = 3;
        }
    }
    else{
        //产生异常03：数据非法
        RTU_PORT.TXBuffer[1] |= 0x80;
        RTU_PORT.TXBuffer[2] = 0x03;
        RTU_PORT.usTXIndex = 3;
    }

    //生成CRC16。
    u16CRC = RTU_PORT.CRC16Gen();
    RTU_PORT.TXBuffer[RTU_PORT.usTXIndex++] = u16CRC;
    RTU_PORT.TXBuffer[RTU_PORT.usTXIndex++] = u16CRC >> 8;
    //发送打包后的帧
    RTU_PORT.SendFrame();
}

//Function:0x0F
//强制多个连续的DQ，0xxxx
void RTU_Slave::slaveFunc_0x0F(void)
{
    uint16_t u16Num;  //元件数量
    uint16_t u16DataAddr;  //元件基址
    uint16_t u16CRC;
//    uint16_t j; //CRC装载单元索引。
    uint8_t u8ByteNum;  //所需字节数量
    uint16_t u16ByteIndex;  //基址所在字节索引，从0开始的字节索引。
    uint8_t u8BitIndex;  //基址所在字节的开始位索引，从0开始的位索引。

    //获取元件基址及数量。
    //元件基址在第2、3字节，高字节在前。
    u16DataAddr = (((uint16_t)RTU_PORT.RXBuffer[2]) << 8) | RTU_PORT.RXBuffer[3];
    //数量在第4、5字节,高字节在前
    u16Num = (((uint16_t)RTU_PORT.RXBuffer[4]) << 8) | RTU_PORT.RXBuffer[5];

    //计算所需的字节量。虽然接收帧里面含有字节量(第6字节)，但仍然要比较是否正确。
    u8ByteNum = (u16Num%8)? (u16Num/8+1):u16Num/8;

    //装配数据:
    //从站地址
    RTU_PORT.TXBuffer[0] = ucNodeAddr;
    //功能码
    RTU_PORT.TXBuffer[1] = 0x0F;

    //判断元件数量是否合理。
    //不能超过2000个DQ请求且请求的元件量和传入的字节量匹配。
    if((u16Num >= 0x0001) && (u16Num <= 0x07D0) && (u8ByteNum == RTU_PORT.RXBuffer[6]))
    {
        //判断元件基址和数量是否合适。
        //从站单元地址从0开始。(可以修改为1开始，可视情况定。)
        if((u16DataAddr + u16Num) <= uc0xMaxLen * 8)
        {
            //打包数据帧
            //元件基址
            RTU_PORT.TXBuffer[2] = RTU_PORT.RXBuffer[2];
            RTU_PORT.TXBuffer[3] = RTU_PORT.RXBuffer[3];
            //响应的DQ元件数量.
            RTU_PORT.TXBuffer[4] = RTU_PORT.RXBuffer[4];
            RTU_PORT.TXBuffer[5] = RTU_PORT.RXBuffer[5];

            //定位u16DataAddr所在字节索引。
            u16ByteIndex = u16DataAddr/8;
            //定位所在字节的开始位索引。
            u8BitIndex = u16DataAddr%8;

            uint16_t u16DQ_Index = u16ByteIndex;  //DQ字节开始索引。
            uint8_t u8DQBit_Index = u8BitIndex;  //DQ字节位索引初始值。
            uint8_t u8DQMask = 0x01 << u8DQBit_Index;  //DQ字节初始掩码.
            uint16_t u16Rx_Index = 7;  //接收字节索引初始值。
            uint8_t u8RxBit_Index = 0; //接收字节位索引初始值.
            uint8_t u8RxMask = 0x01;  //接收RX字节初始掩码。

            //把需要强制的数据值(从第7字节开始)赋值给DQ_0xxxx相应的单元。
            for(uint16_t i = 0; i < u16Num; i++)
            {
                if(u8DQBit_Index >= 8)
                {
                    u16DQ_Index++;
                    u8DQBit_Index = 0;
                    u8DQMask = 0x01;
                }

                if(u8RxBit_Index >= 8)
                {
                    u16Rx_Index++;
                    u8RxBit_Index = 0;
                    u8RxMask = 0x01;
                }

                if(RTU_PORT.RXBuffer[u16Rx_Index] &u8RxMask)
                    Source.uc0xxxx[u16DQ_Index] |= u8DQMask;
                else
                    Source.uc0xxxx[u16DQ_Index] &= (u8DQMask ^ 0xFF);
                u8DQMask <<= 1;
                u8RxMask <<= 1;

                u8DQBit_Index++;
                u8RxBit_Index++;
            }
            RTU_PORT.usTXIndex = 6; //j为CRC所在单元。
        }
        else{
            //产生异常02：地址非法
            RTU_PORT.TXBuffer[1] |= 0x80;
            RTU_PORT.TXBuffer[2] = 0x02;
            RTU_PORT.usTXIndex = 3;
        }
    }
    else{
        //产生异常03：数据非法
        RTU_PORT.TXBuffer[1] |= 0x80;
        RTU_PORT.TXBuffer[2] = 0x03;
        RTU_PORT.usTXIndex = 3;
    }

    //生成CRC16。
    u16CRC = RTU_PORT.CRC16Gen();
    RTU_PORT.TXBuffer[RTU_PORT.usTXIndex++] = u16CRC;
    RTU_PORT.TXBuffer[RTU_PORT.usTXIndex++] = u16CRC >> 8;

    //发送打包后的帧
    RTU_PORT.SendFrame();
}

//Function:0x05
//强制单个DQ,0x0000
//数据为2个字节，只有0xFF00和0x0000有效，对应ON和OFF.
void RTU_Slave::slaveFunc_0x05(void)
{
    uint16_t u16DataAddr;  //元件基址
    uint16_t u16Data;  //强制数据值。
    uint16_t u16CRC;

    uint16_t u16ByteIndex;  //基址所在字节索引，从0开始的字节索引。
    uint8_t u8BitIndex;  //基址所在字节的开始位索引，从0开始的位索引。

    //获取元件基址及数量。
    //元件基址在第2、3字节，高字节在前。
    u16DataAddr = (((uint16_t)RTU_PORT.RXBuffer[2]) << 8) | RTU_PORT.RXBuffer[3];

    //装配数据:
    //从站地址
    RTU_PORT.TXBuffer[0] = ucNodeAddr;
    //功能码
    RTU_PORT.TXBuffer[1] = 0x05;

    //判断强制数据值是否正确：只有0x0000和0xFF00有效。
    //数据值在第4、5字节,高字节在前。
    u16Data = (((uint16_t)RTU_PORT.RXBuffer[4] << 8) | RTU_PORT.RXBuffer[5]);
    if(u16Data == 0x0000 | u16Data == 0xFF00)
    {
        //判断元件基址是否合适。
        //从站单元地址从0开始。
        if(u16DataAddr <= uc0xMaxLen * 8)
        {
            //打包数据帧
            //元件基址
            RTU_PORT.TXBuffer[2] = RTU_PORT.RXBuffer[2];
            RTU_PORT.TXBuffer[3] = RTU_PORT.RXBuffer[3];
            //强制值
            RTU_PORT.TXBuffer[4] = RTU_PORT.RXBuffer[4];
            RTU_PORT.TXBuffer[5] = RTU_PORT.RXBuffer[5];

            //定位u16DataAddr所在字节索引。
            u16ByteIndex = u16DataAddr/8;
            //定位所在字节的开始位索引。
            u8BitIndex = u16DataAddr%8;
            uint8_t u8DQMask = 0x01 << u8BitIndex;  //DQ字节掩码.

            //向DQ相应的单元写强制值。
            if(u16Data == 0xFF00)
                Source.uc0xxxx[u16ByteIndex] |= u8DQMask;
            else
                Source.uc0xxxx[u16ByteIndex] &= (u8DQMask ^ 0xFF);

            RTU_PORT.usTXIndex = 6; //j为CRC所在单元。
        }
        else{
            //产生异常02：地址非法
            RTU_PORT.TXBuffer[1] |= 0x80;
            RTU_PORT.TXBuffer[2] = 0x02;
            RTU_PORT.usTXIndex = 3;
        }
    }
    else{
        //产生异常03：数据非法
        RTU_PORT.TXBuffer[1] |= 0x80;
        RTU_PORT.TXBuffer[2] = 0x03;
        RTU_PORT.usTXIndex = 3;
    }

    //生成CRC16。
    u16CRC = RTU_PORT.CRC16Gen();
    RTU_PORT.TXBuffer[RTU_PORT.usTXIndex++] = u16CRC;
    RTU_PORT.TXBuffer[RTU_PORT.usTXIndex++] = u16CRC >> 8;
    //发送打包后的帧
    RTU_PORT.SendFrame();
}

//Function:0x02
//读取多个连续的输入离散量DI,1xxxx
void RTU_Slave::slaveFunc_0x02(void)
{
    uint16_t u16Num;  //元件数量
    uint16_t u16DataAddr;  //元件基址
    uint16_t u16CRC;
    uint16_t u16ByteIndex;  //基址所在字节索引，从0开始的字节索引。
    uint8_t u8BitIndex;  //基址所在字节的开始位索引，从0开始的位索引。

    //获取元件基址及数量。
    //元件基址在第2、3字节，高字节在前。
    u16DataAddr = (((uint16_t)RTU_PORT.RXBuffer[2]) << 8) | RTU_PORT.RXBuffer[3];
    //数量在第4、5字节,高字节在前
    u16Num = (((uint16_t)RTU_PORT.RXBuffer[4]) << 8) | RTU_PORT.RXBuffer[5];

    //装配数据:
    //从站地址
    RTU_PORT.TXBuffer[0] = ucNodeAddr;
    //功能码
    RTU_PORT.TXBuffer[1] = 0x02;

    //判断元件数量是否合理。
    //不能超过2000(0x07D0)个DQ请求。
    if((u16Num >= 0x0001) && (u16Num <= 0x07D0))
    {
        //判断元件基址和数量是否合适。
        //从站单元地址从0开始。(可以修改为1开始，可视情况定。)
        if((u16DataAddr + u16Num) <= uc1xMaxLen * 8)
        {
            uint8_t u8ByteNum;  //所需字节数量

            //打包数据帧
            //响应的数据字节数量
            u8ByteNum = (u16Num%8)? (u16Num/8+1):u16Num/8;
            RTU_PORT.TXBuffer[2] = u8ByteNum;

            //定位u16DataAddr所在字节索引。
            u16ByteIndex = u16DataAddr/8;
            //定位所在字节的开始位索引。
            u8BitIndex = u16DataAddr%8;

            uint16_t u16DI_Index = u16ByteIndex;  //DI字节开始索引。
            uint8_t u8DIBit_Index = u8BitIndex;  //DI字节位索引初始值。
            uint8_t u8DIMask = 0x01 << u8DIBit_Index;  //DI字节初始掩码.
            uint16_t u16Tx_Index = 0;  //发送字节索引初始值。
            uint8_t u8TxBit_Index = 0; //发送字节位索引初始值.
            uint8_t u8TxMask = 0x01;  //TX字节初始掩码。

            //把需要响应的数据值赋值给RTU_PORT.TXBuffer[3]开始的单元。
            for(uint16_t i = 0; i < u16Num; i++)
            {
                if(u8DIBit_Index >= 8)
                {
                    u16DI_Index++;
                    u8DIBit_Index = 0;
                    u8DIMask = 0x01;
                }

                if(u8TxBit_Index >= 8)
                {
                    u16Tx_Index++;
                    u8TxBit_Index = 0;
                    u8TxMask = 0x01;
                }

                if(Source.uc1xxxx[u16DI_Index] & u8DIMask)
                    RTU_PORT.TXBuffer[u16Tx_Index + 3] |= u8TxMask;
                else
                    RTU_PORT.TXBuffer[u16Tx_Index + 3] &= (u8TxMask ^ 0xFF);
                u8DIMask <<= 1;
                u8TxMask <<= 1;

                u8DIBit_Index++;
                u8TxBit_Index++;
            }
            //最后一个TX数据字节高位填充0
            for(; u8TxBit_Index < 8; u8TxBit_Index++)
            {
                RTU_PORT.TXBuffer[u16Tx_Index + 3] &= (u8TxMask ^ 0xFF);
                u8TxMask <<= 1;
            }

            RTU_PORT.usTXIndex = u8ByteNum + 3; //j为CRC所在单元。
        }
        else{
            //产生异常02：地址非法
            RTU_PORT.TXBuffer[1] |= 0x80;
            RTU_PORT.TXBuffer[2] = 0x02;
            RTU_PORT.usTXIndex = 3;
        }
    }
    else{
        //产生异常03：数据非法
        RTU_PORT.TXBuffer[1] = RTU_PORT.TXBuffer[1] + 0x80;
        RTU_PORT.TXBuffer[2] = 0x03;
        RTU_PORT.usTXIndex = 3;
    }

    //生成CRC16。
    u16CRC = RTU_PORT.CRC16Gen();
    RTU_PORT.TXBuffer[RTU_PORT.usTXIndex++] = u16CRC;
    RTU_PORT.TXBuffer[RTU_PORT.usTXIndex++] = u16CRC >> 8;
    //发送打包后的帧
    RTU_PORT.SendFrame();
}


//Function:0x04
//读取多个连续的输入寄存器AI,3xxxx
void RTU_Slave::slaveFunc_0x04(void)
{
    uint16_t u16Num;  //元件数量
    uint16_t u16DataAddr;  //元件基址
    uint16_t u16CRC;

    //获取元件基址及数量。
    //元件基址在第2、3字节，高字节在前。
    u16DataAddr = (((uint16_t)RTU_PORT.RXBuffer[2]) << 8) | RTU_PORT.RXBuffer[3];
    //数量在第4、5字节,高字节在前.
    u16Num = (((uint16_t)RTU_PORT.RXBuffer[4]) << 8) | RTU_PORT.RXBuffer[5];

    //装配数据:
    //从站地址
    RTU_PORT.TXBuffer[0] = ucNodeAddr;
    //功能码
    RTU_PORT.TXBuffer[1] = 0x04;

    //判断元件数量是否合理。
    //不能超过125个AI请求。
    if((u16Num >= 0x0001) && (u16Num <= 0x007D))
    {
        //判断元件基址和数量是否合适。
        //从站单元地址从0开始。(可以修改为1开始，可视情况定。)
        //提供的AI_3xxxx是16bit数组，所以一个单元是一个元件数据。
        if((u16DataAddr + u16Num) <= uc3xMaxLen)
        {
            uint8_t u8ByteNum;  //所需字节数量

            //打包数据帧
            //响应的数据字节数量:元件数量*2.
            //可以使用u16Num<<1,因为不能超过125，所以此处u16Num高8位是0.
            u8ByteNum = RTU_PORT.RXBuffer[5] << 1;
            RTU_PORT.TXBuffer[2] = u8ByteNum;
            //把需要响应的数据值赋值给RTU_PORT.TXBuffer[3]开始的单元。
            for(uint16_t i = 0; i < u8ByteNum; i += 2)
            {
                //16位AI资源数据分拆：高字节在前。
                RTU_PORT.TXBuffer[3 + i] = Source.us3xxxx[i/2 + u16DataAddr] >> 8; //16位数据高8位字节。
                RTU_PORT.TXBuffer[3 + i + 1] = Source.us3xxxx[i/2 + u16DataAddr];  //16位数据低8位字节。
            }

            RTU_PORT.usTXIndex = u8ByteNum + 3; //j为CRC所在单元。
        }
        else{
            //产生异常02：地址非法
            RTU_PORT.TXBuffer[1] = RTU_PORT.TXBuffer[1] + 0x80;
            RTU_PORT.TXBuffer[2] = 0x02;
            RTU_PORT.usTXIndex = 3;
        }
    }
    else{
        //产生异常03：数据非法
        RTU_PORT.TXBuffer[1] = RTU_PORT.TXBuffer[1] + 0x80;
        RTU_PORT.TXBuffer[2] = 0x03;
        RTU_PORT.usTXIndex = 3;
    }

    //生成CRC16。
    u16CRC = RTU_PORT.CRC16Gen();
    RTU_PORT.TXBuffer[RTU_PORT.usTXIndex++] = u16CRC;
    RTU_PORT.TXBuffer[RTU_PORT.usTXIndex++] = u16CRC >> 8;
    //发送打包后的帧
    RTU_PORT.SendFrame();

}

//Function:0x03
//读取多个连续的保持寄存器HoldReg,4xxxx
void RTU_Slave::slaveFunc_0x03(void)
{
    uint16_t u16Num;  //元件数量
    uint16_t u16DataAddr;  //元件基址
    uint16_t u16CRC;

    //获取元件基址及数量。
    //元件基址在第2、3字节，高字节在前。
    u16DataAddr = (((uint16_t)RTU_PORT.RXBuffer[2]) << 8) | RTU_PORT.RXBuffer[3];
    //数量在第4、5字节,高字节在前
    u16Num = (((uint16_t)RTU_PORT.RXBuffer[4]) << 8) | RTU_PORT.RXBuffer[5];

    //装配数据:
    //从站地址
    RTU_PORT.TXBuffer[0] = ucNodeAddr;
    //功能码
    RTU_PORT.TXBuffer[1] = 0x03;

    //判断元件数量是否合理。
    //不能超过125个HR请求。
    if((u16Num >= 0x0001) && (u16Num <= 0x007D))
    {
        //判断元件基址和数量是否合适。
        //从站单元地址从0开始。(可以修改为1开始，可视情况定。)
        //提供的AI_3xxxx是16bit数组，所以一个单元是一个元件数据。
        if((u16DataAddr + u16Num) <= uc4xMaxLen)
        {
            uint8_t u8ByteNum;  //所需字节数量

            //打包数据帧
            //响应的数据字节数量:元件数量*2.
            //可以使用u16Num<<1,因为不能超过125，所以此处u16Num高8位是0.
            u8ByteNum = RTU_PORT.RXBuffer[5] << 1;
            RTU_PORT.TXBuffer[2] = u8ByteNum;
            //把需要响应的数据值赋值给RTU_PORT.TXBuffer[3]开始的单元。
            for(uint16_t i = 0; i < u8ByteNum; i += 2)
            {
                //16位AI资源数据分拆：高字节在前。
                RTU_PORT.TXBuffer[3 + i] = Source.us4xxxx[i/2 + u16DataAddr] >> 8; //16位数据高8位字节。
                RTU_PORT.TXBuffer[3 + i + 1] = Source.us4xxxx[i/2 + u16DataAddr];  //16位数据低8位字节。
            }

            RTU_PORT.usTXIndex = u8ByteNum + 3; //j为CRC所在单元。

        }
        else{
            //产生异常02：地址非法
            RTU_PORT.TXBuffer[1] = RTU_PORT.TXBuffer[1] + 0x80;
            RTU_PORT.TXBuffer[2] = 0x02;
            RTU_PORT.usTXIndex = 3;
        }
    }
    else{
        //产生异常03：数据非法
        RTU_PORT.TXBuffer[1] = RTU_PORT.TXBuffer[1] + 0x80;
        RTU_PORT.TXBuffer[2] = 0x03;
        RTU_PORT.usTXIndex = 3;
    }

    //生成CRC16。
    u16CRC = RTU_PORT.CRC16Gen();
    RTU_PORT.TXBuffer[RTU_PORT.usTXIndex++] = u16CRC;
    RTU_PORT.TXBuffer[RTU_PORT.usTXIndex++] = u16CRC >> 8;
    //发送打包后的帧
    RTU_PORT.SendFrame();
}

//Function:0x10
//写多个连续的保持寄存器HoldReg,4xxxx
void RTU_Slave::slaveFunc_0x10(void)
{
    uint16_t u16Num;  //元件数量
    uint16_t u16DataAddr;  //元件基址
    uint16_t u16CRC;
    uint8_t u8ByteNum;  //所需字节数量

    //获取元件基址及数量。
    //元件基址在第2、3字节，高字节在前。
    u16DataAddr = (((uint16_t)RTU_PORT.RXBuffer[2]) << 8) | RTU_PORT.RXBuffer[3];
    //数量在第4、5字节,高字节在前
    u16Num = (((uint16_t)RTU_PORT.RXBuffer[4]) << 8) | RTU_PORT.RXBuffer[5];

    //计算所需的字节量。虽然接收帧里面含有字节量(第6字节)，但仍然要比较是否正确。
    u8ByteNum = u16Num << 1;

    //装配数据:
    //从站地址
    RTU_PORT.TXBuffer[0] = ucNodeAddr;
    //功能码
    RTU_PORT.TXBuffer[1] = 0x10;

    //判断元件数量是否合理。
    //不能超过123个HR请求且请求的元件量和传入的字节量匹配。
    if((u16Num >= 0x0001) && (u16Num <= 0x007B) && (u8ByteNum == RTU_PORT.RXBuffer[6]))
    {
        //判断元件基址和数量是否合适。
        //从站单元地址从0开始。(可以修改为1开始，可视情况定。)
        if((u16DataAddr + u16Num) <= uc4xMaxLen)
        {
            //打包数据帧
            //元件基址
            RTU_PORT.TXBuffer[2] = RTU_PORT.RXBuffer[2];
            RTU_PORT.TXBuffer[3] = RTU_PORT.RXBuffer[3];
            //响应的HR元件数量.
            RTU_PORT.TXBuffer[4] = RTU_PORT.RXBuffer[4];
            RTU_PORT.TXBuffer[5] = RTU_PORT.RXBuffer[5];

            //把赋值的数据(从第7字节开始)赋值给HR_4xxxx相应的单元。
            for(uint16_t i = 0; i < u8ByteNum; i+=2)
            {
                Source.us4xxxx[i/2 + u16DataAddr] = RTU_PORT.RXBuffer[7 + i]; //高字节
                Source.us4xxxx[i/2 + u16DataAddr] <<= 8;
                Source.us4xxxx[i/2 + u16DataAddr] |= RTU_PORT.RXBuffer[7 + i + 1]; //低字节
            }

            RTU_PORT.usTXIndex = 6; //j为CRC所在单元。

        }
        else{
            //产生异常02：地址非法
            RTU_PORT.TXBuffer[1] = RTU_PORT.TXBuffer[1] + 0x80;
            RTU_PORT.TXBuffer[2] = 0x02;
            RTU_PORT.usTXIndex = 3;
        }
    }
    else{
        //产生异常03：数据非法
        RTU_PORT.TXBuffer[1] = RTU_PORT.TXBuffer[1] + 0x80;
        RTU_PORT.TXBuffer[2] = 0x03;
        RTU_PORT.usTXIndex = 3;
    }

    //生成CRC16。
    u16CRC = RTU_PORT.CRC16Gen();
    RTU_PORT.TXBuffer[RTU_PORT.usTXIndex++] = u16CRC;
    RTU_PORT.TXBuffer[RTU_PORT.usTXIndex++] = u16CRC >> 8;
    //发送打包后的帧
    RTU_PORT.SendFrame();
}

//异常01,请求的功能不支持。
void RTU_Slave::default_NonSupport(void)
{
    uint16_t u16CRC;
    //装配数据:
    //从站地址
    RTU_PORT.TXBuffer[0] = ucNodeAddr;
    //功能码
    RTU_PORT.TXBuffer[1] = RTU_PORT.RXBuffer[1] + 0x80;
    //异常码01
    RTU_PORT.TXBuffer[2] = 0x01;
    RTU_PORT.usTXIndex = 3;

    //生成CRC16。
    u16CRC = RTU_PORT.CRC16Gen();
    RTU_PORT.TXBuffer[RTU_PORT.usTXIndex++] = u16CRC;
    RTU_PORT.TXBuffer[RTU_PORT.usTXIndex++] = u16CRC >> 8;

    //发送打包后的帧
    RTU_PORT.SendFrame();
    //printf("This functiosn nonSupport!---%d\n",RTU_PORT.TXBuffer[1]);
}

//---------------------------------------------------------------------------




