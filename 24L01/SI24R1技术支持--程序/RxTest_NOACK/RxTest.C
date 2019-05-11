//=======================================================
//           Si24R1 Rx TEST
//write by: Huangzengfeng
//Date:     2011-05-17
//Ver:      V0.1
//=======================================================

//********************************************************************
//		      			头文件区
//********************************************************************

#include <STC11F60XE.H>		//关于寄存器、位的定义

//#include <intrins.h>		//内部函数调用，如_nop_() 、_crol_()、cror()

//#include <absacc.h>			//绝对地址访问，如XBYTE[0x0fff0]

#include <stdio.h>			//printf等

#include <defDataType.h>	//自定义的数据类型，如uchar 、uint 

#include <Si24R1.h>		//Si24R1引脚、寄存器等定义



//=======================================================
//           单片机时钟定义定义
//=======================================================
#define T                  12            //单片机指令周期 T=6 双倍速 T=12 单倍速
#define XTAL               11059200     //晶振频率


//======================================================
//            中断方式的串口定义
//======================================================
#define BAUDRATE           9600       //串口波特率
#define OLEN               64            //输出缓冲区的大小

idata uchar   outbuf[OLEN];                   //发送缓冲区的存储
idata uchar   ostart           = 0;           //发送缓冲区起始索引
idata uchar   oend             = 0;           //发送缓冲区起始索引
bit           sendfull         = 0;           //标志-：标识发送缓冲区满
bit           sendactive       = 0;           //标志：标识发送器激活



//----------------------------    引脚定义   	----------------------------------
sbit LED0 = P2^0;
sbit LED1 = P2^1;
sbit LED2 = P3^2;
sbit LED3 = P3^3;
sbit LED4 = P3^4;
sbit LED5 = P2^5;
sbit LED6 = P2^6;
sbit LED7 = P2^7;


//----------------------------     常量   	----------------------------------
#define DELAY_TIMES 	5000
#define TX_ADR_WIDTH   	5  		// 5字节宽度的发送/接收地址
#define TX_PLOAD_WIDTH	32



//----------------------------   全局变量   ----------------------------------
uchar code TX_ADDRESS[TX_ADR_WIDTH] = {0xAA,0xAA,0xAA,0xAA,0x00};  // 定义一个静态发送地址
uchar TX_BUF[TX_PLOAD_WIDTH];
uchar RX_BUF[TX_PLOAD_WIDTH];


//----------------------------    标志位	----------------------------------
uchar bdata status_reg;
sbit  RX_DR	 = status_reg^6;
sbit  TX_DS	 = status_reg^5;
sbit  MAX_RT = status_reg^4;

//----------------------------   函数声明   ----------------------------------
void system_init(void);
 void led_on(uchar on_flag);
//====  spi =======
void  spi_init(void);										//SPI引脚初始化
uchar spi_rw_byte(uchar byte);								//SPI写字节 
uchar spi_rw_reg(uchar reg, uchar value);					//SPI写寄存器
uchar spi_rd_reg(uchar reg);								//SPI读寄存器
uchar spi_write_buf(uchar reg, uchar *pBuf, uchar bytes);	//SPI写TX FIFO
uchar spi_read_buf(uchar reg, uchar *pBuf, uchar bytes);	//SPI读RX FIFO

//=================================
void rx_mode(void);											//PRX mode
void tx_mode(uchar *buf);									//PTX mode buf: payload

//=================================
void  horseRaceLightTest(uchar times);
void  delay(void);

//=============================================================================
// 			  				函数及子程序   				       
//=============================================================================

//---------------------------------------------------------------
//----------------    中断处理子程序区  -------------------------
//---------------------------------------------------------------

//******************************************************************
//		T1 中断,用作系统多任务的定时器			  
//		功能：提供各任务的分时复用所需时间(注意开中断!)   
//		基准时间片5ms		   			  
//******************************************************************
/*
void timer1_systimer() interrupt 3 using 2
{
    
}
*/


//=============================================================================
//=============================================================================
//函数：main()					    
//功能：主程序			         
//=============================================================================
//=============================================================================
void main(void)
{
    //ulong counter = 0;
	uchar flag;
	uchar i;
	//uchar *buf;
	
	flag = 0;
    led_on(0);
	system_init();
	 led_on(0);
	for(i=0;i<TX_PLOAD_WIDTH;i++)
	{
		RX_BUF[i] = 0;
	}
	
	printf("\n=======================================================\n");
	printf("===================  Si24R1 Rx TEST  ===============\n");
	printf("===================        NO ACK       ===============\n");
	printf("=======================================================\n");

//	
	//printf("Start to Recevie......\n");

	rx_mode();							  					//接收数据

	while(1)
	{
		while(IRQ);			  

		status_reg = spi_rd_reg(STATUS);	  					// 读状态寄存器
		if(RX_DR == 1)				  			  				// 判断是否接收到数据
		{
			spi_read_buf(R_RX_PAYLOAD, RX_BUF, TX_PLOAD_WIDTH);  // 从RX FIFO读出数据到RX_BUF[]中
			spi_rw_reg(W_REGISTER + STATUS, status_reg);  		// 清除RX_DS中断标志
			flag = 1;
		}

		if(flag == 1)//接收完成
		{	horseRaceLightTest(1); 
			flag = 0;

			printf("\nReceived Data:\n");

			printf("==================== #%d ====================\n",(uint)RX_BUF[0]);
			
			for(i=1;i<TX_PLOAD_WIDTH;i++)
			{
				
				printf("%d ",(uint)RX_BUF[i]);			
				if(i%10==0)
				{
					printf("\n");
				}
			}	
			printf("\n============================================\n\n");


		   
		}
	}
	


   	/*
	while(1)
	{
		while(IRQ);
		status_reg = spi_rw_byte(NOP);                    // 返回状态寄存器
		spi_rw_byte(FLUSH_TX);
		spi_rw_reg(W_REGISTER + STATUS, status_reg);  // 清除TX_DS或MAX_RT中断标志
		IRQ = 1;
	
	  	spi_write_buf(W_TX_PAYLOAD,buf, TX_PLOAD_WIDTH);                     	// 写数据包到TX FIFO

		LED0 = ~LED0;
		delay();
	}
	*/
	


	/*
	test mode change
	rx_mode();
	
	horseRaceLightTest(10);
	
	CE = 0;

	horseRaceLightTest(10);

	spi_rw_reg(W_REGISTER + CONFIG, 0x0c);              				// CRC使能，16位CRC校验，power down，接收模式
	*/


	//while(1);

	//horseRaceLightTest(5);  
	
	//CE = 0;

	/*
	while(1)
    {
		horseRaceLightTest(1); 
		for(i = 0;i < 100;i++)
			delay();
  		P2 = spi_rd_reg(R_REGISTER + FIFO_STATUS);  	
	
	}
	*/


}//main() end





//==========================================================================
//							系统初始化模块				 	  
//==========================================================================


//==================================================================
//================          串口初始化        ======================
//==================================================================
void init_uart(void)
{
     PCON = 0x80;   //SMOD  -  -  - GF1 GF0 PD IDL
                    //SMOD串口波特率加倍位，1--加倍

     SCON = 0x40;   //SM0  SM1  SM2 REN  TB8 RB8  TI RI
                    //SM0,SM1 模式选择：00-8位移位寄存器，      01-10位异步(定时器1波特率可调)
                    //                  10-11位异步(f/32或f/64) 11-11位异步(定时器1波特率可调)
                    //SM2 ：            1-只有RB8为1，RI才置位     0-接收到字符RI就置位
                    //REN：             1-允许串行口接收  0-禁止串行口接收
                    //TB8,RB9：         发送/接收的第9位数据
                    //TI：              发送完成中断标志位
                    //RI:               接收完成中断标志位
                    //方式1,禁止接收

     TMOD = 0x20;   //       T1/C1        |         T0/C0
                    //GATE  C/T反  M1  M0   GATE  C/T反   M1  M0
                    //GATE:   1-双重启动控制，TR0/TR1+INT0/INT1   0-TR0/TR1控制
                    //C/T反： 1-Couter   0-Timer
                    //M1,M0:  00-13位定时/计数器  TL低5位，TH高8位
                    //        01-16位定时器
                    //        10-常数自动装入的8位定时器/计数器
                    //        11-仅属于T/C0的两个8为定时器，计数器
                    //定时器1 定时方式2，作为串口波特率时钟

     TH1 = (uchar)( 256- (XTAL/(16L*T*BAUDRATE)));  //T=6 双倍速 T=12 单倍速
     TR1 = 1;                                       //启动定时器T1
     ES  = 1;                                       //允许串行中断

}

//==================================================================
//================        串口中断程序        ======================
//==================================================================
static void com_isr(void) interrupt 4 using 1
{
    if(TI != 0)
    {
        TI = 0;                                         //清零中断标志位
        if(ostart != oend)                              //若字符在缓冲区
        {
                SBUF     = outbuf[ostart++ & (OLEN-1)]; //发送字符
                sendfull = 0;                           //清零sendfull标志

        }
        else                                            //若所有字符发送完成
        {
                sendactive = 0;                         //清零sendactive
        }
    }
}


//=================================================================
//写字符到SBUF或发送缓冲区
//=================================================================
void putbuf(char c)
{
    if(!sendfull)                                         //若缓冲区未满则发送
    {
        if(!sendactive)                                   //若发送器未激活
        {
            sendactive = 1;                               //则直接送第一个字符到SBUF，激活发送器
            SBUF       = c;                               //启动发送
         }
        else
        {
            ES = 0;                                             //关串口中断
            outbuf[ oend++  & (OLEN-1) ] = c;                   //放字符到发送缓冲区
            if (  ( (oend ^ ostart)  & (OLEN-1))  == 0 )
            {
                sendfull = 1;                                   //缓冲区满，置位sendfull标志
            }                                                   //开串口中断
            ES = 1;
        }
    }

}

//=========================================
//    printf()使用putchar()输出一个字符
//=========================================
char putchar(char c)
{
        if(c == '\n')
        {
              while(sendfull);                              //等待缓冲区有空间
              putbuf(0x0D);                                 //在LF前发送CR
        }
        while(sendfull);                                    //等待缓冲区有空间
        putbuf(c);                                          //把字符放入缓冲区
        return(c);

}
//=============================================================================
//函数：system_init()					    
//功能：系统初始化,完成中断、定时器、外设等硬软件的初始化     	    
//=============================================================================
void system_init(void)
{

	horseRaceLightTest(1);        
	spi_init();				   //SPI 初始化
	init_uart();               //串口初始化
    EA  = 1;                   //允许所有中断
        
}


//=====================================================================================
//函数：spi_init()
//功能: SPI引脚初始化     
//=====================================================================================
void spi_init(void)
{
	CE  = 0;        // 待机
	CSN = 1;        // SPI禁止
	SCK = 0;        // SPI时钟置低
	IRQ = 1;        // 中断复位
}
//=====================================================================================
//函数：spi_rw_byte()
//功能：根据SPI协议，写一字节数据到Si24R1，同时从Si24R1读出一字节
//=====================================================================================
uchar spi_rw_byte(uchar byte)
{
	uchar i;

   	for(i = 0; i < 8; i++)          // 循环8次
   	{
   		MOSI 	= (byte & 0x80);   	// byte最高位输出到MOSI
   		byte  <<= 1;             	// 低一位移位到最高位
   		SCK 	= 1;                // 拉高SCK，Si24R1从MOSI读入1位数据，同时从MISO输出1位数据
   		byte   |= MISO;       		// 读MISO到byte最低位
   		SCK 	= 0;            	// SCK置低
   	}
    return(byte);           		// 返回读出的一字节
}

//=====================================================================================
//函数：spi_rw_reg()
//功能：写数据value到reg寄存器，同时返回状态寄存器值
//=====================================================================================
uchar spi_rw_reg(uchar reg, uchar value)
{
	uchar status;

  	CSN 	= 0;                   	// CSN置低，开始传输数据
  	status = spi_rw_byte(reg);   	// 选择寄存器，同时返回状态字
  	spi_rw_byte(value);             // 然后写数据到该寄存器
  	CSN 	= 1;                   	// CSN拉高，结束数据传输
  	return(status);            		// 返回状态寄存器
}

//=====================================================================================
//函数：spi_rd_reg()
//功能：从reg寄存器读一个字节的数据
//=====================================================================================
uchar spi_rd_reg(uchar reg)
{
	uchar value;

  	CSN 	= 0;                	// CSN置低，开始传输数据
  	spi_rw_byte(reg);              	// 选择寄存器
  	value 	= spi_rw_byte(0);      	// 然后从该寄存器读数据 
  	CSN 	= 1;                	// CSN拉高，结束数据传输
  	return(value);            		// 返回寄存器数据
}

//=====================================================================================
//函数：spi_read_buf()
//功能：从reg寄存器读出bytes个字节，通常用来读取接收通道数据 或 接收/发送地址
//=====================================================================================
uchar spi_read_buf(uchar reg, uchar *pBuf, uchar bytes)
{

	uchar status;
	uchar i; 	

  	CSN 	= 0;                   	 // CSN置低，开始传输数据
  	status  = spi_rw_byte(reg);      // 选择寄存器，同时返回状态字
  	for(i = 0; i < bytes; i++)
    {
   		pBuf[i] = spi_rw_byte(0);    // 逐个字节从Si24R1读出
	}
  	CSN 	= 1;                     // CSN拉高，结束数据传输
  	return(status);             	 // 返回状态寄存器
}

//=====================================================================================
//函数：spi_write_buf()
//功能：把pBuf缓存中的数据写入到Si24R1，通常用来写入发射通道数据 或 接收/发送地址
//=====================================================================================
uchar spi_write_buf(uchar reg, uchar *pBuf, uchar bytes)
{
	uchar status, i;
  	CSN 	= 0;                     // CSN置低，开始传输数据
  	status 	= spi_rw_byte(reg);      // 选择寄存器，同时返回状态字
  	for(i = 0; i < bytes; i++)
    {
   		spi_rw_byte(pBuf[i]);        // 逐个字节写入Si24R1
	}
  	CSN 	= 1;                     // CSN拉高，结束数据传输
  	return(status);             	 // 返回状态寄存器
}


//=====================================================================================
//函数：rx_mode()
//功能：这个函数设置Si24R1为接收模式，等待接收发送设备的数据包
//=====================================================================================
void rx_mode(void)
{
	CE = 0;

	spi_write_buf(W_REGISTER + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH);	// 接收设备接收通道0使用和发送设备相同的发送地址
  	
//spi_rw_reg(W_REGISTER + EN_AA, 0x01);               				// 使能接收通道0自动应答
  
	spi_rw_reg(W_REGISTER + EN_RXADDR, 0x01);           				// 使能接收通道0
  	spi_rw_reg(W_REGISTER + RF_CH, 0x40);                 				// 选择射频通道0x40
  	spi_rw_reg(W_REGISTER + RX_PW_P0, TX_PLOAD_WIDTH);  				// 接收通道0选择和发送通道相同有效数据宽度
	spi_rw_reg(W_REGISTER + FEATURE, 0x04); 

//  	spi_rw_reg(W_REGISTER + RF_SETUP, 0x20);            				// 数据传输率250kbps,-18dbm TX power
//  	spi_rw_reg(W_REGISTER + RF_SETUP, 0x00);            				// 数据传输率1Mbps,-18dbm TX power
  	spi_rw_reg(W_REGISTER + RF_SETUP, 0x07);            				// 数据传输率1Mbps,7dbm TX power

  	spi_rw_reg(W_REGISTER + CONFIG, 0x0f);              				// Rx IRQ 使能，CRC使能，16位CRC校验，上电，RX mode	
   	CE = 1;                                            					// 拉高CE启动接收设备
}

//=====================================================================================
//函数：tx_mode()
//功能: 这个函数设置Si24R1为发送模式，（CE=1持续至少10us），130us后启动发射，
//	    数据发送结束后，发送模块自动转入接收模式等待应答信号。
//=====================================================================================
void tx_mode(uchar *buf)
{

	CE = 0;

	spi_rw_reg(FLUSH_TX,0x00);

  	spi_write_buf(W_REGISTER + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);     		// 写入发送地址

  	//spi_write_buf(W_REGISTER + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH);  	// 为了应答接收设备，接收通道0地址和发送地址相同

  	spi_write_buf(W_TX_PAYLOAD, buf, TX_PLOAD_WIDTH);                     	// 写数据包到TX FIFO

	spi_rw_reg(W_REGISTER + SETUP_AW, 0x03);  						 		// 5 byte Address width
  	//spi_rw_reg(W_REGISTER + EN_AA, 0x00);       						 	// 使能接收通道0自动应答
  	spi_rw_reg(W_REGISTER + EN_RXADDR, 0x01);   						 	// 使能接收通道0
  	spi_rw_reg(W_REGISTER + SETUP_RETR, 0x00);  						 	// 禁止自动重发。     自动重发延时等待250us+86us，自动重发10次
  	spi_rw_reg(W_REGISTER + RF_CH, 0x40);         					     	// 选择射频通道0x40
  	spi_rw_reg(W_REGISTER + RF_SETUP, 0x07);    						 	// 数据传输率1Mbps(**与24L01不同**),7dbm TX power
  	
	spi_rw_reg(W_REGISTER + CONFIG, 0x5e);      					     	// TX_DS IRQ enable CRC使能，16位CRC校验，上电, PTX mode	
  
	CE = 1;


}


//=====================================================================================
//函数: horseRaceLightTest()
//功能：跑马灯测试				         
//=====================================================================================
void horseRaceLightTest(uchar times)
{
	uchar i;
	i = 0;
	while(i < times)
	{
		i++;
		LED0 = ~LED0;
	//	delay();

		LED1 = ~LED1;
	//	delay();	
		
		LED2 = ~LED2;
	//	delay();

		LED3 = ~LED3;
	//	delay();

		LED4 = ~LED4;
	//	delay();

		LED5 = ~LED5;
	//	delay();

		LED6 = ~LED6;
	//	delay();

		LED7 = ~LED7;
		delay();
    	delay();
	//	delay();
  		delay();
		delay();
		LED7 = ~LED7;
	//	delay();


		LED6 = ~LED6;
	//	delay();
		
		LED5 = ~LED5;
	//	delay();

		LED4 = ~LED4;
	//	delay();

		LED3 = ~LED3;
	//	delay();

		LED2 = ~LED2;
	//	delay();

		LED1 = ~LED1;
	//	delay();	
		
		LED0 = ~LED0;

	}

}


//=====================================================================================
//函数: delay()
//功能：延迟		
//=====================================================================================
void delay(void)
{
	unsigned long i;
	for(i=0;i < DELAY_TIMES;i++);
}



void led_on(uchar on_flag)
{
	bit i ;
	i = 0 ;
	if(on_flag)
	{   i = 1 ;
		LED0 = i ;
		LED1 = i ;
		LED2 = i ;
		LED3 = i ;
		LED4 = i ;
		LED5 = i ;
		LED6 = i ;
		LED7 = i ;
	}
}
