//�жϷ�����������extern "C"{  }��Χ,��ֹ�������ı亯����ΪC++��ʽ�ĳ�������.

#include "dev.h"
#include "RTU_Master.h"
#include "RTU_Config.h"

//���ļ�����Ϊ��վʱ���жϺ�����
#if MASTERORSLAVE == bMaster

#ifdef __cplusplus
    extern "C" {
#endif
//#include <stdio.h>
//����2�����жϷ�����
//�����ڽ��յ�һ���ֽ�ʱ�������ж�.
void USART2_IRQHandler(void)
{
    u8 ucData;
    if(USART_GetITStatus(USART2, USART_IT_RXNE) == SET)
    {
        LED1_ON;
        //��λӦ��ʱ��ʱ����ֹͣ������
        RTU_PORT_ALIAS.timeRespTimeOut_Stop();
        RTU_PORT_ALIAS.portStatus.bTimeOut = false;

        //�����ֽ����ݲ��Զ�����жϱ�ʶ.
        ucData                             = USART_ReceiveData(USART2);
        RTU_PORT_ALIAS.portStatus.bBusy    = true;  //æ
        RTU_PORT_ALIAS.portStatus.bReadEnb = false; //֡���ɶ�ȡ��

        //������ֽ��մ���,�����ô�����Ϣ.
        //����������ɾ���������ִ���ʱCRC16У���ͨ������
        /*
         *if(USART_GetFlagStatus(USART2, USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE))
         *{
         *   RTU_PORT_ALIAS.portStatus.usErrMsg = 3;  //ͨѶ���մ���
         *   RTU_PORT_ALIAS.portStatus.bErr     = true;
         *}
         *else
         *{
         *    RTU_PORT_ALIAS.portStatus.bErr = false;
         *}
         */

        //����ֽڽ����޴������ת������,����������t3.5��ʱ��.
        //�����򲻴������ݵ����ջ�������������CRCУ�����Ҳ�������ս������
       if(!RTU_PORT_ALIAS.portStatus.bErr)
        {
            //ת�����յ����ֽ�����.
            RTU_PORT_ALIAS.saveAData(ucData);
            //��λ����������t3.5��ʱ��,�������֡�Ƿ����.
            RTU_PORT_ALIAS.timeFrameEnd_Start();
        }
    }
}

//TIM6������֡������ʱҲ���ڽ��ճ�ʱ��⡣
//������֡������ʱʱ�������ڷ��ͽ����ĸ���֡����ʱ��Ҳ���ڽ���ʱ���֡�Ƿ������
void TIM6_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM6, TIM_IT_Update) == SET)
    {
        //��λt3.5��ʱ����ֹͣ��ʱ���.
        RTU_PORT_ALIAS.timeFrameEnd_Stop();

        //�ȴ�����֡ʱ���ٽ������ݣ���ģʽ��Ϊ����,������յ������ݱ���ӵ�֡ĩ������⡣
        RTU_PORT_ALIAS.RS485_TX();  //����ʹ��

        // ��֡�������������ʱ��
        if(RTU_PORT_ALIAS.whichTimerRun() == FRAMEEND_TIMERRUN)
        {
            //�����ڽ���ʱУ��CRC16.
            if(RTU_PORT_ALIAS.portStatus.bMode == RXMODE)
            {
                //printf("���ս���\r\n");
                //�ж�����֡����Ч��.
                //ֻ��ͨѶ����֡�������жϣ�Ҳ��CRCУ�顣
                if(RTU_PORT_ALIAS.CRC16Check())
                {
                    RTU_PORT_ALIAS.portStatus.bErr     = false;   //����֡�ɶ�ʱbErrΪfalse�����Կ���ȡ����bErr���жϡ�
                    RTU_PORT_ALIAS.portStatus.bReadEnb = true;  //֡�ɶ�ȡ��
                }
                //���CRCʧ�ܣ�������֡�����½��ա�
                else
                {
                    RTU_PORT_ALIAS.portStatus.unErrCount++;//ͨѶ�������ͳ�ơ�
                    RTU_PORT_ALIAS.portStatus.bErr     = true;
                    RTU_PORT_ALIAS.portStatus.usErrMsg = 2; //CRCУ��ʧ�ܡ�
                    RTU_PORT_ALIAS.portStatus.bReadEnb = false;  //֡���ɶ���
                    //������������֡���������������ա�
                    //RTU_PORT_ALIAS.ReceiveFrame();
                }
                //����ϵͳ״̬�������.
                RTU_PORT_ALIAS.portStatus.bBusy = false;
                LED1_OFF;
            }
            //������ڷ�������У�顣
            else
            {
                //����ϵͳ״̬�������.
                RTU_PORT_ALIAS.portStatus.bBusy = false;
            }
        }
        //����ʱ���������ʱ��
        else if(RTU_PORT_ALIAS.whichTimerRun() == TIMEOUT_TIMERRUN)
        {
            RTU_PORT_ALIAS.portStatus.bErr     = true;
            RTU_PORT_ALIAS.portStatus.usErrMsg = 4;
            RTU_PORT_ALIAS.portStatus.bTimeOut = true;
            //��λӦ��ʱ��ʱ�䲢ֹͣ������
            RTU_PORT_ALIAS.timeRespTimeOut_Stop();
            RTU_PORT_ALIAS.portStatus.bBusy = false;
        }
    }
}

//ͨѶ��ʱ���жϷ�����
//�ж�ʱ����һ����վͨѶ��
//�ж�һ��ִ��һ�����е�ͨѶ����������ñ�֤�Ƚ��ٵ�ͨѶ������
//Ҳ���Է�ʱͨѶ��ͨѶһ��ִ�����е�һ��ͨѶ���´��ж�ͨѶ��ִ����һ��ͨѶ,�������ж�ʱ��Ƚ϶̵ķ�ʽ��
void TIM7_IRQHandler(void)
{
    extern float usDataTemp[50];
    extern RTU_Master myNeo_Master;
    //ͨѶ����
    static int iStep = 1;

    if(TIM_GetITStatus(TIM7, TIM_IT_Update) == SET)
    {
        //NVIC_ClearPendingIRQ(TIM7_IRQn);//test
        TIM_Cmd(TIM7, DISABLE);
        TIM_ClearITPendingBit(TIM7, TIM_IT_Update); //�����ʱ���жϸ��±�ʶ.
        TIM_SetCounter(TIM7, 0);

        //write HoldReg F0x03
        if(iStep == 1)
        {
            myNeo_Master.master(4, bRead, 40000, 10);
            iStep = 2;
        }
        else if(iStep == 2)
        {
            myNeo_Master.read32bitData_BMode((u32*)usDataTemp, 5);//���Խ����slave���õ��Ǵ��ģʽ��Ҳ���ǰ���Э��Ҫ����ʾ��
            myNeo_Master.master(4, bWrite, 40020, 10);
            iStep = 1;
        }

         TIM_ClearITPendingBit(TIM7, TIM_IT_Update); //�����ʱ���жϸ��±�ʶ.
         TIM_SetCounter(TIM7, 0);
         TIM_Cmd(TIM7, ENABLE);
    }
}

#ifdef __cplusplus
}
#endif

#endif
