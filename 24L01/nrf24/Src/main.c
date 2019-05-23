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
//24L01���ͽ������ݿ�ȶ���
#define TX_ADR_WIDTH    5   	//5�ֽڵĵ�ַ���
#define RX_ADR_WIDTH    5   	//5�ֽڵĵ�ַ���
#define TX_PLOAD_WIDTH  6  	//32�ֽڵ��û����ݿ��
#define RX_PLOAD_WIDTH  6  	//32�ֽڵ��û����ݿ��

const u8 TX_ADDRESS[TX_ADR_WIDTH]={0x34,0x43,0x10,0x10,0x01}; //���͵�ַ
const u8 RX_ADDRESS[RX_ADR_WIDTH]={0xaa,0X5a,0x78,0x10,0x01}; //���͵�ַ

//���NRF24L01�޸�SPI1����
void NRF24L01_SPI_Init(void)
{
	__HAL_SPI_DISABLE(&hspi1);               //�ȹر�SPI1
	hspi1.Init.CLKPolarity=SPI_POLARITY_LOW; //����ͬ��ʱ�ӵĿ���״̬Ϊ�͵�ƽ
	hspi1.Init.CLKPhase=SPI_PHASE_1EDGE;     //����ͬ��ʱ�ӵĵ�1�������أ��������½������ݱ�����
	HAL_SPI_Init(&hspi1);
	__HAL_SPI_ENABLE(&hspi1);                //ʹ��SPI1
}
//SPI1 ��дһ���ֽ�
//TxData:Ҫд����ֽ�
//����ֵ:��ȡ�����ֽ�
u8 SPI1_ReadWriteByte(u8 TxData)
{
	u8 Rxdata;
	HAL_SPI_TransmitReceive(&hspi1,&TxData,&Rxdata,1, 1000);       
	return Rxdata;					//�����յ�������
}
//��ʼ��24L01��IO��
void NRF24L01_Init(void)
{
	GPIO_InitTypeDef GPIO_Initure;
	__HAL_RCC_GPIOB_CLK_ENABLE();			//����GPIOBʱ��
	__HAL_RCC_GPIOG_CLK_ENABLE();			//����GPIOGʱ��

	//GPIOB14��ʼ������:�������
	GPIO_Initure.Pin=GPIO_PIN_14; 			//PB14
	GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //�������
	GPIO_Initure.Pull=GPIO_PULLUP;          //����
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //����
	HAL_GPIO_Init(GPIOB,&GPIO_Initure);     //��ʼ��

	//GPIOG6,7�������
	GPIO_Initure.Pin=GPIO_PIN_6|GPIO_PIN_7;	//PG6,7
	HAL_GPIO_Init(GPIOB,&GPIO_Initure);     //��ʼ��

	//GPIOG.8��������
	GPIO_Initure.Pin=GPIO_PIN_8;			//PG8
	GPIO_Initure.Mode=GPIO_MODE_INPUT;      //����
	HAL_GPIO_Init(GPIOB,&GPIO_Initure);     //��ʼ��

	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_SET);//PB14���1,��ֹSPI FLASH����NRF��ͨ�� 

	MX_SPI1_Init();							//��ʼ��SPI1
	NRF24L01_SPI_Init();					//���NRF���ص��޸�SPI������
	HAL_GPIO_WritePin(NRF_CE_GPIO_Port,NRF_CE_Pin,GPIO_PIN_RESET);//NRF24L01_CE=0; 							//ʹ��24L01
	HAL_GPIO_WritePin(NRF_CS_GPIO_Port,NRF_CS_Pin,GPIO_PIN_SET);//NRF24L01_CSN=1;							//SPIƬѡȡ��
}
//SPIд�Ĵ���
//reg:ָ���Ĵ�����ַ
//value:д���ֵ
u8 NRF24L01_Write_Reg(u8 reg,u8 value)
{
	u8 status;	
	HAL_GPIO_WritePin(NRF_CS_GPIO_Port,NRF_CS_Pin,GPIO_PIN_RESET);//NRF24L01_CSN=0;                 //ʹ��SPI����
	status =SPI1_ReadWriteByte(reg);//���ͼĴ����� 
	SPI1_ReadWriteByte(value);      //д��Ĵ�����ֵ
	HAL_GPIO_WritePin(NRF_CS_GPIO_Port,NRF_CS_Pin,GPIO_PIN_SET);//NRF24L01_CSN=1;                 //��ֹSPI����	   
	return(status);       		    //����״ֵ̬
}
//��ȡSPI�Ĵ���ֵ
//reg:Ҫ���ļĴ���
u8 NRF24L01_Read_Reg(u8 reg)
{
	u8 reg_val;	    
	HAL_GPIO_WritePin(NRF_CS_GPIO_Port,NRF_CS_Pin,GPIO_PIN_RESET);//NRF24L01_CSN=0;             //ʹ��SPI����		
	SPI1_ReadWriteByte(reg);    //���ͼĴ�����
	reg_val=SPI1_ReadWriteByte(0XFF);//��ȡ�Ĵ�������
	HAL_GPIO_WritePin(NRF_CS_GPIO_Port,NRF_CS_Pin,GPIO_PIN_SET);//NRF24L01_CSN=1;             //��ֹSPI����		    
	return(reg_val);            //����״ֵ̬
}
//��ָ��λ��дָ�����ȵ�����
//reg:�Ĵ���(λ��)
//*pBuf:����ָ��
//len:���ݳ���
//����ֵ,�˴ζ�����״̬�Ĵ���ֵ
u8 NRF24L01_Write_Buf(u8 reg, u8 *pBuf, u8 len)
{
	u8 status,u8_ctr;	    
	HAL_GPIO_WritePin(NRF_CS_GPIO_Port,NRF_CS_Pin,GPIO_PIN_RESET);//NRF24L01_CSN=0;             //ʹ��SPI����
	status = SPI1_ReadWriteByte(reg);//���ͼĴ���ֵ(λ��),����ȡ״ֵ̬
	for(u8_ctr=0; u8_ctr<len; u8_ctr++)
	{
		SPI1_ReadWriteByte(*pBuf++); //д������	 
	}
	HAL_GPIO_WritePin(NRF_CS_GPIO_Port,NRF_CS_Pin,GPIO_PIN_SET);//NRF24L01_CSN=1;             //�ر�SPI����
	return status;              //���ض�����״ֵ̬
}
//��ָ��λ�ö���ָ�����ȵ�����
//reg:�Ĵ���(λ��)
//*pBuf:����ָ��
//len:���ݳ���
//����ֵ,�˴ζ�����״̬�Ĵ���ֵ 
u8 NRF24L01_Read_Buf(u8 reg,u8 *pBuf,u8 len)
{
	u8 status,u8_ctr;
	HAL_GPIO_WritePin(NRF_CS_GPIO_Port,NRF_CS_Pin,GPIO_PIN_RESET);//NRF24L01_CSN=0;//ʹ��SPI����
	status=SPI1_ReadWriteByte(reg);//���ͼĴ���ֵ(λ��),����ȡ״ֵ̬
	for(u8_ctr=0;u8_ctr<len;u8_ctr++)pBuf[u8_ctr]=SPI1_ReadWriteByte(0XFF);//��������
	HAL_GPIO_WritePin(NRF_CS_GPIO_Port,NRF_CS_Pin,GPIO_PIN_SET);//NRF24L01_CSN=1;//�ر�SPI����
	return status;             //���ض�����״ֵ̬
}
//���24L01�Ƿ����
//����ֵ:0���ɹ�;1��ʧ��	
u8 NRF24L01_Check(void)
{
	u8 buf[5]={0XA5,0XA5,0XA5,0XA5,0XA5};
	u8 i;
	//SPI1_SetSpeed(SPI_BAUDRATEPRESCALER_8); //spi�ٶ�Ϊ10.5Mhz����24L01�����SPIʱ��Ϊ10Mhz,�����һ��û��ϵ��
	NRF24L01_Write_Buf(NRF_WRITE_REG+TX_ADDR,buf,5);//д��5���ֽڵĵ�ַ.
	NRF24L01_Read_Buf(TX_ADDR,buf,5); //����д��ĵ�ַ  
	for(i=0;i<5;i++)
	{
		if(buf[i]!=0XA5)
		{
			break;
		}
	}
	if(i!=5)return 1;//���24L01����
	return 0;		 //��⵽24L01
}
//�ú�����ʼ��NRF24L01��RXģʽ
//����RX��ַ,дRX���ݿ��,ѡ��RFƵ��,�����ʺ�LNA HCURR
//��CE��ߺ�,������RXģʽ,�����Խ���������		   
void NRF24L01_RX_Mode(void)
{
	HAL_GPIO_WritePin(NRF_CE_GPIO_Port,NRF_CE_Pin,GPIO_PIN_RESET);//NRF24L01_CE=0;	  
	NRF24L01_Write_Buf(NRF_WRITE_REG+RX_ADDR_P0,(u8*)RX_ADDRESS,RX_ADR_WIDTH);//дRX�ڵ��ַ
	NRF24L01_Write_Reg(NRF_WRITE_REG+EN_AA,0x00);       //ʹ��ͨ��0���Զ�Ӧ��    
	NRF24L01_Write_Reg(NRF_WRITE_REG+SETUP_AW,0x01);
	NRF24L01_Write_Reg(NRF_WRITE_REG+EN_RXADDR,0x01);   //ʹ��ͨ��0�Ľ��յ�ַ  	 
	NRF24L01_Write_Reg(NRF_WRITE_REG+RF_CH,10);	        //����RFͨ��Ƶ��		  
	NRF24L01_Write_Reg(NRF_WRITE_REG+RX_PW_P0,6);//ѡ��ͨ��0����Ч���ݿ�� 	    
	NRF24L01_Write_Reg(NRF_WRITE_REG+RF_SETUP,0x07);    //����TX�������,0db����,2Mbps,���������濪��   
	NRF24L01_Write_Reg(NRF_WRITE_REG+CONFIG, 0x0f);     //���û�������ģʽ�Ĳ���;PWR_UP,EN_CRC,16BIT_CRC,����ģʽ 
	HAL_GPIO_WritePin(NRF_CE_GPIO_Port,NRF_CE_Pin,GPIO_PIN_SET);//NRF24L01_CE=1; //CEΪ��,�������ģʽ 
}
//����NRF24L01����һ������
//txbuf:�����������׵�ַ
//����ֵ:0��������ɣ��������������
u8 NRF24L01_RxPacket(u8 *rxbuf)
{
	u8 sta;		    							   
	//SPI1_SetSpeed(SPI_BAUDRATEPRESCALER_8); //spi�ٶ�Ϊ6.75Mhz��24L01�����SPIʱ��Ϊ10Mhz��   
	sta=NRF24L01_Read_Reg(STATUS);          //��ȡ״̬�Ĵ�����ֵ    	 
	NRF24L01_Write_Reg(NRF_WRITE_REG+STATUS,sta); //���TX_DS��MAX_RT�жϱ�־
	if(sta&RX_OK)//���յ�����
	{
		NRF24L01_Read_Buf(RD_RX_PLOAD,rxbuf,RX_PLOAD_WIDTH);//��ȡ����
		NRF24L01_Write_Reg(FLUSH_RX,0xff);  //���RX FIFO�Ĵ��� 
		return 0; 
	}	   
	return 1;//û�յ��κ�����
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
		if(NRF24L01_RxPacket(tmp_buf)==0)//һ�����յ���Ϣ,����ʾ����
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
