#ifndef __TEMPLATE_H
#define __TEMPLATE_H

#include <stm32f10x.h>

#ifdef __cplusplus
    extern "C" {
#endif

#ifndef __cplusplus  
//c++���Ѿ�����bool���ͣ�����bool�ǹؼ��֡�
typedef enum
{    
    false = (u8)0,
    true = !false
} bool;
#endif

#define Is_BOOL(bTest)  ((bTest == true) || (bTest == false))

#ifdef __cplusplus
}
#endif

#endif
