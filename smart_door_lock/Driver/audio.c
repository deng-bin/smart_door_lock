#include "uart.h"
#include "debug.h"
void audio_init()
{
    Usart3_Init();
}

void audio_play(u8 num)
{
    u8 string[]={0x7e,0x05,0x41,0x00,num,0x05^0x41^0x00^num,0xef};
    u8 i;
    for(i=0;i<7;i++)
    {
        USART_SendData(USART3,string[i]);
        while(  USART_GetFlagStatus(USART3, USART_FLAG_TC)==0 );
    }
}

void audio_yinliang(u8 yinliang)
{
    u8 string[]={0x7e,0x04,0x31,yinliang,0x04^0x31^yinliang,0xef};
    u8 i;
    for(i=0;i<6;i++)
    {
        USART_SendData(USART3,string[i]);
        while(  USART_GetFlagStatus(USART3, USART_FLAG_TC)==0 );
    }
}
