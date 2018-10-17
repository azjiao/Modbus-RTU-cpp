/*********************************************************
���ܣ� Modbus-RTU��վͨѶЭ��
������ ʵ���˴�վͨѶ�ļ���������
��ƣ� azjiao
�汾�� 0.1
���ڣ� 2018��10��10��
*********************************************************/
#include "RTU_Slave.h"

//RTU_Slave���캯��
RTU_Slave::RTU_Slave(RTU_DataCtrl* ptr, u8 ucAddr) : ucNodeAddr(ucAddr)
{
    pRTUPort = ptr;
};

//��վ������
void RTU_Slave::slaveService(void)
{
    //������յ�������֡.
    if(RTU_PORT.modbusStatus.bReadEnb)
    {
        //����޴������
        //��֡�ɶ�ʱ��ʵbErrΪfalse������ȡ���жϡ�
        if(!RTU_PORT.modbusStatus.bErr)
        {
            //�����жϴ�վ��ַ�Ƿ������
            //վ��ַ���������,�������ա�
            if(RTU_PORT.RXBuffer[0] != ucNodeAddr)
            {
                RTU_PORT.ReceiveFrame();
                return;
            }

            //��ȡ���������ж�
            switch(RTU_PORT.RXBuffer[1])
            {
                case slaveFunc0x01:slaveFunc_0x01();  //�����DQ_0xxxx
                          break;
                case slaveFunc0x0F:slaveFunc_0x0F();  //д���DQ_0xxxx
                          break;
                case slaveFunc0x05:slaveFunc_0x05();  //д����DQ_0xxxx
                          break;
                case slaveFunc0x02:slaveFunc_0x02();  //�����DI_1xxxx
                          break;
                case slaveFunc0x04:slaveFunc_0x04();  //�����AI_3xxxx
                          break;
                case slaveFunc0x03:slaveFunc_0x03();  //�����HoldReg_4xxxx
                          break;
                case slaveFunc0x10:slaveFunc_0x10();  //д���HoldReg_4xxxx
                          break;
                default:  default_NonSupport();  //��֧�ֵĹ��ܴ���

            }
            //������ת����ա�
            RTU_PORT.ReceiveFrame();
        }
    }
}

//Function:0x01
//��ȡ���������DQ��0xxxx
//ִ�к���ʱ��վ��ַ�Ѿ������
void RTU_Slave::slaveFunc_0x01(void)
{
    uint16_t u16Num;  //Ԫ������
    uint16_t u16DataAddr;  //Ԫ����ַ
    uint16_t u16CRC;
    uint16_t u16ByteIndex;  //��ַ�����ֽ���������0��ʼ���ֽ�������
    uint8_t u8BitIndex;  //��ַ�����ֽڵĿ�ʼλ��������0��ʼ��λ������

    //��ȡԪ����ַ��������
    //Ԫ����ַ�ڵ�2��3�ֽڣ����ֽ���ǰ��
    u16DataAddr = (((uint16_t)RTU_PORT.RXBuffer[2]) << 8) | RTU_PORT.RXBuffer[3];
    //�����ڵ�4��5�ֽ�,���ֽ���ǰ
    u16Num = (((uint16_t)RTU_PORT.RXBuffer[4]) << 8) | RTU_PORT.RXBuffer[5];

    //װ������:
    //��վ��ַ
    RTU_PORT.TXBuffer[0] = ucNodeAddr;
    //������
    RTU_PORT.TXBuffer[1] = 0x01;

    //�ж�Ԫ�������Ƿ����
    //���ܳ���2000(0x07D0)��DQ����
    if((u16Num >= 0x0001) && (u16Num <= 0x07D0))
    {
        //�ж�Ԫ����ַ�������Ƿ���ʡ�
        //��վ��Ԫ��ַ��0��ʼ��(�����޸�Ϊ1��ʼ�������������)
        if((u16DataAddr + u16Num) <= (uc0xMaxLen * 8))
        {
            uint8_t u8ByteNum;  //�����ֽ�����

            //�������֡
            //��Ӧ�������ֽ�����
            u8ByteNum = (u16Num%8)? (u16Num/8+1):u16Num/8;
            RTU_PORT.TXBuffer[2] = u8ByteNum;

            //��λu16DataAddr�����ֽ�������
            u16ByteIndex = u16DataAddr/8;
            //��λ�����ֽڵĿ�ʼλ������
            u8BitIndex = u16DataAddr%8;

            uint16_t u16DQ_Index = u16ByteIndex;  //DQ�ֽڿ�ʼ������
            uint8_t u8DQBit_Index = u8BitIndex;  //DQ�ֽ�λ������ʼֵ��
            uint8_t u8DQMask = 0x01 << u8DQBit_Index;  //DQ�ֽڳ�ʼ����.
            uint16_t u16Tx_Index = 0;  //�����ֽ�������ʼֵ��
            uint8_t u8TxBit_Index = 0; //�����ֽ�λ������ʼֵ.
            uint8_t u8TxMask = 0x01;  //TX�ֽڳ�ʼ���롣

            //����Ҫ��Ӧ������ֵ��ֵ��TXBuffer[3]��ʼ�ĵ�Ԫ��
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
            //���һ��TX�����ֽڸ�λ���0
            for(; u8TxBit_Index < 8; u8TxBit_Index++)
            {
                RTU_PORT.TXBuffer[u16Tx_Index + 3] &= (u8TxMask ^ 0xFF);
                u8TxMask <<= 1;
            }

            RTU_PORT.usTXIndex = u8ByteNum + 3; //jΪCRC���ڵ�Ԫ��

        }
        else{
            //�����쳣02����ַ�Ƿ�
            RTU_PORT.TXBuffer[1] |= 0x80;
            RTU_PORT.TXBuffer[2] = 0x02;
            RTU_PORT.usTXIndex = 3;
        }
    }
    else{
        //�����쳣03�����ݷǷ�
        RTU_PORT.TXBuffer[1] |= 0x80;
        RTU_PORT.TXBuffer[2] = 0x03;
        RTU_PORT.usTXIndex = 3;
    }

    //����CRC16��
    u16CRC = RTU_PORT.CRC16Gen();
    RTU_PORT.TXBuffer[RTU_PORT.usTXIndex++] = u16CRC;
    RTU_PORT.TXBuffer[RTU_PORT.usTXIndex++] = u16CRC >> 8;
    //���ʹ�����֡
    RTU_PORT.SendFrame();
}

//Function:0x0F
//ǿ�ƶ��������DQ��0xxxx
void RTU_Slave::slaveFunc_0x0F(void)
{
    uint16_t u16Num;  //Ԫ������
    uint16_t u16DataAddr;  //Ԫ����ַ
    uint16_t u16CRC;
//    uint16_t j; //CRCװ�ص�Ԫ������
    uint8_t u8ByteNum;  //�����ֽ�����
    uint16_t u16ByteIndex;  //��ַ�����ֽ���������0��ʼ���ֽ�������
    uint8_t u8BitIndex;  //��ַ�����ֽڵĿ�ʼλ��������0��ʼ��λ������

    //��ȡԪ����ַ��������
    //Ԫ����ַ�ڵ�2��3�ֽڣ����ֽ���ǰ��
    u16DataAddr = (((uint16_t)RTU_PORT.RXBuffer[2]) << 8) | RTU_PORT.RXBuffer[3];
    //�����ڵ�4��5�ֽ�,���ֽ���ǰ
    u16Num = (((uint16_t)RTU_PORT.RXBuffer[4]) << 8) | RTU_PORT.RXBuffer[5];

    //����������ֽ�������Ȼ����֡���溬���ֽ���(��6�ֽ�)������ȻҪ�Ƚ��Ƿ���ȷ��
    u8ByteNum = (u16Num%8)? (u16Num/8+1):u16Num/8;

    //װ������:
    //��վ��ַ
    RTU_PORT.TXBuffer[0] = ucNodeAddr;
    //������
    RTU_PORT.TXBuffer[1] = 0x0F;

    //�ж�Ԫ�������Ƿ����
    //���ܳ���2000��DQ�����������Ԫ�����ʹ�����ֽ���ƥ�䡣
    if((u16Num >= 0x0001) && (u16Num <= 0x07D0) && (u8ByteNum == RTU_PORT.RXBuffer[6]))
    {
        //�ж�Ԫ����ַ�������Ƿ���ʡ�
        //��վ��Ԫ��ַ��0��ʼ��(�����޸�Ϊ1��ʼ�������������)
        if((u16DataAddr + u16Num) <= uc0xMaxLen * 8)
        {
            //�������֡
            //Ԫ����ַ
            RTU_PORT.TXBuffer[2] = RTU_PORT.RXBuffer[2];
            RTU_PORT.TXBuffer[3] = RTU_PORT.RXBuffer[3];
            //��Ӧ��DQԪ������.
            RTU_PORT.TXBuffer[4] = RTU_PORT.RXBuffer[4];
            RTU_PORT.TXBuffer[5] = RTU_PORT.RXBuffer[5];

            //��λu16DataAddr�����ֽ�������
            u16ByteIndex = u16DataAddr/8;
            //��λ�����ֽڵĿ�ʼλ������
            u8BitIndex = u16DataAddr%8;

            uint16_t u16DQ_Index = u16ByteIndex;  //DQ�ֽڿ�ʼ������
            uint8_t u8DQBit_Index = u8BitIndex;  //DQ�ֽ�λ������ʼֵ��
            uint8_t u8DQMask = 0x01 << u8DQBit_Index;  //DQ�ֽڳ�ʼ����.
            uint16_t u16Rx_Index = 7;  //�����ֽ�������ʼֵ��
            uint8_t u8RxBit_Index = 0; //�����ֽ�λ������ʼֵ.
            uint8_t u8RxMask = 0x01;  //����RX�ֽڳ�ʼ���롣

            //����Ҫǿ�Ƶ�����ֵ(�ӵ�7�ֽڿ�ʼ)��ֵ��DQ_0xxxx��Ӧ�ĵ�Ԫ��
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
            RTU_PORT.usTXIndex = 6; //jΪCRC���ڵ�Ԫ��
        }
        else{
            //�����쳣02����ַ�Ƿ�
            RTU_PORT.TXBuffer[1] |= 0x80;
            RTU_PORT.TXBuffer[2] = 0x02;
            RTU_PORT.usTXIndex = 3;
        }
    }
    else{
        //�����쳣03�����ݷǷ�
        RTU_PORT.TXBuffer[1] |= 0x80;
        RTU_PORT.TXBuffer[2] = 0x03;
        RTU_PORT.usTXIndex = 3;
    }

    //����CRC16��
    u16CRC = RTU_PORT.CRC16Gen();
    RTU_PORT.TXBuffer[RTU_PORT.usTXIndex++] = u16CRC;
    RTU_PORT.TXBuffer[RTU_PORT.usTXIndex++] = u16CRC >> 8;

    //���ʹ�����֡
    RTU_PORT.SendFrame();
}

//Function:0x05
//ǿ�Ƶ���DQ,0x0000
//����Ϊ2���ֽڣ�ֻ��0xFF00��0x0000��Ч����ӦON��OFF.
void RTU_Slave::slaveFunc_0x05(void)
{
    uint16_t u16DataAddr;  //Ԫ����ַ
    uint16_t u16Data;  //ǿ������ֵ��
    uint16_t u16CRC;

    uint16_t u16ByteIndex;  //��ַ�����ֽ���������0��ʼ���ֽ�������
    uint8_t u8BitIndex;  //��ַ�����ֽڵĿ�ʼλ��������0��ʼ��λ������

    //��ȡԪ����ַ��������
    //Ԫ����ַ�ڵ�2��3�ֽڣ����ֽ���ǰ��
    u16DataAddr = (((uint16_t)RTU_PORT.RXBuffer[2]) << 8) | RTU_PORT.RXBuffer[3];

    //װ������:
    //��վ��ַ
    RTU_PORT.TXBuffer[0] = ucNodeAddr;
    //������
    RTU_PORT.TXBuffer[1] = 0x05;

    //�ж�ǿ������ֵ�Ƿ���ȷ��ֻ��0x0000��0xFF00��Ч��
    //����ֵ�ڵ�4��5�ֽ�,���ֽ���ǰ��
    u16Data = (((uint16_t)RTU_PORT.RXBuffer[4] << 8) | RTU_PORT.RXBuffer[5]);
    if(u16Data == 0x0000 | u16Data == 0xFF00)
    {
        //�ж�Ԫ����ַ�Ƿ���ʡ�
        //��վ��Ԫ��ַ��0��ʼ��
        if(u16DataAddr <= uc0xMaxLen * 8)
        {
            //�������֡
            //Ԫ����ַ
            RTU_PORT.TXBuffer[2] = RTU_PORT.RXBuffer[2];
            RTU_PORT.TXBuffer[3] = RTU_PORT.RXBuffer[3];
            //ǿ��ֵ
            RTU_PORT.TXBuffer[4] = RTU_PORT.RXBuffer[4];
            RTU_PORT.TXBuffer[5] = RTU_PORT.RXBuffer[5];

            //��λu16DataAddr�����ֽ�������
            u16ByteIndex = u16DataAddr/8;
            //��λ�����ֽڵĿ�ʼλ������
            u8BitIndex = u16DataAddr%8;
            uint8_t u8DQMask = 0x01 << u8BitIndex;  //DQ�ֽ�����.

            //��DQ��Ӧ�ĵ�Ԫдǿ��ֵ��
            if(u16Data == 0xFF00)
                Source.uc0xxxx[u16ByteIndex] |= u8DQMask;
            else
                Source.uc0xxxx[u16ByteIndex] &= (u8DQMask ^ 0xFF);

            RTU_PORT.usTXIndex = 6; //jΪCRC���ڵ�Ԫ��
        }
        else{
            //�����쳣02����ַ�Ƿ�
            RTU_PORT.TXBuffer[1] |= 0x80;
            RTU_PORT.TXBuffer[2] = 0x02;
            RTU_PORT.usTXIndex = 3;
        }
    }
    else{
        //�����쳣03�����ݷǷ�
        RTU_PORT.TXBuffer[1] |= 0x80;
        RTU_PORT.TXBuffer[2] = 0x03;
        RTU_PORT.usTXIndex = 3;
    }

    //����CRC16��
    u16CRC = RTU_PORT.CRC16Gen();
    RTU_PORT.TXBuffer[RTU_PORT.usTXIndex++] = u16CRC;
    RTU_PORT.TXBuffer[RTU_PORT.usTXIndex++] = u16CRC >> 8;
    //���ʹ�����֡
    RTU_PORT.SendFrame();
}

//Function:0x02
//��ȡ���������������ɢ��DI,1xxxx
void RTU_Slave::slaveFunc_0x02(void)
{
    uint16_t u16Num;  //Ԫ������
    uint16_t u16DataAddr;  //Ԫ����ַ
    uint16_t u16CRC;
    uint16_t u16ByteIndex;  //��ַ�����ֽ���������0��ʼ���ֽ�������
    uint8_t u8BitIndex;  //��ַ�����ֽڵĿ�ʼλ��������0��ʼ��λ������

    //��ȡԪ����ַ��������
    //Ԫ����ַ�ڵ�2��3�ֽڣ����ֽ���ǰ��
    u16DataAddr = (((uint16_t)RTU_PORT.RXBuffer[2]) << 8) | RTU_PORT.RXBuffer[3];
    //�����ڵ�4��5�ֽ�,���ֽ���ǰ
    u16Num = (((uint16_t)RTU_PORT.RXBuffer[4]) << 8) | RTU_PORT.RXBuffer[5];

    //װ������:
    //��վ��ַ
    RTU_PORT.TXBuffer[0] = ucNodeAddr;
    //������
    RTU_PORT.TXBuffer[1] = 0x02;

    //�ж�Ԫ�������Ƿ����
    //���ܳ���2000(0x07D0)��DQ����
    if((u16Num >= 0x0001) && (u16Num <= 0x07D0))
    {
        //�ж�Ԫ����ַ�������Ƿ���ʡ�
        //��վ��Ԫ��ַ��0��ʼ��(�����޸�Ϊ1��ʼ�������������)
        if((u16DataAddr + u16Num) <= uc1xMaxLen * 8)
        {
            uint8_t u8ByteNum;  //�����ֽ�����

            //�������֡
            //��Ӧ�������ֽ�����
            u8ByteNum = (u16Num%8)? (u16Num/8+1):u16Num/8;
            RTU_PORT.TXBuffer[2] = u8ByteNum;

            //��λu16DataAddr�����ֽ�������
            u16ByteIndex = u16DataAddr/8;
            //��λ�����ֽڵĿ�ʼλ������
            u8BitIndex = u16DataAddr%8;

            uint16_t u16DI_Index = u16ByteIndex;  //DI�ֽڿ�ʼ������
            uint8_t u8DIBit_Index = u8BitIndex;  //DI�ֽ�λ������ʼֵ��
            uint8_t u8DIMask = 0x01 << u8DIBit_Index;  //DI�ֽڳ�ʼ����.
            uint16_t u16Tx_Index = 0;  //�����ֽ�������ʼֵ��
            uint8_t u8TxBit_Index = 0; //�����ֽ�λ������ʼֵ.
            uint8_t u8TxMask = 0x01;  //TX�ֽڳ�ʼ���롣

            //����Ҫ��Ӧ������ֵ��ֵ��RTU_PORT.TXBuffer[3]��ʼ�ĵ�Ԫ��
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
            //���һ��TX�����ֽڸ�λ���0
            for(; u8TxBit_Index < 8; u8TxBit_Index++)
            {
                RTU_PORT.TXBuffer[u16Tx_Index + 3] &= (u8TxMask ^ 0xFF);
                u8TxMask <<= 1;
            }

            RTU_PORT.usTXIndex = u8ByteNum + 3; //jΪCRC���ڵ�Ԫ��
        }
        else{
            //�����쳣02����ַ�Ƿ�
            RTU_PORT.TXBuffer[1] |= 0x80;
            RTU_PORT.TXBuffer[2] = 0x02;
            RTU_PORT.usTXIndex = 3;
        }
    }
    else{
        //�����쳣03�����ݷǷ�
        RTU_PORT.TXBuffer[1] = RTU_PORT.TXBuffer[1] + 0x80;
        RTU_PORT.TXBuffer[2] = 0x03;
        RTU_PORT.usTXIndex = 3;
    }

    //����CRC16��
    u16CRC = RTU_PORT.CRC16Gen();
    RTU_PORT.TXBuffer[RTU_PORT.usTXIndex++] = u16CRC;
    RTU_PORT.TXBuffer[RTU_PORT.usTXIndex++] = u16CRC >> 8;
    //���ʹ�����֡
    RTU_PORT.SendFrame();
}


//Function:0x04
//��ȡ�������������Ĵ���AI,3xxxx
void RTU_Slave::slaveFunc_0x04(void)
{
    uint16_t u16Num;  //Ԫ������
    uint16_t u16DataAddr;  //Ԫ����ַ
    uint16_t u16CRC;

    //��ȡԪ����ַ��������
    //Ԫ����ַ�ڵ�2��3�ֽڣ����ֽ���ǰ��
    u16DataAddr = (((uint16_t)RTU_PORT.RXBuffer[2]) << 8) | RTU_PORT.RXBuffer[3];
    //�����ڵ�4��5�ֽ�,���ֽ���ǰ.
    u16Num = (((uint16_t)RTU_PORT.RXBuffer[4]) << 8) | RTU_PORT.RXBuffer[5];

    //װ������:
    //��վ��ַ
    RTU_PORT.TXBuffer[0] = ucNodeAddr;
    //������
    RTU_PORT.TXBuffer[1] = 0x04;

    //�ж�Ԫ�������Ƿ����
    //���ܳ���125��AI����
    if((u16Num >= 0x0001) && (u16Num <= 0x007D))
    {
        //�ж�Ԫ����ַ�������Ƿ���ʡ�
        //��վ��Ԫ��ַ��0��ʼ��(�����޸�Ϊ1��ʼ�������������)
        //�ṩ��AI_3xxxx��16bit���飬����һ����Ԫ��һ��Ԫ�����ݡ�
        if((u16DataAddr + u16Num) <= uc3xMaxLen)
        {
            uint8_t u8ByteNum;  //�����ֽ�����

            //�������֡
            //��Ӧ�������ֽ�����:Ԫ������*2.
            //����ʹ��u16Num<<1,��Ϊ���ܳ���125�����Դ˴�u16Num��8λ��0.
            u8ByteNum = RTU_PORT.RXBuffer[5] << 1;
            RTU_PORT.TXBuffer[2] = u8ByteNum;
            //����Ҫ��Ӧ������ֵ��ֵ��RTU_PORT.TXBuffer[3]��ʼ�ĵ�Ԫ��
            for(uint16_t i = 0; i < u8ByteNum; i += 2)
            {
                //16λAI��Դ���ݷֲ𣺸��ֽ���ǰ��
                RTU_PORT.TXBuffer[3 + i] = Source.us3xxxx[i/2 + u16DataAddr] >> 8; //16λ���ݸ�8λ�ֽڡ�
                RTU_PORT.TXBuffer[3 + i + 1] = Source.us3xxxx[i/2 + u16DataAddr];  //16λ���ݵ�8λ�ֽڡ�
            }

            RTU_PORT.usTXIndex = u8ByteNum + 3; //jΪCRC���ڵ�Ԫ��
        }
        else{
            //�����쳣02����ַ�Ƿ�
            RTU_PORT.TXBuffer[1] = RTU_PORT.TXBuffer[1] + 0x80;
            RTU_PORT.TXBuffer[2] = 0x02;
            RTU_PORT.usTXIndex = 3;
        }
    }
    else{
        //�����쳣03�����ݷǷ�
        RTU_PORT.TXBuffer[1] = RTU_PORT.TXBuffer[1] + 0x80;
        RTU_PORT.TXBuffer[2] = 0x03;
        RTU_PORT.usTXIndex = 3;
    }

    //����CRC16��
    u16CRC = RTU_PORT.CRC16Gen();
    RTU_PORT.TXBuffer[RTU_PORT.usTXIndex++] = u16CRC;
    RTU_PORT.TXBuffer[RTU_PORT.usTXIndex++] = u16CRC >> 8;
    //���ʹ�����֡
    RTU_PORT.SendFrame();

}

//Function:0x03
//��ȡ��������ı��ּĴ���HoldReg,4xxxx
void RTU_Slave::slaveFunc_0x03(void)
{
    uint16_t u16Num;  //Ԫ������
    uint16_t u16DataAddr;  //Ԫ����ַ
    uint16_t u16CRC;

    //��ȡԪ����ַ��������
    //Ԫ����ַ�ڵ�2��3�ֽڣ����ֽ���ǰ��
    u16DataAddr = (((uint16_t)RTU_PORT.RXBuffer[2]) << 8) | RTU_PORT.RXBuffer[3];
    //�����ڵ�4��5�ֽ�,���ֽ���ǰ
    u16Num = (((uint16_t)RTU_PORT.RXBuffer[4]) << 8) | RTU_PORT.RXBuffer[5];

    //װ������:
    //��վ��ַ
    RTU_PORT.TXBuffer[0] = ucNodeAddr;
    //������
    RTU_PORT.TXBuffer[1] = 0x03;

    //�ж�Ԫ�������Ƿ����
    //���ܳ���125��HR����
    if((u16Num >= 0x0001) && (u16Num <= 0x007D))
    {
        //�ж�Ԫ����ַ�������Ƿ���ʡ�
        //��վ��Ԫ��ַ��0��ʼ��(�����޸�Ϊ1��ʼ�������������)
        //�ṩ��AI_3xxxx��16bit���飬����һ����Ԫ��һ��Ԫ�����ݡ�
        if((u16DataAddr + u16Num) <= uc4xMaxLen)
        {
            uint8_t u8ByteNum;  //�����ֽ�����

            //�������֡
            //��Ӧ�������ֽ�����:Ԫ������*2.
            //����ʹ��u16Num<<1,��Ϊ���ܳ���125�����Դ˴�u16Num��8λ��0.
            u8ByteNum = RTU_PORT.RXBuffer[5] << 1;
            RTU_PORT.TXBuffer[2] = u8ByteNum;
            //����Ҫ��Ӧ������ֵ��ֵ��RTU_PORT.TXBuffer[3]��ʼ�ĵ�Ԫ��
            for(uint16_t i = 0; i < u8ByteNum; i += 2)
            {
                //16λAI��Դ���ݷֲ𣺸��ֽ���ǰ��
                RTU_PORT.TXBuffer[3 + i] = Source.us4xxxx[i/2 + u16DataAddr] >> 8; //16λ���ݸ�8λ�ֽڡ�
                RTU_PORT.TXBuffer[3 + i + 1] = Source.us4xxxx[i/2 + u16DataAddr];  //16λ���ݵ�8λ�ֽڡ�
            }

            RTU_PORT.usTXIndex = u8ByteNum + 3; //jΪCRC���ڵ�Ԫ��

        }
        else{
            //�����쳣02����ַ�Ƿ�
            RTU_PORT.TXBuffer[1] = RTU_PORT.TXBuffer[1] + 0x80;
            RTU_PORT.TXBuffer[2] = 0x02;
            RTU_PORT.usTXIndex = 3;
        }
    }
    else{
        //�����쳣03�����ݷǷ�
        RTU_PORT.TXBuffer[1] = RTU_PORT.TXBuffer[1] + 0x80;
        RTU_PORT.TXBuffer[2] = 0x03;
        RTU_PORT.usTXIndex = 3;
    }

    //����CRC16��
    u16CRC = RTU_PORT.CRC16Gen();
    RTU_PORT.TXBuffer[RTU_PORT.usTXIndex++] = u16CRC;
    RTU_PORT.TXBuffer[RTU_PORT.usTXIndex++] = u16CRC >> 8;
    //���ʹ�����֡
    RTU_PORT.SendFrame();
}

//Function:0x10
//д��������ı��ּĴ���HoldReg,4xxxx
void RTU_Slave::slaveFunc_0x10(void)
{
    uint16_t u16Num;  //Ԫ������
    uint16_t u16DataAddr;  //Ԫ����ַ
    uint16_t u16CRC;
    uint8_t u8ByteNum;  //�����ֽ�����

    //��ȡԪ����ַ��������
    //Ԫ����ַ�ڵ�2��3�ֽڣ����ֽ���ǰ��
    u16DataAddr = (((uint16_t)RTU_PORT.RXBuffer[2]) << 8) | RTU_PORT.RXBuffer[3];
    //�����ڵ�4��5�ֽ�,���ֽ���ǰ
    u16Num = (((uint16_t)RTU_PORT.RXBuffer[4]) << 8) | RTU_PORT.RXBuffer[5];

    //����������ֽ�������Ȼ����֡���溬���ֽ���(��6�ֽ�)������ȻҪ�Ƚ��Ƿ���ȷ��
    u8ByteNum = u16Num << 1;

    //װ������:
    //��վ��ַ
    RTU_PORT.TXBuffer[0] = ucNodeAddr;
    //������
    RTU_PORT.TXBuffer[1] = 0x10;

    //�ж�Ԫ�������Ƿ����
    //���ܳ���123��HR�����������Ԫ�����ʹ�����ֽ���ƥ�䡣
    if((u16Num >= 0x0001) && (u16Num <= 0x007B) && (u8ByteNum == RTU_PORT.RXBuffer[6]))
    {
        //�ж�Ԫ����ַ�������Ƿ���ʡ�
        //��վ��Ԫ��ַ��0��ʼ��(�����޸�Ϊ1��ʼ�������������)
        if((u16DataAddr + u16Num) <= uc4xMaxLen)
        {
            //�������֡
            //Ԫ����ַ
            RTU_PORT.TXBuffer[2] = RTU_PORT.RXBuffer[2];
            RTU_PORT.TXBuffer[3] = RTU_PORT.RXBuffer[3];
            //��Ӧ��HRԪ������.
            RTU_PORT.TXBuffer[4] = RTU_PORT.RXBuffer[4];
            RTU_PORT.TXBuffer[5] = RTU_PORT.RXBuffer[5];

            //�Ѹ�ֵ������(�ӵ�7�ֽڿ�ʼ)��ֵ��HR_4xxxx��Ӧ�ĵ�Ԫ��
            for(uint16_t i = 0; i < u8ByteNum; i+=2)
            {
                Source.us4xxxx[i/2 + u16DataAddr] = RTU_PORT.RXBuffer[7 + i]; //���ֽ�
                Source.us4xxxx[i/2 + u16DataAddr] <<= 8;
                Source.us4xxxx[i/2 + u16DataAddr] |= RTU_PORT.RXBuffer[7 + i + 1]; //���ֽ�
            }

            RTU_PORT.usTXIndex = 6; //jΪCRC���ڵ�Ԫ��

        }
        else{
            //�����쳣02����ַ�Ƿ�
            RTU_PORT.TXBuffer[1] = RTU_PORT.TXBuffer[1] + 0x80;
            RTU_PORT.TXBuffer[2] = 0x02;
            RTU_PORT.usTXIndex = 3;
        }
    }
    else{
        //�����쳣03�����ݷǷ�
        RTU_PORT.TXBuffer[1] = RTU_PORT.TXBuffer[1] + 0x80;
        RTU_PORT.TXBuffer[2] = 0x03;
        RTU_PORT.usTXIndex = 3;
    }

    //����CRC16��
    u16CRC = RTU_PORT.CRC16Gen();
    RTU_PORT.TXBuffer[RTU_PORT.usTXIndex++] = u16CRC;
    RTU_PORT.TXBuffer[RTU_PORT.usTXIndex++] = u16CRC >> 8;
    //���ʹ�����֡
    RTU_PORT.SendFrame();
}

//�쳣01,����Ĺ��ܲ�֧�֡�
void RTU_Slave::default_NonSupport(void)
{
    uint16_t u16CRC;
    //װ������:
    //��վ��ַ
    RTU_PORT.TXBuffer[0] = ucNodeAddr;
    //������
    RTU_PORT.TXBuffer[1] = RTU_PORT.RXBuffer[1] + 0x80;
    //�쳣��01
    RTU_PORT.TXBuffer[2] = 0x01;
    RTU_PORT.usTXIndex = 3;

    //����CRC16��
    u16CRC = RTU_PORT.CRC16Gen();
    RTU_PORT.TXBuffer[RTU_PORT.usTXIndex++] = u16CRC;
    RTU_PORT.TXBuffer[RTU_PORT.usTXIndex++] = u16CRC >> 8;

    //���ʹ�����֡
    RTU_PORT.SendFrame();
    //printf("This functiosn nonSupport!---%d\n",RTU_PORT.TXBuffer[1]);
}

//---------------------------------------------------------------------------




