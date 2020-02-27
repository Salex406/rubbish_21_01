/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */     
#include "cmsis_os.h"
#include "gui.h"
#include "stm32f769i_discovery_lcd.h"
#include "stdio.h"
#include "mechanics.h"
#include "ScanerCodes_GM66.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

typedef enum {
    SCAN_CODE,
    
}Task;

extern const uint16_t RightButtonX;
extern const uint16_t LeftButtonX;
extern const uint16_t TopButtonY;
extern const uint16_t BottomButtonY;
extern const uint16_t ContainerFlapsY;
extern const uint16_t ContainerYellowFlapX;
extern const uint16_t ContainerGreenFlapX;
extern const uint16_t ContainerBlueFlapX;
extern const uint16_t ContainerBottomX;
extern const uint16_t ContainerBottomY;
extern const uint16_t NumOnContainerY;
extern const uint8_t DistanceBetweenStrings48;
	
extern DMA2D_HandleTypeDef hdma2d;
extern LTDC_HandleTypeDef hltdc;
extern ADC_HandleTypeDef hadc1;
extern UART_HandleTypeDef huart6;
extern UART_HandleTypeDef huart1;
extern struct Image images[20];
extern Screen CurrentScreen;

TaskHandle_t xScanTaskH;
TaskHandle_t xPressingTaskH;
extern xQueueHandle gui_msg_q;
extern osMessageQId TOUCH_Queue;

uint8_t Code[20] = {0};
uint8_t* test = Code;
uint8_t Command[20];
uint8_t Data_Recieve, Data_Analize, ScanCMD = 0; 
uint32_t count = 0;
HAL_StatusTypeDef Status;

uint8_t DoorScreenState=0;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void StartBlinkTask(void const * argument);
void ScreensDrawer(void const * argument);
void StartTouchTask(void const * argument);
void StartDefaultTask(void const * argument);
void StartPressingTask(void const * argument);
/* USER CODE END FunctionPrototypes */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];
  
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}                   
/* USER CODE END GET_IDLE_TASK_MEMORY */

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

void StartBlinkTask(void const * argument)
{
  for(;;)
  {

		HAL_GPIO_TogglePin(GPIOJ,GPIO_PIN_5);
    osDelay(800);
  }
}


void StartPressingTask(void const * argument)
{
	xPressingTaskH=xTaskGetCurrentTaskHandle();
	uint8_t uart_str[50];
  for(;;)
  {
		ulTaskNotifyTake( pdTRUE, portMAX_DELAY );
		HAL_NVIC_DisableIRQ(EXTI0_IRQn);
		HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
		
		//there is no carriage
		if(HAL_GPIO_ReadPin(K_K1_PORT,K_K1_Pin)==0)driveCarriageToPos(START);
		
		PressState s = drivePress(DOWN);
		switch (s)
		{
			case OK:
			{
				sprintf((char*)uart_str,"ptask ok\n\r");
				HAL_UART_Transmit(&huart1,uart_str,strlen((char*)uart_str),100);
				
				HAL_NVIC_ClearPendingIRQ(EXTI0_IRQn);
				HAL_NVIC_ClearPendingIRQ(EXTI15_10_IRQn);
				HAL_NVIC_EnableIRQ(EXTI0_IRQn);
				HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
				
				osDelay(50);
				Screen screenl = MAIN;
				xQueueSend(gui_msg_q, &screenl, 0);
				break;
			}
			case OVERCURRENT_ERR:
			{
				sprintf((char*)uart_str,"overcurrent ptask down err\n\r");
				HAL_UART_Transmit(&huart1,uart_str,strlen((char*)uart_str),100);
				
				HAL_NVIC_ClearPendingIRQ(EXTI0_IRQn);
				HAL_NVIC_ClearPendingIRQ(EXTI15_10_IRQn);
				HAL_NVIC_EnableIRQ(EXTI0_IRQn);
				HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
				
				osDelay(50);
				Screen screenl = HELP;
				xQueueSend(gui_msg_q, &screenl, 0);
				break;
			}
			case TIMEOUT_ERR:
			{
				sprintf((char*)uart_str,"timeout ptask err\n\r");
				HAL_UART_Transmit(&huart1,uart_str,strlen((char*)uart_str),100);
				
				HAL_NVIC_ClearPendingIRQ(EXTI0_IRQn);
				HAL_NVIC_ClearPendingIRQ(EXTI15_10_IRQn);
				HAL_NVIC_EnableIRQ(EXTI0_IRQn);
				HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
				
				osDelay(50);
				Screen screenl = HELP;
				xQueueSend(gui_msg_q, &screenl, 0);
				break;
			}
		}
  }
}

uint8_t UartData[100] = {0};
void StartScanTask(void const * argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
	//uint8_t i=0, j=0, k=0;
	InitBarcodeReader();
	ScanCMD=1;
	xScanTaskH=xTaskGetCurrentTaskHandle();
  for(;;)
  {
		ulTaskNotifyTake( pdTRUE, portMAX_DELAY );
		while(1)
		{
		if(ScanCMD != 0)
		{
			if(count >= 1000)
			{	
				ReadCode(Command);
				Status = HAL_UART_Transmit(&huart6, Command, 9, 21);
				count=0;
			}
			HAL_UART_Receive_IT(&huart6, &UartData[0], 100);
			if(count>=100)
			{	
				//Analyze Data			
				for(uint32_t i = 0; i < sizeof(UartData); i++ )
				{
					//Test Header
					if((UartData[i] == 0x02)&(UartData[i+1] == 0x0)&(UartData[i+2] == 0x0)&(UartData[i+3] == 0x01)&(UartData[i+4] == 0x0)&(UartData[i+5] == 0x33)&(UartData[i+6] == 0x31))
					{	
						//Test Header
						i+=7;
						uint32_t j = 0;
						//Scan OK?
						while((j<13)&(UartData[i]>=0x30)&(UartData[i]<=0x39))
						{
							Code[j] = UartData[i];
							j++;
							i++;
						}
						//if scan is OK, complete scan
						if((j==13)&(UartData[i] == 0x0D))
						{	
							ScanCMD =0;
							for(j = 0; j<sizeof(UartData); j++)
							{		
								UartData[j] = 0;
		
							}
							//Code[13] = (uint8_t)"\0";
							uint8_t res = processCode(Code);
							Screen screenToLoad;
							switch (res)
							{
								case 1: {
										//1 bin button
										DoorScreenState=1;
										screenToLoad = DOOR_OPEN;
										xQueueSend(gui_msg_q, &screenToLoad, 0);break;}
								case 2: {
										//1 bin button
										DoorScreenState=2;
										screenToLoad = DOOR_OPEN;
										xQueueSend(gui_msg_q, &screenToLoad, 0);break;}
								case 3: {
										//1 bin button
										DoorScreenState=3;
										screenToLoad = DOOR_OPEN;
										xQueueSend(gui_msg_q, &screenToLoad, 0);break;}
							}
						}	
					}
				}	
			}
			count++;
		}
		osDelay(1);
	}
		//HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0)
		/*
		if(CurrentScreen == 26)
		{
			//Touchscreen_Handle_NewTouch();
			//LCD_DrawBitmap(0,0,(uint8_t *)screens[0].location);
			DMA2D_DrawImage((uint32_t)images[16].location, 126, 70, 540, 400);
			osDelay(10);
			placePrBar(166,446,168,98,i,PROGRESSBAR_VERTICAL,0xFF00b800);
			placePrBar(350,446,168,98,j,PROGRESSBAR_VERTICAL,LCD_COLOR_YELLOW);
			placePrBar(536,446,168,98,k,PROGRESSBAR_VERTICAL,LCD_COLOR_BLUE);
			osDelay(5);
			DrawPercents();
			osDelay(5);
			PrintFullness(1,i);
			osDelay(5);
			PrintFullness(2,j);
			osDelay(5);
			PrintFullness(3,k);
			i++;
			j+=4;
			k+=11;
			if(i>100)i=0;
			if(j>100)j=0;
			if(k>100)k=0;
			osDelay(200);
		}
    osDelay(80);
		}*/
		
		//osDelay(1000);
	}
}


float u;
void ScreensDrawer(void const * argument)
{
  /* Infinite loop */
  for(;;)
  {
		uint8_t msg = 15;
		if (xQueueReceive(gui_msg_q, &msg, 0) == pdTRUE)
		{
			HAL_NVIC_DisableIRQ(EXTI0_IRQn);
			HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
			if(msg==MAIN)
			{
				//main screen
				CurrentScreen = MAIN;
				LCD_DrawBitmap(0,0,(uint8_t *)images[0].location);
				TFT_DrawBitmapToMem(0,0,(uint8_t *)images[0].location,(uint8_t *)LCD_FB_START_ADDRESS);
				DMA2D_DrawImage((uint32_t)images[1].location, RightButtonX, BottomButtonY, images[1].width, images[1].height);
				DMA2D_DrawImage((uint32_t)images[2].location, LeftButtonX, BottomButtonY, images[2].width, images[2].height);
				//DMA2D_DrawImage((uint32_t)images[3].location, RightButtonX, TopButtonY, 120, 120);
				DMA2D_DrawImage((uint32_t)images[16].location, ContainerBottomX, ContainerBottomY, images[16].width, images[16].height);
				osDelay(10);
				
				DMA2D_DrawImage((uint32_t)images[17].location, ContainerGreenFlapX, ContainerFlapsY, images[17].width, images[17].height);
				osDelay(5);
				DMA2D_DrawImage((uint32_t)images[18].location, ContainerYellowFlapX, ContainerFlapsY, images[18].width, images[18].height);
				osDelay(5);
				DMA2D_DrawImage((uint32_t)images[19].location, ContainerBlueFlapX, ContainerFlapsY, images[19].width, images[19].height);
				osDelay(5);
				
				placePrBar(166,420,168,98,0,PROGRESSBAR_VERTICAL,LCD_COLOR_GREENCONTAINER);
				placePrBar(350,420,168,98,0,PROGRESSBAR_VERTICAL,LCD_COLOR_YELLOW);
				placePrBar(536,420,168,98,0,PROGRESSBAR_VERTICAL,LCD_COLOR_BLUE);
				
				osDelay(10);
				PrintFullness(1,10);
				osDelay(5);
				PrintFullness(2,20);
				osDelay(5);
				PrintFullness(3,30);
				osDelay(5);
				DrawPercents();
				
			uint8_t lcd_string[60] = "";
			BSP_LCD_SetTextColor(LCD_COLOR_BLACK);

			BSP_LCD_SetFont(&rus48);
			BSP_LCD_SetTextColor(LCD_COLOR_DARKGREEN);
			sprintf((char*)lcd_string, "rMASTJL");
			BSP_LCD_DisplayStringAt(130, 434, lcd_string, LEFT_MODE,0);//15 first string
			
			sprintf((char*)lcd_string, "tTFLMP");
			BSP_LCD_DisplayStringAt(330, 434, lcd_string, LEFT_MODE,0);
			
			sprintf((char*)lcd_string, "oFTAMM");
			BSP_LCD_DisplayStringAt(503, 434, lcd_string, LEFT_MODE,0);
			
			DoorScreenState=0;
			}
			
			else if(msg==SETTINGS)
			{
				//settings screen
				CurrentScreen = SETTINGS;
				LCD_DrawBitmap(0,0,(uint8_t *)images[0].location);
				DMA2D_DrawImage((uint32_t)images[4].location, LeftButtonX, TopButtonY, 120, 120);
				
				uint8_t lcd_string[10] = "";
				sprintf((char*)lcd_string,"pASTRPKLJ");
				BSP_LCD_SetTextColor(LCD_COLOR_DARKGREEN);
				BSP_LCD_SetFont(&rus48);
				BSP_LCD_DisplayStringAt(0, 10, lcd_string, CENTER_MODE, 0);
				
				sprintf((char*)lcd_string, "ABCDEFGHIJKLMNOP");
				BSP_LCD_DisplayStringAt(200, 100, lcd_string, LEFT_MODE,0);
				sprintf((char*)lcd_string, "QRSTUVWXYZ[\]^_`a");
				BSP_LCD_DisplayStringAt(200, 135, lcd_string, LEFT_MODE,0);
				HAL_ADC_Start(&hadc1);
				HAL_ADC_PollForConversion(&hadc1,100);
				uint16_t res = HAL_ADC_GetValue(&hadc1);
				HAL_ADC_Stop(&hadc1); 
				uint8_t uart_str[20];
				sprintf((char*)uart_str,"adc: %d\n", res);
				HAL_UART_Transmit(&huart1,uart_str,strlen((char*)uart_str),100);
				//BSP_LCD_DrawCircle(750,10,40);
		
			}
			
			else if(msg==CALL)
			{
				//call screen
				CurrentScreen = CALL;
				LCD_DrawBitmap(0,0,(uint8_t *)images[0].location);
				DMA2D_DrawImage((uint32_t)images[4].location, LeftButtonX, TopButtonY, 120, 120);
				
				uint8_t lcd_string[10] = "";
				sprintf((char*)lcd_string,"rPEEFRHLA");
				BSP_LCD_SetTextColor(LCD_COLOR_DARKGREEN);
				BSP_LCD_SetFont(&rus48);
				BSP_LCD_DisplayStringAt(0, 10, lcd_string, CENTER_MODE, 0);
				osDelay(10);
			}
			else if(msg==HELP)
			{
				//help screen
				CurrentScreen = HELP;
				LCD_DrawBitmap(0,0,(uint8_t *)images[0].location);
				DMA2D_DrawImage((uint32_t)images[4].location, LeftButtonX, TopButtonY, 120, 120);
				
				uint8_t lcd_string[10] = "";
				sprintf((char*)lcd_string,"rPNP[^");
				BSP_LCD_SetTextColor(LCD_COLOR_DARKGREEN);
				BSP_LCD_SetFont(&rus48);
				BSP_LCD_DisplayStringAt(0, 10, lcd_string, CENTER_MODE, 0);
				osDelay(10);
			}
			else if(msg==DOOR_OPEN)
			{
				CurrentScreen = DOOR_OPEN;
				//help screen
				LCD_DrawBitmap(0,0,(uint8_t *)images[0].location);
				//DMA2D_DrawImage((uint32_t)images[4].location, LeftButtonX, TopButtonY, 120, 120);
				
				DMA2D_DrawImage((uint32_t)images[16].location, ContainerBottomX, ContainerBottomY, images[16].width, images[16].height);
				osDelay(10);
				
				uint8_t lcd_string[10] = "";
				sprintf((char*)lcd_string,"pASTRPKLJ");
				BSP_LCD_SetTextColor(LCD_COLOR_DARKGREEN);
				BSP_LCD_SetFont(&rus48);
				sprintf((char*)lcd_string, "rMASTJL");
				BSP_LCD_DisplayStringAt(130, 434, lcd_string, LEFT_MODE,0);//15 first string
			
				sprintf((char*)lcd_string, "tTFLMP");
				BSP_LCD_DisplayStringAt(330, 434, lcd_string, LEFT_MODE,0);
			
				sprintf((char*)lcd_string, "oFTAMM");
				BSP_LCD_DisplayStringAt(503, 434, lcd_string, LEFT_MODE,0);
				
				
				if(DoorScreenState==0)
				{
				//bin is not chosen(initial screen)
				BSP_LCD_SetTextColor(LCD_COLOR_RED);
				sprintf((char*)lcd_string, "vEAMJTF LR]ZLU S BUT]MLJ");
				BSP_LCD_DisplayStringAt(20, 8, lcd_string, CENTER_MODE,0);
				BSP_LCD_SetTextColor(LCD_COLOR_DARKGREEN);
				sprintf((char*)lcd_string, "tLAOJRUKTF ZTRJWLPE JMJ");
				BSP_LCD_DisplayStringAt(20, 8+DistanceBetweenStrings48, lcd_string, CENTER_MODE,0);
				sprintf((char*)lcd_string, "C]BFRJTF BAL CRUYOU`");
				BSP_LCD_DisplayStringAt(20, 8+DistanceBetweenStrings48*2, lcd_string, CENTER_MODE,0);
				ScanCMD = 1;
					
				xTaskNotifyGive(xScanTaskH);
				}
				else if(DoorScreenState==1)
				{
				//after click on 1 bin
				ScanCMD = 0;
				DMA2D_DrawImage((uint32_t)images[17].location, ContainerGreenFlapX, ContainerFlapsY, images[17].width, images[17].height);
				sprintf((char*)lcd_string, "rPNFSTJTF NUSPR C LANFRU");
				BSP_LCD_DisplayStringAt(20, 8, lcd_string, CENTER_MODE,0);
				sprintf((char*)lcd_string, "JMJ C]BFRJTF ERUDPK BAL");
				BSP_LCD_DisplayStringAt(20, 8+DistanceBetweenStrings48, lcd_string, CENTER_MODE,0);
				}
				else if(DoorScreenState==2)
				{
				ScanCMD = 0;
				DMA2D_DrawImage((uint32_t)images[18].location, ContainerYellowFlapX, ContainerFlapsY, images[18].width, images[18].height);
				sprintf((char*)lcd_string, "rPNFSTJTF NUSPR C LANFRU");
				BSP_LCD_DisplayStringAt(20, 8, lcd_string, CENTER_MODE,0);
				sprintf((char*)lcd_string, "JMJ C]BFRJTF ERUDPK BAL");
				BSP_LCD_DisplayStringAt(20, 8+DistanceBetweenStrings48, lcd_string, CENTER_MODE,0);
				}
				else if(DoorScreenState==3)
				{
				ScanCMD = 0;
				DMA2D_DrawImage((uint32_t)images[19].location, ContainerBlueFlapX, ContainerFlapsY, images[19].width, images[19].height);
				sprintf((char*)lcd_string, "rPNFSTJTF NUSPR C LANFRU");
				BSP_LCD_DisplayStringAt(20, 8, lcd_string, CENTER_MODE,0);
				sprintf((char*)lcd_string, "JMJ C]BFRJTF ERUDPK BAL");
				BSP_LCD_DisplayStringAt(20, 8+DistanceBetweenStrings48, lcd_string, CENTER_MODE,0);
				}
				
			}
			else if(msg==PRESSING)
			{
				CurrentScreen = PRESSING;
				LCD_DrawBitmap(0,0,(uint8_t *)images[0].location);
				//DMA2D_DrawImage((uint32_t)images[4].location, LeftButtonX, TopButtonY, 120, 120);
				BSP_LCD_SetTextColor(LCD_COLOR_DARKGREEN);
				uint8_t lcd_string[30] = "";
				BSP_LCD_SetFont(&rus48);
				sprintf((char*)lcd_string, "oUSPR QRFSSUFTSa,");
				BSP_LCD_DisplayStringAt(20, 60, lcd_string, LEFT_MODE,0);
				sprintf((char*)lcd_string, "QPEPHEJTF...");
				BSP_LCD_DisplayStringAt(20, 60+DistanceBetweenStrings48, lcd_string, LEFT_MODE,0);
				xTaskNotifyGive(xPressingTaskH);
			}
			HAL_NVIC_ClearPendingIRQ(EXTI0_IRQn);
			HAL_NVIC_ClearPendingIRQ(EXTI15_10_IRQn);
			HAL_NVIC_EnableIRQ(EXTI0_IRQn);
			HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
		}
    osDelay(100);
  }
}

void StartTouchTask(void const * argument)
{
  /* Infinite loop */
	osEvent event;
	uint8_t lcd_string[60] = "";
	Screen screenToLoad;
  for(;;)
  {
		event = osMessageGet(TOUCH_Queue, 100);
  	if (event.status == osEventMessage)
		{
			//HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_13,GPIO_PIN_SET);
			HAL_NVIC_DisableIRQ(EXTI0_IRQn);
			HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
			uint16_t x = 10000, y = 10000;
			get_touch_pos(&x, &y);
			print_touch_pos(x, y);
			switch(CurrentScreen){
					case SETTINGS: 
						//actions for SETTINGS SCREEN
						if(x > 0 && x < 115 && y > 0 && y < 115)
						{
							//left top button
							//sprintf((char*)lcd_string, "left top");
							//BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 45, lcd_string, LEFT_MODE, 1);
							screenToLoad = MAIN;
							xQueueSend(gui_msg_q, &screenToLoad, 0);
						}
						break;
					case MAIN:
						/*if(x > 700 && y < 115)
						{
							//right top button
							sprintf((char*)lcd_string, "right top");
							BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 45, lcd_string, LEFT_MODE, 1);
							screenToLoad = CALL;
							xQueueSend(gui_msg_q, &screenToLoad, 0);
						}*/
						if(x > 700 && y > 320 && x <= 800 && y <= 480)
						{
								//right bottom button
								sprintf((char*)lcd_string, "right bottom");
								BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 45, lcd_string, LEFT_MODE, 1);
								screenToLoad = HELP;
								xQueueSend(gui_msg_q, &screenToLoad, 0);
						}
						else if(x < 115 && y > 320 && y <=480)
						{
								//left bottom button
								sprintf((char*)lcd_string, "left bottom");
								BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 45, lcd_string, LEFT_MODE, 1);
								screenToLoad = SETTINGS;
								xQueueSend(gui_msg_q, &screenToLoad, 0);
						}
						break;
					
					case CALL:
						if(x > 0 && x < 115 && y > 0 && y < 115)
						{
							//left top button
							screenToLoad = MAIN;
							xQueueSend(gui_msg_q, &screenToLoad, 0);
						}
						break;
					
					case HELP:
						if(x > 0 && x < 115 && y > 0 && y < 115)
						{
							//left top button
							screenToLoad = MAIN;
							xQueueSend(gui_msg_q, &screenToLoad, 0);
						}
						break;
					case DOOR_OPEN:
						if(x > 0 && x < 115 && y > 0 && y < 115)
						{
							//left top button
							screenToLoad = MAIN;
							xQueueSend(gui_msg_q, &screenToLoad, 0);
						}
						else if(x > ContainerBottomX && x < 280 && y > ContainerBottomY && y < 480 && DoorScreenState!=1)
						{
							//1 bin button
							DoorScreenState=1;

							//vTaskSuspend(xScanTaskHandle);
							screenToLoad = DOOR_OPEN;
							xQueueSend(gui_msg_q, &screenToLoad, 0);
							
						}
						else if(x > 350 && x < 450 && y > ContainerBottomY && y < 480 && DoorScreenState!=2)
						{
							//2 bin button
							DoorScreenState=2;

							screenToLoad = DOOR_OPEN;
							xQueueSend(gui_msg_q, &screenToLoad, 0);
							
						}
						else if(x > 520 && x < 630 && y > ContainerBottomY && y < 480 && DoorScreenState!=3)
						{
							//3 bin button

							DoorScreenState=3;
							screenToLoad = DOOR_OPEN;
							xQueueSend(gui_msg_q, &screenToLoad, 0);
							
						}
						break;
						/*
					case PRESSING:
						if(x > 0 && x < 115 && y > 0 && y < 115)
						{
							screenToLoad = MAIN;
							xQueueSend(gui_msg_q, &screenToLoad, 0);
						}
						break;*/
					default:
						break;
				}
		HAL_NVIC_ClearPendingIRQ(EXTI0_IRQn);
		HAL_NVIC_ClearPendingIRQ(EXTI15_10_IRQn);
		HAL_NVIC_EnableIRQ(EXTI0_IRQn);
		HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
	}
  osDelay(40);
  }
}
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
