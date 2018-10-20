//�жϷ�����������extern "C"{  }��Χ,��ֹ�������ı亯����ΪC++��ʽ�ĳ�������.

#include "dev.h"
#include "RTU_RXTX.h"

#ifdef __cplusplus
    extern "C" {
#endif
#include <stdio.h>
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
        ucData = USART_ReceiveData(USART2);
        RTU_PORT_ALIAS.portStatus.bBusy = true;  //æ
        RTU_PORT_ALIAS.portStatus.bReadEnb = false; //֡���ɶ�ȡ��

        //������ֽ��մ���,�����ô�����Ϣ.
        if(USART_GetFlagStatus(USART2, USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE))
        {
           RTU_PORT_ALIAS.portStatus.usErr = 3;  //ͨѶ���մ���
           RTU_PORT_ALIAS.portStatus.bErr = true;
        }
        else
        {            
            RTU_PORT_ALIAS.portStatus.bErr = false;
        }

        //����ֽڽ����޴��ҽ��ջ�����û��Խ�������ת������,����������t3.5��ʱ��.
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

//t3.5��ʱ���жϷ�������
//t1.5���ڼ���ֽڽ����Ƿ�ʱ,����汾ȡ���ֽ������Լ�⡣
//t3.5���ڼ��֡�Ƿ����.
//��t3.5�����ڽ��ռ��֡����Ҳ���ڷ���֡����ʱ.
//�������t3.5�ж����ʾ����֡������֡���ͽ���,ת�����״̬.
void TIM6_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM6, TIM_IT_Update) == SET)
    {
        //���֡�ɶ����ȴ�����֡ʱ���ٽ������ݣ���ģʽ��Ϊ����,������յ������ݱ���ӵ�֡ĩ������⡣
        RTU_PORT_ALIAS.RS485_TX();  //����ʹ��
        
        //��λӦ��ʱ��ʱ����ֹͣ������
        RTU_PORT_ALIAS.timeRespTimeOut_Stop();
        RTU_PORT_ALIAS.portStatus.bTimeOut = false;
        //��λt3.5��ʱ����ʧ��,ֹͣ��ʱ���.
        RTU_PORT_ALIAS.timeFrameEnd_Stop();

        //�����ڽ���ʱУ��CRC16.
        if(RTU_PORT_ALIAS.portStatus.bMode == bRXMode)
        {
            printf("���ս���\r\n");
            //�ж�����֡����Ч��.
            //ֻ��ͨѶ����֡�������жϣ�Ҳ��CRCУ�顣
            if(RTU_PORT_ALIAS.CRC16Check())
            {
                RTU_PORT_ALIAS.portStatus.bErr = false;   //����֡�ɶ�ʱbErrΪfalse�����Կ���ȡ����bErr���жϡ�               
                RTU_PORT_ALIAS.portStatus.bReadEnb = true;  //֡�ɶ�ȡ��                
            }
            //���CRCʧ�ܣ�������֡�����½��ա�
            else
            {
                RTU_PORT_ALIAS.portStatus.unErrCount++;//ͨѶ�������ͳ�ơ�
                RTU_PORT_ALIAS.portStatus.bErr = true;
                RTU_PORT_ALIAS.portStatus.usErr = 2; //CRCУ��ʧ�ܡ�
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
            printf("���ͽ���\r\n");
        }
    }
}

//Ӧ��ʱ��ʱ���жϷ�����
//������վģʽ,����վ����ͨѶ�ȴ���վӦ��ʱ����Ƿ�ʱ.
//����ʱ�����ճ�ʱ��
void TIM7_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM7, TIM_IT_Update) == SET)
    {
        printf("in TIM7,��ʱ!\r\n");
        RTU_PORT_ALIAS.portStatus.bErr = true;
        RTU_PORT_ALIAS.portStatus.usErr = 4;
        RTU_PORT_ALIAS.portStatus.bTimeOut = true;
        //��λӦ��ʱ��ʱ�䲢ֹͣ������
        RTU_PORT_ALIAS.timeRespTimeOut_Stop();
    }
}

#ifdef __cplusplus
}
#endif
