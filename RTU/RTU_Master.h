/*********************************************************
  ���ܣ� Modbus-RTU��վͨѶЭ��
  ������ ʵ���˴�վͨѶ�ļ���������
  ��ƣ� azjiao
  �汾�� 0.1
  ���ڣ� 2018��10��12��
 *********************************************************/
#ifndef __RTU_MASTER_H
#define __RTU_MASTER_H

#include "RTU_RXTX.h"

//�û����ݻ�������󳤶��ֽ�����
const u16 usUser_BufMaxLen = (u16)256;

const bool bRead = false;
const bool bWrite = true;

typedef  struct
{
    bool bDone;       //��վһ��ͨѶ��ɣ����ݿɶ�ȡ��1:�ش������ݿ��Զ�ȡ��
    bool bBusy;       //æ�����ڽ��գ����տ�ʼ��һ��æ��
    bool bErr;        //����֡�д���.
    bool bTimeOut;    //Ӧ��ʱ�� �ɹ鲢��usErr��.���ǵ�bErrΪtrue��usErrMsg=4�ĵȼ�bool����
    u16  usErrMsg;    //ͨѶ������Ϣ.0:�޴�;1:�Ǳ�վ��Ϣ(��վ���ô���1);2:֡CRC����;3:�ֽڽ��ճ���,4:���ճ�ʱ��
    u32  unErrCount;  //ʧ�ܼ�����
}ModbusStatus_Struct;

class RTU_Master
{
    private:
        //�����͵���վ�������Լ��Ӵ�վ��ȡ�����ݡ�
        //������ݻ�������ŵ����û����ݣ���������֡���ݡ�����ŵ���ԭʼ�����������Լ���������Ĵ�վ���ݡ�
        //����Modbus-RTUЭ�飬���ݵĴ�Ű��ոߵ�ַ�ֽڴ�����ݵ��ֽڵķ�ʽ��Ҳ���Ǵ��ģʽ��
        struct
        {
            u8 ucData[usUser_BufMaxLen];
            u16 usIndex;
        }User_DataBuffer;

        //��վʹ�õ�RTU_DataCtrl ʵ����ָ��
        RTU_DataCtrl* pRTUPort;
        //������վʹ�õ�RTU_DataCtrl ʵ���ı���
        #define RTU_PORT  (*(pRTUPort))

        //��վ֧�ֵ�RTUЭ�鹦���롣
        enum
        {
            masterFunc0x01 = 0x01,
            masterFunc0x02 = 0x02,
            masterFunc0x03 = 0x03,
            masterFunc0x04 = 0x04,
            masterFunc0x0F = 0x0F,
            masterFunc0x10 = 0x10,
        } FunCode;
        //Э�鹦���봦������
        void masterFunc_0x01(u8 ucNodeAddr, u16 usDataAddr, u16 usNum);
        void masterFunc_0x02(u8 ucNodeAddr, u16 usDataAddr, u16 usNum);
        void masterFunc_0x03(u8 ucNodeAddr, u16 usDataAddr, u16 usNum);
        void masterFunc_0x04(u8 ucNodeAddr, u16 usDataAddr, u16 usNum);
        void masterFunc_0x0F(u8 ucNodeAddr, u16 usDataAddr, u16 usNum);
        void masterFunc_0x10(u8 ucNodeAddr, u16 usDataAddr, u16 usNum);
        //������������֡�ı�������뺯����
        bool enCode_0x01(u8 ucNodeAddr, u16 usDataAddr, u16 usNum);
        bool unCode_0x01(u8 ucNodeAddr);
        bool enCode_0x02(u8 ucNodeAddr, u16 usDataAddr, u16 usNum);
        bool unCode_0x02(u8 ucNodeAddr);
        bool enCode_0x03(u8 ucNodeAddr, u16 usDataAddr, u16 usNum);
        bool unCode_0x03(u8 ucNodeAddr);
        bool enCode_0x04(u8 ucNodeAddr, u16 usDataAddr, u16 usNum);
        bool unCode_0x04(u8 ucNodeAddr);
        bool enCode_0x0F(u8 ucNodeAddr, u16 usDataAddr, u16 usNum);
        bool unCode_0x0F(u8 ucNodeAddr);
        bool enCode_0x10(u8 ucNodeAddr, u16 usDataAddr, u16 usNum);
        bool unCode_0x10(u8 ucNodeAddr);



    public:
        RTU_Master(RTU_DataCtrl* ptr = &RTU_PORT_ALIAS);
        ~RTU_Master(void){pRTUPort = nullptr; };

        ModbusStatus_Struct masterStatus;  //��վ״̬�֡�
         //��վ��ʼ��: ����Ӳ�����������ݿ�ʼ������
        void master_Init(u32 unBR = BAUDRATE, u16 usDB = DATABIT, u16 usSB = STOPBIT, u16 usPt = PARITY)
        {
            RTU_PORT.RTU_Init(unBR, usDB ,usSB, usPt);
        };
        //�����ݻ�����д��8λ����
        void write2Buffer(u8* ucPtrSource, u16 usLen);
        //�����ݻ���������8λ����
        void read4Buffer(u8* ucPtrDes, u16 usLen);

        //�����ݻ�����д��16λ����(short int��float...)��С��ģʽ����Դ��
        void write16bitData_LMode(u16* ptrSource, u16 usNum);
        //�����ݻ�����д��16λ����(short int��float...)�����ģʽ����Դ��
        void write16bitData_BMode(u16* ptrSource, u16 usNum);

        //�����ݻ���������16λ����(short int��float...)��С��ģʽ����Դ��
        //ֻ��������RTUЭ��ʱ�Ÿ�����Ҫ����ȡ�Ĳ����÷�ʽ��
        void read16bitData_LMode(u16* ptrDes, u16 usNum);

        //�����ݻ���������16λ����(short int��float...)�����ģʽ����Դ��
        //���ǰ���RTUЭ��Ӧ�ò��õĶ�ȡ��ʽ��
        void read16bitData_BMode(u16* ptrDes, u16 usNum);

        //�����ݻ�����д��32λ����(������float....):С��ģʽ����Դ��
        //Modbus-RTU������ֱ��֧��32λ���ֽ����ݣ���Ҫ��ϲ���ʹ�á�
        //���ԣ�32λ���ݲ������ϸ��մ��ģʽ������ģ����ǰ����ݷֳ�2��16λ���ݣ���ÿ��16λ�����а��մ�������䡣
        //����������16λ���ݵ�˳����û��Ҫ����Ҫ������Ҫ����ת�������ӷ�����һ�²��ܹ���ȷ��ȡ��
        void write32bitData_LMode(u32* ptrSource, u16 usNum);
        //�����ݻ�����д��32λ����(������float....):���ģʽ����Դ��
        void write32bitData_BMode(u32* ptrSource, u16 usNum);
        //�����ݻ���������32λ���ݣ����ģʽ��
        void read32bitData_BMode(u32* ptrDes, u16 usNum);
        //Modbus��վͨѶ����
        void master(u8 ucNodeAddr, bool bRWMode, u16 usData_Addr, u16 usNum);
        //test
        void printBuff(void);

};


#endif /* end of include guard: __RTU_MASTER_H */
