/*********************************************************
  功能： 串口Modbus-RTU通讯试验
  描述： 精英版485接口是USART2口，使用串口2进行Modbus-RTU试验。
  设计： azjiao
  版本： 0.1
  日期： 2018年07月18日
 *********************************************************/

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "dev.h"
#include "RTU_Config.h"


void assert_failed(uint8_t* file, uint32_t line)
{
    //printf("Param Error! in file name: xxx, in line %d\n",line);
    //while(1);
}


#if MASTERORSLAVE == bMaster
    #include "RTU_Master.h"
    RTU_Master myProtocol2;
    static TimerType timer_RTU_Comm;
    //使用TIM7作为定时通讯的定时器。每500ms通讯一次。
    BaseTimer CommTimer(7, 100, bUNITMS, 2, 1); 
    float usDataTemp[50] = {1236.45,4567.343,5932,9.0003}; //usDataTemp用于主站通讯中接收自和发送到从站的数据。
#else
    #include "RTU_Slave.h"
    RTU_Slave myProtocol1;
#endif

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    led_Init();
    key_Init();
    delay_Init();
    beep_Init();
    Usart1_Init(19200);    

    printf("in main....\r\n");
    

#if (MASTERORSLAVE == bMaster)               
    printf("in main after master init....\r\n");
    printf("OK!,myProtocol data is:\r\n");    
    printf("ucDataTemp的原始数据是：\r\n");
    for(int i = 0; i < 32; i++)
    {
        if(i % 8 == 0)
            printf("\r\n");
        printf("0x%x\t", *(((u8*)&usDataTemp) +i));

    }
    printf("\r\n");

    //主站初始化。
    myProtocol2.master_Init();
    //通讯定时器初始化。
    CommTimer.timer_Init(bTIMERSTART);
    
    bool bTComm_Act = true;            
   // Iwdg_Init(4, 625);  //独立看门狗初始化：预分频系数4对应64，RLR值为625，这样看门狗定时1s。    
    while(1)
    {                
        TimeON(bTComm_Act, 500U, &timer_RTU_Comm);     
        
        if(!bTComm_Act)
            bTComm_Act = true;
              
        if(timer_RTU_Comm.bQ) 
        //if(key0_Scan(false))
        {          
            bTComm_Act = false;
            if(myProtocol2.masterStatus.bErr)
                printf("通讯出错！出错次数=%d\r\n,上次出错代码=%d\r\n", myProtocol2.masterStatus.unErrCount, myProtocol2.masterStatus.usErrMsg);
            
            for(int i = 0; i < 5; i++)
            {
                printf("%f\t", *(((float*)&usDataTemp) + i));
            }
            printf("\r\n");           
        }
      //  Iwdg_Feed();
    }



#else

    Iwdg_Init(4, 625);  //独立看门狗初始化：预分频系数4对应64，RLR值为625，这样看门狗定时1s。
    myProtocol1.slave_Init();
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
        if((myProtocol1.read_SourceDQ(10)) != 0)  //0080开始的8位DQ。
            printf("00010 is:%d\r\n", myProtocol1.read_SourceDQ(10));
         Iwdg_Feed();
    }

#endif
}





