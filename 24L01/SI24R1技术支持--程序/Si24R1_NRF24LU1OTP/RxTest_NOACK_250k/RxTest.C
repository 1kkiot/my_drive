#include <stdio.h>			//printf等
#include <defDataType.h>	//自定义的数据类型，如uchar 、uint 
#include <nRF24U1OTP.h>		//nRF24LU1OTP引脚、寄存器等定义

#define BAUDRATE           9600       //串口波特率
#define OLEN               64            //输出缓冲区的大小

idata uchar   outbuf[OLEN];                   //发送缓冲区的存储
idata uchar   ostart           = 0;           //发送缓冲区起始索引
idata uchar   oend             = 0;           //发送缓冲区起始索引
bit  sendfull = 0;           //标志-：标识发送缓冲区满
bit  sendactive = 0;           //标志：标识发送器激活

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

void  spi_init(void);										//SPI引脚初始化
uchar spi_rw_byte(uchar byte);								//SPI写字节 
uchar spi_rw_reg(uchar reg, uchar value);					//SPI写寄存器
uchar spi_rd_reg(uchar reg);								//SPI读寄存器
uchar spi_write_buf(uchar reg, uchar *pBuf, uchar bytes);	//SPI写TX FIFO
uchar spi_read_buf(uchar reg, uchar *pBuf, uchar bytes);	//SPI读RX FIFO

//=================================
void rx_mode(void);											//PRX mode
void  delay(void);
uchar treg ;

void main(void)
{
	uchar flag;
	uchar i;

// IO CONFIG 
	DPS = 0x00 ;
	P0DIR = 0xF2 ;  
	P0ALT = 0xC6 ; 
	P0EXP = 0x00 ; 
	CLKCTL = 0x30;
//	P1DIR = 0x03 ;
//	P1ALT = 0x0F ; // enable RF IOs
	///////////////
	
	P0 = 0x01 ; 
	flag = 0;	
	system_init();
	// spi_rw_reg(W_REGISTER + EN_RXADDR, 0xAA);           				// 使能接收通道0
	// while(1);
//	spi_rw_reg(W_REGISTER + RX_ADDR_P2, 0xAA);   
//	treg = spi_rd_reg(RX_ADDR_P2);
//	printf("0x%x \n" , (uint)treg);

//	while(1);

	
	for(i=0;i<TX_PLOAD_WIDTH;i++)
	{
		RX_BUF[i] = 0;
	}
	
	printf("\n=======================================================\n");
	printf("===================  nRF24LU1OTP Rx TEST  ===============\n");
	printf("===================        NO ACK       ===============\n");
	printf("=======================================================\n");



	
	printf("Start to Recevie......\n");

	rx_mode();			//接收数据

	while(1)
	{
		while(~RFIRQ);			  
		
		status_reg = spi_rd_reg(STATUS);	  					// 读状态寄存器
		if(RX_DR == 1)				  			  				// 判断是否接收到数据
		{
			spi_read_buf(R_RX_PAYLOAD, RX_BUF, TX_PLOAD_WIDTH);  // 从RX FIFO读出数据到RX_BUF[]中
			spi_rw_reg(W_REGISTER + STATUS, status_reg|0x40);  		// 清除RX_DS中断标志
			flag = 1;
		}

		if(flag == 1)//接收完成
		{
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
	


   

}//main() end


void init_uart(void)
{
     PCON = 0x80;   
     SCON = 0x40;
	 WDCON =0x80 ; // SELECT INTERNAL BAUD RATE 
	 S0RELL = 0xF3 ; 
	 S0RELH = 0x03 ; //baud rate = 9600   

     ES  = 1;  

}

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
void system_init(void)
{
 
	spi_init();				   //SPI 初始化
	init_uart();               //串口初始化
    EA  = 1;                   //允许所有中断
        
}

//=====================================================================================
void spi_init(void)
{
	RFCTL = 0x15 ;  // enable RFSPI 	
	RFCON = 0x06 ;  // csn = 1 , ce = 0  enbale RF CLOCK
}

//=====================================================================================
uchar spi_rw_byte(uchar byte)
{	
	RFSBUF = byte ; 
	RFRDY=0;
	while(~RFRDY) ;
	RFRDY = 0 ;
    return(RFSBUF);           		// 返回读出的一字节
}


//=====================================================================================
uchar spi_rw_reg(uchar reg, uchar value)
{
	uchar status;

  //	RFCON = 0x00 ;                   	// CSN置低，开始传输数据
    RFCON = RFCON & 0xFD  ;
  	status = spi_rw_byte(reg);      	// 选择寄存器，同时返回状态字
  	spi_rw_byte(value);                 // 然后写数据到该寄存器 
   RFCON = RFCON | 0x02 ;
  	return(status);            	    	// 返回状态寄存器
}

//=====================================================================================
uchar spi_rd_reg(uchar reg)
{
	uchar value;

  	//RFCON = 0x00;                	// CSN置低，开始传输数据
	RFCON = RFCON & 0xFD  ;
  	spi_rw_byte(reg);              	// 选择寄存器
  	value 	= spi_rw_byte(0);      	// 然后从该寄存器读数据 
  	//RFCON = 0x02;                	// CSN拉高，结束数据传输
   RFCON = RFCON | 0x02 ;
  	return(value);            		// 返回寄存器数据
}


uchar spi_read_buf(uchar reg, uchar *pBuf, uchar bytes)
{

	uchar status;
	uchar i; 	

  //	RFCON = 0x00;                   	 // CSN置低，开始传输数据
  	RFCON = RFCON & 0xFD  ;
  	status  = spi_rw_byte(reg);      // 选择寄存器，同时返回状态字
  	for(i = 0; i < bytes; i++)
    {
   		pBuf[i] = spi_rw_byte(0);    // 逐个字节从nRF24L01+读出
	}
 // 	RFCON = 0x02;                     // CSN拉高，结束数据传输
 	RFCON = RFCON | 0x02 ;
  	return(status);             	 // 返回状态寄存器
}

//=====================================================================================
uchar spi_write_buf(uchar reg, uchar *pBuf, uchar bytes)
{
	uchar status, i;
  //	RFCON = 0x00;  
  	RFCON = RFCON & 0xFD  ;                   // CSN置低，开始传输数据
  	status 	= spi_rw_byte(reg);      // 选择寄存器，同时返回状态字
  	for(i = 0; i < bytes; i++)
    {
   		spi_rw_byte(pBuf[i]);        // 逐个字节写入nRF24L01+
	}
  //	RFCON = 0x02;                     // CSN拉高，结束数据传输
  	RFCON = RFCON | 0x02 ;
  	return(status);             	 // 返回状态寄存器
}


//=====================================================================================
void rx_mode(void)
{
	//CE = 0 ;
	RFCON = RFCON & 0xFE ;

	spi_write_buf(W_REGISTER + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH);	// 接收设备接收通道0使用和发送设备相同的发送地址
  	
	spi_rw_reg(W_REGISTER + EN_RXADDR, 0x01);           				// 使能接收通道0
	treg = spi_rd_reg(0x01);
	printf("%x \n" , (uint)treg);

  	spi_rw_reg(W_REGISTER + RF_CH, 0x40);                 				// 选择射频通道0x40
  	spi_rw_reg(W_REGISTER + RX_PW_P0, TX_PLOAD_WIDTH);  				// 接收通道0选择和发送通道相同有效数据宽度

  	spi_rw_reg(W_REGISTER + RF_SETUP, 0x26);            				// 数据传输率250kbps,0dbm TX power

  	spi_rw_reg(W_REGISTER + CONFIG, 0x2f);              				// Rx IRQ 使能，CRC使能，16位CRC校验，上电，RX mode	
   //	CE = 1;                                            					// 拉高CE启动接收设备
   RFCON = RFCON | 0x01;  
}



//=====================================================================================
//函数: delay()	
//=====================================================================================
void delay(void)
{
	unsigned long i;
	for(i=0;i < DELAY_TIMES;i++);
}
