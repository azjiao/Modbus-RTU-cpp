//�жϷ�����������extern "C"{  }��Χ,��ֹ�������ı亯����ΪC++��ʽ�ĳ�������.

#include "dev.h"
#include "RTU_Master.h"
#include "RTU_Config.h"
#include <stdio.h>

//���ļ�����Ϊ��վʱ���жϺ�����
#if MASTERORSLAVE == bMaster

#ifdef __cplusplus
    extern "C" {
#endif
//#include <stdio.h>
//����2�����жϷ�����
//�����ڽ�����;��������ʱ�жϣ�����һ���ֽڵĽ���ʱ��(�����ϵͳ����).
void USART2_IRQHandler(void)
{
    //ɾ�������жϡ���ʼ��ʱ�Ѿ��ر��˸��жϡ�    
    
    //����2���У�һ֡ͨѶ������
    if(USART_GetITStatus(USART2, USART_IT_IDLE) == SET)
    {
        //ͨ����ȡ���ͻ���ռĴ�������������жϡ�
        //dataTmp = USART2->SR;
        u8 dataTmp = USART2->DR;
        //�ȴ�����֡ʱ���ٽ������ݣ���ģʽ��Ϊ����,������յ������ݱ���ӵ�֡ĩ������⡣
        RTU_PORT_ALIAS.RS485_TX();  //����ʹ��
        
        //��λӦ��ʱ��ʱ����ֹͣ������
        RTU_PORT_ALIAS.timeRespTimeOut_Stop();
        RTU_PORT_ALIAS.portStatus.bTimeOut = false;
        
        //ȷ�����յ����ֽ�������     
        RTU_PORT_ALIAS.usRXIndex = FRAME_MAXLEN - DMA_GetCurrDataCounter(DMA1_Channel6); 
        
        //�ж�����֡����Ч��.
        //ֻ��ͨѶ����֡�������жϣ�Ҳ��CRCУ�顣
        if(RTU_PORT_ALIAS.CRC16Check())
        {
            RTU_PORT_ALIAS.portStatus.bErr     = false;   //����֡�ɶ�ʱbErrΪfalse�����Կ���ȡ����bErr���жϡ�
            RTU_PORT_ALIAS.portStatus.bReadEnb = true;  //֡�ɶ�ȡ��
        }
        //���CRCʧ�ܣ�������֡��
        else
        {
            RTU_PORT_ALIAS.portStatus.unErrCount++;//ͨѶ�������ͳ�ơ�
            RTU_PORT_ALIAS.portStatus.bErr     = true;
            RTU_PORT_ALIAS.portStatus.usErrMsg = 2; //CRCУ��ʧ�ܡ�
            RTU_PORT_ALIAS.portStatus.bReadEnb = false;  //֡���ɶ���
        }
        //����ϵͳ״̬�������.
        RTU_PORT_ALIAS.portStatus.bBusy = false;
        LED1_OFF;
        
        //��������DMA��ʹ֮�������´��͵���������ͷ��
        DMA_Cmd(DMA1_Channel6, DISABLE );  //�ر�USART1 TX DMA1 ��ָʾ��ͨ��      
        DMA_SetCurrDataCounter(DMA1_Channel6,FRAME_MAXLEN);//DMAͨ����DMA����Ĵ�С
        DMA_Cmd(DMA1_Channel6, ENABLE);  //ʹ��USART1 TX DMA1 ��ָʾ��ͨ�� 
    }   
}

//TIM6���ڽ��ճ�ʱ���ͷ��͸���֡������ʱ��
//������֡������ʱʱ�����ڷ��ͽ����ĸ���֡����ʱ��
void TIM6_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM6, TIM_IT_Update) == SET)
    {       
        // ��֡�������������ʱ�����ڸ��ӷ���֡������ʱ��
        if(RTU_PORT_ALIAS.whichTimerRun() == FRAMEEND_TIMERRUN)
        {        
            //��λt3.5��ʱ����ֹͣ��ʱ���.
            RTU_PORT_ALIAS.timeFrameEnd_Stop();             
            //����ϵͳ״̬�������.
            RTU_PORT_ALIAS.portStatus.bBusy = false;                        
        }
        //����ʱ���������ʱ��
        else if(RTU_PORT_ALIAS.whichTimerRun() == TIMEOUT_TIMERRUN)
        {
            RTU_PORT_ALIAS.portStatus.bErr     = true;
            RTU_PORT_ALIAS.portStatus.usErrMsg = 4;
            RTU_PORT_ALIAS.portStatus.bTimeOut = true;
            //��λӦ��ʱ��ʱ����ֹͣ������
            RTU_PORT_ALIAS.timeRespTimeOut_Stop();
            RTU_PORT_ALIAS.portStatus.bBusy = false;
            LED1_OFF;
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
