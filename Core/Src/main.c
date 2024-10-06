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
//#include "cJSON.h"
//#include "ssd1306.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
uint8_t str[3]={"0"};
uint8_t str2[3]={"0"};
int timeRepeat = 0;
int stopStep = 0;
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
uint8_t oldModem4[250];
uint8_t smsNum[6];
uint8_t unreedSms[15] = {0};
uint8_t smsText[] = "Hello";
uint8_t csqlvl[]="99";
uint8_t tnumber1[] = "77777777777";
uint8_t tnumber2[] = "77777777777";
uint8_t tnumber3[] = "77777777777";
uint8_t backNumber[] = "77777777777";
uint16_t *idBase0 = (uint16_t*)(UID_BASE);
uint16_t *idBase1 = (uint16_t*)(UID_BASE + 0x02);
uint32_t *idBase2 = (uint32_t*)(UID_BASE + 0x04);
uint32_t *idBase3 = (uint32_t*)(UID_BASE + 0x08);
uint8_t unicID[64] = {0,};
int step=0, rxNew = false;
bool ready=false;
bool answ = false, checkPhones = true, echoMode = true, errTransmit = false;
uint8_t bat[8];
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void s800LSend(uint8_t *text, int nums) {
	HAL_UART_Transmit(&huart2, text, nums, 0xFFFF);
	HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", 2, 0xFFFF);
	if (echoMode) {
		HAL_UART_Transmit(&huart1, text, nums, 0xFFFF);
		HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n", 2, 0xFFFF);
	}
	return;
}

int s800lMessAdd(uint8_t* text) {
	int i = 0;
	for (i; i<250; i++) {
		if (text[i]=='!') return i;
	}
}

void buttStart() {
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 0);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 0);
	HAL_Delay(3000);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 1);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 1);
}

void buttOpen() {
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 0);
	for (int i = 0; i<5; i++) {
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 0);
		HAL_Delay(100);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 1);
		HAL_Delay(100);
	}
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 1);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 1);
}

void buttClose() {
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, 0);
	for (int i = 0; i<3; i++) {
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 0);
		HAL_Delay(250);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 1);
		HAL_Delay(250);
	}
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 1);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 1);
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
			uint8_t ext[250];
			if (answ)
				sprintf(ext, "AT+HTTPPARA=\"URL\",\"http://simple.spamigor.ru/api/gst?csq=%s&bat=%s&mes=%s&id=%s\"", csqlvl, bat, smsText, unicID);
			else if (!checkPhones)
				sprintf(ext, "AT+HTTPPARA=\"URL\",\"http://simple.spamigor.ru/api/gst?csq=%s&bat=%s&id=%s\"", csqlvl, bat, unicID);
			else sprintf(ext, "AT+HTTPPARA=\"URL\",\"http://simple.spamigor.ru/api/phn?id=%s\"", unicID);
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
			uint8_t ext[23] = {0};
			sprintf(ext, "AT+CMGS=\"+%s\"", backNumber);
			s800LSend(ext, 22);
			break;
		}
		case 24: {
			ready = false;
			step=24;
			uint8_t ggg[20];
			sprintf(ggg, "%s%c", smsText, (uint8_t)0x1A);
			s800LSend(ggg, strlen(ggg));
			break;
		}
		case 99: {
			ready = true;
			s800LSend((uint8_t*)"AT+CFUN=1,1", 22);
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
			if (strstr((char*)oldModem3, (char*)"0,1")) step++;
			ready=true;
			break;
		}
		case 4: {
			if (strstr((char*)text, (char*)"OK")) step++;
			else if (strstr((char*)text, (char*)"ERROR")) {
				step=10;
				errTransmit = true;
			}
			ready=true;
			break;
		}
		case 5: {
			if (strstr((char*)text, (char*)"OK")) step++;
			else if (strstr((char*)text, (char*)"ERROR")) {
				step=10;
				errTransmit = true;
			}
			ready=true;
			break;
		}
		case 6: {
			if (strstr((char*)text, (char*)"OK")) step++;
			else step=10;
			ready=true;
			break;
		}
		case 7: {
			if (strstr((char*)text, (char*)"OK")) step++;
			else step=10;
			ready=true;
			break;
		}
		case 8: {
			ready=true;
			if (strstr((char*)text, (char*)"200")) step++;
			else if (strstr((char*)text, (char*)"0,60")) step=99;
			else if ((strstr((char*)text, (char*)"0,40"))||
					(strstr((char*)text, (char*)"0,50"))) {
				step=10;
				errTransmit = true;
			}
			else ready=false;
			break;
		}
		case 9: {
			if (!answ) {
				if (strstr((char*)text, (char*)"OK")) {
					if (checkPhones) {
						if (strstr((char*)oldModem2, (char*)"t1"))
							for (int i = 0; i<11; i++) tnumber1[i] = oldModem2[i+7];
						if (strstr((char*)oldModem2, (char*)"t2"))
							for (int i = 0; i<11; i++) tnumber2[i] = oldModem2[i+26];
						if (strstr((char*)oldModem2, (char*)"t3"))
							for (int i = 0; i<11; i++) tnumber3[i] = oldModem2[i+45];
						checkPhones = false;
					}
					else if (strstr((char*)oldModem2, "phones")) {
						checkPhones = true;
						answ = false;
					}
					else if (strstr((char*)oldModem2, "res")) {
						if (oldModem2[6]=='t') {
							memset(smsText, 0, strlen(smsText));
							buttStart();
							strcpy(smsText, (uint8_t*)"Start");
						}
						else if (oldModem2[17]=='t') {
							memset(smsText, 0, strlen(smsText));
							buttOpen();
							strcpy(smsText, (uint8_t*)"Open");
						}
						else if (oldModem2[28]=='t') {
							memset(smsText, 0, strlen(smsText));
							buttClose();
							strcpy(smsText, (uint8_t*)"Closed");
						}
						answ = true;
					}
					step=7;
				}
				else {
					step=10;
					answ = false;
					errTransmit = true;
				}
				ready=true;
			}
			else {
				if (strstr((char*)text, (char*)"OK")) step++;
				else {
					step=10;
					errTransmit = true;
				}
				answ = false;
				ready=true;
				memset(smsText, 0, strlen(smsText));
			}
			break;
		}
		case 10: {
			if (strstr((char*)text, (char*)"OK")) step=11;
			else if (strstr((char*)text, (char*)"ERROR")) step=11;
			ready=true;
			break;
		}
		case 11: {
			if (strstr((char*)text, (char*)"OK")) {
				step=12;
				if (errTransmit) step = 0;
			}
			else step=0;
			errTransmit = false;
			ready=true;
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
					else if (text[i]==',') start = true;
				}
				step=20;
			}
			else step=12;
			ready=true;
			break;
		}
		case 20: {
			if (strstr((char*)text, (char*)"OK")) step++;
			else step=12;
			ready=true;
			break;
		}
		case 21: {
			memset(smsText, 0, sizeof(smsText));
			strcpy(smsText, oldModem3);
			bool numberCorrect = false;
			if (strstr((char*)oldModem4, (char*)tnumber1)) {
				numberCorrect=true;
				sprintf(backNumber, "%s", tnumber1);
			}
			else if (strstr((char*)oldModem4, (char*)tnumber2)) {
				numberCorrect=true;
				sprintf(backNumber, "%s", tnumber2);
			}
			else if (strstr((char*)oldModem4, (char*)tnumber3)) {
				numberCorrect=true;
				sprintf(backNumber, "%s", tnumber3);
			}
			if (numberCorrect) {
				if (strstr(oldModem3, "tart")) {
					buttStart();
				}
				else if (strstr(oldModem3, "pen")) {
					buttOpen();
				}
				else if (strstr(oldModem3, "lose")) {
					buttClose();
				}
				step++;
			}
			else {
				step = 0;
				strncat(smsText, (uint8_t*)"-stranger", 9);
			}
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
				if (stopStep==12) step=0;
				else step=stopStep;
			}
			else if (strstr((char*)text, (char*)">")) {
				if (stopStep==12) step=0;
				else step=stopStep;
			}
			else {
				step=12;
			}
			ready=true;
			break;
		}
		case 99: {
			if (strstr((char*)text, (char*)"OK")) {
				step=0;
				ready=true;
			}
			else ready = true;
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
			if (echoMode) HAL_UART_Transmit_DMA(&huart1, modemString2, modemStringLength2);
			for (int i = modemStringLength2; i<250; i++)modemString2[i]=(uint8_t)0x00;
			for (int i = 0; i<250; i++) oldModem4[i] = oldModem3[i];
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
			if (strstr((char*)modemString1, (char*)"echoON")) {
				echoMode = true;
				HAL_UART_Transmit_DMA(&huart1, (uint8_t*)"echo mode ON\r\n", 14);
			}
			else if (strstr((char*)modemString1, (char*)"echoOFF")) {
				echoMode = false;
				HAL_UART_Transmit_DMA(&huart1, (uint8_t*)"echo mode OFF\r\n", 14);
			}
			else if (strstr((char*)modemString1, (char*)"gprs")) {
				timeRepeat = 0;
				HAL_UART_Transmit_DMA(&huart1, (uint8_t*)"update start\r\n", 14);
			}
			else if (echoMode) HAL_UART_Transmit_DMA(&huart2, modemString1, modemStringLength1);
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

  sprintf(unicID, "%x-%x-%lx-%lx", *idBase0, *idBase1, *idBase2, *idBase3);
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  /*SSD1306_Init();
  SSD1306_GotoXY(0, 44); //Устанавливаем курсор в позицию 0;44. Сначала по горизонтали, потом вертикали.
  SSD1306_Puts("Hello, habrahabr!!", &Font_7x10, SSD1306_COLOR_WHITE); //пишем надпись в выставленной позиции шрифтом "Font_7x10" белым цветом.
  SSD1306_DrawCircle(10, 33, 7, SSD1306_COLOR_WHITE);
  SSD1306_UpdateScreen();*/
  for (int i = 0; i<10; i++) {
	  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
	  HAL_Delay(250);
  }

  HAL_UART_Transmit(&huart1,(uint8_t*)"start\r\n",7,0xFFFF);
  HAL_UART_Transmit(&huart1,(uint8_t*)"echoON - for echo mode\r\n",24,0xFFFF);
  HAL_UART_Transmit(&huart1,(uint8_t*)"echoOFF - for normal mode\r\n",27,0xFFFF);
  HAL_UART_Transmit(&huart1,(uint8_t*)"gprs - gprs update\r\n",20,0xFFFF);
  HAL_UART_Transmit(&huart2,(uint8_t*)"AT\r\n",8,0xFFFF);

  HAL_UART_Receive_IT(&huart2,str,1);

  HAL_Delay(2000);
  ready=true;

  timeRepeat = HAL_GetTick();
  int lightTime = 0;
  HAL_UART_Receive_IT(&huart1,str2,1);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  if (ready||
			  ((HAL_GetTick()-timeRepeat)>(5*60*1000))||
			  (((HAL_GetTick()-timeRepeat)>(10*1000))&&(step==0))) {
		  txATcommand();
		  if ((((HAL_GetTick()-timeRepeat)>(2*60*1000))&&(step==12))||
				  ((HAL_GetTick()-timeRepeat)>(5*60*1000))||
				  (((HAL_GetTick()-timeRepeat)>(10*1000))&&(step==0))){
			  step = 0;
			  timeRepeat = HAL_GetTick();
			  txATcommand();
		  }
		  else if (step!=12) timeRepeat = HAL_GetTick();
		  if ((HAL_GetTick()-lightTime)>(30*1000)) {
			  lightTime = HAL_GetTick();
			  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
			  HAL_Delay(100);
			  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
		  }
	  }
	  if (strstr(modemString2, "\r\n")) {
		/*if (!strstr(modemString2, "OK")) {
			SSD1306_Fill(SSD1306_COLOR_BLACK);
			SSD1306_GotoXY(0, 10);
			SSD1306_Puts(oldModem3, &Font_7x10, SSD1306_COLOR_WHITE);
			SSD1306_GotoXY(0, 25);
			SSD1306_Puts(oldModem2, &Font_7x10, SSD1306_COLOR_WHITE);
			SSD1306_GotoXY(0, 40);
			SSD1306_Puts(oldModem, &Font_7x10, SSD1306_COLOR_WHITE);
			SSD1306_UpdateScreen();
		}*/
		if (rxNew) {
			if (strstr(oldModem, "+CMTI")) {
				stopStep = step;
				step = 12;
				rxATcommand(oldModem);
			}
			else if ((step==1))
				rxATcommand(oldModem3);
			else rxATcommand(oldModem);
			rxNew = false;
		}
	  }
	 if (step==12) HAL_Delay(1000);
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

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12|GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PB12 PB13 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

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
