/*
���ܣ� �Կ⺯���Ķ��η�װ��
������ ��װGPIO_Init()����ΪMy_GPIO_Init()��������ʼ�����ṹ���ں�������ʵ�֣�ֻ�贫���ʼ�����ݲ���Ҫ�����ʼ���ṹ��ʹ�ⲿ���ó�ʼ����ü򵥡�
��ƣ� azjiao
�汾�� 0.1
���ڣ� 2018��09��06��
*/
#include <stm32f10x.h>
#include "dev.h"

//GPIO_Init()�⺯���Ķ��η�װ��
//����ǰ����ʹ���õ�GPIO����ʱ������ʹ�ܡ�
void My_GPIO_Init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIOMode_TypeDef GPIO_Mode, GPIOSpeed_TypeDef GPIO_Speed)
{
    GPIO_InitTypeDef GPIO_InitStruct; //������ʼ�����ݽṹ��
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed;
    GPIO_Init(GPIOx, &GPIO_InitStruct);
}

//USART_Init()�⺯���Ķ��η�װ��
//����ǰ����ʹ���õ�USARTx����ʱ������ʹ�ܡ�
void My_USART_Init(USART_TypeDef* USARTx, u32 unBaudRate, u16 usWordLength, u16 usStopBits, u16 usParity, u16 usMode, u16 usHardwareFlowControl)
{
    USART_InitTypeDef  USART_InitStruct;  //������ʼ�����ݽṹ��
    
    USART_InitStruct.USART_HardwareFlowControl = usHardwareFlowControl;
    USART_InitStruct.USART_BaudRate = unBaudRate;
    USART_InitStruct.USART_Mode = usMode;
    USART_InitStruct.USART_WordLength = usWordLength;
    USART_InitStruct.USART_StopBits = usStopBits; 
    USART_InitStruct.USART_Parity = usParity;
    
    USART_Init(USART2, &USART_InitStruct);
}

//NVIC_Init()�⺯���Ķ��η�װ��
void My_NVIC_Init(u8 ucIRQChannel, u8 ucPrePriority, u8 ucSubPriority, FunctionalState NVIC_IRQChannelCmd)
{
    NVIC_InitTypeDef  NVIC_InitStruct;  //������ʼ�����ݽṹ��
    
    //�����ж����ȼ�
    NVIC_InitStruct.NVIC_IRQChannel = ucIRQChannel;  //���������ȼ����ж�.
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = ucPrePriority;  //��ռ���ȼ�
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = ucSubPriority;  //��Ӧ���ȼ�
    NVIC_InitStruct.NVIC_IRQChannelCmd = NVIC_IRQChannelCmd;  //ʹ�ܻ�ʧ��.
    NVIC_Init(&NVIC_InitStruct);
}
