#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"
#include "nRF24L01_API.h"

//软件SPI模拟
/************************************************
模块引脚备注：
模块竖直放置,天线向左放置  单片机引脚备注
IRQ(PG8)   MISO(PB14)
MOSI(PB15) SCK (PB13)
CSN(PG7)   CE  (PG6)
VCC  			 GND


CE：模式控制线
CSN：SPI片选线
SCK：SPI时钟线
VCC: 需要注意这里VCC的供电是3.3V
MOSI：SPI数据线(主机输出，从机输入)
MISO：SPI数据线(主机输入，从机输出)
IRQ：中断信号线。中断时变为低电平，在以下三种情况变低：
Tx FIFO 发完并且收到ACK（使能ACK情况下）、Rx FIFO 收到数据、达到最大重发次数。

调试设置:
电脑端:NRF24L01 配置
波特率:9600
目标地址:31 32 33 34 35
本地地址:31 32 33 34 35
通信频率: 2.400GHz
校验方式: 16位CRC校验
空中速率：2Mbps

单片机1端: NRF24L01 配置
波特率:9600
目标地址:31 32 33 34 35
本地地址:31 32 33 34 35
通信频率: 2.400GHz
校验方式: 16位CRC校验
空中速率：2Mbps


************************************************/


int main(void)
{
    u8 key,mode;
    u8 tmp_buf[33],rece_buf[32];
    delay_init();	    	 //延时函数初始化
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
    uart_init(115200);	 	//串口初始化为115200
    LED_Init();		  			//初始化与LED连接的硬件接口
    KEY_Init();					//初始化按键
    LCD_Init();			   		//初始化LCD
    NRF24L01_Init();    		//初始化NRF24L01
    POINT_COLOR=RED;			//设置字体为红色
    while(NRF24L01_Check())
    {
        LCD_ShowString(30,130,200,16,16,"NRF24L01 Error");
        delay_ms(200);
        LCD_Fill(30,130,239,130+16,WHITE);
        delay_ms(200);
    }
    LCD_ShowString(30,130,200,16,16,"NRF24L01 OK");
    NRF24L01_RT_Init();	//设置参数
    rece_buf[1]=0xc9;					//上
    rece_buf[2]=0xcf;					//上
    rece_buf[3]=0xba;					//海
    rece_buf[4]=0xa3;					//海
    rece_buf[5]=0xb1;					//宝
    rece_buf[6]=0xa6;					//宝
    rece_buf[7]=0xc7;					//嵌
    rece_buf[8]=0xb6;					//嵌
    rece_buf[9]=0xb5;					//电
    rece_buf[10]=0xe7;				//电
    rece_buf[11]=0xd7;				//子
    rece_buf[12]=0xd3;				//子
    rece_buf[0]=12;						//一共要发送12个字节，rece_buf[0]必须是12！！！！！！
    SEND_BUF(rece_buf);    //发送参数
    while(1)
    {
        //NRF_IRQ中断线 中断时变为低电平，在以下三种情况变低：Tx FIFO 发完并且收到ACK（使能ACK情况下）、Rx FIFO 收到数据、达到最大重发次数。
        if(NRF_IRQ==0)	 	// 如果无线模块接收到数据
        {
            if(NRF24L01_RxPacket(tmp_buf)==0)  //设置为接收模式  接收完毕
            {
                if(	tmp_buf[1]=='1')		 //第1位以后是收到的命令数据，rece_buf[0]是数据位数长度
                    LED1=!LED1;
                if(	tmp_buf[1]=='2')	 	 //第1位以后是收到的命令数据，rece_buf[0]是数据位数长度
                    LED0=!LED0;
            }
        }


    }
}


