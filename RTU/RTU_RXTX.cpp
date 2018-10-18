/*********************************************************
���ܣ� Modbus RTUͨѶ�ĵײ�ͨѶʵ��
������ ��485�˿ڵĳ�ʼ���Լ�ModbusͨѶ����Ҫ�ļ�����ʱ���ĳ�ʼ����
��ƣ� azjiao
�汾�� 0.1
���ڣ� 2018��10��08��
*********************************************************/
#include <stdio.h>
#include "dev.h"
#include "RTU_RXTX.h"

//����RTU_DataCtrl ʵ��
RTU_DataCtrl Neo_RTUPort;

//Port_RS485���캯�������˿ڳ�ʼ�����ݸ�ֵ��
Port_RS485::Port_RS485(u32 unBR, u16 usDB, u16 usSB, u16 usPt)
{
    setPortParam(unBR, usDB, usSB, usPt);
}

//�޸Ķ˿ں�.
void Port_RS485::setPortNo(u8 ucPNo)
{
    ucPortNo = ucPNo;
}

//�޸Ķ˿ڳ�ʼ������.
void Port_RS485::setPortParam(u32 unBR, u16 usDB, u16 usSB, u16 usPt)
{
    RS485Init.unBaudRate = unBR;  //������

    switch(usDB)  //����λ
    {
        case 8:
            RS485Init.usDataBit = USART_WordLength_8b;
            break;
        case 9:
            RS485Init.usDataBit = USART_WordLength_9b;
            break;
    }

    switch(usSB)  //ֹͣλ
    {
        case 1:
            RS485Init.usStopBit = USART_StopBits_1;
            break;
        case 2:
            RS485Init.usStopBit = USART_StopBits_2;
            break;
        default:  //һ����1��ֹͣλ��
            RS485Init.usStopBit = USART_StopBits_1;
    }

    switch(usPt)  //У��λ
    {
        case 0:
            RS485Init.usParity = USART_Parity_No;  //��У��
            break;
        case 1:
            RS485Init.usParity = USART_Parity_Odd; //��У��
            break;
        case 2:
            RS485Init.usParity = USART_Parity_Even;  //żУ��
            break;
    }
}

//����ͨѶ�˿�Ӳ����
void Port_RS485::initPort(void)
{
    //��Ӣ���485����USART2��,ֻ��Կ�����ʵ�֡�
    //USART2����PA2��PA3��Ϊ�շ��ڡ�(USART2��ȫ˫����)
    //RS485��ΧоƬ���շ�ʹ�ܶ�ʹ��PD7���ơ�(RS485��Ҫ��˫��ģʽ)
    if (ucPortNo == 2)
    {
        //ʹ��GPIOA��GPIOD����ʱ�ӡ�
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOD, ENABLE);
        //USART2ʱ��ʹ��
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

        //PA2��PA3��������
        //TX������PA2��,���������������
        My_GPIO_Init(GPIOA, GPIO_Pin_2, GPIO_Mode_AF_PP, GPIO_Speed_50MHz);
        //RX������PA3�ڣ�����������(Ƶ�ʲ���������ʱ����)��
        My_GPIO_Init(GPIOA, GPIO_Pin_3, GPIO_Mode_IN_FLOATING);
        RCC_APB1PeriphResetCmd(RCC_APB1Periph_USART2,ENABLE);//��λ����2
        RCC_APB1PeriphResetCmd(RCC_APB1Periph_USART2,DISABLE);//ֹͣ��λ

        //PD7���ã����ڿ���RS485��ƽת��оƬ�շ�ʹ�ܡ�����Ϊ�������.
        My_GPIO_Init(GPIOD, GPIO_Pin_7, GPIO_Mode_Out_PP, GPIO_Speed_50MHz);
    }
}

//����ͨѶ�˿�ͨѶ������
void Port_RS485::configCommParam(void)
{
    if (ucPortNo == 2)
    {
        //����485��ͨѶ�����Բ�����
        My_USART_Init(USART2, RS485Init.unBaudRate, RS485Init.usDataBit, RS485Init.usStopBit, RS485Init.usParity, USART_Mode_Rx | USART_Mode_Tx);

        //����USART2�ж����ȼ�
        My_NVIC_Init(USART2_IRQn, 2, 2, ENABLE);
        //ʹ�ܴ���2�����ж�
        USART_ITConfig(USART2, USART_IT_RXNE,ENABLE);
        //ʹ�ܴ���2
        USART_Cmd(USART2, ENABLE);
    }
}

//485�˿ڳ�ʼ��
void Port_RS485::RS485_Init(void)
{
    initPort();  //����ͨѶ�˿�Ӳ����
    configCommParam();  //485�˿ڳ�ʼ��.
}

//---------------------------------------------------------------------------
//Port_RTU���캯��
Port_RTU::Port_RTU(u32 unBR, u16 usDB, u16 usSB, u16 usPt) : \
            Port_RS485(unBR, usDB, usSB, usPt), \
            ucT15_35No(6), ucTrespNo(7)
{
    float fOneByteTime;  //һ���ֽڷ��͵�ʱ��(us)��
    u32 unBaudRate = getBaudRate();
    //���ݲ����ʼ���һ���ֽڷ��͵�ʱ�䡣
    if(unBaudRate <= 19200)
        fOneByteTime = (11/(float)unBaudRate) * 1000000;  //һ���ֽڷ��͵�ʱ��(us)��
    else
        fOneByteTime = 1750; //ֻ�в����ʴ���19200ʱ��ʹ�ù̶���ʱ��

    usT15_us = 1.5 * fOneByteTime;  //t1.5��ʱʱ��us.
    usT35_us = 3.5 * fOneByteTime;  //t3.5��ʱʱ��us.
    usTresponse_ms = usTimeOut;  //��ʱ��ʱֵ(500)ms.
    T15_35.setProperty(ucT15_35No, usT35_us, bUnitus);    //��t3.5������ȡ�����ֽ�����⡣
    Trespond.setProperty(ucTrespNo, usTresponse_ms, bUnitms);
}

//��̬��Ա��ʱ���ĳ�ʼ��
//ʹ��ȱʡ���캯����û�г�ʼ����ʱ�����ԡ�
BaseTimer Port_RTU::T15_35;
BaseTimer Port_RTU::Trespond;

//RTU�˿�ͨѶ��ʼ��.
void Port_RTU::portRTU_Init(void)
{
    //�˿�Ӳ����ʼ���Լ�ͨѶ���Բ�������.
    RS485_Init();

    //����T15_35,����Ʋ�����ֽ�����ֻ���֡������Ӧ��ʱ������T15_35��ʱ����t3.5��ʽ����,(���Ĳ���ȱʡ)�ݲ�������ʱ��.
 //   T15_35.Timer_Init(ucT15_35No, usT35_us, bUnitus);
    //FixME:�����Ѿ��ڹ��캯���г�ʼ������ʱ�����ԣ�����ʹ�����³�ʼ����ʱ����(�򻯳�ʼ����ֻ��һ�������������ã�������ν�Ĵ���)
    T15_35.timer_Init(bTimerStop);
    //����Trespond,(���Ĳ���ȱʡ)�ݲ�������ʱ��.
 //   Trespond.Timer_Init(ucTrespNo, usTresponse_ms, bUnitms);
    //FixME:�����Ѿ��ڹ��캯���г�ʼ������ʱ�����ԣ�����ʹ�����³�ʼ����ʱ����
    Trespond.timer_Init(bTimerStop);

    //Ĭ��Ϊ����ģʽ
    RS485_RX();
}

//------------------------------------------------------------------------------------------------
//RTU_DataCtrl���캯��
RTU_DataCtrl::RTU_DataCtrl(u32 unBR, u16 usDB, u16 usSB, u16 usPt) : \
                   Port_RTU(unBR, usDB,usSB, usPt)
{
    //������ݻ�����.
    usRXIndex = usTXIndex = 0;
}


//CRC16У��������
//��У������Ϊ���ͻ���������.
//����ԭ��16λCRC�����ֽ���ǰ��
u16 RTU_DataCtrl::CRC16Gen(u8* ucPtr, u16 usLen)
{
    u16 CRC16 = 0xFFFF;  //CRCԤ��ֵ
    for(u16 i = 0; i < usLen; i++)  //���δ����������ݡ�
    {
        CRC16 ^= *(ucPtr + i);   //�������

        for(u8 bit = 0; bit < 8; bit++)
        {
            //��������Ƴ�λΪ1�������ƺ�������ʽ��
            if(CRC16 & 0x0001)
            {
                CRC16 >>= 1;
                CRC16 ^= 0xA001;  //У�����ʽ��CRC16_Modbus��׼��
            }
            //����ֻ������һλ��
            else{
                CRC16 >>= 1;
            }
        }
    }
    return (CRC16);
}

//ʹ�ý��ջ�����������Ϊ����У���������,����Ĭ�ϵķ�ʽ.
u16 RTU_DataCtrl::CRC16Gen(void)
{
    return CRC16Gen(TXBuffer, usTXIndex);
}

//CRC16У��,���У����ȷ����true,���򷵻�false.
bool RTU_DataCtrl::CRC16Check(u8* ucPtr, u16 usLen)
{
    //ʹ�����°Ѻ�CRC��������У�飬������ɵ�CRCΪ0x00����ȷ��������ȷ��
    if(CRC16Gen(ucPtr, usLen) == (u16)0)
        return true;
    else
        return false;
}

//CRC16У��
//��У������Ϊ���ջ���������,����Ĭ�ϵķ�ʽ.
bool RTU_DataCtrl::CRC16Check(void)
{
    return CRC16Check(RXBuffer, usRXIndex);
}


//��������֡
void RTU_DataCtrl::ReceiveFrame(void)
{
    //��ս��ջ�����.
    resetRXBuffer();
    //��λ״̬�����ڽ���.
    resetStatus4RX();
    //���ö˿ڴ��ڽ���ʹ��״̬.
    RS485_RX();
}

//��������֡
void RTU_DataCtrl::SendFrame(void)
{
    //��λ״̬�����ڷ���.
    resetStatus4TX();
    //���ö˿ڴ��ڷ���ʹ��״̬.
    RS485_TX();

    for(u16 i = 0; i < usTXIndex; i++)
    {
        USART_SendData(USART2,TXBuffer[i]);
        LED0_ON;
        while(USART_GetFlagStatus(USART2,USART_FLAG_TXE) != SET);
    }
    //�ȴ�ȫ���������ݷ�����ϡ�
    while(USART_GetFlagStatus(USART2, USART_FLAG_TC) != SET);
    //֡����ʱ������t3.5��
    //��ʱ������ʹbBusy��λ��
    //T15_35.timer_ResetONOFF(bTimerStart);
    timeFrameEnd_Start();
    //�ȴ�֡���������
    while(portStatus.bBusy);
    LED0_OFF;
}



