#include "debug.h"
/*
 * OUT PP
 * R4 PD11
 * R3 PD9
 * R2 PE15
 * R1 PE13
 *
 * IPU
 * C1 PE11
 * C2 PE9
 * C3 PE7
 * C4 PC5
 */
void key_init()
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);  // 使能GPIOD时钟
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11|GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);  // 使能GPIOE时钟
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15|GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);  // 使能GPIOE时钟
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11|GPIO_Pin_9|GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);  // 使能GPIOE时钟
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

u8 key_read()
{
    u8 temp=0;
    GPIO_ResetBits(GPIOD, GPIO_Pin_11); GPIO_SetBits(GPIOD, GPIO_Pin_9); GPIO_SetBits(GPIOE, GPIO_Pin_15); GPIO_SetBits(GPIOE, GPIO_Pin_13);
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_11)==0)temp=13;
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_9)==0)temp=9;
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_7)==0)temp=5;
    if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_5)==0)temp=1;

    GPIO_SetBits(GPIOD, GPIO_Pin_11); GPIO_ResetBits(GPIOD, GPIO_Pin_9); GPIO_SetBits(GPIOE, GPIO_Pin_15); GPIO_SetBits(GPIOE, GPIO_Pin_13);
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_11)==0)temp=14;
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_9)==0)temp=10;
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_7)==0)temp=6;
    if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_5)==0)temp=2;

    GPIO_SetBits(GPIOD, GPIO_Pin_11); GPIO_SetBits(GPIOD, GPIO_Pin_9); GPIO_ResetBits(GPIOE, GPIO_Pin_15); GPIO_SetBits(GPIOE, GPIO_Pin_13);
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_11)==0)temp=15;
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_9)==0)temp=11;
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_7)==0)temp=7;
    if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_5)==0)temp=3;

    GPIO_SetBits(GPIOD, GPIO_Pin_11); GPIO_SetBits(GPIOD, GPIO_Pin_9); GPIO_SetBits(GPIOE, GPIO_Pin_15); GPIO_ResetBits(GPIOE, GPIO_Pin_13);
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_11)==0)temp=16;
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_9)==0)temp=12;
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_7)==0)temp=8;
    if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_5)==0)temp=4;

    return temp;
}
