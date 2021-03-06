#include "nRF24L01_API.h"
#include"sys.h"
/*********     24L01发送接收数据宽度定义	  ***********/
#define TX_ADR_WIDTH    5     //5字节地址宽度
#define RX_ADR_WIDTH    5     //5字节地址宽度
#define TX_PLOAD_WIDTH  32    //32字节有效数据宽度
#define RX_PLOAD_WIDTH  32    //32字节有效数据宽度

const uchar TX_ADDRESS[TX_ADR_WIDTH]= {0x78,0x78,0x78,0x78,0x78}; 	//本地地址

const uchar RX_ADDRESS0[RX_ADR_WIDTH]= {0xE7,0xD3,0xF0,0x35,0x77}; //接收地址0通道
const uchar RX_ADDRESS1[RX_ADR_WIDTH]= {0xC2,0xC2,0xC2,0xC2,0xC2}; 	//接收地址1通道
const uchar RX_ADDRESS2[1]= {0xC3}; 																//接收地址2通道
const uchar RX_ADDRESS3[1]= {0xC4}; 																//接收地址3通道
const uchar RX_ADDRESS4[1]= {0xC5}; 																//接收地址4通道
const uchar RX_ADDRESS5[1]= {0xC6}; 																//接收地址5通道


uchar RevTempDate0[32];																						//通道0接收数据
uchar RevTempDate1[32];																						//通道1接收数据
uchar RevTempDate2[32];																						//通道2接收数据
uchar RevTempDate3[32];																						//通道3接收数据
uchar RevTempDate4[32];																						//通道4接收数据
uchar RevTempDate5[32];																						//通道5接收数据

#define NRF_CSN   PGout(7)  //片选信号，推挽输出  
#define NRF_IRQ   PGin(8)  //IRQ主机数据输入,上拉输入
#define NRF_SCK   PBout(13)  //发送接收模式选择推挽输出
#define NRF_CE   	PGout(6)  //PG6 推完输出
#define NRF_MOSI  PBout(15)  //PB15 MOSI 主机推挽输出 （根据速率初始化PC_CR2寄存器）
#define NRF_MISO  PBin(14)  //PB14 MISO

void delay__us(uchar num)
{
    uchar i,j;
    for(i=0; i>num; i++)
        for(j=100; j>0; j--);
}
void delay_150us(void)
{
    uint i;
    for(i=0; i>600; i++);
}

uchar SPI_RW(uchar byte)
{
    uchar bit_ctr;
    for(bit_ctr=0; bit_ctr<8; bit_ctr++) // 输出8位
    {
        if((uchar)(byte&0x80)==0x80)
            NRF_MOSI=1; 			// MSB TO MOSI
        else
            NRF_MOSI=0;
        byte=(byte<<1);					// shift next bit to MSB
        NRF_SCK=1;
        byte|=NRF_MISO;	        		// capture current MISO bit
        NRF_SCK=0;
    }
    return byte;
}
/*********************************************/
/* 函数功能：给24L01的寄存器写值（一个字节） */
/* 入口参数：reg   要写的寄存器地址          */
/*           value 给寄存器写的值            */
/* 出口参数：status 状态值                   */
/*********************************************/
uchar NRF24L01_Write_Reg(uchar reg,uchar value)
{
    uchar status;

    NRF_CSN=0;                  //CSN=0;
    status = SPI_RW(reg);		//发送寄存器地址,并读取状态值
    SPI_RW(value);
    NRF_CSN=1;                  //CSN=1;

    return status;
}
/*************************************************/
/* 函数功能：读24L01的寄存器值 （一个字节）      */
/* 入口参数：reg  要读的寄存器地址               */
/* 出口参数：value 读出寄存器的值                */
/*************************************************/
uchar NRF24L01_Read_Reg(uchar reg)
{
    uchar value;

    NRF_CSN=0;              //CSN=0;
    SPI_RW(reg);			//发送寄存器值(位置),并读取状态值
    value = SPI_RW(NOP);
    NRF_CSN=1;             	//CSN=1;

    return value;
}
/*********************************************/
/* 函数功能：读24L01的寄存器值（多个字节）   */
/* 入口参数：reg   寄存器地址                */
/*           *pBuf 读出寄存器值的存放数组    */
/*           len   数组字节长度              */
/* 出口参数：status 状态值                   */
/*********************************************/
uchar NRF24L01_Read_Buf(uchar reg,uchar *pBuf,uchar len)
{
    uchar status,u8_ctr;
    NRF_CSN=0;                   	//CSN=0
    status=SPI_RW(reg);				//发送寄存器地址,并读取状态值
    for(u8_ctr=0; u8_ctr<len; u8_ctr++)
        pBuf[u8_ctr]=SPI_RW(0XFF);		//读出数据
    NRF_CSN=1;                 		//CSN=1
    return status;        			//返回读到的状态值
}
/**********************************************/
/* 函数功能：给24L01的寄存器写值（多个字节）  */
/* 入口参数：reg  要写的寄存器地址            */
/*           *pBuf 值的存放数组               */
/*           len   数组字节长度               */
/**********************************************/
uchar NRF24L01_Write_Buf(uchar reg, uchar *pBuf, uchar len)
{
    uchar status,u8_ctr;
    NRF_CSN=0;
    status = SPI_RW(reg);			//发送寄存器值(位置),并读取状态值
    for(u8_ctr=0; u8_ctr<len; u8_ctr++)
        SPI_RW(*pBuf++); 				//写入数据
    NRF_CSN=1;
    return status;          		//返回读到的状态值
}

/*********************************************/
/* 函数功能：24L01接收数据                   */
/* 入口参数：rxbuf 接收数据数组              */
/* 返回值： 0   成功收到数据                 */
/*          1   没有收到数据                 */
/*********************************************/
uchar NRF24L01_RxPacket(void)
{
    uchar state;
    state=NRF24L01_Read_Reg(STATUS);  			//读取状态寄存器的值
    NRF24L01_Write_Reg(nRF_WRITE_REG+STATUS,state); //清除TX_DS或MAX_RT中断标志
    if(state&RX_OK)								//接收到数据
    {
        NRF_CE = 0;
        switch(state&0X0E) {
        case 0x00:
            NRF24L01_Read_Buf(RD_RX_PLOAD,RevTempDate0,RX_PLOAD_WIDTH);
            break;			//读取通道0数据
        case 0x02:
            NRF24L01_Read_Buf(RD_RX_PLOAD,RevTempDate1,RX_PLOAD_WIDTH);
            break;			//读取通道1数据
        case 0x04:
            NRF24L01_Read_Buf(RD_RX_PLOAD,RevTempDate2,RX_PLOAD_WIDTH);
            break;			//读取通道2数据
        case 0x06:
            NRF24L01_Read_Buf(RD_RX_PLOAD,RevTempDate3,RX_PLOAD_WIDTH);
            break;			//读取通道3数据
        case 0x08:
            NRF24L01_Read_Buf(RD_RX_PLOAD,RevTempDate4,RX_PLOAD_WIDTH);
            break;			//读取通道4数据
        case 0x0A:
            NRF24L01_Read_Buf(RD_RX_PLOAD,RevTempDate5,RX_PLOAD_WIDTH);
            break;			//读取通道5数据
        }
        NRF24L01_Write_Reg(FLUSH_RX,0xff);					//清除RX FIFO寄存器
        NRF_CE = 1;
        delay_150us();
        return 0;
    }
    return 1;//没收到任何数据
}
/**********************************************/
/* 函数功能：设置24L01为发送模式              */
/* 入口参数：txbuf  发送数据数组              */
/* 返回值； 0x10    达到最大重发次数，发送失败*/
/*          0x20    成功发送完成              */
/*          0xff    发送失败                  */
/**********************************************/
uchar NRF24L01_TxPacket(uchar *txbuf)
{
    uchar state;

    NRF_CE=0;												//CE拉低，使能24L01配置
    NRF24L01_Write_Buf(WR_TX_PLOAD,txbuf,TX_PLOAD_WIDTH);	//写数据到TX BUF  32个字节
    NRF_CE=1;												//CE置高，使能发送
    while(NRF_IRQ==1);										//等待发送完成
    state=NRF24L01_Read_Reg(STATUS);  						//读取状态寄存器的值
    NRF24L01_Write_Reg(nRF_WRITE_REG+STATUS,state); 			//清除TX_DS或MAX_RT中断标志
    if(state&MAX_TX)										//达到最大重发次数
    {
        NRF24L01_Write_Reg(FLUSH_TX,0xff);					//清除TX FIFO寄存器
        return MAX_TX;
    }
    if(state&TX_OK)											//发送完成
    {
        return TX_OK;
    }
    return 0xff;											//发送失败
}

/********************************************/
/* 函数功能：检测24L01是否存在              */
/* 返回值；  0  存在                        */
/*           1  不存在                      */
/********************************************/
uchar NRF24L01_Check(void)
{
    uchar check_in_buf[5]= {0x11,0x22,0x33,0x44,0x55};
    uchar check_out_buf[5]= {0x00};

    NRF_SCK=0;
    NRF_CSN=1;
    NRF_CE=0;

    NRF24L01_Write_Buf(nRF_WRITE_REG+TX_ADDR, check_in_buf, 5);

    NRF24L01_Read_Buf(nRF_READ_REG+TX_ADDR, check_out_buf, 5);

    if((check_out_buf[0] == 0x11)&&\
            (check_out_buf[1] == 0x22)&&\
            (check_out_buf[2] == 0x33)&&\
            (check_out_buf[3] == 0x44)&&\
            (check_out_buf[4] == 0x55))return 0;
    else return 1;
}


void NRF24L01_RT_Init(void)
{
    NRF_CE=0;
    NRF24L01_Write_Reg(nRF_WRITE_REG+RX_PW_P0,RX_PLOAD_WIDTH);//选择通道0的有效数据宽度
    NRF24L01_Write_Reg(nRF_WRITE_REG+RX_PW_P1,RX_PLOAD_WIDTH);//选择通道1的有效数据宽度
    NRF24L01_Write_Reg(nRF_WRITE_REG+RX_PW_P2,RX_PLOAD_WIDTH);//选择通道2的有效数据宽度
    NRF24L01_Write_Reg(nRF_WRITE_REG+RX_PW_P3,RX_PLOAD_WIDTH);//选择通道3的有效数据宽度
    NRF24L01_Write_Reg(nRF_WRITE_REG+RX_PW_P4,RX_PLOAD_WIDTH);//选择通道4的有效数据宽度
    NRF24L01_Write_Reg(nRF_WRITE_REG+RX_PW_P5,RX_PLOAD_WIDTH);//选择通道5的有效数据宽度

    NRF24L01_Write_Reg(FLUSH_RX,0xff);									//清除RX FIFO寄存器
//    NRF24L01_Write_Buf(nRF_WRITE_REG+TX_ADDR,(uchar*)TX_ADDRESS,TX_ADR_WIDTH);//写TX节点地址
    NRF24L01_Write_Buf(nRF_WRITE_REG+RX_ADDR_P0,(uchar*)RX_ADDRESS0,RX_ADR_WIDTH); //设置TX节点地址,主要为了使能ACK
    NRF24L01_Write_Buf(nRF_WRITE_REG+RX_ADDR_P1,(uchar*)RX_ADDRESS1,RX_ADR_WIDTH); //设置TX节点地址,主要为了使能ACK
    NRF24L01_Write_Buf(nRF_WRITE_REG+RX_ADDR_P2,(uchar*)RX_ADDRESS2,1); //设置TX节点地址,主要为了使能ACK
    NRF24L01_Write_Buf(nRF_WRITE_REG+RX_ADDR_P3,(uchar*)RX_ADDRESS3,1); //设置TX节点地址,主要为了使能ACK
    NRF24L01_Write_Buf(nRF_WRITE_REG+RX_ADDR_P4,(uchar*)RX_ADDRESS4,1); //设置TX节点地址,主要为了使能ACK
    NRF24L01_Write_Buf(nRF_WRITE_REG+RX_ADDR_P5,(uchar*)RX_ADDRESS5,1); //设置TX节点地址,主要为了使能ACK

    NRF24L01_Write_Reg(nRF_WRITE_REG+EN_AA,0x3F);     //使能通道0~5的自动应答
    NRF24L01_Write_Reg(nRF_WRITE_REG+EN_RXADDR,0x3F); //使能通道0~5的接收地址

    NRF24L01_Write_Reg(nRF_WRITE_REG+SETUP_RETR,0x1a);//设置自动重发间隔时间:500us + 86us;最大自动重发次数:10次
    NRF24L01_Write_Reg(nRF_WRITE_REG+RF_CH,0);        //设置RF通道为2.400GHz  频率=2.4+0GHz
    NRF24L01_Write_Reg(nRF_WRITE_REG+RF_SETUP,0x0F);  //设置TX发射参数,0db增益,2Mbps,低噪声增益开启
    NRF24L01_Write_Reg(nRF_WRITE_REG+CONFIG,0x0f);    //配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,接收模式,开启所有中断
    NRF_CE=1;									  //CE置高，使能发送
}




//发送函数
void SEND_BUF(uchar *buf)
{
    NRF_CE=0;
    NRF24L01_Write_Reg(nRF_WRITE_REG+CONFIG,0x0e);    //转化为发送模式
    NRF_CE=1;
    delay__us(15);
    NRF24L01_TxPacket(buf);
    NRF_CE=0;
    NRF24L01_Write_Reg(nRF_WRITE_REG+CONFIG, 0x0f);   //转化为接收模式
    NRF_CE=1;
}


void NRF24L01_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOE|RCC_APB2Periph_GPIOG, ENABLE);


    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOG, &GPIO_InitStructure);  /*GPIO口初始化*///--->PG6---CE

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);  /*GPIO口初始化*///--->PB13--SPI_SCK   PB15--SPI_MOSI

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIO_InitStructure);  /*GPIO口初始化*///--->PE6---LED

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//上拉输入
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);  /*GPIO口初始化*///--->PB14--SPI_MISO

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//上拉输入
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOG, &GPIO_InitStructure);  /*GPIO口初始化*///--->PG8--IRQ

    GPIO_ResetBits(GPIOE,GPIO_Pin_6);               //PE6--LED 输出高
    GPIO_SetBits(GPIOB,GPIO_Pin_14);                //PB14--SPI_MISO 输出高
    GPIO_SetBits(GPIOG,GPIO_Pin_8);                //PG8--IRQ 输出高
    GPIO_ResetBits(GPIOA,GPIO_Pin_0);               //PA0 输出低
}