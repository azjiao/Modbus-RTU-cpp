/*********************************************************
���ܣ� Modbus RTUͨѶ���û������ļ���
������ �������ļ���.hͷ�ļ���������ѭ.h�ļ�����
��ƣ� azjiao
�汾�� 0.1
���ڣ� 2018��10��11��
*********************************************************/
#ifndef __RTU_CONFIG_H
#define __RTU_CONFIG_H

#include "RTU_RXTX.h"
//-----------------------------------------------------
//�û�����
const bool MASTERORSLAVE = bSlave;   //bMaster��վ����bSlave��վģʽ

const u8  NODEADDR = 3;  //Modbusվ���ַ,��վ��ַ����.
const u32 BAUDRATE = 9600;  //������
const u16 DATABIT = 8;  //����λ����
const u16 STOPBIT = 1;  //ֹͣλ
const u16 PARITY = 0;  //��żУ�飺0��У�飬1��У�飬2żУ��
const u16 usTimeOut = 500;  //��վӦ��ʱʱ��,��λms.

//�����жϷ�����ʹ�õ�RTU_DataCtrlʵ���ı���:���RTU_DataCtrl��ʵ�����ı䣬��һ���ı������Neo_RTUPortΪ���衣
class RTU_DataCtrl;
extern RTU_DataCtrl Neo_RTUPort;
#define RTU_PORT_ALIAS  Neo_RTUPort
//�������������ʹ���˺��ж����治һ���Ĵ�վ��ʵ����myNeo_Slaver�����ô˺���ȡ�����Ա�ʹmyNeo_Slaver���á�
#define myNeo_Slaver myProtocol1


#endif /* end of include guard: __RTU_CONFIG_H */

