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
        My_NVIC_Init(USART2_IRQn, 1, 1, ENABLE);
        //ʹ�ܴ���2�����ж�
        USART_ITConfig(USART2, USART_IT_RXNE,ENABLE);
        //ʹ�ܴ���2�����ж�
        //USART_ITConfig(USART2, USART_IT_IDLE,ENABLE);
        
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
//ucT15_35No ��ucTrespNoʹ��ͬһ����ʱ��TIM6,���ɱ䡣
Port_RTU::Port_RTU(u32 unBR, u16 usDB, u16 usSB, u16 usPt) : \
        Port_RS485(unBR, usDB, usSB, usPt), \
        ucT15_35No(6), ucTrespNo(6)
{
    float fOneByteTime;  //һ���ֽڷ��͵�ʱ��(us)��
    u32 unBaudRate = getBaudRate();
    //���ݲ����ʼ���һ���ֽڷ��͵�ʱ�䡣
    if(unBaudRate <= 19200)
        fOneByteTime = (11/(float)unBaudRate) * 1000000;  //һ���ֽڷ��͵�ʱ��(us)��
    else
        fOneByteTime = 1750; //ֻ�в����ʴ���19200ʱ��ʹ�ù̶���ʱ��

    //usT15_us = 1.5 * fOneByteTime;  //t1.5��ʱʱ��us.
    unT35_us = 3.5 * fOneByteTime;  //t3.5��ʱʱ��us.
    unTresponse_us = TIMEOUTVAL * 1000;  //��ʱ��ʱֵ(500)ms.

    T15_35.setProperty(ucT15_35No, unT35_us, bUNITUS, 0, 3);    //��t3.5������ȡ�����ֽ�����⡣
    //֡�����ͽ��ճ�ʱ���ʹ��ͬһ����ʱ��,ֻ��Trespond���Խ��г�ʼ������Ϊͬһ���ȼ���
    Trespond.setProperty(ucTrespNo, unTresponse_us, bUNITUS, 0, 3); //��ʱ����֡�������ʹ��ͬһ����ʱ����
}

//��̬��Ա��ʱ���ĳ�ʼ��
//ʹ��ȱʡ���캯����û�г�ʼ����ʱ�����ԡ�
//BaseTimer Port_RTU::T15_35;
//BaseTimer Port_RTU::Trespond;

//RTU�˿�ͨѶ��ʼ��.
void Port_RTU::portRTU_Init(void)
{
    //�˿�Ӳ����ʼ���Լ�ͨѶ���Բ�������.
    RS485_Init();

    //����T15_35,����Ʋ�����ֽ�����ֻ���֡������Ӧ��ʱ������T15_35��ʱ����t3.5��ʽ����,(���Ĳ���ȱʡ)�ݲ�������ʱ��.
    T15_35.timer_Init(bTIMERSTOP);
    //ʹ��ͬһ����ʱ��Ӳ���������ظ���ʼ��Trespond��
    //Ĭ��Ϊ����ģʽ
    RS485_RX();
}

//------------------------------------------------------------------------------------------------

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
    timeFrameEnd_Start();
    //�ȴ�֡���������
    while(portStatus.bBusy);
    LED0_OFF;
}


//RS485�ڵ�DMA��������:������Ҫʹ�ý��ջ�����RXBuffer��DMA��Ŀ�ģ����Է��ڱ����ж��������ࡣ
void RTU_DataCtrl::portReceiveDMA_Config(void)
{
    DMA_InitTypeDef DMA_InitStructure;
    //ʹ��DMA1���ߡ�
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    USART_DMACmd(USART2,USART_DMAReq_Rx,ENABLE);   //ʹ�ܴ���2 DMA����
    
    DMA_DeInit(DMA1_Channel6);   //��DMA2��ͨ��6�Ĵ�������Ϊȱʡֵ  ����2�Ľ��ն�Ӧ����DMA1ͨ��6
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART2->DR;  //DMA����ADC����ַ
    //DMA_InitStructure.DMA_MemoryBaseAddr = (u32)RXBuffer;  //DMA�ڴ����ַ
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)RXBuffer;  //DMA�ڴ����ַ
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //���ݴ��䷽�򣬴������ȡ���͵��ڴ�
    DMA_InitStructure.DMA_BufferSize = FRAME_MAXLEN;  //DMAͨ���Ļ���Ĵ�С
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //�����ַ�Ĵ�������
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //�ڴ��ַ�Ĵ�������
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //�������ݿ��Ϊ8λ
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //�洢�����ݿ��Ϊ8λ
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  //��������������ģʽ
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMAͨ��ӵ�������ȼ� 
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��
    DMA_Init(DMA1_Channel6, &DMA_InitStructure);  //����DMA_InitStruct��ָ���Ĳ�����ʼ��DMA��ͨ��:DMA1��ͨ��6����ΪUSART2_RX��
            
    DMA_Cmd(DMA1_Channel6, ENABLE);  //��ʽ����DMA����
}

