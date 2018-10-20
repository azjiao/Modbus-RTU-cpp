/*********************************************************
  ���ܣ� ����Modbus-RTUͨѶ����
  ������ ��Ӣ��485�ӿ���USART2�ڣ�ʹ�ô���2����Modbus-RTU���顣
  ��ƣ� azjiao
  �汾�� 0.1
  ���ڣ� 2018��07��18��
 *********************************************************/

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "dev.h"

#define MORS  true

void assert_failed(uint8_t* file, uint32_t line)
{
    //printf("Param Error! in file name: xxx, in line %d\n",line);
    //while(1);
}

#if MORS
#include "RTU_Master.h"
RTU_Master myProtocol2;
#else
#include "RTU_Slave.h"
RTU_Slave myProtocol1;
#endif
static TimerType timer_RTU_Comm;
int main(void)
{
    int iStep = 0;
    int iCount = 0;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    led_Init();
    key_Init();
    delay_Init();
    beep_Init();
    Usart1_Init(19200);

    // Iwdg_Init(4, 625);  //�������Ź���ʼ����Ԥ��Ƶϵ��4��Ӧ64��RLRֵΪ625���������Ź���ʱ1s��

    printf("in main....\r\n");

#if MORS
    myProtocol2.master_Init();
    printf("in main after master init....\r\n");

    printf("OK!,myProtocol data is:\r\n");
    u8 ucDataTemp[50] = {100,200,124,255,234,199,1,33,44,55,99};
    //ģ��С��ģʽ��
    //u16 usDataTemp[50] = {100,200,300,400,500,600,700,800,900,1000,1024};
    float usDataTemp[50] = {1236.45,4567.343,5932,9.0003}; //float is 4byte data.

    float usTemp = 0;
    printf("ucDataTemp��ԭʼ�����ǣ�\r\n");
    for(int i = 0; i < 32; i++)
    {
        if(i % 8 == 0)
            printf("\r\n");
        printf("0x%x\t", *(((u8*)&usDataTemp) +i));

    }

    printf("\r\n");
    int iErr = Neo_RTUPort.portStatus.unErrCount;
    printf("count of commit is:%d\r\n", iCount);
    printf("count of err is:%d\r\n", iErr);
    printf("-----------------------------\r\n");
    printf("�򻺳���д��С�����ݣ�\r\n");
    //myProtocol2.write16bitData_LMode((u16*)usDataTemp,50);
    myProtocol2.write32bitData_LMode((u32*)usDataTemp,50);
    myProtocol2.printBuff();//�����Դ��ģʽ�洢��
    iStep = 1;

    bool bTComm_Act = true;
    bool bMode;
    u8 ucNodeAddr;
    u32 unDataAddr;
    u16 usDataNum;

    while(1)
    {                
        TimeON(bTComm_Act, 500U, &timer_RTU_Comm);     
        
        if(!bTComm_Act)
            bTComm_Act = true;
              
        if(timer_RTU_Comm.bQ)  
        {          
            //write HoldReg F0x03
            if(iStep == 1)
            {
                ucNodeAddr = 4;
                bMode = bRead;
                unDataAddr = 40000;
                usDataNum = 10;
                myProtocol2.master(ucNodeAddr, bMode, unDataAddr, usDataNum);
                if(RTU_PORT_ALIAS.portStatus.bErr)
                    printf("Err!--> iCount = %d,iStep = %d.\tbErr=%d,usErr=%d\r\n", iCount, iStep, RTU_PORT_ALIAS.portStatus.bErr, RTU_PORT_ALIAS.portStatus.usErr);
                //���ͨѶ��ɻ�ͨѶ����ת����һ����
                if(myProtocol2.masterStatus.bDone || RTU_PORT_ALIAS.portStatus.bErr)
                    iStep = 2;                
            }
            if(iStep == 2)
            {                       
                myProtocol2.read32bitData_BMode((u32*)usDataTemp, 5);//���Խ����slave���õ��Ǵ��ģʽ��Ҳ���ǰ���Э��Ҫ����ʾ��                
                
                for(int i = 0; i < 5; i++)
                {
                    printf("%f\t", *(((float*)&usDataTemp) + i));
                }
                printf("\r\n");
                
                ucNodeAddr = 4;
                bMode = bWrite;
                unDataAddr = 40020;
                usDataNum = 10;
                myProtocol2.master(ucNodeAddr, bMode, unDataAddr, usDataNum);
                if(RTU_PORT_ALIAS.portStatus.bErr)
                    printf("Err!--> iCount = %d,iStep = %d.\tbErr=%d,usErr=%d\r\n", iCount, iStep, RTU_PORT_ALIAS.portStatus.bErr, RTU_PORT_ALIAS.portStatus.usErr);
                if(myProtocol2.masterStatus.bDone || RTU_PORT_ALIAS.portStatus.bErr)
                    iStep = 3;
             }                
            
            if(iStep == 3)
            {
                iStep = 1;
                //printf("-----------------------------\r\n");
            }           
           
           ++iCount;
            if(iCount % 20 == 0)
                printf("iCount = %d,iStep = %d.\tbErr=%d,usErr=%d\r\n", iCount, iStep, RTU_PORT_ALIAS.portStatus.bErr, RTU_PORT_ALIAS.portStatus.usErr);

            bTComm_Act = false;
        }

    }





#else


    myProtocol1.slave_Init();

    printf("OK!,myProtocol data is:\r\n");
    Neo_RTUPort.printData();
    while(1)
    {
        myProtocol1.slaveService();
        if(Neo_RTUPort.portStatus.bBusy){
            LED1_OFF;
            LED0_ON;
        }
        else{
            LED0_OFF;
            LED1_ON;
        }
        //test
        if((myProtocol1.read_SourceHg(10)) != 0)
            printf("40010 is:%d\r\n", myProtocol1.read_SourceHg(10));
        if((myProtocol1.read_SourceDQ(10)) != 0)  //0080��ʼ��8λDQ��
            printf("00010 is:%d\r\n", myProtocol1.read_SourceDQ(10));
        // Iwdg_Feed();
    }

#endif
}





