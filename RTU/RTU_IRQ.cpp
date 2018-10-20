//中断服务函数必须以extern "C"{  }包围,防止编译器改变函数名为C++格式的长函数名.

#include "dev.h"
#include "RTU_RXTX.h"

#ifdef __cplusplus
    extern "C" {
#endif
#include <stdio.h>
//串口2接收中断服务函数
//当串口接收到一个字节时发生该中断.
void USART2_IRQHandler(void)
{
    u8 ucData;
    if(USART_GetITStatus(USART2, USART_IT_RXNE) == SET)
    {        
        LED1_ON;
        //复位应答超时定时器并停止工作。
        RTU_PORT_ALIAS.timeRespTimeOut_Stop();
        RTU_PORT_ALIAS.portStatus.bTimeOut = false;

        //接收字节数据并自动清除中断标识.
        ucData = USART_ReceiveData(USART2);
        RTU_PORT_ALIAS.portStatus.bBusy = true;  //忙
        RTU_PORT_ALIAS.portStatus.bReadEnb = false; //帧不可读取。

        //如果出现接收错误,则设置错误信息.
        if(USART_GetFlagStatus(USART2, USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE))
        {
           RTU_PORT_ALIAS.portStatus.usErr = 3;  //通讯接收错误
           RTU_PORT_ALIAS.portStatus.bErr = true;
        }
        else
        {            
            RTU_PORT_ALIAS.portStatus.bErr = false;
        }

        //如果字节接收无错且接收缓冲区没有越限则接收转储数据,并重新启动t3.5定时器.
        //否则，则不储存数据到接收缓冲区，将导致CRC校验错误，也会让最终结果错误。
       if(!RTU_PORT_ALIAS.portStatus.bErr)
        {
            //转储接收到的字节数据.
            RTU_PORT_ALIAS.saveAData(ucData);
            //复位并重新启动t3.5定时器,监测数据帧是否结束.
            RTU_PORT_ALIAS.timeFrameEnd_Start();
        }
    }
}

//t3.5定时器中断服务函数。
//t1.5用于监测字节接收是否超时,这个版本取消字节连续性监测。
//t3.5用于监测帧是否结束.
//但t3.5即用于接收监测帧结束也用于发送帧间延时.
//如果发生t3.5中断则表示接收帧结束或帧发送结束,转入空闲状态.
void TIM6_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM6, TIM_IT_Update) == SET)
    {
        //如果帧可读，等待处理帧时不再接收数据，把模式改为发送,以免接收到的数据被添加到帧末造成问题。
        RTU_PORT_ALIAS.RS485_TX();  //发送使能
        
        //复位应答超时定时器并停止工作。
        RTU_PORT_ALIAS.timeRespTimeOut_Stop();
        RTU_PORT_ALIAS.portStatus.bTimeOut = false;
        //复位t3.5定时器并失能,停止定时监测.
        RTU_PORT_ALIAS.timeFrameEnd_Stop();

        //当处于接收时校验CRC16.
        if(RTU_PORT_ALIAS.portStatus.bMode == bRXMode)
        {
            printf("接收结束\r\n");
            //判断数据帧的有效性.
            //只对通讯数据帧本身作判断，也即CRC校验。
            if(RTU_PORT_ALIAS.CRC16Check())
            {
                RTU_PORT_ALIAS.portStatus.bErr = false;   //数据帧可读时bErr为false，所以可以取消对bErr的判断。               
                RTU_PORT_ALIAS.portStatus.bReadEnb = true;  //帧可读取。                
            }
            //如果CRC失败，则丢弃本帧，重新接收。
            else
            {
                RTU_PORT_ALIAS.portStatus.unErrCount++;//通讯错误次数统计。
                RTU_PORT_ALIAS.portStatus.bErr = true;
                RTU_PORT_ALIAS.portStatus.usErr = 2; //CRC校验失败。
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
            printf("发送结束\r\n");
        }
    }
}

//应答超时定时器中断服务函数
//用于主站模式,当主站发起通讯等待从站应答时监测是否超时.
//发生时：接收超时。
void TIM7_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM7, TIM_IT_Update) == SET)
    {
        printf("in TIM7,超时!\r\n");
        RTU_PORT_ALIAS.portStatus.bErr = true;
        RTU_PORT_ALIAS.portStatus.usErr = 4;
        RTU_PORT_ALIAS.portStatus.bTimeOut = true;
        //复位应答超时定时其并停止工作。
        RTU_PORT_ALIAS.timeRespTimeOut_Stop();
    }
}

#ifdef __cplusplus
}
#endif
