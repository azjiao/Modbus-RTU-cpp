/*********************************************************
���ܣ� Modbus RTUͨѶ�ĵײ�ͨѶʵ��
������ ��485�˿ڵĳ�ʼ���Լ�ModbusͨѶ����Ҫ�ļ�����ʱ���ĳ�ʼ����
��ƣ� azjiao
�汾�� 0.1
���ڣ� 2018��10��08��
*********************************************************/
#ifndef __MODBUS_RXTX_H
#define __MODBUS_RXTX_H

#include "bitBand.h"
#include "baseTimer.h"

// ֡��󳤶�:�ֽ���.Modbus�涨һ֡���Ȳ�����256�ֽڡ�
const u16 usFRAME_MAXLEN = 256;
const bool bTXMode = false;  //����ģʽ
const bool bRXMode = true;   //����ģʽ
const bool bMaster = true;
const bool bSlave = false;

#include "RTU_Config.h"
//------------------------------------------------------
//RS485�˿ڳ�ʼ�����ݽṹ����
typedef struct
{
    u32 unBaudRate;  //������
    u16 usDataBit;  //����λ�ֳ�
    u16 usStopBit;  //ֹͣλ
    u16 usParity;  //У��λ��0�� 1�� 2ż
} RS485Init_Struct;

//modbusͨѶ״̬��
typedef  struct
{
    bool bMode;  //����bTXMode:����ģʽ,����bRXMode:����״̬.
    bool bReadEnb; //���յ���֡�ɶ�ȡ��ʶ��0�����ɶ�ȡ��1���ɶ�ȡ��
    bool bDone;  //��վһ��ͨѶ��ɣ����ݿɶ�ȡ��1:�ش������ݿ��Զ�ȡ��
    bool bBusy;         //æ�����ڽ��գ����տ�ʼ��һ��æ��
    bool bErr;          //����֡�д���.
    bool bTimeOut;  //Ӧ��ʱ�� �ɹ鲢��usErr��.
    u16  usErr;     //ͨѶ������Ϣ.0:�޴�;1:�Ǳ�վ��Ϣ(��վ���ô���1);2:֡CRC����;3:�ֽڽ��ճ���.
    u32  unErrCount;  //У��ʧ�ܼ�����
}ModbusStatus_Struct;

//----------------------------------------------------------------------------------
//RS485�˿��ࣺ���ڳ�ʼ��485�˿ڡ�
class Port_RS485
{
    private:
        RS485Init_Struct RS485Init;
        u8             ucPortNo = 2;  //USART��ţ����ھ�Ӣ����2,��USART2��
        void initPort(void);  //����ͨѶ�˿�Ӳ����
        void configCommParam(void);  //����ͨѶ�˿�ͨѶ������
        void setPortNo(u8 ucPNo);  //�޸Ķ˿ں�.
    public:
        Port_RS485(u32 unBR, u16 usDB, u16 usSB, u16 usPt);
        ~Port_RS485(){};

        u32  getBaudRate() { return RS485Init.unBaudRate;}  //��ȡ������.       
        void setPortParam(u32 unBR, u16 usDB, u16 usSB, u16 usPt);  //�޸Ķ˿ڳ�ʼ������.        
        void RS485_Init(void);  //����485�ڵ�Ӳ����ͨѶ���Բ�����
};

//Modbus_RTU�˿��ࡣ
class Port_RTU : Port_RS485
{
    private:
        u16 usT15_us;  //t1.5��ʱʱ�䡣
        u16 usT35_us;  //t3.5��ʱʱ�䡣
        u16 usTresponse_ms;  //Ӧ��ʱʱ�䡣
        u8  ucT15_35No;  //t1.5��t3.5�����õĶ�ʱ�����.����t1.5��t3.5����ͬʱ��������������ʹ��ͬһ����ʱ����
        u8  ucTrespNo;  //��ʱ��ʱ�����.

    public:
        Port_RTU(u32 unBR, u16 usDB, u16 usSB, u16 usPt);
        ~Port_RTU(){};

        //����������ʱ��,������Ҫ���ж�����������,������Ƴ�public.
        static BaseTimer T15_35;    //t1.5��t3.5���ö�ʱ��.
        static BaseTimer Trespond;  //��ʱ��ʱ��.
            
        void timeRespTimeOut_Stop(void){Trespond.timer_ResetONOFF(bTimerStop);};
        void timeRespTimeOut_Start(void){Trespond.timer_ResetONOFF(bTimerStart);};
        void timeFrameEnd_Start(void) {T15_35.timer_ResetONOFF(bTimerStart);};
        void timeFrameEnd_Stop(void) {T15_35.timer_ResetONOFF(bTimerStop);}
        void RS485_TX(void) {PDout(7) = 1;}  //ʹPD7Ϊ1ʹ�ܷ��͡�
        void RS485_RX(void) {PDout(7) = 0;}  //ʹPD7Ϊ0ʹ�ܽ��ա�
        //����Modbus-RTU���ö˿ڵ�ͨѶ������
        void setRTU_PortParam(u32 unBR, u16 usDB, u16 usSB, u16 usPt)
        {
            setPortParam(unBR, usDB, usSB, usPt);
        };
        //��Port_RTU˽�����Գ�ʼ���˿ڡ�
        void portRTU_Init(void);
        //�ø���������ʼ���˿ڡ�
        void portRTU_Init(u32 unBR, u16 usDB, u16 usSB, u16 usPt)
        {
            setPortParam(unBR, usDB, usSB, usPt);
            portRTU_Init();
        };
};

//Modbus_RTU�˿������շ�������.
class RTU_DataCtrl : public Port_RTU
{
    private:        
        //����CRCУ����
        u16 CRC16Gen(u8* ucPtr, u16 usLen);
        //CRC16У��
        bool CRC16Check(u8* ucPtr, u16 usLen);
        //��ս��ջ�����.
        void resetRXBuffer(void){usRXIndex = 0;};
        //��λ״̬��׼����������֡
        void resetStatus4RX(void)
        {
            modbusStatus.bReadEnb = false;  //��λ���ݿɶ�ȡ��ʶ��
            modbusStatus.bDone = false;     //��λͨѶ��ɱ�ʶ.
            modbusStatus.bErr = false;
            modbusStatus.bTimeOut = false;
            modbusStatus.bMode = bRXMode;  //���ڽ���״̬,�Ա��ڽ���һ֡��У��CRC.
            modbusStatus.bBusy = false;  //����ϵͳ״̬�������.
        };
        //��λ״̬��׼����������֡
        void resetStatus4TX(void)
        {
            modbusStatus.bDone = false;     //��λͨѶ��ɱ�ʶ��
            modbusStatus.bErr = false;
            modbusStatus.bTimeOut = false;
            modbusStatus.bMode = bTXMode;  //���ڷ���״̬.
            modbusStatus.bBusy = true;  //����ϵͳ״̬�������.
        };
    public:
        RTU_DataCtrl(u32 unBR = BAUDRATE, u16 usDB = DATABIT, u16 usSB = STOPBIT, u16 usPt = PARITY);
        ~RTU_DataCtrl(){};

        //���ݻ�����
        u8 RXBuffer[usFRAME_MAXLEN];  //���ջ�����
        u16 usRXIndex;  //���ջ�������ǰ����
        u8 TXBuffer[usFRAME_MAXLEN];  //���ͻ�����
        u16 usTXIndex;  //���ͻ�������ǰ����            
        ModbusStatus_Struct  modbusStatus;  //ͨѶ״̬��,�ⲿ�жϵ���Ҫ����״̬��,��������Ϊpublic.
            
        //ʹ�ø���������ʼ���˿ڣ�Ҳ�����޲�����ʼ��(ʹ��ȱʡ����).    
        void RTU_Init(u32 unBR = BAUDRATE, u16 usDB = DATABIT, u16 usSB = STOPBIT, u16 usPt = PARITY)
        {
            portRTU_Init(unBR, usDB, usSB, usPt);            
        };
        u16  CRC16Gen(void);  //Ĭ��ʹ��TXBuffer��Ϊ����У���������.
        bool CRC16Check(void);  //Ĭ��ʹ��RXBuffer��Ϊ���У�����Ƿ���ȷ������.
        //�洢һ�����յ������ݵ����ջ�����.
        void saveAData(u8 ucData)
        {
            if(usRXIndex < usFRAME_MAXLEN)
            {
                RXBuffer[usRXIndex++] = ucData;
            }
        };

        //��������֡
        void ReceiveFrame(void);
        //��������֡
        void SendFrame(void);
};


#endif /* end of include guard: __MODBUS_RXTX_H */
