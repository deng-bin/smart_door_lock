#include "debug.h"
// ��������PWM���ں�ռ�ձ�
#define PWM_PERIOD 20000  // PWM����Ϊ20ms (20000us)
#define PWM_MIN 500       // ��С����Ϊ0.5ms (500us)
#define PWM_MAX 2500      // �������Ϊ2.5ms (2500us)

// ��ʼ��TIM2������PWM�ź�
void TIM2_PWM_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);  // ʹ��TIM2ʱ��

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    // ����TIM2�Ļ�������
    TIM_TimeBaseStructure.TIM_Period = PWM_PERIOD - 1;  // �����Զ���װֵ
    TIM_TimeBaseStructure.TIM_Prescaler = 96 - 1;       // ����Ԥ��Ƶ����96MHz/96 = 1MHz (1us)
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    // ����TIM2��PWMģʽ
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = PWM_MIN;  // ��ʼռ�ձ�Ϊ��Сֵ
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

    // ����TIM2ͨ��1
    TIM_OC1Init(TIM2, &TIM_OCInitStructure);  // ʹ��TIM2��ͨ��1
    TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);  // ʹ��TIM2ͨ��1��Ԥװ�ؼĴ���


    TIM_ARRPreloadConfig(TIM2, ENABLE);               // ʹ��TIM2���Զ���װ�Ĵ���Ԥװ��

    TIM_Cmd(TIM2, ENABLE);  // ʹ��TIM2

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  // ʹ��GPIOAʱ��
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // ����PA0Ϊ�������������TIM2ͨ��1��
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void lock(u8 enable)//1����  0����
{
    if(enable)
    {
        TIM_SetCompare1(TIM2,500);
    }
    else
    {
        TIM_SetCompare1(TIM2,1500);
    }
}

void Tim3_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    NVIC_InitTypeDef NVIC_InitStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    TIM_TimeBaseInitStruct.TIM_ClockDivision=0;//ʱ�ӷָ�
    TIM_TimeBaseInitStruct.TIM_CounterMode=TIM_CounterMode_Up;//����Ϊ���ϼ���
    TIM_TimeBaseInitStruct.TIM_Period=arr;//�Զ���װ��ֵ��5000Ϊ500ms
    TIM_TimeBaseInitStruct.TIM_Prescaler=psc;//���÷�Ƶֵ

    TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStruct);
    TIM_ITConfig(TIM3, TIM_IT_Update|TIM_IT_Trigger, ENABLE);
    NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    TIM_Cmd(TIM3, ENABLE);
}
