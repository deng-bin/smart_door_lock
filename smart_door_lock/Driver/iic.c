#include "debug.h"
//SCL PE6
//SDA PE5
//---------------------------------------------------------------
#define I2C_SCL_0   GPIO_ResetBits(GPIOE, GPIO_Pin_6)
#define I2C_SCL_1   GPIO_SetBits(GPIOE, GPIO_Pin_6)
#define I2C_SDA_0   GPIO_ResetBits(GPIOE, GPIO_Pin_5)
#define I2C_SDA_1   GPIO_SetBits(GPIOE, GPIO_Pin_5)
#define I2C_SDA     GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_5)
//--------------------�ܽŶ���-----------------------------------

//---------------------------------------------------------------

#define TRUE        1
#define FALSE       0

#define DELAY_nT    2           // ��ʱ6����������(2+2*DELAY_nT),
                                // (��ʱʱ�����5us,��˻�������
                                // ���ݾ���Ƶ�ʶ���)
//***********************************************************************
// �������ƣ�I2C_Start()
// ��    �ܣ�����I2C����
// ��ڲ�������
// ���ڲ�������
//***********************************************************************
void AT24C02_Init()
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Pin=GPIO_Pin_5|GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
    GPIO_Init(GPIOE,&GPIO_InitStructure);
}

void I2C_Start(void)
{
    I2C_SDA_1;
    I2C_SCL_1;
    Delay_Us(7);
    I2C_SDA_0;
    Delay_Us(7);
    I2C_SCL_0;
    Delay_Us(7);
}
//***********************************************************************
// �������ƣ�I2C_Stop()
// ��    �ܣ���ֹI2C����
// ��ڲ�������
// ���ڲ�������
//***********************************************************************
void I2C_Stop(void)
{

    I2C_SDA_0;
    I2C_SCL_1;
    Delay_Us(7);
    I2C_SDA_1;
    Delay_Us(7);
    I2C_SCL_0;
    Delay_Us(7);
}


void I2CSendAck(unsigned char ackbit)
{
    I2C_SCL_0;

    if(ackbit)I2C_SDA_1;
    else I2C_SDA_0;
    Delay_Us(7);
    I2C_SCL_1;
    Delay_Us(7);
    I2C_SCL_0;
    I2C_SDA_1;
    Delay_Us(7);
}
//***********************************************************************
// �������ƣ�I2C_Send_Bit_0()
// ��    �ܣ����ͱ���0
// ��ڲ�������
// ���ڲ�������
//***********************************************************************
void I2C_Send_Bit_0(void)
{

    I2C_SDA_0;
    I2C_SCL_1;
    Delay_Us(7);
    I2C_SCL_0;
    Delay_Us(7);
}
//***********************************************************************
// �������ƣ�I2C_Send_Bit_1()
// ��    �ܣ����ͱ���1
// ��ڲ�������
// ���ڲ�������
//***********************************************************************
void I2C_Send_Bit_1(void)
{

    I2C_SDA_1;
    I2C_SCL_1;
    Delay_Us(7);
    I2C_SCL_0;
    Delay_Us(7);
}
//***********************************************************************
// �������ƣ�I2C_Check_Ack()
// ��    �ܣ�������һ���ֽں�����豸��Ӧ���ź�
// ��ڲ�������
// ���ڲ���������ֵΪTrue���ɹ�
//           ����ֵΪFalse��ʧ��
//***********************************************************************
unsigned char I2C_Check_Ack(void)
{
    unsigned char F0;
    I2C_SDA_1;
    I2C_SCL_1;
    Delay_Us(7);
    F0 = I2C_SDA;
    Delay_Us(7);
    I2C_SCL_0;
    Delay_Us(7);
    if( F0 == 1 ) return FALSE;
    else return TRUE;
}
//***********************************************************************
// �������ƣ�I2C_Write8Bit()
// ��    �ܣ���I2C����д��8bit����
// ��ڲ�����I2C_data     ��Ҫд��I2C���ߵ�8bit����
// ���ڲ�������
//***********************************************************************
void I2C_Write8Bit(unsigned char I2C_data)
{
    unsigned char i;

    for( i=0;i<8;i++ )
    {
        if( (I2C_data<<i)&0x80 )
            I2C_Send_Bit_1();
        else
            I2C_Send_Bit_0();
    }
}
//***********************************************************************
// �������ƣ�I2C_Read8Bit()
// ��    �ܣ���I2C���߽���8bit����
// ��ڲ�������
// ���ڲ���������ֵΪ��I2C�����Ͻ��յ���8bit����
//***********************************************************************
unsigned char I2C_Read8Bit(void)
{
    unsigned char F0;
    unsigned char I2C_data = 0,i;

    for( i = 0; i < 8; i++ )
    {
        I2C_data = I2C_data << 1;
        I2C_SDA_1;
        I2C_SCL_1;
        Delay_Us(7);
        F0 = I2C_SDA;
        Delay_Us(7);
        I2C_SCL_0;
        if( F0 == 1 )
            I2C_data = I2C_data | 0x01;
    }
    return I2C_data;
}
unsigned char num=0;
#define slave   0xa6
void w_24cXX(unsigned char slave_add,unsigned char byte_add,unsigned char byte_add2,unsigned char dat)
{
    I2C_Start();
    I2C_Write8Bit(slave_add);
    I2C_Check_Ack();
    I2C_Write8Bit(byte_add);
    I2C_Check_Ack();
    I2C_Write8Bit(byte_add2);
    I2C_Check_Ack();
    I2C_Write8Bit(dat);
    I2C_Check_Ack();
    I2C_Stop();
}
unsigned char r_24cXX(unsigned char slave_add,unsigned char byte_add,unsigned char byte_add2)
{
    unsigned char temp=0;
    I2C_Start();
    I2C_Write8Bit(slave_add);
    I2C_Check_Ack();
    I2C_Write8Bit(byte_add);
    I2C_Check_Ack();
    I2C_Write8Bit(byte_add2);
    I2C_Check_Ack();
    I2C_Start();
    I2C_Write8Bit(slave_add+0x01);
    I2C_Check_Ack();
    temp=I2C_Read8Bit();
    return temp;
    I2C_Check_Ack();
    I2C_Stop();
}

void EEPROM_Write(u8* string,u8 addr,u8 len)
{
        I2C_Start();
        I2C_Write8Bit(0xa6);
        I2C_Check_Ack();

        I2C_Write8Bit(addr);
        I2C_Check_Ack();


        while(len--)
        {
        I2C_Write8Bit(*string++);
        I2C_Check_Ack();
        Delay_Us(200);
        }

        I2C_Stop();
}

void EEPROM_Read(u8* string,u8 addr,u8 len)
{
    I2C_Start();
    I2C_Write8Bit(0xa6);
    I2C_Check_Ack();
    I2C_Write8Bit(addr);
    I2C_Check_Ack();

    I2C_Start();
    I2C_Write8Bit(0xa6+0x01);
    I2C_Check_Ack();

    while(len--)
    {
     *string++= I2C_Read8Bit();
     if(len)
     {
     I2CSendAck(0);
     }
     else
     {
         I2CSendAck(1);
     }
    }
}
