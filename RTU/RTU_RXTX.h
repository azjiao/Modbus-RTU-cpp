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
//#include <stdio.h>

// ֡��󳤶�:�ֽ���.Modbus�涨һ֡���Ȳ�����256�ֽڡ�
const u16  FRAME_MAXLEN      = 256;
const bool TXMODE            = false;  //����ģʽ
const bool RXMODE            = true;   //����ģʽ
const u8   FRAMEEND_TIMERRUN = 1; //֡�������������
const u8   TIMEOUT_TIMERRUN  = 2;  //��ʱ���������

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
    bool bMode;  //����TXMODE:����ģʽ,����RXMODE:����״̬.
    bool bReadEnb; //���յ���֡�ɶ�ȡ��ʶ��0�����ɶ�ȡ��1���ɶ�ȡ��
    bool bBusy;         //æ�����ڽ��գ����տ�ʼ��һ��æ��
    bool bErr;          //����֡�д���.
    bool bTimeOut;  //Ӧ��ʱ�� �ɹ鲢��usErrMsg��.
    u16  usErrMsg;     //ͨѶ������Ϣ.0:�޴�;1:�Ǳ�վ��Ϣ(��վ���ô���1);2:֡CRC����;3:�ֽڽ��ճ���,4:���ճ�ʱ��
    u32  unErrCount;  //У��ʧ�ܼ�����
}PortStatus_Struct;

//----------------------------------------------------------------------------------
//RS485�˿��ࣺ���ڳ�ʼ��485�˿ڡ�
class Port_RS485
{
    private:
        RS485Init_Struct RS485Init;  //�˿ڲ�����
        u8             ucPortNo = 2;  //USART��ţ����ھ�Ӣ����2,��USART2��
        void initPort(void);  //����ͨѶ�˿�Ӳ����
        void configCommParam(void);  //����ͨѶ�˿�ͨѶ������

    public:
        Port_RS485(u32 unBR, u16 usDB, u16 usSB, u16 usPt){setPortParam(unBR, usDB, usSB, usPt);};
        ~Port_RS485(){};

        u32  getBaudRate() { return RS485Init.unBaudRate;}  //��ȡ������.
        void setPortParam(u32 unBR, u16 usDB, u16 usSB, u16 usPt);  //�޸Ķ˿ڳ�ʼ������.
        void RS485_Init(void);  //����485�ڵ�Ӳ����ͨѶ���Բ�����
        void RS485_TX(void) {PDout(7) = 1; }  //ʹPD7Ϊ1ʹ�ܷ��͡�
        void RS485_RX(void) {PDout(7) = 0; }  //ʹPD7Ϊ0ʹ�ܽ��ա�
};

//Modbus_RTU�˿��ࡣ
class Port_RTU : Port_RS485
{
    private:
        //    u16 usT15_us;  //t1.5��ʱʱ�䡣
        u32 unT35_us;  //t3.5��ʱʱ�䡣
        u32 unTresponse_us;  //Ӧ��ʱʱ�䡣
        u8  ucT15_35No;  //t1.5��t3.5�����õĶ�ʱ�����.����t1.5��t3.5����ͬʱ��������������ʹ��ͬһ����ʱ����
        u8  ucTrespNo;  //��ʱ��ʱ�����.

        BaseTimer T15_35;    //t1.5��t3.5���ö�ʱ��.
        BaseTimer Trespond;  //��ʱ��ʱ��.
        bool bSameTimer;    //t1.5_t3.5ʹ��ͬһ����ʱ���ı�ʶ����ucT15_35��ucTrespNoһ��ʱbSameTimerΪtrue;
        u8 ucWhichTimer;            //Ŀǰ���ĸ���ʱ��������FRAMEEND_TIMERRUN֡�������������FRAMEEND_TIMERRUN��ʱ�������

    public:
        //�Դ���Ĳ�����ʼ�����࣬���������ö�ʱ�����ԣ�ͬʱ���ݶ�ʱ����ž����Ƿ���ͬһ����ʱ����
        Port_RTU(u32 unBR, u16 usDB, u16 usSB, u16 usPt);
        ~Port_RTU(){};

        //��Port_RTU˽�����Գ�ʼ���˿ڡ���ʼ�����ö˿�Ӳ���Ͷ�ʱ����
        void portRTU_Init(void);
        //�ø���������ʼ���˿ڡ�
        void portRTU_Init(u32 unBR, u16 usDB, u16 usSB, u16 usPt)
        {
            setPortParam(unBR, usDB, usSB, usPt);  //ʹ������ӿڳ�ʼ���������ԡ�
            portRTU_Init();
        };

        bool getSameTimer(){return bSameTimer;}
        //��ѯ�ĸ���ʱ��������
        u8 whichTimerRun() {return ucWhichTimer;};

        //���¸�λ��������ʱ��ʱ����
        void timeRespTimeOut_Start(void)
        {
            Trespond.timer_ResetONOFF(bTIMERSTART);
            ucWhichTimer = TIMEOUT_TIMERRUN;
        }

        void timeRespTimeOut_Stop(void)
        {
            Trespond.timer_ResetONOFF(bTIMERSTOP);
        }
        //void timeFrameEnd_Start(void) {T15_35.timer_Init(bTimerStart); T15_35.timer_ResetONOFF(bTimerStart);ucWhichTimer = FRAMEEND_TIMERRUN;};
        void timeFrameEnd_Start(void)
        {
            T15_35.timer_ResetONOFF(bTIMERSTART);
            ucWhichTimer = FRAMEEND_TIMERRUN;
        }
        void timeFrameEnd_Stop(void)
        {
            T15_35.timer_ResetONOFF(bTIMERSTOP);
        }

        //���л�����ӿڡ�
        using Port_RS485::RS485_TX ;
        using Port_RS485::RS485_RX ;
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
            portStatus.bReadEnb = false;  //��λ���ݿɶ�ȡ��ʶ��
            portStatus.bErr     = false;
            portStatus.bTimeOut = false;
            portStatus.bMode    = RXMODE;  //���ڽ���״̬,�Ա��ڽ���һ֡��У��CRC.
            portStatus.bBusy    = false;  //����ϵͳ״̬�������.
        };
        //��λ״̬��׼����������֡
        void resetStatus4TX(void)
        {
            portStatus.bReadEnb = false;  //��λ���ݿɶ�ȡ��ʶ��
            portStatus.bErr     = false;
            portStatus.bTimeOut = false;
            portStatus.bMode    = TXMODE;  //���ڷ���״̬.
            portStatus.bBusy    = true;  //����ϵͳ״̬�������.
        };
    public:
        RTU_DataCtrl(u32 unBR = BAUDRATE, u16 usDB = DATABIT, u16 usSB = STOPBIT, u16 usPt = PARITY): \
            Port_RTU(unBR, usDB,usSB, usPt), \
            usRXIndex(0),usTXIndex(0) {};

        ~RTU_DataCtrl(){};

        //���ݻ�����
        u8 RXBuffer[FRAME_MAXLEN];  //���ջ�����
        u16 usRXIndex;  //���ջ�������ǰ����
        u8 TXBuffer[FRAME_MAXLEN];  //���ͻ�����
        u16 usTXIndex;  //���ͻ�������ǰ����
        PortStatus_Struct  portStatus;  //ͨѶ״̬��,�ⲿ�жϵ���Ҫ����״̬��,��������Ϊpublic.

        //ʹ�ø���������ʼ���˿ڣ�Ҳ�����޲�����ʼ��(ʹ��ȱʡ����).
        //��վ�ʹ�վ��ʹ�øýӿڳ�ʼ��ͨѶ�˿ڡ�
        void RTU_Init(u32 unBR = BAUDRATE, u16 usDB = DATABIT, u16 usSB = STOPBIT, u16 usPt = PARITY)
        {
            portRTU_Init(unBR, usDB, usSB, usPt);  //ʹ������ӿڳ�ʼ���˿�.
        };
        u16  CRC16Gen(void);  //Ĭ��ʹ��TXBuffer��Ϊ����У���������.
        bool CRC16Check(void);  //Ĭ��ʹ��RXBuffer��Ϊ���У�����Ƿ���ȷ������.
        //�洢һ�����յ������ݵ����ջ�����.
        void saveAData(u8 ucData)
        {
            if(usRXIndex < FRAME_MAXLEN)
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
