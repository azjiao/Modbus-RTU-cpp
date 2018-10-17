/*********************************************************
  ���ܣ� Modbus-RTU��վͨѶЭ��
  ������ ʵ���˴�վͨѶ�ļ���������
  ��ƣ� azjiao
  �汾�� 0.1
  ���ڣ� 2018��10��10��
 *********************************************************/
#ifndef __RTU_SLAVE_H
#define __RTU_SLAVE_H

#include "RTU_RXTX.h"

//���幩ͨѶ�ķ�������Դ�����������.
const u8 uc0xMaxLen = 32;  //�ڲ�λ������coil/8,����uc0xMaxLen*8����
const u8 uc1xMaxLen = 32;  //������ɢ��DI/8������uc1xMaxLen*8����
const u8 uc3xMaxLen = 64;   //����洢��AI
const u8 uc4xMaxLen = 64;   //���ּĴ���HoldReg

//����ͨѶ��Դ�ṹ
typedef struct
{
    u8  uc0xxxx[uc0xMaxLen];  //�ڲ�λ������coil
    u8  uc0xIndex;
    u8  uc1xxxx[uc1xMaxLen];  //������ɢ��DI
    u8  uc1xIndex;
    u16 us3xxxx[uc3xMaxLen];  //����洢��AI
    u8  uc3xIndex;
    u16 us4xxxx[uc4xMaxLen];  //���ּĴ���HoldReg
    u8  uc4xIndex;
} Source_Type;

//�����ⲿ����Ĺ���RTU_DataCtrlʵ��
extern RTU_DataCtrl Neo_RTUPort;

//------------------------------------------------------------------------
//Modbut_RTU��վ��.
class RTU_Slave
{
    private:
        u8  ucNodeAddr;  //վ��.
        Source_Type Source;  //��վ�ṩͨѶ����Դ.
        RTU_DataCtrl* pRTUPort;  //��վʹ�õ�RTU_DataCtrlʵ����ָ�롣
        //�����վʹ�õ�RTU_DataCtrl ʵ���ı�����
        #define RTU_PORT  (*(pRTUPort))   
        //��վ֧�ֵ�RTUЭ�鹦����.
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
        //Э�鹦���봦������
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

        //��վ��ʼ��: ����Ӳ�����������ݿ�ʼ������
        void slave_Init(u32 unBR = BAUDRATE, u16 usDB = DATABIT, u16 usSB = STOPBIT, u16 usPt = PARITY)
        {
            RTU_PORT.RTU_Init(unBR, usDB ,usSB, usPt);
        };        

        //��վ������
        void slaveService(void);

        //��Դ�Ķ�д������(��Դӳ��)
        //DQ���ݵ�д��0xxxx
        //DQ���ݱ������ֽ�Ϊ��λд�룬��д���λ�����ֽ�������š�
        void write_SourceDQ(u8 ucData, u8 ucIndex)
        {
            if((ucIndex >= 0) && (ucIndex < uc0xMaxLen))
            {
                Source.uc0xxxx[ucIndex] = ucData;
            }
        };
        //DQ����0xxxx�Ķ�ȡ
        //һ�ζ�ȡָ��λ�õ�һ���ֽ����ݡ�
        u8 read_SourceDQ(u8 ucIndex)
        {
            if((ucIndex > 0) && (ucIndex < uc0xMaxLen))
            {
                return Source.uc0xxxx[ucIndex];
            }
        };
        //DI����д��1xxxx
        void write_SourceDI(u8 ucData, u8 ucIndex);
        //DI����1xxxx�Ķ�ȡ
        u8 read_SourceDI(u8 ucIndex);
        //AI����3x��д��3xxxx
        //AI��16λ���ݣ���2�ֽ�Ϊһ����λд��ucIndexָ������Դ����
        //ӳ��ʱ���ı�usData�����Ĵ洢˳��
        //FIXME:STM32F10x��С��ģʽ��
        void write_SourceAI(u16 usData, u8 ucIndex)
        {
            if((ucIndex >= 0) && (ucIndex < uc3xMaxLen)) //�޷�������û��Ҫ��0�Ƚϣ����߼�˼ά����Ҫ������ȥ����
            {
                Source.us3xxxx[ucIndex] = usData;
            }
        };
        //����Դ��3xxxx��ȡָ��λ�õ�AI����
        u16 read_SourceAI(u8 ucIndex)
        {
            if((ucIndex >= 0) && (ucIndex < uc3xMaxLen))
            {
                return Source.us3xxxx[ucIndex];
            }
        };
        //HoldRegд��4xxxx
        //HoldReg��16λ���ݣ���2�ֽ�Ϊһ����λд��ucIndexָ������Դ����
        //ӳ��ʱ���ı�usData�����Ĵ洢˳��
        //FIXME:STM32F10x��С��ģʽ
        void write_SourceHg(u16 usData, u8 ucIndex)
        {
            if((ucIndex >= 0) && (ucIndex < uc4xMaxLen))
            {
                Source.us4xxxx[ucIndex] = usData;
            }
        };
        //HoldReg����4xxxx�Ķ�ȡ
        u16 read_SourceHg(u8 ucIndex)
        {
            if((ucIndex >= 0) && (ucIndex < uc4xMaxLen))
            {
                return Source.us4xxxx[ucIndex];
            }
        };
};



#endif /* end of include guard: __RTU_SLAVE_H */
