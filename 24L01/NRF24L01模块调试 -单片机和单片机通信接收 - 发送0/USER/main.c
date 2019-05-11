#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"
#include "nRF24L01_API.h"

//���SPIģ��  STM32F103ZET6
/************************************************
ģ�����ű�ע��
ģ����ֱ����,�����������  ��Ƭ�����ű�ע
IRQ(PG8)   MISO(PB14)
MOSI(PB15) SCK (PB13)
CSN(PG7)   CE  (PG6)
VCC  			 GND


CE��ģʽ������
CSN��SPIƬѡ��
SCK��SPIʱ����
VCC: ��Ҫע������VCC�Ĺ�����3.3V
MOSI��SPI������(����������ӻ�����)
MISO��SPI������(�������룬�ӻ����)
IRQ���ж��ź��ߡ��ж�ʱ��Ϊ�͵�ƽ�����������������ͣ�
Tx FIFO ���겢���յ�ACK��ʹ��ACK����£���Rx FIFO �յ����ݡ��ﵽ����ط�������

���Ͷ�0�˿�
��Ƭ��1��: NRF24L01 ����
������:9600
Ŀ���ַ:0xE7,0xD3,0xF0,0x35,0x77
���ص�ַ:0xE7,0xD3,0xF0,0x35,0x77
ͨ��Ƶ��: 2.400GHz
У�鷽ʽ: 16λCRCУ��
�������ʣ�2Mbps


************************************************/


int main(void)
{
    u8 key,mode;
    u8 tmp_buf[33],rece_buf[32];
    delay_init();	    	 //��ʱ������ʼ��
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
    uart_init(115200);	 	//���ڳ�ʼ��Ϊ115200
    LED_Init();		  			//��ʼ����LED���ӵ�Ӳ���ӿ�
    KEY_Init();					//��ʼ������
    LCD_Init();			   		//��ʼ��LCD
    NRF24L01_Init();    		//��ʼ��NRF24L01
    POINT_COLOR=RED;			//��������Ϊ��ɫ
    while(NRF24L01_Check())
    {
        LCD_ShowString(30,130,200,16,16,"NRF24L01 Error");
        delay_ms(200);
        LCD_Fill(30,130,239,130+16,WHITE);
        delay_ms(200);
    }
    LCD_ShowString(30,130,200,16,16,"NRF24L01 OK");
    NRF24L01_RT_Init();	      //����NRF24L01
    rece_buf[1]=0x01;	
    rece_buf[2]=0x01;		
    rece_buf[3]=0x01;				
    rece_buf[0]=2;						//һ��Ҫ����1���ֽڣ�rece_buf[0]������1
    while(1)
    {
        //NRF_IRQ�ж��� �ж�ʱ��Ϊ�͵�ƽ�����������������ͣ�Tx FIFO ���겢���յ�ACK��ʹ��ACK����£���Rx FIFO �յ����ݡ��ﵽ����ط�������
        if(NRF_IRQ==0)	 	// �������ģ����յ�����
        {
            if(NRF24L01_RxPacket(tmp_buf)==0)  //����Ϊ����ģʽ  �������
            {
                if(	tmp_buf[1]=='1')		 //��1λ�Ժ����յ����������ݣ�rece_buf[0]������λ������
                    LED1=!LED1;
                if(	tmp_buf[1]=='2')	 	 //��1λ�Ժ����յ����������ݣ�rece_buf[0]������λ������
                    LED0=!LED0;
            }
        }
			 SEND_BUF(rece_buf);   //��������
			 delay_ms(500);

    }
}


