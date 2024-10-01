/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "cJSON.h"
#include "ssd1306.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
uint8_t str[3]={"0"};
uint8_t str2[3]={"0"};
int timeRepeat = 0;
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
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;
DMA_HandleTypeDef hdma_usart2_tx;
DMA_HandleTypeDef hdma_usart2_rx;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
uint8_t modemString2[250];
int modemStringLength2 = 0;
uint8_t modemString1[250];
int modemStringLength1 = 0;
bool modemEndMess = false;
uint8_t oldModem[250];
uint8_t oldModem2[250];
uint8_t oldModem3[250];
uint8_t smsNum[6];
uint8_t unreedSms[15] = {0};
uint8_t smsText[] = "Hello";
uint8_t csqlvl[]="99";
int step=0, rxNew = false;
bool ready=false;
bool comOpen = false, comClose = false, comStart = false, answ = false;
uint8_t bat[8];
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void s800LSend(uint8_t *text, int nums) {
	HAL_UART_Transmit(&huart2, text, nums, 0xFFFF);
	HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", 2, 0xFFFF);
	HAL_UART_Transmit(&huart1, text, nums, 0xFFFF);
	HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n", 2, 0xFFFF);
	return;
}

int s800lMessAdd(uint8_t* text) {
	int i = 0;
	for (i; i<250; i++) {
		if (text[i]=='!') return i;
	}
}

void txATcommand() {
	//HAL_Delay(500);
	switch (step) {
		case 0: {
			ready=false;
			s800LSend((uint8_t*)"ATE0", 4);
			break;
		}
		case 1: {
			ready=false;
			s800LSend((uint8_t*)"AT+CSQ", 6);
			break;
		}
		case 2: {
			ready=false;
			s800LSend((uint8_t*)"AT+CBC", 6);
			break;
		}
		case 3: {
			ready=false;
			s800LSend((uint8_t*)"AT+CREG?", 8);
			break;
		}
		case 4: {
			ready=false;
			s800LSend((uint8_t*)"AT+SAPBR=1,1", 12);
			HAL_Delay(2000);
			break;
		}
		case 5: {
			ready=false;
			s800LSend((uint8_t*)"AT+HTTPINIT", 11);
			break;
		}
		case 6: {
			ready=false;
			s800LSend((uint8_t*)"AT+HTTPPARA=\"CID\",1",19);
			break;
		}
		case 7: {
			ready=false;
			bat[7]=0;
			//uint8_t end[] = "\"";
			//uint8_t mess[] = "AT+HTTPPARA=\"URL\",\"http://simple.spamigor.ru/api/gst?csq=19&bat=99&mes=";//"AT+HTTPPARA=\"URL\",\"http://simple.spamigor.ru/api/test?a=send%20from%20stm32mod&sms=";
			uint8_t ext[250];
			if (answ) snprintf(ext, 250, "AT+HTTPPARA=\"URL\",\"http://simple.spamigor.ru/api/gst?csq=%s&bat=%s&mes=.%s.\"", csqlvl, bat, smsText);
			else snprintf(ext, 250, "AT+HTTPPARA=\"URL\",\"http://simple.spamigor.ru/api/gst?csq=%s&bat=%s\"", csqlvl, bat);
			s800LSend(ext, strlen(ext));
			break;
		}
		case 8: {
			ready=false;
			s800LSend((uint8_t*)"AT+HTTPACTION=0",15);
			break;
		}
		case 9: {
			ready=false;
			s800LSend((uint8_t*)"AT+HTTPREAD",11);
			break;
		}
		case 10: {
			ready=false;
			s800LSend((uint8_t*)"AT+HTTPTERM",11);
			break;
		}
		case 11: {
			ready=false;
			s800LSend((uint8_t*)"AT+SAPBR=0,1", 12);
			break;
		}
		case 20: {
			ready = false;
			s800LSend((uint8_t*)"AT+CMGF=1", 9);
			break;
		}
		case 21: {
			ready=false;
			uint8_t ext[15];
			uint8_t at[] = "AT+CMGR=";
			snprintf(ext, 15, "%s%s", at, smsNum);
			s800LSend(ext, s800lMessAdd(ext));
			break;
		}
		case 22: {
			ready = false;
			s800LSend((uint8_t*)"AT+CMGDA=\"DEL ALL\"", 18);
			break;
		}
		case 23: {
			step=24;
			ready = true;
			s800LSend((uint8_t*)"AT+CMGS=\"+79999811066\"", 22);
			break;
		}
		case 24: {
			ready = false;
			step=24;
			uint8_t ggg[20];
			sprintf(ggg, "%s%c", (uint8_t*)"stm32 is working", (uint8_t)0x1A);
			s800LSend(ggg, 17);
			break;
		}
	}
	return;
}

void rxATcommand(uint8_t* text) {
	switch (step) {
		case 0: {
			if (strstr((char*)text, (char*)"OK")) {
				step++;
				ready=true;
			}
			else if (strstr((char*)text, (char*)"CMTI")) {
				step=20;
				ready=true;
			}
			else {
				step=0;
				ready=true;
			}
			break;
		}
		case 1: {
			if (strstr((char*)oldModem3, (char*)"CSQ")) {
				csqlvl[0]=oldModem3[6];
				csqlvl[1]=oldModem3[7];
				step++;
			}
			ready=true;
			break;
		}
		case 2: {
			if (strstr((char*)oldModem3, (char*)"CBC")) {
				bat[0]=oldModem3[8];
				bat[1]=oldModem3[9];
				bat[2]=oldModem3[10];
				bat[3]=oldModem3[11];
				bat[4]=oldModem3[12];
				bat[5]=oldModem3[13];
				bat[6]=oldModem3[14];
				bat[7]=oldModem3[15];
				step++;
			}
			ready=true;
			break;
		}
		case 3: {
			if (strstr((char*)oldModem3, (char*)"0,1")) {
				step++;
				ready=true;
				break;
			}
			else {
				ready=false;
			}
			break;
		}
		case 4: {
			if (strstr((char*)text, (char*)"OK")) {
				step++;
			}
			else if (strstr((char*)text, (char*)"ERROR")){
				step=10;
			}
			ready=true;
			break;
		}
		case 5: {
			if (strstr((char*)text, (char*)"OK")) {
				step++;
			}
			else if (strstr((char*)text, (char*)"ERROR")){
				step=10;
			}
			ready=true;
			break;
		}
		case 6: {
			if (strstr((char*)text, (char*)"OK")) {
				step++;
				ready=true;
			}
			else {
				step=10;
				ready=true;
			}
			break;
		}
		case 7: {
			if (strstr((char*)text, (char*)"OK")) {
				step++;
				ready=true;
			}
			else {
				step=10;
				ready=true;
			}
			break;
		}
		case 8: {
			if (strstr((char*)text, (char*)"200")) {
				step++;
				ready=true;
			}
			else if ((strstr((char*)text, (char*)"0,60"))||(strstr((char*)text, (char*)"0,40"))) {
				step=10;
				ready=true;
			}
			break;
		}
		case 9: {
			if (!answ) {
				if (strstr((char*)text, (char*)"OK")) {
					if (strstr((char*)oldModem2, "res")) {
						comStart = false;
						comOpen = false;
						comClose = false;
						memset(smsText, 0, strlen(smsText));
						if (oldModem2[6]=='t') {
							comStart=true;
							strcpy(smsText, (uint8_t*)"Start");
							HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 0);
							HAL_Delay(100);
							HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 1);
							HAL_Delay(100);
							HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 0);
							HAL_Delay(100);
							HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 1);
							HAL_Delay(100);
							HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 0);
							HAL_Delay(100);
							HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 1);
							HAL_Delay(100);
							HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 0);
							HAL_Delay(100);
							HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 1);
							HAL_Delay(100);
						}
						else if (oldModem2[17]=='t') {
							comOpen=true;
							strcpy(smsText, (uint8_t*)"Open");
							HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 0);
						}
						else if (oldModem2[28]=='t') {
							comClose=true;
							strcpy(smsText, (uint8_t*)"Closed");
							HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 1);
						}
					}
					step=7;
					answ = true;
					ready=true;
				}
				else {
					step=10;
					answ = false;
					ready=true;
				}
			}
			else {
				if (strstr((char*)text, (char*)"OK")) {
					step++;
					answ = false;
					ready=true;
				}
				else {
					step=10;
					answ = false;
					ready=true;
				}
			}
			break;
		}
		case 10: {
			step=11;
			ready=true;
			break;
		}
		case 11: {
			if (strstr((char*)text, (char*)"OK")) {
				step=12;
				ready=true;
			}
			else {
				step=0;
				ready=true;
			}
			break;
		}
		case 12: {
			if (strstr((char*)text, (char*)"CMTI")) {
				bool start = false;
				int j = 0;
				for (int i = 0; i<250; i++) {
					if (start) {
						if ((text[i]=='\r')||(text[i]=='\n')||(text[i]=='\0')) {
							smsNum[j]='!';  //стоп-символ
							break;
						}
						else {
							smsNum[j]=text[i];
							j++;
						}
					}
					else {
						if (text[i]==',') start = true;
					}
				}
				step=20;
				ready=true;
			}
			else {
				step=12;
				ready=true;
			}
			break;
		}
		case 20: {
			if (strstr((char*)text, (char*)"OK")) {
				step++;
				ready=true;
			}
			else {
				step=12;
				ready=true;
			}
			break;
		}
		case 21: {
			memset(smsText, 0, sizeof(smsText));
			strcpy(smsText, oldModem3);
			step++;
			ready=true;
			break;
		}
		case 22: {
			step++;
			ready=true;
			break;
		}
		case 23: {
			step++;
			ready=true;
			break;
		}
		case 24: {
			if (strstr((char*)text, (char*)"OK")) {
				step=0;
				ready=true;
			}
			else if (strstr((char*)text, (char*)">")) {
				step=0;
				ready=true;
			}
			else {
				step=12;
				ready=true;
			}
			break;
		}
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart == &huart2) {
		if (str[0]!='\n') {
			modemString2[modemStringLength2] = str[0];
			modemStringLength2++;
		}
		else {
			modemString2[modemStringLength2] = str[0];
			modemStringLength2++;
			HAL_UART_Transmit_DMA(&huart1, modemString2, modemStringLength2);
			for (int i = modemStringLength2; i<250; i++)modemString2[i]=(uint8_t)0x00;
			for (int i = 0; i<250; i++) oldModem3[i] = oldModem2[i];
			for (int i = 0; i<250; i++) oldModem2[i] = oldModem[i];
			for (int i = 0; i<250; i++) oldModem[i] = modemString2[i];
			modemStringLength2 = 0;
			oldModem[strlen(oldModem)-1]=0;
			oldModem[strlen(oldModem)-1]=0;
			rxNew=true;
		}
		HAL_UART_Receive_IT(&huart2,str,1);
	}
	else {
		if (str2[0]!='\n') {
			modemString1[modemStringLength1] = str2[0];
			modemStringLength1++;
		}
		else {
			modemString1[modemStringLength1] = str2[0];
			modemStringLength1++;
			HAL_UART_Transmit_DMA(&huart2, modemString1, modemStringLength1);
			modemStringLength1 = 0;

		}
		HAL_UART_Receive_IT(&huart1,str2,1);
	}
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

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
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  SSD1306_Init();
  SSD1306_GotoXY(0, 44); //Устанавливаем курсор в позицию 0;44. Сначала по горизонтали, потом вертикали.
  SSD1306_Puts("Hello, habrahabr!!", &Font_7x10, SSD1306_COLOR_WHITE); //пишем надпись в выставленной позиции шрифтом "Font_7x10" белым цветом.
  SSD1306_DrawCircle(10, 33, 7, SSD1306_COLOR_WHITE);
  SSD1306_UpdateScreen();

  HAL_UART_Transmit(&huart1,(uint8_t*)"start\r\n",7,0xFFFF);
  HAL_UART_Transmit(&huart2,(uint8_t*)"AT+CSQ\r\n",8,0xFFFF);

  HAL_UART_Receive_IT(&huart2,str,1);

  HAL_Delay(2000);
  ready=true;

  timeRepeat = HAL_GetTick();
  int repeetTime = 0;
  HAL_UART_Receive_IT(&huart1,str2,1);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  if (ready) {
		  txATcommand();
		  if ((HAL_GetTick()-timeRepeat)>(2*60*1000)) {
			  step = 0;
			  timeRepeat = HAL_GetTick();
			  txATcommand();
		  }
		  else repeetTime++;
	  }
	  if (strstr(modemString2, "\r\n")) {
		if (!strstr(modemString2, "OK")) {
			SSD1306_Fill(SSD1306_COLOR_BLACK);
			SSD1306_GotoXY(0, 10);
			SSD1306_Puts(oldModem3, &Font_7x10, SSD1306_COLOR_WHITE);
			SSD1306_GotoXY(0, 25);
			SSD1306_Puts(oldModem2, &Font_7x10, SSD1306_COLOR_WHITE);
			SSD1306_GotoXY(0, 40);
			SSD1306_Puts(oldModem, &Font_7x10, SSD1306_COLOR_WHITE);
			SSD1306_UpdateScreen();
		}
		if (rxNew) {
			if (strstr(oldModem, "+CMTI")) {
				sprintf(unreedSms, "%s", oldModem);
			}
			else if ((step==1))
				rxATcommand(oldModem3);
			else rxATcommand(oldModem);
			rxNew = false;
		}
	  }
	 if (step>=12) HAL_Delay(1000);
	 if ((unreedSms[0]!=0)&&(step==12)){
		 rxATcommand(unreedSms);
		 memset(unreedSms, 0, 15);
	 }
	 else HAL_Delay(250);

	  //HAL_UART_Transmit(&huart1,modemString,10,0xFFFF);
	  //HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
  /* DMA1_Channel5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);
  /* DMA1_Channel6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);
  /* DMA1_Channel7_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel7_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
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
  __disable_irq();
  while (1)
  {
  }
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
