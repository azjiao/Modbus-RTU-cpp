#include <stdio.h>
#include "usart1.h"
#include "RTU_Master.h"

//RTU_Master ���캯����
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

//�����ݻ�����д����
//ucPtrSource������Դ�ֽ�ָ�룬usLen�Ǵ�д�����������
void RTU_Master::write2Buffer(u8* ucPtrSource, u16 usLen)
{
    int i = 0;
    for(i = 0; (i < usLen) && (i < usUser_BufMaxLen); i++)
    {
        User_DataBuffer.ucData[i] = *(ucPtrSource + i);
    }
    User_DataBuffer.usIndex = i;
}

//�����ݻ�����������
//usLen����Ҫ�����������ֽ�����ucPtrDes�Ǳ����Ŀ���ֽ�ָ�롣
//���뱣֤Ŀ�Ŀռ��ѷ��䡣
void RTU_Master::read4Buffer(u8* ucPtrDes, u16 usLen )
{
    int i = 0;
    for(i = 0; (i < usLen) && (i < usUser_BufMaxLen); i++)
    {
        *(ucPtrDes + i) = User_DataBuffer.ucData[i];
    }
}

//�����ݻ�����д��16λ����(short int��float...)��С��ģʽ����Դ��
//��С�˶���ָ���������Դ���ݴ�ŷ�ʽ����,����User_Databuffer��˵�ǰ���Modbus-RTUЭ��Ҫ��Ĵ��ģʽ����ŵģ��Ա㰴˳���ȴ��͸��ֽڡ�
//С��ģʽ��STM32F10x���õ����ݴ洢��ʽ��
void RTU_Master::write16bitData_LMode(u16* ptrSource, u16 usNum)
{
    int i = 0;
    for(i = 0; (i < usNum) && (i < (usUser_BufMaxLen >> 1)); i++)
    {
        User_DataBuffer.ucData[2*i] = (*(ptrSource + i) & 0xFF00) >> 8;  //�͵�ַ��Ÿ��ֽ�, Դ���ݵĸ��ֽ��ڸߵ�ַ.
        User_DataBuffer.ucData[2*i + 1] = (*(ptrSource + i)) & 0xFF;  //�ߵ�ַ��ŵ��ֽ�
        //other.
        //*((u16*)User_DataBuffer.ucData + i) = ((*(ptrSource + i)) & 0xFF << 8) | ((*(ptrSource + i) & 0xFF00) >> 8);
        //-----------------------------------------(          ���ֽ�         )--------(       ���ֽ�          )
    }
}

//�����ݻ�����д��16λ����(short int��float...)�����ģʽ����Դ��
//��С�˶���ָ���������Դ���ݴ�ŷ�ʽ����,����User_Databuffer��˵�ǰ���Modbus-RTUЭ��Ҫ��Ĵ��ģʽ����ŵģ��Ա㰴˳���ȴ��͸��ֽڡ�
//���ģʽ�Ǹ���PLC�ȿ��Ƶ�Ԫ���õ����ݴ洢��ʽ��
//���������Ҫ���ڴӸ�����ô��ģʽ���豸�ɼ����������ݵ�ͨѶ����
void RTU_Master::write16bitData_BMode(u16* ptrSource, u16 usNum)
{
    for(int i = 0; (i < usNum) && (i < (usUser_BufMaxLen >> 1)); i++)
    {
        //Ϊ�˱��⺯�����õĿ��������ﲻ����write2Buffer()������
        User_DataBuffer.ucData[2*i] = *(ptrSource + i) & 0xFF;  //�͵�ַ��Ÿ��ֽ�,Դ���ݸ��ֽ��ڵ͵�ַ��
        User_DataBuffer.ucData[2*i + 1] = (*(ptrSource + i) & 0xFF00) >> 8;  //�ߵ�ַ��ŵ��ֽ�
        //other.
        //*((u16*)User_DataBuffer.ucData + i) = ((*(ptrSource + i) & 0xFF00)) | (*(ptrSource + i) & 0xFF);
        //-------------------------------------(          ���ֽ�         )-----(       ���ֽ�          )
    }
}

//�����ݻ���������16λ����(short int��float...)��С��ģʽ����Դ��
//��С�˶���ָ���������Դ���ݴ�ŷ�ʽ����,����User_Databuffer��˵�ǰ���Modbus-RTUЭ��Ҫ��Ĵ��ģʽ����ŵġ�
//С��ģʽ��STM32F10x���õ����ݴ洢��ʽ��
//����������С��ģʽ����ȡUser_DataBuffer�е����ݣ��������������С��ģʽ��ʾ��ptrDes�����С�
//���������Modbus-RTUЭ�鴫�����ݣ����ͻ��������ݻ��Զ��ѵ��ֽڷ��ڸߵ�ַ����ϵͳ��Ҫ����С��ģʽ����ȡ����ȷ��
//���ԣ�������ֻ��Э�����õ�ʱ���ʹ�á�
void RTU_Master::read16bitData_LMode(u16* ptrDes, u16 usNum)
{
    for(int i = 0; (i < usNum) && (i < (usUser_BufMaxLen >> 1)); i++)
    {
        *(ptrDes + i) = User_DataBuffer.ucData[2*i + 1] << 8 | ((u16)User_DataBuffer.ucData[2*i]);
        //other.
        //*(ptrDes + i) = *((u16*)User_DataBuffer.ucData + i);
    }
}

//�����ݻ���������16λ����(short int��float...)�����ģʽ����Դ��
//��С�˶���ָ���������Դ���ݴ�ŷ�ʽ����,����User_Databuffer��˵�ǰ���Modbus-RTUЭ��Ҫ��Ĵ��ģʽ����ŵġ�
//���ģʽ�Ǹ���PLC�ȿ��Ƶ�Ԫ���õ����ݴ洢��ʽ
//���������մ��ģʽ��ȡUser_DataBuffer�е����ݣ��������������С��ģʽ��ʾ��ptrDes�����С�
//�������Modbus-RTUЭ�鴫�����ݣ����ͻ��������ݻ��Զ��ѵ��ֽڷ��ڸߵ�ַ����ϵͳ��Ҫ���մ��ģʽ����ȡ����ȷ��
void RTU_Master::read16bitData_BMode(u16* ptrDes, u16 usNum)
{
    for(int i = 0; (i < usNum) && (i < (usUser_BufMaxLen >> 1)); i++)
    {
        *(ptrDes + i) = User_DataBuffer.ucData[2*i] << 8 | ((u16)User_DataBuffer.ucData[2*i + 1]);
    }
}

//�����ݻ�����д��32λ����(������float....):С��ģʽ����Դ��
//Modbus-RTU������ֱ��֧��32λ���ֽ����ݣ���Ҫ��ϲ���ʹ�á�
//���ԣ�32λ���ݲ������ϸ��մ��ģʽ������ģ����ǰ����ݷֳ�2��16λ���ݣ���ÿ��16λ�����а��մ�������䡣
//����������16λ���ݵ�˳����û��Ҫ����Ҫ������Ҫ����ת�������ӷ�����һ�²��ܹ���ȷ��ȡ��
//����ķ����ǲ��ã�Դ�����ֽ�������M->L:A B C D,д����ֽ�����M->L:D C B A��Ҳ�������ģʽ��š�
void RTU_Master::write32bitData_LMode(u32* ptrSource, u16 usNum)
{
    int i;
    for(i = 0; (i < usNum) && (i < (usUser_BufMaxLen >> 2)); i++)
    {
        User_DataBuffer.ucData[4*i]     = ((*(ptrSource + i)) & 0xFF000000) >> 24;  //�͵�ַ��Ÿ��ֽ�, Դ���ݵĸ��ֽ��ڸߵ�ַ.
        User_DataBuffer.ucData[4*i + 1] = ((*(ptrSource + i)) & 0x00FF0000) >> 16;  //�ߵ�ַ��ŵ��ֽ�
        User_DataBuffer.ucData[4*i + 2] = ((*(ptrSource + i)) & 0xFF00) >> 8;       //�͵�ַ��Ÿ��ֽ�, Դ���ݵĸ��ֽ��ڸߵ�ַ.
        User_DataBuffer.ucData[4*i + 3] = (*(ptrSource + i)) & 0xFF;                //�ߵ�ַ��ŵ��ֽ�
    }
}

//�����ݻ�����д��32λ����(������float....):���ģʽ����Դ��
void RTU_Master::write32bitData_BMode(u32* ptrSource, u16 usNum)
{
    int i;
    for(i = 0; (i < usNum) && (i < (usUser_BufMaxLen >> 2)); i++)
    {
        User_DataBuffer.ucData[4*i + 3] = ((*(ptrSource + i)) & 0xFF000000) >> 24;  //�͵�ַ��Ÿ��ֽ�, Դ���ݵĸ��ֽ��ڸߵ�ַ.
        User_DataBuffer.ucData[4*i + 2] = ((*(ptrSource + i)) & 0x00FF0000) >> 16;  //�ߵ�ַ��ŵ��ֽ�
        User_DataBuffer.ucData[4*i + 1] = ((*(ptrSource + i)) & 0xFF00) >> 8;       //�͵�ַ��Ÿ��ֽ�, Դ���ݵĸ��ֽ��ڸߵ�ַ.
        User_DataBuffer.ucData[4*i]     = (*(ptrSource + i)) & 0xFF;                //�ߵ�ַ��ŵ��ֽ�
    }
}

//�����ݻ���������32λ���ݣ����ģʽ��
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
    printf("�������������ǣ�\r\n");
    for(i = 0; i < 10; i++)
    {
        printf("0x%x\t", User_DataBuffer.ucData[i]);
    }
    printf("\r\n");
}

//Modbus��վͨѶ����
//ucNodeAddr:��վ��ַ
//bMode_RW:ģʽ0����1д
//usDataAddr:�����վ���ݵ�ַ,��Ԫ����ַ����Ԫ�����͡�
//usNum:����byte����,�����Ƕ�ȡ��Ԫ��������Ҳ������д���Ԫ��������
//ucPtr:��վ�ش����ݻ�����������͵ĸ�ֵ���ݡ�
void RTU_Master::master(u8 ucNodeAddr, bool bMode_RW, u16 usDataAddr, u16 usNum)
{
    //masterStatus.bBusy = RTU_PORT.portStatus.bBusy;

    //�ж�Ԫ������:HoldReg,��ʶΪ4xxxx.
    if(usDataAddr >= 40000 && usDataAddr < 50000)
    {
        //д����������д���HoldReg,Func=0x10
        if(bMode_RW == bWrite)
        {
            masterFunc_0x10(ucNodeAddr, usDataAddr, usNum);
        }
        //�����������������HoldReg,Func=0x03
        else{
            masterFunc_0x03(ucNodeAddr, usDataAddr, usNum);
        }
    }

    //�ж�Ԫ�����ͣ�AI����ʶΪ3xxxx.
    if(usDataAddr >= 30000 && usDataAddr < 40000)
    {
        //3xxxxֻ֧�ֶ�����
        if(bMode_RW == bRead)
        {
            masterFunc_0x04(ucNodeAddr, usDataAddr, usNum);
        }
    }

    //�ж�Ԫ�����ͣ�DI����ʶΪ1xxxx.
    if(usDataAddr >= 10000 && usDataAddr < 20000)
    {
        //1xxxxֻ֧�ֶ�����
        if(bMode_RW == bRead)
        {
            masterFunc_0x02(ucNodeAddr, usDataAddr, usNum);
        }
    }

    //�ж�Ԫ�����ͣ�DQ����ʶΪ0xxxx.
    //if(usDataAddr >= (u16)0 && usDataAddr < 9999)
    if(usDataAddr < 9999)  //�޷���������0�Ƚ������塣
    {
        //ǿ��DQ
        if(bMode_RW == bWrite)
        {
            masterFunc_0x0F(ucNodeAddr, usDataAddr, usNum);
        }
        //��DQ
        else{
            masterFunc_0x01(ucNodeAddr, usDataAddr, usNum);
        }
    }
}

//--------------------------------function code:0x10---------------------------------------------------
//Function:0x10
//д��������ı��ּĴ�����
void RTU_Master::masterFunc_0x10(u8 ucNodeAddr, u16 usDataAddr, u16 usNum)
{
    bool bUnCode = false;

    masterStatus.bDone = false;
    //Ԫ����ַ 4xxxx
    u16 Addr = usDataAddr - 40000;

    //�������ݴ��Ϊ����֡��
    enCode_0x10(ucNodeAddr, Addr, usNum);
    //����
    RTU_PORT.SendFrame();
       
    //���ͺ�ת����ա�
    RTU_PORT.ReceiveFrame();
    //Ӧ��ʱ���ʹ�ܡ�
    RTU_PORT.timeRespTimeOut_Start();
 

    while((!RTU_PORT.portStatus.bReadEnb) && !RTU_PORT.portStatus.bErr);

    if(RTU_PORT.portStatus.bReadEnb)
        bUnCode = unCode_0x10(ucNodeAddr);

    judge("F0x10", bUnCode);
}


//����0x10
//�����Ľ�������TX_Struct�С�
//ucPtrָ��ֵ�ֽ����顣
bool RTU_Master::enCode_0x10(u8 ucNodeAddr, u16 usDataAddr, u16 usNum)
{
    u16 CRC16;
    u16 j;
    if(usNum > 123) return false;  //Ԫ���������ܴ���123.

    //���ͻ���������
    RTU_PORT.usTXIndex = 0;
    //��վ��ַ
    RTU_PORT.TXBuffer[0] = ucNodeAddr;
    RTU_PORT.TXBuffer[0] = ucNodeAddr;
    //������
    RTU_PORT.TXBuffer[1] = 0x10;
    //Ԫ����ַ
    RTU_PORT.TXBuffer[2] = usDataAddr >> 8;
    RTU_PORT.TXBuffer[3] = usDataAddr;
    //Ԫ������
    RTU_PORT.TXBuffer[4] = usNum >> 8; //���ֽ�
    RTU_PORT.TXBuffer[5] = usNum;  //���ֽ�
    //���ݳ���:usNum*2
    RTU_PORT.TXBuffer[6] = usNum << 1;
    //������,����ȡ��������usNum.
    for(int i = 0; i < RTU_PORT.TXBuffer[6]; i++)
    {
        RTU_PORT.TXBuffer[7 + i] = *(User_DataBuffer.ucData + i);
    }

    //����CRC����
    //j��CRC��ſ�ʼ��Ԫ,���ֽ���ǰ��
    j = RTU_PORT.TXBuffer[6] + 7;
    RTU_PORT.usTXIndex = j;
    //���ɵ�CRC16���ֽ���ǰ��
    CRC16 = RTU_PORT.CRC16Gen();
    //���CRC16,���ֽ���ǰ
    RTU_PORT.TXBuffer[j] = CRC16;
    RTU_PORT.TXBuffer[j+1] = CRC16 >> 8;
    //֡�����ֽ�����
    RTU_PORT.usTXIndex = j + 2;
    return true;
}

//�������ݽ���0x10
//ucNodeAddr�Ǵ�վվ�š�
bool RTU_Master::unCode_0x10(u8 ucNodeAddr)
{

    //������յ��Ĺ������λ��Ϊ1�������������
    if(!(RTU_PORT.RXBuffer[1] & 0x80))
    {
        //ʡ�ԱȽϷ��ص��������ݣ������롢Ԫ����ַ��Ԫ��������
        //��վ����ȷ
        if(RTU_PORT.RXBuffer[0] == ucNodeAddr)
            return true;
        else
            return false;
    }
    //�����վû�з�����ᳬʱ.
    //�����վ�쳣��
    else
    {
        return false;
    }
}

//--------------------------------function code:0x03---------------------------------------------------
//Function:0x03
//����������ı��ּĴ�����
void RTU_Master::masterFunc_0x03(u8 ucNodeAddr, u16 usDataAddr, u16 usNum)
{
    bool bUnCode = false;
    masterStatus.bDone = false;
    //Ԫ����ַ:4xxxx
    u16 Addr = usDataAddr - 40000;

    //�������ݴ��Ϊ����֡��
    enCode_0x03(ucNodeAddr, Addr, usNum);
    //����
    RTU_PORT.SendFrame();
        
    //���ͺ�ת����ա�
    RTU_PORT.ReceiveFrame();
    //Ӧ��ʱ���ʹ�ܡ�
    RTU_PORT.timeRespTimeOut_Start();

    //�ȴ����գ�ת����ղ�һ��ͨѶ�ӿ�æ�����Ի���������Ƿ�ɶ���
    while((!RTU_PORT.portStatus.bReadEnb) && !RTU_PORT.portStatus.bErr);

    if(RTU_PORT.portStatus.bReadEnb)
        bUnCode = unCode_0x03(ucNodeAddr);

    judge("F0x03", bUnCode);
}

//Function:0x03
//����0x03
//�����Ľ�������TX_Struct�С�
bool RTU_Master::enCode_0x03(u8 ucNodeAddr, u16 usDataAddr, u16 usNum)
{
    u16 CRC16;

    if(usNum > 125) return false; //����ȡ������Ԫ���������ܳ���125����
    //���ͻ���������
    RTU_PORT.usTXIndex = 0;
    //��վ��ַ
    RTU_PORT.TXBuffer[0] = ucNodeAddr;
    //������
    RTU_PORT.TXBuffer[1] = 0x03;
    //Ԫ����ַ
    RTU_PORT.TXBuffer[2] = usDataAddr >> 8;
    RTU_PORT.TXBuffer[3] = usDataAddr;
    //Ԫ������
    RTU_PORT.TXBuffer[4] = usNum >> 8; //���ֽ�
    RTU_PORT.TXBuffer[5] = usNum;  //���ֽ�

    //����CRC����
    //���ɵ�CRC16���ֽ���ǰ��
    RTU_PORT.usTXIndex = 6;
    CRC16 = RTU_PORT.CRC16Gen();
    //���CRC16,���ֽ���ǰ
    RTU_PORT.TXBuffer[6] = CRC16;
    RTU_PORT.TXBuffer[7] = CRC16 >> 8;
    //֡�����ֽ�����
    RTU_PORT.usTXIndex = 8;
    return true;
}

//�������ݽ���0x03
//Mb_Addr�Ǵ�վվ�š�
//ucPtr���ڴ�Ŷ�ȡ��HoldRegֵ��
bool RTU_Master::unCode_0x03(u8 ucNodeAddr)
{
    //������յ��Ĺ������λ��Ϊ1�������������
    if(!(RTU_PORT.RXBuffer[1] & 0x80))
    {
        //��վ����ȷ�ҹ�������ȷ
        if((RTU_PORT.RXBuffer[0] == ucNodeAddr) && (RTU_PORT.RXBuffer[1] == 0x03))
        {
            //�Խ��յ���HoldRegֵ����ת����ֵ������3��ʼ��
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
    //�����վ�쳣
    else
    {
        return false;
    }
}

//--------------------------------function code:0x04---------------------------------------------------
//Function:0x04
//��AI:����洢��
void RTU_Master::masterFunc_0x04(u8 ucNodeAddr, u16 usDataAddr, u16 usNum)
{
    bool bUnCode = false;
    masterStatus.bDone = false;
    //Ԫ����ַ:3xxxx
    u16 Addr = usDataAddr - 30000;

    //�������ݴ��Ϊ����֡��
    enCode_0x04(ucNodeAddr, Addr, usNum);
    //����
    RTU_PORT.SendFrame();
    //���ͺ�ת����ա�
    RTU_PORT.ReceiveFrame();
    //Ӧ��ʱ���ʹ�ܡ�
    RTU_PORT.timeRespTimeOut_Start();

    //�ȴ�����
    while((!RTU_PORT.portStatus.bReadEnb) && !RTU_PORT.portStatus.bErr);

    if(RTU_PORT.portStatus.bReadEnb)
        bUnCode = unCode_0x04(ucNodeAddr);

    judge("F0x04", bUnCode);
}

//����0x04
//�����Ľ�������TX_Struct�С�
bool RTU_Master::enCode_0x04(u8 ucNodeAddr, u16 usDataAddr, u16 usNum)
{
    u16 CRC16;

    if(usNum > 125) return false; //����ȡ������Ԫ���������ܳ���125����
    //���ͻ���������
    RTU_PORT.usTXIndex = 0;
    //��վ��ַ
    RTU_PORT.TXBuffer[0] = ucNodeAddr;
    //������
    RTU_PORT.TXBuffer[1] = 0x04;
    //Ԫ����ַ
    RTU_PORT.TXBuffer[2] = usDataAddr >> 8;
    RTU_PORT.TXBuffer[3] = usDataAddr;
    //Ԫ������
    RTU_PORT.TXBuffer[4] = usNum >> 8; //���ֽ�
    RTU_PORT.TXBuffer[5] = usNum;  //���ֽ�

    //����CRC����
    //���ɵ�CRC16���ֽ���ǰ��
    RTU_PORT.usTXIndex = 6;
    CRC16 = RTU_PORT.CRC16Gen();
    //���CRC16,���ֽ���ǰ
    RTU_PORT.TXBuffer[6] = CRC16;
    RTU_PORT.TXBuffer[7] = CRC16 >> 8;
    //֡�����ֽ�����
    RTU_PORT.usTXIndex = 8;
    return true;
}

//�������ݽ���0x04
//ucPtr���ڴ�Ŷ�ȡ��HoldRegֵ��
bool RTU_Master::unCode_0x04(u8 ucNodeAddr)
{
    //������յ��Ĺ������λ��Ϊ1�������������
    if(!(RTU_PORT.RXBuffer[1] & 0x80))
    {
        //��վ����ȷ�ҹ�������ȷ
        if((RTU_PORT.RXBuffer[0] == ucNodeAddr) && (RTU_PORT.RXBuffer[1] == 0x04))
        {
            //�Խ��յ���AIֵ����ת����ֵ������3��ʼ��
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
    //�����վ�쳣
    else{
        return false;
    }
}


/*------------------------------------------------------------*/
//--------------------------------function code:0x02---------------------------------------------------
//Function:0x02
//�����������������ɢ��DI
void RTU_Master::masterFunc_0x02(u8 ucNodeAddr, u16 usDataAddr, u16 usNum)
{
    bool bUnCode = false;
    masterStatus.bDone = false;
    //Ԫ����ַ:1xxxx
    u16 Addr = usDataAddr - 10000;

    //�������ݴ��Ϊ����֡��
    enCode_0x02(ucNodeAddr, Addr, usNum);
    //����
    RTU_PORT.SendFrame();
    //���ͺ�ת����ա�
    RTU_PORT.ReceiveFrame();
    //Ӧ��ʱ���ʹ�ܡ�
    RTU_PORT.timeRespTimeOut_Start();

    //�ȴ�����ת��
    while((!RTU_PORT.portStatus.bReadEnb) && !RTU_PORT.portStatus.bTimeOut);

    if(RTU_PORT.portStatus.bReadEnb)
        bUnCode = unCode_0x02(ucNodeAddr);

    judge("F0x02", bUnCode);
}

//����0x02
bool RTU_Master::enCode_0x02(u8 ucNodeAddr, u16 usDataAddr, u16 usNum)
{
    u16 CRC16;

    if(usNum > 2000) return false; //����ȡ������Ԫ���������ܳ���2000����
    //���ͻ���������
    RTU_PORT.usTXIndex = 0;
    //��վ��ַ
    RTU_PORT.TXBuffer[0] = ucNodeAddr;
    //������
    RTU_PORT.TXBuffer[1] = 0x02;
    //Ԫ����ַ
    RTU_PORT.TXBuffer[2] = usDataAddr >> 8;
    RTU_PORT.TXBuffer[3] = usDataAddr;
    //Ԫ������
    RTU_PORT.TXBuffer[4] = usNum >> 8; //���ֽ�
    RTU_PORT.TXBuffer[5] = usNum;  //���ֽ�

    //����CRC����
    //���ɵ�CRC16���ֽ���ǰ��
    RTU_PORT.usTXIndex = 6;
    CRC16 = RTU_PORT.CRC16Gen();
    //���CRC16,���ֽ���ǰ
    RTU_PORT.TXBuffer[6] = CRC16;
    RTU_PORT.TXBuffer[7] = CRC16 >> 8;
    //֡�����ֽ�����
    RTU_PORT.usTXIndex = 8;
    return true;
}

//�������ݽ���0x02
//ucPtr���ڴ�Ŷ�ȡ��DIֵ��
bool RTU_Master::unCode_0x02(u8 ucNodeAddr)
{
    //������յ��Ĺ������λ��Ϊ1�������������
    if(!(RTU_PORT.RXBuffer[1] & 0x80))
    {
        //��վ����ȷ�ҹ�������ȷ
        if((RTU_PORT.RXBuffer[0] == ucNodeAddr) && (RTU_PORT.RXBuffer[1] == 0x02))
        {
            //�Խ��յ���DIֵ����ת����ֵ������3��ʼ��
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
    //�����վ�쳣
    else{
        return false;
    }
}

//--------------------------------function code:0x0F---------------------------------------------------
//Function:0x0F
//д��������������ɢ��DQ
void RTU_Master::masterFunc_0x0F(u8 ucNodeAddr, u16 usDataAddr, u16 usNum)
{
    bool bUnCode = false;

    masterStatus.bDone = false;
    //Ԫ����ַ 0xxxx
    u16 Addr = usDataAddr - 0;

    //�������ݴ��Ϊ����֡��
    enCode_0x0F(ucNodeAddr, Addr, usNum);
    //����
    RTU_PORT.SendFrame();
    //���ͺ�ת����ա�
    RTU_PORT.ReceiveFrame();

    //Ӧ��ʱ���ʹ�ܡ�
    RTU_PORT.timeRespTimeOut_Start();

    //�ȴ�����
    while((!RTU_PORT.portStatus.bReadEnb) && !RTU_PORT.portStatus.bTimeOut);

    if(RTU_PORT.portStatus.bReadEnb)
        bUnCode = unCode_0x0F(ucNodeAddr);

    judge("F0x0F", bUnCode);
}

//����
bool RTU_Master::enCode_0x0F(u8 ucNodeAddr, u16 usDataAddr, u16 usNum)
{
    u16 CRC16;
    u16 j;
    if(usNum > 1976) return false;  //Ԫ���������ܴ���1976.

    //���ͻ���������
    RTU_PORT.usTXIndex = 0;
    //��վ��ַ
    RTU_PORT.TXBuffer[0] = ucNodeAddr;
    //������
    RTU_PORT.TXBuffer[1] = 0x0F;
    //Ԫ����ַ
    RTU_PORT.TXBuffer[2] = usDataAddr >> 8;
    RTU_PORT.TXBuffer[3] = usDataAddr;
    //Ԫ������
    RTU_PORT.TXBuffer[4] = usNum >> 8; //���ֽ�
    RTU_PORT.TXBuffer[5] = usNum;  //���ֽ�
    //���ݳ���:(usNum%8)?usNum/8+1:usNum/8;
    RTU_PORT.TXBuffer[6] = (usNum%8)? usNum/8+1: usNum/8;
    //������,����ȡ��������usNum.
    for(int i = 0; i < RTU_PORT.TXBuffer[6]; i++)
    {
        RTU_PORT.TXBuffer[7 + i] = *(User_DataBuffer.ucData + i);
    }

    //����CRC����
    //j��CRC��ſ�ʼ��Ԫ,���ֽ���ǰ��
    j = RTU_PORT.TXBuffer[6] + 7;
    //���ɵ�CRC16���ֽ���ǰ��
    RTU_PORT.usTXIndex = j;
    CRC16 = RTU_PORT.CRC16Gen();
    //���CRC16,���ֽ���ǰ
    RTU_PORT.TXBuffer[j] = CRC16;
    RTU_PORT.TXBuffer[j+1] = CRC16 >> 8;
    //֡�����ֽ�����
    RTU_PORT.usTXIndex = j + 2;
    return true;
}

//�������ݽ���
//ucPtr���ڴ�Ŷ�ȡ��DIֵ��
bool RTU_Master::unCode_0x0F(u8 ucNodeAddr)
{
    //������յ��Ĺ������λ��Ϊ1�������������
    if(!(RTU_PORT.RXBuffer[1] & 0x80))
    {
        //ʡ�ԱȽϷ��ص��������ݣ������롢Ԫ����ַ��Ԫ��������
        //��վ����ȷ
        if(RTU_PORT.RXBuffer[0] == ucNodeAddr)
            return true;
        else
            return false;
    }
    //�����վû�з�����ᳬʱ.
    //�����վ�쳣��
    else{
        return false;
    }
}

//--------------------------------function code:0x01---------------------------------------------------
//Function:0x01
//��ȡ������������ɢ��DQ
void RTU_Master::masterFunc_0x01(u8 ucNodeAddr, u16 usDataAddr, u16 usNum)
{
    bool bUnCode = false;

    masterStatus.bDone = false;
    //Ԫ����ַ:0xxxx
    u16 Addr = usDataAddr - 00000;

    //�������ݴ��Ϊ����֡��
    enCode_0x01(ucNodeAddr, Addr, usNum);
    //����
    RTU_PORT.SendFrame();
    //���ͺ�ת����ա�
    RTU_PORT.ReceiveFrame();
    //Ӧ��ʱ���ʹ�ܡ�
    RTU_PORT.timeRespTimeOut_Start();

    //�ȴ�����
    while((!RTU_PORT.portStatus.bReadEnb) && !RTU_PORT.portStatus.bTimeOut);

    if(RTU_PORT.portStatus.bReadEnb)
        bUnCode = unCode_0x01(ucNodeAddr);

    judge("F0x01", bUnCode);
}

//����
bool RTU_Master::enCode_0x01(u8 ucNodeAddr, u16 usDataAddr, u16 usNum)
{
    u16 CRC16;

    if(usNum > 2000) return false; //����ȡ������Ԫ���������ܳ���2000����
    //���ͻ���������
    RTU_PORT.usTXIndex = 0;
    //��վ��ַ
    RTU_PORT.TXBuffer[0] = ucNodeAddr;
    //������
    RTU_PORT.TXBuffer[1] = 0x01;
    //Ԫ����ַ
    RTU_PORT.TXBuffer[2] = usDataAddr >> 8;
    RTU_PORT.TXBuffer[3] = usDataAddr;
    //Ԫ������
    RTU_PORT.TXBuffer[4] = usNum >> 8; //���ֽ�
    RTU_PORT.TXBuffer[5] = usNum;  //���ֽ�

    //����CRC����
    //���ɵ�CRC16���ֽ���ǰ��
    RTU_PORT.usTXIndex = 6;
    CRC16 = RTU_PORT.CRC16Gen();
    //���CRC16,���ֽ���ǰ
    RTU_PORT.TXBuffer[6] = CRC16;
    RTU_PORT.TXBuffer[7] = CRC16 >> 8;
    //֡�����ֽ�����
    RTU_PORT.usTXIndex = 8;
    return true;
}

//�������ݽ���
bool RTU_Master::unCode_0x01(u8 ucNodeAddr)
{
    //������յ��Ĺ������λ��Ϊ1�������������
    if(!(RTU_PORT.RXBuffer[1] & 0x80))
    {
        //��վ����ȷ�ҹ�������ȷ
        if((RTU_PORT.RXBuffer[0] == ucNodeAddr) && (RTU_PORT.RXBuffer[1] == 0x01))
        {
            //�Խ��յ���DIֵ����ת����ֵ������3��ʼ��
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
    //�����վ�쳣
    else{
        return false;
    }
}


//��������֡�жϡ�
void RTU_Master::judge(char* ptrFunCode, bool bUnCode)
{
    if(RTU_PORT.portStatus.bReadEnb)
    {
        //����������ݲ�����
        if(!bUnCode)
        {
            masterStatus.bErr = true;
            masterStatus.usErrMsg = 1;
            printf("%s����֡��������Ϣ���������վ����֡ \n\n\n\n", ptrFunCode);
            Usart_SendFrame(USART1, User_DataBuffer.ucData, User_DataBuffer.usIndex);
            printf("\n");
        }
        //������ȷ��
        else
        {
            //��λ��ɡ�д��������Ҫת�����ص��������ݡ�
            masterStatus.bDone = true;
            masterStatus.bErr = false;
        }
    }
    //�������CRCУ��ʧ�ܡ�
    else if(RTU_PORT.portStatus.bErr && RTU_PORT.portStatus.usErr == 2)
    {
        masterStatus.bErr = true;
        masterStatus.usErrMsg = 2;
        printf("%s����֡CRC16У��ʧ�ܣ�\n", ptrFunCode);
        //��ӡ֡����
        for(int i = 0; i < RTU_PORT.usRXIndex; i++)
            printf("0x%x ", RTU_PORT.RXBuffer[i]);
        printf("\r\n");
    }
    //���Ӧ��ʱ�򲻽���,�����¸������ط��������һ֡���ͣ���
    else if(RTU_PORT.portStatus.bTimeOut)
    {
        masterStatus.bErr = true;
        masterStatus.usErrMsg = 4;
        RTU_PORT.portStatus.unErrCount++;//ͨѶ�������ͳ�ơ�
        printf("%s���ճ�ʱ!\n", ptrFunCode);
    }
}

