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

    // Iwdg_Init(4, 625);  //独立看门狗初始化：预分频系数4对应64，RLR值为625，这样看门狗定时1s。

    printf("in main....\r\n");

#if MORS
    myProtocol2.master_Init();
    printf("in main after master init....\r\n");

    printf("OK!,myProtocol data is:\r\n");
    u8 ucDataTemp[50] = {100,200,124,255,234,199,1,33,44,55,99};
    //模拟小端模式。
    //u16 usDataTemp[50] = {100,200,300,400,500,600,700,800,900,1000,1024};
    float usDataTemp[50] = {1236.45,4567.343,5932,9.0003}; //float is 4byte data.

    float usTemp = 0;
    printf("ucDataTemp的原始数据是：\r\n");
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
    printf("向缓冲区写入小端数据：\r\n");
    //myProtocol2.write16bitData_LMode((u16*)usDataTemp,50);
    myProtocol2.write32bitData_LMode((u32*)usDataTemp,50);
    myProtocol2.printBuff();//总是以大端模式存储。
    iStep = 1;

    bool bTComm_Act = true;
    while(1)
    {
        TimeON(bTComm_Act, 10U, &timer_RTU_Comm);
        if(!bTComm_Act)
            bTComm_Act = true;

        if(timer_RTU_Comm.bQ)
        {
            //for(int i = 0; i < 500000; i++);

            //write HoldReg F0x03
            if(iStep == 1)
            {
                //printf("in Step 1\r\n");
                myProtocol2.master(4, bRead, 40000, 10);
                if(myProtocol2.masterStatus.bDone)
                {
                    iStep = 2;
                }
            }
            if(iStep == 2)
            {
                //printf("in Step 2\r\n");
                //printf("从从站读出的原始数据是：\r\n");
                // myProtocol2.printBuff(); //经过读取，slave以大端模式存放和发送。
                // 以32位整数方式读取,正常是以大端模式读取。
                myProtocol2.read32bitData_BMode((u32*)usDataTemp, 5);//测试结果：slave采用的是大端模式，也就是按照协议要求显示。

                //            for(int i = 0; i < 10; i++)
                //            {
                //                usTemp = *(usDataTemp + i);
                //                printf("大端模式读取的数据为：%f\r\n", usTemp);
                //            }

                myProtocol2.master(4, bWrite, 40020, 20);
                if(myProtocol2.masterStatus.bDone)
                {
                    iStep = 1;
                    //printf("-----------------------------\r\n");
                }
            }

            if(++iCount % 200 == 0)
                printf("iCount = %d\r\n", iCount);

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
        if((myProtocol1.read_SourceDQ(10)) != 0)  //0080开始的8位DQ。
            printf("00010 is:%d\r\n", myProtocol1.read_SourceDQ(10));
        // Iwdg_Feed();
    }

#endif
}





