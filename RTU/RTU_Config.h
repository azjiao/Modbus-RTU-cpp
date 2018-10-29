/*********************************************************
  功能： Modbus RTU通讯的用户配置文件。
  描述： 本配置文件是.h头文件，必须遵循.h文件规则。
  设计： azjiao
  版本： 0.1
  日期： 2018年10月11日
 *********************************************************/
#ifndef __RTU_CONFIG_H
#define __RTU_CONFIG_H
#include <stm32f10x.h>


//-----------------------------------------------------
//用户配置

#define bMaster true
#define bSlave false
//bMaster主站还是bSlave从站模式
#define MASTERORSLAVE bMaster

//通讯参数
const u8  NODEADDR = 3;  //Modbus站点地址,主站地址忽略.
const u32 BAUDRATE = 9600;  //波特率
const u16 DATABIT  = 8;  //数据位长度
const u16 STOPBIT  = 1;  //停止位
const u16 PARITY   = 0;  //奇偶校验：0无校验，1奇校验，2偶校验
//从站应答超时时间,单位ms.最大范围为655ms。
//使用TIM6做应答超时定时器，和帧结束共用一个定时器。
const u16 TIMEOUTVAL = 500;


//定义中断服务中使用的RTU_DataCtrl实例的别名:如果RTU_DataCtrl的实例名改变，请一并改变这里的Neo_RTUPort为所需。
class RTU_DataCtrl;
extern RTU_DataCtrl Neo_RTUPort;

//RTU_PORT_ALIAS是中断函数所使用的RTU_DataCtrl实例名，也是主站类中RTU_DataCtrl指针指向的实例名。
#define RTU_PORT_ALIAS  Neo_RTUPort
//如果主程序里面使用了和中断里面不一样的从站类实例名myNeo_Slaver，则用此宏来取别名以便使myNeo_Slaver可用。
#define myNeo_Slaver myProtocol1
//如果主程序里面使用了和中断里面不一样的主站类实例名myNeo_Master，则用此宏来取别名以便使myNeo_Master可用。
#define myNeo_Master myProtocol2

#endif /* end of include guard: __RTU_CONFIG_H */

