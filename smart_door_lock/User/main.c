/*----------------------------------ͷ�ļ�����--------------------------------------------------*/
#include "debug.h"        //�������ϵͳͷ�ļ�
#include "string.h"       //�ַ�����������ͷ�ļ�
#include "lcd.h"          //LCD��Ļ����ͷ�ļ�
#include "pic.h"          //ͼƬ��ʾ����ͷ�ļ�
#include "timer.h"        //��ʱ������ͷ�ļ�
#include "key.h"          //���󰴼�����ͷ�ļ�
#include "uart.h"         //����ͨ������ͷ�ļ�
#include "audio.h"        //����ģ������ͷ�ļ�
#include "as608.h"        //AS608ָ��ģ������ͷ�ļ�
#include "esp8266.h"      //ESP8266 WiFiģ������ͷ�ļ�
#include "iic.h"          //IICͨ������ͷ�ļ�

/*----------------------------------��������---------------------------------------------------*/
// ��ʱ��3�жϷ������������ж����ԣ�������ϵͳ��ʱ����ʱ����
void TIM3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
// ����3�жϷ������������жϣ���������3�Ľ���/����
void USART3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
// �ַ����ȽϺ������Ƚ������ַ���ǰlenλ�Ƿ�һ�£�һ�·���1�����򷵻�0
u8  string_chek(u8* string1,u8* string2,u8 len);
// ��ռ��������ݴ���
void key_clear();
// �ַ������ƺ�������string1��ǰlenλ���Ƶ�string2
void string_copy(u8* string1,u8* string2,u8 len);
// ����2�жϷ������������жϣ�������RFIDģ��ͨ��
void USART2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
// RFID��Ƭ��֤��������鵱ǰ�����Ƿ����Ѵ洢�б���
u8 rfid_chek();
// ����7�жϷ������������жϣ�������ָ��ģ��AS608ͨ��
void UART7_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
// ����6�жϷ������������жϣ�������ESP8266 WiFiģ��ͨ��
void UART6_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
// �ȴ�ָ��ģ����Ӧ�ĺ���
void Ps_Wait();

/*----------------------------------��������-----------------------------------------------------*/
unsigned long int uwtick;             //ϵͳ���뼶��ʱ�������ɶ�ʱ��3ά��
u8 key_val,key_old,key_down,key_up;   //������ر�������ǰֵ����ֵ�����±�־���ͷű�־������������
u8 password[6]={1,1,1,1,1,1};         //Ĭ�Ͽ������루6λ��
u8 password_cmd[6]={1,2,3,4,5,6};     //����Ա���루�����޸����á�¼�뿨Ƭ/ָ�ƣ�
u8 key_temp[7]={10,10,10,10,10,10};   //���������ݴ��������6λ����ʼ��ֵ10��ʾ������)
u8 key_index;                         //����������������¼�����������λ����
u8 key_index_old;                     //��һ�εİ������������ڼ������仯��
u16 time5000ms;                       //5���ʱ�������Ŵ򿪺��Զ����ż�ʱ��
u8 lock_flag=1;                       //��״̬��־��1-������0-����
u8 password_error;                    //�������������ﵽ3������15�룩
u16 time15s=15;                       //�����������������ʱ��15�룩
u16 time1000ms;                       //1���ʱ������������������ʱ���£�
u8 show_flag;                         //��Ļ��ʾ���ֱ�־��0-����������1-���ѿ�����2-����������
u8 show_flag_old;                     //��һ�ε���ʾ��־�����ڼ����ʾ�仯��
u8 mode;                              
//ϵͳģʽ������
//0-��ҳ��������������1-�޸����루δ��֤����Ա����2-�޸����루����֤��                              
//3-¼�뿨Ƭ��δ��֤����4-¼�뿨Ƭ������֤����5-¼��ָ�ƣ�δ��֤����6-¼��ָ�ƣ�����֤��
u8 rfid_index;                        //RFIDģ������������������ڽ������ţ�
u8 rfid_temp[4]={0};                  //��ʱ�洢��ǰ��ȡ�Ŀ��ţ�4�ֽڣ�
u8 rfid[4][4]={0};                    //�洢����Ȩ��RFID���ţ����4�ţ�
u8 rfid_password_index;               //�Ѵ洢��Ƭ����������¼��ǰ�洢���ڼ��ţ�
u8 uart7_rec_string[20]={0};          //����7��ָ��ģ�飩���ջ���
u8 uart7_rec_index;                   //����7��������
u8 uart7_rec_tick;                    //����7���ճ�ʱ��ʱ
u8 ps_wait_flag;                      //ָ��ģ��ȴ���Ӧ��־
u8 as608_proc_falg;                   //ָ��ģ��״̬��־
u8 as608_proc_falg_old;               //��һ�ε�ָ��ģ��״̬��־
u8 as608_store_index=1;               //ָ�ƴ洢��������1��ʼ�洢��
u8 uart6_rec_string[256]={0};         //����6��ESP8266�����ջ���
u8 uart6_rec_tick;                    //����6���ճ�ʱ��ʱ
u8 uart6_rec_index;                   //����6��������

/*----------------------------------���������������������롢ģʽ�л���������֤���߼�-----------------------------*/
void key_proc()
{
    /*���������߼���ͨ��������ⰴ��״̬�仯ʵ������*/
    key_val=key_read();                        //��ȡ��ǰ����ֵ
    key_down=key_val&(key_val^key_old);        //���㰴�����±�־����ǰΪ1����һ��Ϊ0��
    key_up=~key_val&(key_val^key_old);         //���㰴���ͷű�־����ǰΪ0����һ��Ϊ1��
    key_old=key_val;                           //������һ�ΰ���ֵ
    if(password_error==3)return;               //���������3�Σ�����״̬������������
    /*��������ʱ���Ű�����Ч*/
    if(key_down)                               //�������
    {
        audio_play(1);                         //���Ű�����Ч
    }
     /*���ݰ��µİ���ִֵ�ж�Ӧ����*/
    switch(key_down)
    {
        // ���ּ�1-3���������룬�����ݴ�������������
        case 1:
        key_temp[key_index]=1;
        key_index++;
        break;
        case 2:
        key_temp[key_index]=2;
        key_index++;
        break;
        case 3:
        key_temp[key_index]=3;
        key_index++;
        break;
        // ����4������ҳģʽ�£�����"�޸����루δ��֤����Ա��"ģʽ
        case 4:
        if(mode==0)
        {
            mode=1;
            audio_play(6);                     //����ģʽ�л���ʾ��
            password_error=0;                  //���ô������
            key_clear();                       //������뻺��
        }
        break;
        // ���ּ�5-7����������
        case 5:
        key_temp[key_index]=4;
        key_index++;
        break;
        case 6:
        key_temp[key_index]=5;
        key_index++;
        break;
        case 7:
        key_temp[key_index]=6;
        key_index++;
        break;
        // ����8������ҳģʽ�£�����"¼��ָ�ƣ�δ��֤����Ա��"ģʽ
        case 8:
        if(mode==0)
        {
            mode=5;
            audio_play(18);                    //����ָ��¼����ʾ��
            password_error=0;
            key_clear();
        }
        break;
        // ���ּ�9����������
        case 9:
        key_temp[key_index]=7;
        key_index++;
        break;
        case 10:
        key_temp[key_index]=8;
        key_index++;
        break;
        case 11:
        key_temp[key_index]=9;
        key_index++;
        break;
        // ����12������ҳģʽ�£�����"¼�뿨Ƭ��δ��֤����Ա��"ģʽ
        case 12:
        if(mode==0)
        {
            mode=3;
            audio_play(13);                    // ���ſ�Ƭ¼����ʾ��
            key_clear();
        }
        break;
        // ����13��������루���˳���ǰģʽ��
        case 13:
        key_clear();                           //������뻺��
        if(mode==6)                            //����ָ��¼��ģʽ���˳������ָ�ƻ���
        {
            PS_Empty();
            mode=0;
        }
        break;
        // ����14����������0
        case 14:
        key_temp[key_index]=0;
        key_index++;
        break;
        // ����15��ɾ�����һλ����
        case 15:
        if(key_index)
        {
            key_index--;
            key_temp[key_index]=10;
        }
        break;
        // ����16��ȷ�ϼ������ݵ�ǰģʽִ�ж�Ӧȷ�ϲ�����
        case 16:
        switch(mode)
        {
            case 0:                                      //��ҳģʽ����֤���������Ƿ���ȷ
            if( string_chek(key_temp,password,6) )       //������ȷ
            {
                lock_flag=0;                             //����
                show_flag=1;                             //��ʾ"���ѿ���"
                audio_play(3);                           //���ſ�����ʾ��
                key_clear();                             //�������
                password_error=0;
            }
            else                                         //�������
            {
                audio_play(4);                           //���Ŵ�����ʾ��
                key_clear();
                if(++password_error==3)                  //�ۼ�3�δ�������15��
                {
                    audio_play(5);
                    show_flag=2;
                }
            }
            break;
            case 1:                                      //�޸����루δ��֤����Ա������֤����Ա����
            if( string_chek(key_temp,password_cmd,6) )   //����Ա������ȷ
            {
                mode=2;                                  //��������Ȩ�޸�ģʽ
                audio_play(7);                           //������֤�ɹ���ʾ��
                key_clear();
            }
            else                                         //����Ա�������
            {
                audio_play(9);
                if(++password_error==3)
                {
                    audio_play(10);
                    show_flag=2;
                }
            }
            break;
            case 2:                                      //�޸����루����Ȩ��������������
            string_copy(key_temp,password,6);            //���������뵽password����
            audio_play(8);                               //�����޸ĳɹ���ʾ��
            mode=0;                                      //������ҳ
            key_clear();
            break;
            case 3:                                      //¼�뿨Ƭ��δ��֤����Ա������֤����Ա����
            if( string_chek(key_temp,password_cmd,6) )
            {
                mode=4;                                  //��������Ȩ¼��ģʽ
                audio_play(14);                          //������֤�ɹ���ʾ��
                key_clear();
            }
            else
            {
                audio_play(9);
                if(++password_error==3)
                {
                    audio_play(10);
                    show_flag=2;
                }
            }
            break;
            case 5:                                      //¼��ָ�ƣ�δ��֤����Ա������֤����Ա����
            if( string_chek(key_temp,password_cmd,6) )
            {
                mode=6;                                  //��������Ȩ¼��ģʽ
                audio_play(19);                          //������֤�ɹ���ʾ��
                key_clear();
            }
            else 
            {
                audio_play(9);
                if(++password_error==3)
                {
                    audio_play(10);
                    show_flag=2;
                }
            }
            break;
        }
        break;
    }
}

/*----------------------------------���������������ƶ��������-----------------------------------------*/
void lock_proc()
{
    lock(lock_flag);  //���������ƺ���������lock_flagִ�п���/������
}

/*----------------------------------ESP8266 WiFiģ�鴦����������WiFi�������ݣ�ִ��Զ�̿�������-----------*/
void esp8266_proc()
{
    if(uart6_rec_index==0)return;                                            //�޽������ݣ�ֱ�ӷ���
    if(uart6_rec_tick>10)                                                    //���ճ�ʱ��10ms������������
    {
        usart1_send_string(uart6_rec_string,uart6_rec_index);                //ת��������1�������ã�
        char* add1 = strstr(uart6_rec_string,"appkey");                      // �������յ���JSON�����е�appkey��appstring
        int appkey=0;
        sscanf(add1,"appkey\":%d",&appkey);                                  //��ȡappkey���������ͣ�
        char* add2 = strstr(uart6_rec_string,"appstring");
        int appstring=0;
        sscanf(add2,"appstring\":\"%d",&appstring);                          //��ȡappstring�����������
        // Ԥ��ظ�����
        u8 temp2_string[]="AT+MQTTPUB=0,\"$sys/fv0Z45eAUB/ch32/thing/property/post\",\"{\\\"id\\\":\\\"123\\\"\\,\\\"params\\\":{\\\"appkey\\\":{\\\"value\\\":0}}}\",0,0\r\n";
        // ����appkeyִ�ж�Ӧ����
        switch(appkey)
        {
            //Զ�̿���
            case 1:
            lock_flag=0;
            break;
            //Զ������
            case 2:
            lock_flag=1;
            break;
            //Զ���޸����루appstringΪ6λ�����룩
            case 3:
            password[0]=appstring/100000%10;
            password[1]=appstring/10000%10;
            password[2]=appstring/1000%10;
            password[3]=appstring/100%10;
            password[4]=appstring/10%10;
            password[5]=appstring%10;
            EEPROM_Write(password, 8, 6);
            // �����޸ĳɹ��ظ�
            u8 temp_string[]="AT+MQTTPUB=0,\"$sys/fv0Z45eAUB/ch32/thing/property/post\",\"{\\\"id\\\":\\\"123\\\"\\,\\\"params\\\":{\\\"appkey\\\":{\\\"value\\\":6}}}\",0,0\r\n";
            uart6_send_string(temp_string,sizeof(temp_string)-1);
            break;
            //Զ�̵���������appstringΪ����ֵ��
            case 4:
            audio_yinliang(appstring);
            // �����������ڳɹ��ظ�
            u8 temp3_string[]="AT+MQTTPUB=0,\"$sys/fv0Z45eAUB/ch32/thing/property/post\",\"{\\\"id\\\":\\\"123\\\"\\,\\\"params\\\":{\\\"appkey\\\":{\\\"value\\\":7}}}\",0,0\r\n";
            uart6_send_string(temp3_string,sizeof(temp3_string)-1);
            break;
            //��������ظ�
            case 7:
            uart6_send_string(temp2_string,sizeof(temp2_string)-1);
            break;
            //��������ظ�
            case 8:
            uart6_send_string(temp2_string,sizeof(temp2_string)-1);
            break;
        }

        uart6_rec_index=0;                     //��ս���������׼���´ν���
    }
}

/*----------------------------------ָ��ģ��AS608������������ָ��¼�����֤�߼�------------------------------*/
void as608_proc()
{
    //��ȡָ��ģ��״̬���ţ�PA1�������״̬�仯
    as608_proc_falg=GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1);
    if(as608_proc_falg==as608_proc_falg_old)return;                          //״̬δ�仯���˳�
    as608_proc_falg_old=as608_proc_falg;
    if(as608_proc_falg==0)return;                                            //״̬��Ч���˳�
    //����ָ��¼��ģʽ������Ȩ��
    if(mode==6)
    {
        PS_GetImage();                                                       //�ɼ�ָ��ͼ���ݴ���
        Ps_Wait();                                                           //��������ģ��1
        PS_GenChar(1);
        Ps_Wait();
        PS_GetImage();                                                       //�ٴβɼ�ͼ�����׼ȷ�ԣ�
        Ps_Wait();
        PS_GenChar(2);                                                       //��������ģ��2
        Ps_Wait();
        PS_RegModel();                                                       //�ϲ�ģ��1��2������ָ��ģ��
        Ps_Wait();
        PS_StoreChar(as608_store_index);                                     //����ָ��ģ�͵�ָ������
        as608_store_index++;                                                 //�����洢����
        Ps_Wait();
        audio_play(21);                                                      //����¼��ɹ���ʾ��
        mode=0;                                                              //������ҳ
    }
    //������ҳģʽ��ָ����֤��
    if(mode==0)
    {
        PS_GetImage();                                                       //�ɼ�ָ��ͼ��
        Ps_Wait();        
        PS_GenChar(2);                                                       //��������ģ��
        Ps_Wait();
        PS_Search();                                                         //���Ѵ洢ָ��������ƥ��
        Ps_Wait();
        //��ƥ��ɹ���ƥ���>50��
        if(uart7_rec_string[13]>50)
        {
            audio_play(16);                                                  //������֤�ɹ���ʾ��
            lock_flag=0;                                                     //����
            show_flag=1;
        }
        //ƥ��ʧ��
        else
        {
            audio_play(17);                                                  //������֤ʧ����ʾ��
        }
    }
}

/*----------------------------------��Ļ��������������Ļ��ʾ���ݣ��������롢״̬��ʾ�ȣ�----------------------------*/
void lcd_proc()
{
    //������λ���仯�����°������룩������������������ʾ
    if(key_index!=key_index_old)
    {
        u8 i=key_index;
        LCD_Fill(16,45,112,66,YELLOW);                                       //����ɫ����
        if(key_index==7)key_index=6;                                         //�����������6λ
        while(i--)                                                           //ѭ����ʾ�����"*"������ʵ�����룩
        {
            if(i<6)
            LCD_ShowChar(20+16*i,45,'*',RED,YELLOW,16,0);
        }
        key_index_old=key_index;                                             //��������ֵ
    }
    // ����ʾ��־�仯�����¶���״̬����
    if(show_flag!=show_flag_old)
    {
        LCD_Fill(0,0,128,32,WHITE);                                          //����ɫ
        show_flag_old=show_flag;
    }
    //������ʾ��־��ʾ��Ӧ����
    switch(show_flag)
    {
        case 0:                                                              //��������
        lcd_show_chinese(0,0,"��������������������",RED,WHITE,16,0);
        break;
        case 1:                                                              //���ѿ���
        lcd_show_chinese(0,0,"���ѿ�������ӭ�ؼ�",RED,WHITE,16,0);
        break;
        case 2:                                                              //���������У���ʾ����ʱ
        LCD_ShowIntNum(50, 16,time15s,2, RED,WHITE,16);
        break;
    }
}

/*----------------------------------��ʱ��3�жϷ�������ϵͳ��ʱ���ģ�1ms����һ��------------------------------*/
void TIM3_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM3, TIM_IT_Update)!=RESET)                          //ȷ�ϸ����ж�
    {
        uwtick++;                                                            //ϵͳ��ʱ���������뼶��
        //���ڽ��ճ�ʱ��ʱ
        uart7_rec_tick++;
        uart6_rec_tick++;
        //�Ŵ�״̬�£���ʱ5����Զ�����
        if(lock_flag==0)
        {
            if(++time5000ms==5000)                                           //��ʱ5��
            {
                time5000ms=0;
                lock_flag=1;                                                 //����
                show_flag=0;                                                 //��ʾ"��������"
                audio_play(2);                                               //����������ʾ��
            }
        }
        //�����������״̬�£�15�뵹��ʱ
        if(password_error==3)
        {
            if(++time1000ms==1000)                                           //һ��
            {
                time1000ms=0;
                if(--time15s==0)                                             //����ʱ����
                {
                    time15s=15;
                    password_error=0;                                        //���ô������
                    show_flag=0;
                }
            }
        }
    }
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);                              //����жϱ�־
}

/*----------------------------------�����������أ������ڵ��ø���������ʵ�ֶ����񲢷�-----------------------------*/
typedef struct
{
    void (*task_func)(void);                                                 //������ָ��
    unsigned long int rate_ms;                                               //����ִ�����ڣ����룩
    unsigned long int last_run;                                              //�����ϴ�ִ�е�ʱ�䣨uwtickֵ��
}task_t;

//�����б����������ĺ��������ںͳ�ʼʱ��
task_t scheduler_task[]={
    {lcd_proc,100,0},                                                        //��Ļˢ�£�100msһ�Σ�
    {key_proc,10,0},                                                         //������⣨10msһ�Σ���Ƶ���ȷ����Ӧ�죩
    {lock_proc,30,0},                                                        //�������ƣ�30msһ�Σ�
    {as608_proc,20,0},                                                       //ָ��ģ�鴦��20msһ�Σ�
    {esp8266_proc,2,0},                                                      //WiFiģ�鴦��2msһ�Σ�ȷ��������Ӧ�������ݣ�
};

unsigned char task_num;                                                      //��������

// ��ʼ�������������������������
void scheduler_init()
{
    task_num=sizeof(scheduler_task)/sizeof(task_t);  //������������
}

//��������������������ڵ��ø�����
void scheduler_run()
{
    unsigned char i;
    for(i=0;i<task_num;i++)                                                  //������������
    {
        unsigned long int now_time=uwtick;                                   //��ǰϵͳʱ��
        //����ǰʱ�� >= �ϴ�ִ��ʱ�� + ���ڣ���ִ������
        if(now_time > (scheduler_task[i].last_run+scheduler_task[i].rate_ms) )
        {
            scheduler_task[i].last_run=now_time;                             //�����ϴ�ִ��ʱ��
            scheduler_task[i].task_func();                                   //ִ������
        }
    }
}

/*----------------------------------����6�жϷ�������ESP8266 WiFiģ�飩������WiFi���ݲ�����------------------*/
void UART6_IRQHandler(void)
{
    u8 temp=0;
    if(USART_GetITStatus(UART6, USART_IT_RXNE) != RESET)                     //���շǿ��ж�
    {
        uart6_rec_tick=0;                                                    //���ó�ʱ��ʱ
        temp=USART_ReceiveData(UART6);                                       //��ȡ��������
        uart6_rec_string[uart6_rec_index]=temp;                              //���뻺��
        uart6_rec_index++;                                                   //��������
    }
    USART_ClearITPendingBit(UART6, USART_IT_RXNE);                           //����жϱ�־
}

/*----------------------------------����7�жϷ�������AS608ָ��ģ�飩������ָ��ģ�����ݲ�����-----------------------*/
void UART7_IRQHandler(void)
{
    u8 temp=0;
    if(USART_GetITStatus(UART7, USART_IT_RXNE) != RESET)                     //���շǿ��ж�
    {
        ps_wait_flag=0;                                                      //�յ����ݣ������ȴ�
        if(uart7_rec_tick>10)uart7_rec_index=0;                              //��ʱ����������
        temp=USART_ReceiveData(UART7);                                       //��ȡ����
        uart7_rec_string[uart7_rec_index]=temp;                              //���뻺��
        uart7_rec_index++;                                                   //��������
        uart7_rec_tick=0;                                                    //���ó�ʱ��ʱ
    }
    USART_ClearITPendingBit(UART7, USART_IT_RXNE);                           //����жϱ�־
}

/*----------------------------------����2�жϷ�������RFIDģ�飩������RFID���Ų�����--------------------------*/
void USART2_IRQHandler(void)
{
    u8 temp;
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)                    //���շǿ��ж�
    {
        temp=USART_ReceiveData(USART2);                                      //��ȡRFIDģ������
        //����Э��������ţ�RFIDģ�����ݸ�ʽ�̶���ǰ6�ֽ�Ϊ֡ͷ����4�ֽ�Ϊ���ţ�
        switch(rfid_index)
        {
            case 0:                                                          //֡ͷ��֤
            if(temp==0x04)rfid_index++;
            break;
            case 1:
            if(temp==0x0c)rfid_index++;
            else rfid_index=0;
            break;
            case 2:
            if(temp==0x02)rfid_index++;
            else rfid_index=0;
            break;
            case 3:
            if(temp==0x30)rfid_index++;
            else rfid_index=0;
            break;
            case 4:
            if(temp==0x00)rfid_index++;
            else rfid_index=0;
            break;
            case 5:
            if(temp==0x04)rfid_index++;
            else rfid_index=0;
            break;
            case 6:
            if(temp==0x00)rfid_index++;
            else rfid_index=0;
            break;
            case 7:                                                          //�洢���ŵ�1�ֽ�
            rfid_temp[0]=temp;
            rfid_index++;
            break;
            case 8:                                                          //�洢���ŵ�2�ֽ�
            rfid_temp[1]=temp;
            rfid_index++;
            break;
            case 9:                                                          //�洢���ŵ�3�ֽ�
            rfid_temp[2]=temp;
            rfid_index++;
            break;
            case 10:                                                         //�洢���ŵ�4�ֽڣ���ɽ���
            rfid_temp[3]=temp;
            rfid_index=0;
            //��ҳģʽ����֤�����Ƿ�����Ȩ
            if(mode==0)
            {
                if( rfid_chek() )                                            //��������Ȩ
                {
                    audio_play(11);                                          //������֤�ɹ���ʾ��
                    lock_flag=0;                                             //����
                }
                else                                                         //����δ��Ȩ
                {
                    audio_play(12);                                          //������֤ʧ����ʾ��
                }
            }
            //��Ƭ¼��ģʽ������Ȩ�������濨��
            if(mode==4)
            {
                string_copy(rfid_temp,rfid[rfid_password_index],4);          //���ƿ��ŵ��洢��
                audio_play(15);                                              //����¼��ɹ���ʾ��
                mode=0;                                                      //������ҳ
                rfid_password_index++;                                       //�����洢����
            }
            break;
        }
    }
    USART_ClearITPendingBit(USART2, USART_IT_RXNE);                          //����жϱ�־
}

/*----------------------------------��ռ��������ݴ��������ݴ�������Ϊ��ʼֵ10����������----------------------------*/
void key_clear()
{
    memset(key_temp,10,6);                                                   //��10����ݴ�������ʾ�����룩
    key_index=0;
}

/*----------------------------------�ַ����ȽϺ������Ƚ������ַ���ǰlenλ�Ƿ���ȫһ��-----------------------------*/
u8  string_chek(u8* string1,u8* string2,u8 len)
{
    while(len--)                                                             //�ӵ�0λ����len-1λ���αȽ�
    {
        if(string1[len]==string2[len]);                                      //��������
        else return 0;                                                       //��һλ�����򷵻�0
    }
    return 1;                                                                //����λ��ȣ�����1
}

/*----------------------------------�ַ������ƺ�������string1��ǰlenλ���Ƶ�string2----------------------------*/
void string_copy(u8* string1,u8* string2,u8 len)
{
    u8 i;
    for (i = 0; i < len; ++i)
    {
        string2[i]=string1[i];                                               //����ֽڸ���
    }
}

/*----------------------------------RFID������֤��������鵱ǰ�����Ƿ����Ѵ洢��rfid�б���------------------------*/
u8 rfid_chek()
{
    u8 i;
    for (i=0; i<rfid_password_index; ++i)                                    //�����Ѵ洢�����п���
    {
        if( string_chek(rfid_temp,rfid[i],4) )return 1;                      //�ҵ�ƥ�俨��
    }
    return 0;                                                                //��ƥ�俨��
}

/*----------------------------------�ȴ�ָ��ģ����Ӧ��ѭ���ȴ���ֱ���յ����ݣ�ps_wait_flag����0��------------------*/
void Ps_Wait()
{
    ps_wait_flag=1;                                                          //�õȴ���־
    do
    {
        Delay_Ms(200);                                                       //��ʱ200ms���ȴ�ģ�鴦��
    }
    while(ps_wait_flag);                                                     //ֱ���յ���Ӧ���˳�
}

/*--------------------------------------������-------------------------------------------------------*/
int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);                          //�ж����ȼ����飨2λ��ռ��2λ��Ӧ��
    SystemCoreClockUpdate();                                                 //����ϵͳʱ������

    //Ӳ����ʼ��
	USART_Printf_Init(115200);                                               //����1��ʼ�������������
	Delay_Init();                                                            //��ʱ������ʼ��
	TIM2_PWM_Init();                                                         //��ʱ��2��ʼ�������PWM���ƣ�
    lock(1);                                                                 //��ʼ״̬������
    LCD_Init();                                                              //LCD��Ļ��ʼ��
    LCD_Fill(0,0,127,127,WHITE);                                             //��Ļ��������ɫ������
    lcd_show_chinese(20,50,"��������",RED,WHITE,16,0);                        //��ʾ������ʾ
    lcd_show_chinese(20,50,"�����ɹ�",RED,WHITE,16,0);                        //�����ɹ�
    Delay_Ms(500);                        
    LCD_ShowPicture(0,0,128,128,gImage_1);                                   //��ʾlogo
    LCD_ShowPicture(0,0,128,128,gImage_2);                                   //��ʾ��ҳ
    lcd_show_chinese(0,0,"��������������������",RED,WHITE,16,0);               //��ʾ��ʼ��ʾ
    LCD_Fill(16,45,112,66,YELLOW);                                           //������������������(��ɫ)
    key_init();                                                              //���󰴼���ʼ��
    Tim3_Init(1000,96-1);                                                    //��ʱ��3��ʼ����1ms�ж�һ�Σ�����ϵͳ��ʱ��
    scheduler_init();                                                        //�����������ʼ��
    audio_init();                                                            //����ģ���ʼ��
    audio_yinliang(20);                                                      //���ó�ʼ����Ϊ20
    Delay_Ms(10);
    audio_play(2);                                                           //����������ʾ��
    Usart2_Init();                                                           //����2��ʼ����RFIDģ�飩
    as608_init();                                                            //ָ��ģ���ʼ��
    esp8266_init();                                                          //ESP8266��ʼ��
    AT24C02_Init();                                                          //EEPROM��ʼ�����洢���ã�
    Delay_Ms(10);
    onenet_init();                                                           //������ƽ̨��ʼ�����Խ�onenet��

    //��ѭ�����������������
    while(1)
    {
        scheduler_run();                                                     //�����ڵ��ø�������
	}
}
