/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define u8 uint8_t 
//24L01发送接收数据宽度定义
#define TX_ADR_WIDTH    5   	//5字节的地址宽度
#define RX_ADR_WIDTH    5   	//5字节的地址宽度
#define TX_PLOAD_WIDTH  6  	//32字节的用户数据宽度
#define RX_PLOAD_WIDTH  6  	//32字节的用户数据宽度

const u8 TX_ADDRESS[TX_ADR_WIDTH]={0x34,0x43,0x10,0x10,0x01}; //发送地址
const u8 RX_ADDRESS[RX_ADR_WIDTH]={0xaa,0X5a,0x78,0x10,0x01}; //发送地址

//针对NRF24L01修改SPI1驱动
void NRF24L01_SPI_Init(void)
{
	__HAL_SPI_DISABLE(&hspi1);               //先关闭SPI1
	hspi1.Init.CLKPolarity=SPI_POLARITY_LOW; //串行同步时钟的空闲状态为低电平
	hspi1.Init.CLKPhase=SPI_PHASE_1EDGE;     //串行同步时钟的第1个跳变沿（上升或下降）数据被采样
	HAL_SPI_Init(&hspi1);
	__HAL_SPI_ENABLE(&hspi1);                //使能SPI1
}
//SPI1 读写一个字节
//TxData:要写入的字节
//返回值:读取到的字节
u8 SPI1_ReadWriteByte(u8 TxData)
{
	u8 Rxdata;
	HAL_SPI_TransmitReceive(&hspi1,&TxData,&Rxdata,1, 1000);       
	return Rxdata;					//返回收到的数据
}
//初始化24L01的IO口
void NRF24L01_Init(void)
{
	GPIO_InitTypeDef GPIO_Initure;
	__HAL_RCC_GPIOB_CLK_ENABLE();			//开启GPIOB时钟
	__HAL_RCC_GPIOG_CLK_ENABLE();			//开启GPIOG时钟

	//GPIOB14初始化设置:推挽输出
	GPIO_Initure.Pin=GPIO_PIN_14; 			//PB14
	GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //推挽输出
	GPIO_Initure.Pull=GPIO_PULLUP;          //上拉
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //高速
	HAL_GPIO_Init(GPIOB,&GPIO_Initure);     //初始化

	//GPIOG6,7推挽输出
	GPIO_Initure.Pin=GPIO_PIN_6|GPIO_PIN_7;	//PG6,7
	HAL_GPIO_Init(GPIOB,&GPIO_Initure);     //初始化

	//GPIOG.8上拉输入
	GPIO_Initure.Pin=GPIO_PIN_8;			//PG8
	GPIO_Initure.Mode=GPIO_MODE_INPUT;      //输入
	HAL_GPIO_Init(GPIOB,&GPIO_Initure);     //初始化

	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_SET);//PB14输出1,防止SPI FLASH干扰NRF的通信 

	MX_SPI1_Init();							//初始化SPI1
	NRF24L01_SPI_Init();					//针对NRF的特点修改SPI的设置
	HAL_GPIO_WritePin(NRF_CE_GPIO_Port,NRF_CE_Pin,GPIO_PIN_RESET);//NRF24L01_CE=0; 							//使能24L01
	HAL_GPIO_WritePin(NRF_CS_GPIO_Port,NRF_CS_Pin,GPIO_PIN_SET);//NRF24L01_CSN=1;							//SPI片选取消
}
//SPI写寄存器
//reg:指定寄存器地址
//value:写入的值
u8 NRF24L01_Write_Reg(u8 reg,u8 value)
{
	u8 status;	
	HAL_GPIO_WritePin(NRF_CS_GPIO_Port,NRF_CS_Pin,GPIO_PIN_RESET);//NRF24L01_CSN=0;                 //使能SPI传输
	status =SPI1_ReadWriteByte(reg);//发送寄存器号 
	SPI1_ReadWriteByte(value);      //写入寄存器的值
	HAL_GPIO_WritePin(NRF_CS_GPIO_Port,NRF_CS_Pin,GPIO_PIN_SET);//NRF24L01_CSN=1;                 //禁止SPI传输	   
	return(status);       		    //返回状态值
}
//读取SPI寄存器值
//reg:要读的寄存器
u8 NRF24L01_Read_Reg(u8 reg)
{
	u8 reg_val;	    
	HAL_GPIO_WritePin(NRF_CS_GPIO_Port,NRF_CS_Pin,GPIO_PIN_RESET);//NRF24L01_CSN=0;             //使能SPI传输		
	SPI1_ReadWriteByte(reg);    //发送寄存器号
	reg_val=SPI1_ReadWriteByte(0XFF);//读取寄存器内容
	HAL_GPIO_WritePin(NRF_CS_GPIO_Port,NRF_CS_Pin,GPIO_PIN_SET);//NRF24L01_CSN=1;             //禁止SPI传输		    
	return(reg_val);            //返回状态值
}
//在指定位置写指定长度的数据
//reg:寄存器(位置)
//*pBuf:数据指针
//len:数据长度
//返回值,此次读到的状态寄存器值
u8 NRF24L01_Write_Buf(u8 reg, u8 *pBuf, u8 len)
{
	u8 status,u8_ctr;	    
	HAL_GPIO_WritePin(NRF_CS_GPIO_Port,NRF_CS_Pin,GPIO_PIN_RESET);//NRF24L01_CSN=0;             //使能SPI传输
	status = SPI1_ReadWriteByte(reg);//发送寄存器值(位置),并读取状态值
	for(u8_ctr=0; u8_ctr<len; u8_ctr++)
	{
		SPI1_ReadWriteByte(*pBuf++); //写入数据	 
	}
	HAL_GPIO_WritePin(NRF_CS_GPIO_Port,NRF_CS_Pin,GPIO_PIN_SET);//NRF24L01_CSN=1;             //关闭SPI传输
	return status;              //返回读到的状态值
}
//在指定位置读出指定长度的数据
//reg:寄存器(位置)
//*pBuf:数据指针
//len:数据长度
//返回值,此次读到的状态寄存器值 
u8 NRF24L01_Read_Buf(u8 reg,u8 *pBuf,u8 len)
{
	u8 status,u8_ctr;
	HAL_GPIO_WritePin(NRF_CS_GPIO_Port,NRF_CS_Pin,GPIO_PIN_RESET);//NRF24L01_CSN=0;//使能SPI传输
	status=SPI1_ReadWriteByte(reg);//发送寄存器值(位置),并读取状态值
	for(u8_ctr=0;u8_ctr<len;u8_ctr++)pBuf[u8_ctr]=SPI1_ReadWriteByte(0XFF);//读出数据
	HAL_GPIO_WritePin(NRF_CS_GPIO_Port,NRF_CS_Pin,GPIO_PIN_SET);//NRF24L01_CSN=1;//关闭SPI传输
	return status;             //返回读到的状态值
}
//检测24L01是否存在
//返回值:0，成功;1，失败	
u8 NRF24L01_Check(void)
{
	u8 buf[5]={0XA5,0XA5,0XA5,0XA5,0XA5};
	u8 i;
	//SPI1_SetSpeed(SPI_BAUDRATEPRESCALER_8); //spi速度为10.5Mhz（（24L01的最大SPI时钟为10Mhz,这里大一点没关系）
	NRF24L01_Write_Buf(NRF_WRITE_REG+TX_ADDR,buf,5);//写入5个字节的地址.
	NRF24L01_Read_Buf(TX_ADDR,buf,5); //读出写入的地址  
	for(i=0;i<5;i++)
	{
		if(buf[i]!=0XA5)
		{
			break;
		}
	}
	if(i!=5)return 1;//检测24L01错误
	return 0;		 //检测到24L01
}
//该函数初始化NRF24L01到RX模式
//设置RX地址,写RX数据宽度,选择RF频道,波特率和LNA HCURR
//当CE变高后,即进入RX模式,并可以接收数据了		   
void NRF24L01_RX_Mode(void)
{
	HAL_GPIO_WritePin(NRF_CE_GPIO_Port,NRF_CE_Pin,GPIO_PIN_RESET);//NRF24L01_CE=0;	  
	NRF24L01_Write_Buf(NRF_WRITE_REG+RX_ADDR_P0,(u8*)RX_ADDRESS,RX_ADR_WIDTH);//写RX节点地址
	NRF24L01_Write_Reg(NRF_WRITE_REG+EN_AA,0x00);       //使能通道0的自动应答    
	NRF24L01_Write_Reg(NRF_WRITE_REG+SETUP_AW,0x01);
	NRF24L01_Write_Reg(NRF_WRITE_REG+EN_RXADDR,0x01);   //使能通道0的接收地址  	 
	NRF24L01_Write_Reg(NRF_WRITE_REG+RF_CH,10);	        //设置RF通信频率		  
	NRF24L01_Write_Reg(NRF_WRITE_REG+RX_PW_P0,6);//选择通道0的有效数据宽度 	    
	NRF24L01_Write_Reg(NRF_WRITE_REG+RF_SETUP,0x07);    //设置TX发射参数,0db增益,2Mbps,低噪声增益开启   
	NRF24L01_Write_Reg(NRF_WRITE_REG+CONFIG, 0x0f);     //配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,接收模式 
	HAL_GPIO_WritePin(NRF_CE_GPIO_Port,NRF_CE_Pin,GPIO_PIN_SET);//NRF24L01_CE=1; //CE为高,进入接收模式 
}
//启动NRF24L01发送一次数据
//txbuf:待发送数据首地址
//返回值:0，接收完成；其他，错误代码
u8 NRF24L01_RxPacket(u8 *rxbuf)
{
	u8 sta;		    							   
	//SPI1_SetSpeed(SPI_BAUDRATEPRESCALER_8); //spi速度为6.75Mhz（24L01的最大SPI时钟为10Mhz）   
	sta=NRF24L01_Read_Reg(STATUS);          //读取状态寄存器的值    	 
	NRF24L01_Write_Reg(NRF_WRITE_REG+STATUS,sta); //清除TX_DS或MAX_RT中断标志
	if(sta&RX_OK)//接收到数据
	{
		NRF24L01_Read_Buf(RD_RX_PLOAD,rxbuf,RX_PLOAD_WIDTH);//读取数据
		NRF24L01_Write_Reg(FLUSH_RX,0xff);  //清除RX FIFO寄存器 
		return 0; 
	}	   
	return 1;//没收到任何数据
}





/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	u8 tmp_buf[20];
	uint32_t i=0;
  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
	NRF24L01_Init();
	NRF24L01_RX_Mode();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		if(NRF24L01_RxPacket(tmp_buf)==0)//一旦接收到信息,则显示出来
		{
			HAL_GPIO_WritePin(GPIOA, LED1_Pin,!HAL_GPIO_ReadPin(GPIOA, LED1_Pin));
		}
		if(i++ >= 6000)
		{
			i=0;
			HAL_GPIO_WritePin(GPIOA, LED2_Pin,!HAL_GPIO_ReadPin(GPIOA, LED2_Pin));
		}
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
