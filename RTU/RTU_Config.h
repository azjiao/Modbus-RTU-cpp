/*********************************************************
功能： Modbus RTU通讯的用户配置文件。
描述： 本配置文件是.h头文件，必须遵循.h文件规则。
设计： azjiao
版本： 0.1
日期： 2018年10月11日
*********************************************************/
#ifndef __RTU_CONFIG_H
#define __RTU_CONFIG_H

#include "RTU_RXTX.h"
//-----------------------------------------------------
//用户配置
const bool MASTERORSLAVE = bSlave;   //bMaster主站还是bSlave从站模式

const u8  NODEADDR = 3;  //Modbus站点地址,主站地址忽略.
const u32 BAUDRATE = 9600;  //波特率
const u16 DATABIT = 8;  //数据位长度
const u16 STOPBIT = 1;  //停止位
const u16 PARITY = 0;  //奇偶校验：0无校验，1奇校验，2偶校验
const u16 usTimeOut = 500;  //从站应答超时时间,单位ms.

//定义中断服务中使用的RTU_DataCtrl实例的别名:如果RTU_DataCtrl的实例名改变，请一并改变这里的Neo_RTUPort为所需。
class RTU_DataCtrl;
extern RTU_DataCtrl Neo_RTUPort;
#define RTU_PORT_ALIAS  Neo_RTUPort
//如果主程序里面使用了和中断里面不一样的从站类实例名myNeo_Slaver，则用此宏来取别名以便使myNeo_Slaver可用。
#define myNeo_Slaver myProtocol1


#endif /* end of include guard: __RTU_CONFIG_H */

