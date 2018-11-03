//中断服务函数必须以extern "C"{  }包围,防止编译器改变函数名为C++格式的长函数名.

#include "dev.h"
#include "RTU_Master.h"
#include "RTU_Config.h"
#include <stdio.h>

//本文件是作为主站时的中断函数。
#if MASTERORSLAVE == bMaster

#ifdef __cplusplus
    extern "C" {
#endif
//#include <stdio.h>
//串口2接收中断服务函数
//当串口接收到一个字节时发生该中断.
void USART2_IRQHandler(void)
{
    u8 ucData;
    if(USART_GetITStatus(USART2, USART_IT_RXNE) == SET)
    {
        LED1_ON;
        //复位应答超时定时器并停止工作。
        //当超时时间远远大于数据接收时间，而超时定时器和帧结束定时器使用同一个定时器时，停止超时定时器显得无用了。
        //这种情况下可以删除停止超时定时器的语句。
        //RTU_PORT_ALIAS.timeRespTimeOut_Stop();
        //RTU_PORT_ALIAS.portStatus.bTimeOut = false;

        //接收字节数据并自动清除中断标识.
        ucData                             = USART_ReceiveData(USART2);
        RTU_PORT_ALIAS.portStatus.bBusy    = true;  //忙
        RTU_PORT_ALIAS.portStatus.bReadEnb = false; //帧不可读取。  
        //当第一次接收到数据字节并停止了超时定时器后就失能接收中断。
        //USART_ITConfig(USART2, USART_IT_RXNE,DISABLE);
        //printf("in rxne\r\n");

        //如果字节接收无错则接收转储数据,并重新启动t3.5定时器.
        //否则，则不储存数据到接收缓冲区，将导致CRC校验错误，也会让最终结果错误。
       
        //转储接收到的字节数据.
        RTU_PORT_ALIAS.saveAData(ucData);
        //RTU_PORT_ALIAS.usRXIndex++;
        //复位并重新启动t3.5定时器,监测数据帧是否结束.
        RTU_PORT_ALIAS.timeFrameEnd_Start();

    }
    /*
    //串口2空闲：一帧通讯结束。
    if(USART_GetITStatus(USART2, USART_IT_IDLE) == SET)
    {
        u8 dataTmp;
        dataTmp = USART2->SR;
        dataTmp = USART2->DR;
        
        //确定接收到的字节数量。     
        RTU_PORT_ALIAS.usRXIndex = FRAME_MAXLEN - DMA_GetCurrDataCounter(DMA1_Channel6); 
        
        //判断数据帧的有效性.
        //只对通讯数据帧本身作判断，也即CRC校验。
        if(RTU_PORT_ALIAS.CRC16Check())
        {
            RTU_PORT_ALIAS.portStatus.bErr     = false;   //数据帧可读时bErr为false，所以可以取消对bErr的判断。
            RTU_PORT_ALIAS.portStatus.bReadEnb = true;  //帧可读取。
        }
        //如果CRC失败，则丢弃本帧。
        else
        {
            RTU_PORT_ALIAS.portStatus.unErrCount++;//通讯错误次数统计。
            RTU_PORT_ALIAS.portStatus.bErr     = true;
            RTU_PORT_ALIAS.portStatus.usErrMsg = 2; //CRC校验失败。
            RTU_PORT_ALIAS.portStatus.bReadEnb = false;  //帧不可读。
        }
        //设置系统状态进入空闲.
        RTU_PORT_ALIAS.portStatus.bBusy = false;
        //LED1_OFF;
        //重新设置DMA，使之可以重新传送到缓冲区开头。
        DMA_Cmd(DMA1_Channel6, DISABLE );  //关闭USART1 TX DMA1 所指示的通道      
        DMA_SetCurrDataCounter(DMA1_Channel6,FRAME_MAXLEN);//DMA通道的DMA缓存的大小
        DMA_Cmd(DMA1_Channel6, ENABLE);  //使能USART1 TX DMA1 所指示的通道 
    }
    */
}

//TIM6即用于帧结束定时也用于接收超时监测。
//当用于帧结束定时时，即用于发送结束的附加帧间延时，也用于接收时检测帧是否结束。
void TIM6_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM6, TIM_IT_Update) == SET)
    {
        //复位t3.5定时器并停止定时监测.
        RTU_PORT_ALIAS.timeFrameEnd_Stop();

        //等待处理帧时不再接收数据，把模式改为发送,以免接收到的数据被添加到帧末造成问题。
        RTU_PORT_ALIAS.RS485_TX();  //发送使能

        // 当帧结束监测器工作时：
        if(RTU_PORT_ALIAS.whichTimerRun() == FRAMEEND_TIMERRUN)
        {
            //当处于接收时校验CRC16.
            if(RTU_PORT_ALIAS.portStatus.bMode == RXMODE)
            {
                
                //printf("接收结束\r\n");
                //判断数据帧的有效性.
                //只对通讯数据帧本身作判断，也即CRC校验。
                if(RTU_PORT_ALIAS.CRC16Check())
                {
                    RTU_PORT_ALIAS.portStatus.bErr     = false;   //数据帧可读时bErr为false，所以可以取消对bErr的判断。
                    RTU_PORT_ALIAS.portStatus.bReadEnb = true;  //帧可读取。
                }
                //如果CRC失败，则丢弃本帧，重新接收。
                else
                {
                    RTU_PORT_ALIAS.portStatus.unErrCount++;//通讯错误次数统计。
                    RTU_PORT_ALIAS.portStatus.bErr     = true;
                    RTU_PORT_ALIAS.portStatus.usErrMsg = 2; //CRC校验失败。
                    RTU_PORT_ALIAS.portStatus.bReadEnb = false;  //帧不可读。
                    //丢弃本次数据帧，并重新启动接收。
                    //RTU_PORT_ALIAS.ReceiveFrame();
                }
                //设置系统状态进入空闲.
                RTU_PORT_ALIAS.portStatus.bBusy = false;
                LED1_OFF;
                
            }
            //如果处于发送则不作校验。
            else
            {
                //设置系统状态进入空闲.
                RTU_PORT_ALIAS.portStatus.bBusy = false;
            }
        }
        //当超时监测器工作时：
        else if(RTU_PORT_ALIAS.whichTimerRun() == TIMEOUT_TIMERRUN)
        {
            RTU_PORT_ALIAS.portStatus.bErr     = true;
            RTU_PORT_ALIAS.portStatus.usErrMsg = 4;
            RTU_PORT_ALIAS.portStatus.bTimeOut = true;
            //复位应答超时定时器并停止工作。
            RTU_PORT_ALIAS.timeRespTimeOut_Stop();
            RTU_PORT_ALIAS.portStatus.bBusy = false;
        }
    }
}

//通讯定时器中断服务函数
//中断时进行一次主站通讯。
//中断一次执行一次所有的通讯处理，所以最好保证比较少的通讯次数。
//也可以分时通讯：通讯一次执行其中的一组通讯，下次中断通讯再执行另一组通讯,合适于中断时间比较短的方式。
void TIM7_IRQHandler(void)
{
    extern float usDataTemp[50];
    extern RTU_Master myNeo_Master;
    //通讯步序。
    static int iStep = 1;

    if(TIM_GetITStatus(TIM7, TIM_IT_Update) == SET)
    {
        //NVIC_ClearPendingIRQ(TIM7_IRQn);//test
        TIM_Cmd(TIM7, DISABLE);
        TIM_ClearITPendingBit(TIM7, TIM_IT_Update); //清除定时器中断更新标识.
        TIM_SetCounter(TIM7, 0);

        //write HoldReg F0x03
        if(iStep == 1)
        {
            myNeo_Master.master(4, bRead, 40000, 10);
            iStep = 2;
        }
        else if(iStep == 2)
        {
            myNeo_Master.read32bitData_BMode((u32*)usDataTemp, 5);//测试结果：slave采用的是大端模式，也就是按照协议要求显示。
            myNeo_Master.master(4, bWrite, 40020, 10);
            iStep = 1;
        }

         TIM_ClearITPendingBit(TIM7, TIM_IT_Update); //清除定时器中断更新标识.
         TIM_SetCounter(TIM7, 0);
         TIM_Cmd(TIM7, ENABLE);
    }
}

#ifdef __cplusplus
}
#endif

#endif
