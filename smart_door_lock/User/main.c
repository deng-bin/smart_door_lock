/*----------------------------------头文件引用--------------------------------------------------*/
#include "debug.h"        //调试相关系统头文件
#include "string.h"       //字符串操作函数头文件
#include "lcd.h"          //LCD屏幕驱动头文件
#include "pic.h"          //图片显示驱动头文件
#include "timer.h"        //定时器驱动头文件
#include "key.h"          //矩阵按键驱动头文件
#include "uart.h"         //串口通信驱动头文件
#include "audio.h"        //语音模块驱动头文件
#include "as608.h"        //AS608指纹模块驱动头文件
#include "esp8266.h"      //ESP8266 WiFi模块驱动头文件
#include "iic.h"          //IIC通信驱动头文件

/*----------------------------------函数声明---------------------------------------------------*/
// 定时器3中断服务函数（快速中断属性），用于系统计时、定时任务
void TIM3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
// 串口3中断服务函数（快速中断），处理串口3的接收/发送
void USART3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
// 字符串比较函数：比较两个字符串前len位是否一致，一致返回1，否则返回0
u8  string_chek(u8* string1,u8* string2,u8 len);
// 清空键盘输入暂存区
void key_clear();
// 字符串复制函数：将string1的前len位复制到string2
void string_copy(u8* string1,u8* string2,u8 len);
// 串口2中断服务函数（快速中断），处理RFID模块通信
void USART2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
// RFID卡片验证函数：检查当前卡号是否在已存储列表中
u8 rfid_chek();
// 串口7中断服务函数（快速中断），处理指纹模块AS608通信
void UART7_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
// 串口6中断服务函数（快速中断），处理ESP8266 WiFi模块通信
void UART6_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
// 等待指纹模块响应的函数
void Ps_Wait();

/*----------------------------------变量声明-----------------------------------------------------*/
unsigned long int uwtick;             //系统毫秒级计时变量，由定时器3维护
u8 key_val,key_old,key_down,key_up;   //按键相关变量：当前值、旧值、按下标志、释放标志（用于消抖）
u8 password[6]={1,1,1,1,1,1};         //默认开锁密码（6位）
u8 password_cmd[6]={1,2,3,4,5,6};     //管理员密码（用于修改设置、录入卡片/指纹）
u8 key_temp[7]={10,10,10,10,10,10};   //按键输入暂存区（最多6位，初始化值10表示无输入)
u8 key_index;                         //按键输入索引（记录已输入的密码位数）
u8 key_index_old;                     //上一次的按键索引（用于检测输入变化）
u16 time5000ms;                       //5秒计时变量（门打开后自动锁门计时）
u8 lock_flag=1;                       //锁状态标志：1-锁定，0-解锁
u8 password_error;                    //密码错误次数（达到3次锁定15秒）
u16 time15s=15;                       //密码错误后的锁定倒计时（15秒）
u16 time1000ms;                       //1秒计时变量（用于锁定倒计时更新）
u8 show_flag;                         //屏幕显示文字标志（0-门已上锁，1-门已开启，2-错误锁定）
u8 show_flag_old;                     //上一次的显示标志（用于检测显示变化）
u8 mode;                              
//系统模式变量：
//0-主页（正常开锁）；1-修改密码（未验证管理员）；2-修改密码（已验证）                              
//3-录入卡片（未验证）；4-录入卡片（已验证）；5-录入指纹（未验证）；6-录入指纹（已验证）
u8 rfid_index;                        //RFID模块接收数据索引（用于解析卡号）
u8 rfid_temp[4]={0};                  //临时存储当前读取的卡号（4字节）
u8 rfid[4][4]={0};                    //存储已授权的RFID卡号（最多4张）
u8 rfid_password_index;               //已存储卡片的索引（记录当前存储到第几张）
u8 uart7_rec_string[20]={0};          //串口7（指纹模块）接收缓存
u8 uart7_rec_index;                   //串口7接收索引
u8 uart7_rec_tick;                    //串口7接收超时计时
u8 ps_wait_flag;                      //指纹模块等待响应标志
u8 as608_proc_falg;                   //指纹模块状态标志
u8 as608_proc_falg_old;               //上一次的指纹模块状态标志
u8 as608_store_index=1;               //指纹存储索引（从1开始存储）
u8 uart6_rec_string[256]={0};         //串口6（ESP8266）接收缓存
u8 uart6_rec_tick;                    //串口6接收超时计时
u8 uart6_rec_index;                   //串口6接收索引

/*----------------------------------按键处理函数：处理按键输入、模式切换、密码验证等逻辑-----------------------------*/
void key_proc()
{
    /*三行消抖逻辑：通过连续检测按键状态变化实现消抖*/
    key_val=key_read();                        //读取当前按键值
    key_down=key_val&(key_val^key_old);        //计算按键按下标志（当前为1，上一次为0）
    key_up=~key_val&(key_val^key_old);         //计算按键释放标志（当前为0，上一次为1）
    key_old=key_val;                           //更新上一次按键值
    if(password_error==3)return;               //若密码错误3次（锁定状态），不处理按键
    /*按键按下时播放按键音效*/
    if(key_down)                               //如果按下
    {
        audio_play(1);                         //播放按键音效
    }
     /*根据按下的按键值执行对应操作*/
    switch(key_down)
    {
        // 数字键1-3：输入密码，存入暂存区并递增索引
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
        // 按键4：在主页模式下，进入"修改密码（未验证管理员）"模式
        case 4:
        if(mode==0)
        {
            mode=1;
            audio_play(6);                     //播放模式切换提示音
            password_error=0;                  //重置错误次数
            key_clear();                       //清空输入缓存
        }
        break;
        // 数字键5-7：输入密码
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
        // 按键8：在主页模式下，进入"录入指纹（未验证管理员）"模式
        case 8:
        if(mode==0)
        {
            mode=5;
            audio_play(18);                    //播放指纹录入提示音
            password_error=0;
            key_clear();
        }
        break;
        // 数字键9：输入密码
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
        // 按键12：在主页模式下，进入"录入卡片（未验证管理员）"模式
        case 12:
        if(mode==0)
        {
            mode=3;
            audio_play(13);                    // 播放卡片录入提示音
            key_clear();
        }
        break;
        // 按键13：清除输入（或退出当前模式）
        case 13:
        key_clear();                           //清空输入缓存
        if(mode==6)                            //若在指纹录入模式，退出并清空指纹缓存
        {
            PS_Empty();
            mode=0;
        }
        break;
        // 按键14：输入数字0
        case 14:
        key_temp[key_index]=0;
        key_index++;
        break;
        // 按键15：删除最后一位输入
        case 15:
        if(key_index)
        {
            key_index--;
            key_temp[key_index]=10;
        }
        break;
        // 按键16：确认键（根据当前模式执行对应确认操作）
        case 16:
        switch(mode)
        {
            case 0:                                      //主页模式：验证输入密码是否正确
            if( string_chek(key_temp,password,6) )       //密码正确
            {
                lock_flag=0;                             //解锁
                show_flag=1;                             //显示"门已开启"
                audio_play(3);                           //播放开锁提示音
                key_clear();                             //清空输入
                password_error=0;
            }
            else                                         //密码错误
            {
                audio_play(4);                           //播放错误提示音
                key_clear();
                if(++password_error==3)                  //累计3次错误，锁定15秒
                {
                    audio_play(5);
                    show_flag=2;
                }
            }
            break;
            case 1:                                      //修改密码（未验证管理员）：验证管理员密码
            if( string_chek(key_temp,password_cmd,6) )   //管理员密码正确
            {
                mode=2;                                  //进入已授权修改模式
                audio_play(7);                           //播放验证成功提示音
                key_clear();
            }
            else                                         //管理员密码错误
            {
                audio_play(9);
                if(++password_error==3)
                {
                    audio_play(10);
                    show_flag=2;
                }
            }
            break;
            case 2:                                      //修改密码（已授权）：保存新密码
            string_copy(key_temp,password,6);            //复制新密码到password数组
            audio_play(8);                               //播放修改成功提示音
            mode=0;                                      //返回主页
            key_clear();
            break;
            case 3:                                      //录入卡片（未验证管理员）：验证管理员密码
            if( string_chek(key_temp,password_cmd,6) )
            {
                mode=4;                                  //进入已授权录入模式
                audio_play(14);                          //播放验证成功提示音
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
            case 5:                                      //录入指纹（未验证管理员）：验证管理员密码
            if( string_chek(key_temp,password_cmd,6) )
            {
                mode=6;                                  //进入已授权录入模式
                audio_play(19);                          //播放验证成功提示音
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

/*----------------------------------门锁处理函数：控制舵机开关锁-----------------------------------------*/
void lock_proc()
{
    lock(lock_flag);  //调用锁控制函数（根据lock_flag执行开锁/上锁）
}

/*----------------------------------ESP8266 WiFi模块处理函数：解析WiFi接收数据，执行远程控制命令-----------*/
void esp8266_proc()
{
    if(uart6_rec_index==0)return;                                            //无接收数据，直接返回
    if(uart6_rec_tick>10)                                                    //接收超时（10ms），处理数据
    {
        usart1_send_string(uart6_rec_string,uart6_rec_index);                //转发到串口1（调试用）
        char* add1 = strstr(uart6_rec_string,"appkey");                      // 解析接收到的JSON数据中的appkey和appstring
        int appkey=0;
        sscanf(add1,"appkey\":%d",&appkey);                                  //提取appkey（命令类型）
        char* add2 = strstr(uart6_rec_string,"appstring");
        int appstring=0;
        sscanf(add2,"appstring\":\"%d",&appstring);                          //提取appstring（命令参数）
        // 预设回复命令
        u8 temp2_string[]="AT+MQTTPUB=0,\"$sys/fv0Z45eAUB/ch32/thing/property/post\",\"{\\\"id\\\":\\\"123\\\"\\,\\\"params\\\":{\\\"appkey\\\":{\\\"value\\\":0}}}\",0,0\r\n";
        // 根据appkey执行对应命令
        switch(appkey)
        {
            //远程开锁
            case 1:
            lock_flag=0;
            break;
            //远程上锁
            case 2:
            lock_flag=1;
            break;
            //远程修改密码（appstring为6位新密码）
            case 3:
            password[0]=appstring/100000%10;
            password[1]=appstring/10000%10;
            password[2]=appstring/1000%10;
            password[3]=appstring/100%10;
            password[4]=appstring/10%10;
            password[5]=appstring%10;
            EEPROM_Write(password, 8, 6);
            // 发送修改成功回复
            u8 temp_string[]="AT+MQTTPUB=0,\"$sys/fv0Z45eAUB/ch32/thing/property/post\",\"{\\\"id\\\":\\\"123\\\"\\,\\\"params\\\":{\\\"appkey\\\":{\\\"value\\\":6}}}\",0,0\r\n";
            uart6_send_string(temp_string,sizeof(temp_string)-1);
            break;
            //远程调节音量（appstring为音量值）
            case 4:
            audio_yinliang(appstring);
            // 发送音量调节成功回复
            u8 temp3_string[]="AT+MQTTPUB=0,\"$sys/fv0Z45eAUB/ch32/thing/property/post\",\"{\\\"id\\\":\\\"123\\\"\\,\\\"params\\\":{\\\"appkey\\\":{\\\"value\\\":7}}}\",0,0\r\n";
            uart6_send_string(temp3_string,sizeof(temp3_string)-1);
            break;
            //其他命令回复
            case 7:
            uart6_send_string(temp2_string,sizeof(temp2_string)-1);
            break;
            //其他命令回复
            case 8:
            uart6_send_string(temp2_string,sizeof(temp2_string)-1);
            break;
        }

        uart6_rec_index=0;                     //清空接收索引，准备下次接收
    }
}

/*----------------------------------指纹模块AS608处理函数：处理指纹录入和验证逻辑------------------------------*/
void as608_proc()
{
    //读取指纹模块状态引脚（PA1），检测状态变化
    as608_proc_falg=GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1);
    if(as608_proc_falg==as608_proc_falg_old)return;                          //状态未变化，退出
    as608_proc_falg_old=as608_proc_falg;
    if(as608_proc_falg==0)return;                                            //状态无效，退出
    //若在指纹录入模式（已授权）
    if(mode==6)
    {
        PS_GetImage();                                                       //采集指纹图像到暂存区
        Ps_Wait();                                                           //生成特征模板1
        PS_GenChar(1);
        Ps_Wait();
        PS_GetImage();                                                       //再次采集图像（提高准确性）
        Ps_Wait();
        PS_GenChar(2);                                                       //生成特征模板2
        Ps_Wait();
        PS_RegModel();                                                       //合并模板1和2，生成指纹模型
        Ps_Wait();
        PS_StoreChar(as608_store_index);                                     //保存指纹模型到指定索引
        as608_store_index++;                                                 //递增存储索引
        Ps_Wait();
        audio_play(21);                                                      //播放录入成功提示音
        mode=0;                                                              //返回主页
    }
    //若在主页模式（指纹验证）
    if(mode==0)
    {
        PS_GetImage();                                                       //采集指纹图像
        Ps_Wait();        
        PS_GenChar(2);                                                       //生成特征模板
        Ps_Wait();
        PS_Search();                                                         //在已存储指纹中搜索匹配
        Ps_Wait();
        //若匹配成功（匹配度>50）
        if(uart7_rec_string[13]>50)
        {
            audio_play(16);                                                  //播放验证成功提示音
            lock_flag=0;                                                     //解锁
            show_flag=1;
        }
        //匹配失败
        else
        {
            audio_play(17);                                                  //播放验证失败提示音
        }
    }
}

/*----------------------------------屏幕处理函数：更新屏幕显示内容（密码输入、状态提示等）----------------------------*/
void lcd_proc()
{
    //若输入位数变化（有新按键输入），更新密码输入区显示
    if(key_index!=key_index_old)
    {
        u8 i=key_index;
        LCD_Fill(16,45,112,66,YELLOW);                                       //填充黄色背景
        if(key_index==7)key_index=6;                                         //限制最大输入6位
        while(i--)                                                           //循环显示输入的"*"（隐藏实际密码）
        {
            if(i<6)
            LCD_ShowChar(20+16*i,45,'*',RED,YELLOW,16,0);
        }
        key_index_old=key_index;                                             //更新索引值
    }
    // 若显示标志变化，更新顶部状态文字
    if(show_flag!=show_flag_old)
    {
        LCD_Fill(0,0,128,32,WHITE);                                          //填充白色
        show_flag_old=show_flag;
    }
    //根据显示标志显示对应文字
    switch(show_flag)
    {
        case 0:                                                              //门已上锁
        lcd_show_chinese(0,0,"门已上锁，请输入密码",RED,WHITE,16,0);
        break;
        case 1:                                                              //门已开启
        lcd_show_chinese(0,0,"门已开启，欢迎回家",RED,WHITE,16,0);
        break;
        case 2:                                                              //错误锁定中，显示倒计时
        LCD_ShowIntNum(50, 16,time15s,2, RED,WHITE,16);
        break;
    }
}

/*----------------------------------定时器3中断服务函数：系统计时核心，1ms触发一次------------------------------*/
void TIM3_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM3, TIM_IT_Update)!=RESET)                          //确认更新中断
    {
        uwtick++;                                                            //系统计时递增（毫秒级）
        //串口接收超时计时
        uart7_rec_tick++;
        uart6_rec_tick++;
        //门打开状态下，计时5秒后自动上锁
        if(lock_flag==0)
        {
            if(++time5000ms==5000)                                           //计时5秒
            {
                time5000ms=0;
                lock_flag=1;                                                 //上锁
                show_flag=0;                                                 //显示"门已上锁"
                audio_play(2);                                               //播放上锁提示音
            }
        }
        //密码错误锁定状态下，15秒倒计时
        if(password_error==3)
        {
            if(++time1000ms==1000)                                           //一秒
            {
                time1000ms=0;
                if(--time15s==0)                                             //倒计时结束
                {
                    time15s=15;
                    password_error=0;                                        //重置错误次数
                    show_flag=0;
                }
            }
        }
    }
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);                              //清除中断标志
}

/*----------------------------------任务调度器相关：按周期调用各任务函数，实现多任务并发-----------------------------*/
typedef struct
{
    void (*task_func)(void);                                                 //任务函数指针
    unsigned long int rate_ms;                                               //任务执行周期（毫秒）
    unsigned long int last_run;                                              //任务上次执行的时间（uwtick值）
}task_t;

//任务列表：定义各任务的函数、周期和初始时间
task_t scheduler_task[]={
    {lcd_proc,100,0},                                                        //屏幕刷新（100ms一次）
    {key_proc,10,0},                                                         //按键检测（10ms一次，高频检测确保响应快）
    {lock_proc,30,0},                                                        //门锁控制（30ms一次）
    {as608_proc,20,0},                                                       //指纹模块处理（20ms一次）
    {esp8266_proc,2,0},                                                      //WiFi模块处理（2ms一次，确保快速响应网络数据）
};

unsigned char task_num;                                                      //任务总数

// 初始化任务调度器：计算任务数量
void scheduler_init()
{
    task_num=sizeof(scheduler_task)/sizeof(task_t);  //计算任务数量
}

//运行任务调度器：按周期调用各任务
void scheduler_run()
{
    unsigned char i;
    for(i=0;i<task_num;i++)                                                  //遍历所有任务
    {
        unsigned long int now_time=uwtick;                                   //当前系统时间
        //若当前时间 >= 上次执行时间 + 周期，则执行任务
        if(now_time > (scheduler_task[i].last_run+scheduler_task[i].rate_ms) )
        {
            scheduler_task[i].last_run=now_time;                             //更新上次执行时间
            scheduler_task[i].task_func();                                   //执行任务
        }
    }
}

/*----------------------------------串口6中断服务函数（ESP8266 WiFi模块）：接收WiFi数据并缓存------------------*/
void UART6_IRQHandler(void)
{
    u8 temp=0;
    if(USART_GetITStatus(UART6, USART_IT_RXNE) != RESET)                     //接收非空中断
    {
        uart6_rec_tick=0;                                                    //重置超时计时
        temp=USART_ReceiveData(UART6);                                       //读取接收数据
        uart6_rec_string[uart6_rec_index]=temp;                              //存入缓存
        uart6_rec_index++;                                                   //递增索引
    }
    USART_ClearITPendingBit(UART6, USART_IT_RXNE);                           //清除中断标志
}

/*----------------------------------串口7中断服务函数（AS608指纹模块）：接收指纹模块数据并缓存-----------------------*/
void UART7_IRQHandler(void)
{
    u8 temp=0;
    if(USART_GetITStatus(UART7, USART_IT_RXNE) != RESET)                     //接收非空中断
    {
        ps_wait_flag=0;                                                      //收到数据，结束等待
        if(uart7_rec_tick>10)uart7_rec_index=0;                              //超时后重置索引
        temp=USART_ReceiveData(UART7);                                       //读取数据
        uart7_rec_string[uart7_rec_index]=temp;                              //存入缓存
        uart7_rec_index++;                                                   //递增索引
        uart7_rec_tick=0;                                                    //重置超时计时
    }
    USART_ClearITPendingBit(UART7, USART_IT_RXNE);                           //清除中断标志
}

/*----------------------------------串口2中断服务函数（RFID模块）：解析RFID卡号并处理--------------------------*/
void USART2_IRQHandler(void)
{
    u8 temp;
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)                    //接收非空中断
    {
        temp=USART_ReceiveData(USART2);                                      //读取RFID模块数据
        //根据协议解析卡号（RFID模块数据格式固定，前6字节为帧头，后4字节为卡号）
        switch(rfid_index)
        {
            case 0:                                                          //帧头验证
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
            case 7:                                                          //存储卡号第1字节
            rfid_temp[0]=temp;
            rfid_index++;
            break;
            case 8:                                                          //存储卡号第2字节
            rfid_temp[1]=temp;
            rfid_index++;
            break;
            case 9:                                                          //存储卡号第3字节
            rfid_temp[2]=temp;
            rfid_index++;
            break;
            case 10:                                                         //存储卡号第4字节，完成解析
            rfid_temp[3]=temp;
            rfid_index=0;
            //主页模式：验证卡号是否已授权
            if(mode==0)
            {
                if( rfid_chek() )                                            //卡号已授权
                {
                    audio_play(11);                                          //播放验证成功提示音
                    lock_flag=0;                                             //解锁
                }
                else                                                         //卡号未授权
                {
                    audio_play(12);                                          //播放验证失败提示音
                }
            }
            //卡片录入模式（已授权）：保存卡号
            if(mode==4)
            {
                string_copy(rfid_temp,rfid[rfid_password_index],4);          //复制卡号到存储区
                audio_play(15);                                              //播放录入成功提示音
                mode=0;                                                      //返回主页
                rfid_password_index++;                                       //递增存储索引
            }
            break;
        }
    }
    USART_ClearITPendingBit(USART2, USART_IT_RXNE);                          //清除中断标志
}

/*----------------------------------清空键盘输入暂存区：将暂存区重置为初始值10，索引归零----------------------------*/
void key_clear()
{
    memset(key_temp,10,6);                                                   //用10填充暂存区（表示无输入）
    key_index=0;
}

/*----------------------------------字符串比较函数：比较两个字符串前len位是否完全一致-----------------------------*/
u8  string_chek(u8* string1,u8* string2,u8 len)
{
    while(len--)                                                             //从第0位到第len-1位依次比较
    {
        if(string1[len]==string2[len]);                                      //相等则继续
        else return 0;                                                       //有一位不等则返回0
    }
    return 1;                                                                //所有位相等，返回1
}

/*----------------------------------字符串复制函数：将string1的前len位复制到string2----------------------------*/
void string_copy(u8* string1,u8* string2,u8 len)
{
    u8 i;
    for (i = 0; i < len; ++i)
    {
        string2[i]=string1[i];                                               //逐个字节复制
    }
}

/*----------------------------------RFID卡号验证函数：检查当前卡号是否在已存储的rfid列表中------------------------*/
u8 rfid_chek()
{
    u8 i;
    for (i=0; i<rfid_password_index; ++i)                                    //遍历已存储的所有卡号
    {
        if( string_chek(rfid_temp,rfid[i],4) )return 1;                      //找到匹配卡号
    }
    return 0;                                                                //无匹配卡号
}

/*----------------------------------等待指纹模块响应：循环等待，直到收到数据（ps_wait_flag被置0）------------------*/
void Ps_Wait()
{
    ps_wait_flag=1;                                                          //置等待标志
    do
    {
        Delay_Ms(200);                                                       //延时200ms（等待模块处理）
    }
    while(ps_wait_flag);                                                     //直到收到响应才退出
}

/*--------------------------------------主函数-------------------------------------------------------*/
int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);                          //中断优先级分组（2位抢占，2位响应）
    SystemCoreClockUpdate();                                                 //更新系统时钟配置

    //硬件初始化
	USART_Printf_Init(115200);                                               //串口1初始化（调试输出）
	Delay_Init();                                                            //延时函数初始化
	TIM2_PWM_Init();                                                         //定时器2初始化（舵机PWM控制）
    lock(1);                                                                 //初始状态：上锁
    LCD_Init();                                                              //LCD屏幕初始化
    LCD_Fill(0,0,127,127,WHITE);                                             //屏幕清屏（白色背景）
    lcd_show_chinese(20,50,"正在启动",RED,WHITE,16,0);                        //显示启动提示
    lcd_show_chinese(20,50,"启动成功",RED,WHITE,16,0);                        //开机成功
    Delay_Ms(500);                        
    LCD_ShowPicture(0,0,128,128,gImage_1);                                   //显示logo
    LCD_ShowPicture(0,0,128,128,gImage_2);                                   //显示主页
    lcd_show_chinese(0,0,"门已上锁，请输入密码",RED,WHITE,16,0);               //显示初始提示
    LCD_Fill(16,45,112,66,YELLOW);                                           //绘制密码输入区背景(黄色)
    key_init();                                                              //矩阵按键初始化
    Tim3_Init(1000,96-1);                                                    //定时器3初始化（1ms中断一次，用于系统计时）
    scheduler_init();                                                        //任务调度器初始化
    audio_init();                                                            //语音模块初始化
    audio_yinliang(20);                                                      //设置初始音量为20
    Delay_Ms(10);
    audio_play(2);                                                           //播放启动提示音
    Usart2_Init();                                                           //串口2初始化（RFID模块）
    as608_init();                                                            //指纹模块初始化
    esp8266_init();                                                          //ESP8266初始化
    AT24C02_Init();                                                          //EEPROM初始化（存储配置）
    Delay_Ms(10);
    onenet_init();                                                           //物联网平台初始化（对接onenet）

    //主循环：运行任务调度器
    while(1)
    {
        scheduler_run();                                                     //按周期调用各任务函数
	}
}
