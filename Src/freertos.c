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
#include "locale.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
struct Image
{
	char  name[15];
	char  filename[15];
	uint16_t width;
	uint16_t height;
	uint32_t* location;
};

typedef enum {
    MAIN,        //0
    SETTINGS,        //1
    CALL,    //2
    HELP,      //3
    DOOR_OPEN
}Screen;

extern const uint16_t RightButtonX;
extern const uint16_t LeftButtonX;
extern const uint16_t TopButtonY;
extern const uint16_t BottomButtonY;

extern DMA2D_HandleTypeDef hdma2d;
extern LTDC_HandleTypeDef hltdc;
extern struct Image images[17];
extern Screen CurrentScreen;

extern xQueueHandle gui_msg_q;
extern osMessageQId TOUCH_Queue;
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

void vApplicationStackOverflowHook( TaskHandle_t xTask,
                                    signed char *pcTaskName ){
																			
		while(1){}																	
}


void StartBlinkTask(void const * argument)
{
  for(;;)
  {

		HAL_GPIO_TogglePin(GPIOJ,GPIO_PIN_5);
    osDelay(800);
  }
}



void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
	uint8_t i=0, j=0, k=0;
  for(;;)
  {
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
		osDelay(1000);
	}
}

void ScreensDrawer(void const * argument)
{
  /* Infinite loop */
  for(;;)
  {
		uint8_t msg = 15;
		if (xQueueReceive(gui_msg_q, &msg, 0) == pdTRUE)
		{
			EXTI->IMR=(0<<EXTI_EMR_MR13_Pos)|(1<<EXTI_EMR_MR0_Pos); //disable interrupts
			if(msg==MAIN)
			{
				//main screen
				LCD_DrawBitmap(0,0,(uint8_t *)images[0].location);
				DMA2D_DrawImage((uint32_t)images[1].location, RightButtonX, BottomButtonY, 120, 120);
				DMA2D_DrawImage((uint32_t)images[2].location, LeftButtonX, BottomButtonY, 120, 120);
				//DMA2D_DrawImage((uint32_t)images[3].location, RightButtonX, TopButtonY, 120, 120);
				DMA2D_DrawImage((uint32_t)images[16].location, 126, 70, 540, 400);
				
				placePrBar(166,446,168,98,0,PROGRESSBAR_VERTICAL,LCD_COLOR_GREENCONTAINER);
				placePrBar(350,446,168,98,0,PROGRESSBAR_VERTICAL,LCD_COLOR_YELLOW);
				placePrBar(536,446,168,98,0,PROGRESSBAR_VERTICAL,LCD_COLOR_BLUE);
				
				osDelay(10);
				PrintFullness(1,0);
				osDelay(5);
				PrintFullness(2,0);
				osDelay(5);
				PrintFullness(3,0);
				osDelay(5);
				DrawPercents();
				CurrentScreen = MAIN;
				
			uint8_t lcd_string[60] = "";
			BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
      //BSP_LCD_SetFont(&Font12);
			BSP_LCD_SetFont(&rus48);
			//setlocale(LC_ALL, "RU");
			//sprintf((char*)lcd_string, "абвгдеёжзийклмнопрстуфхцчшщъыьэюя");
			BSP_LCD_SetTextColor(LCD_COLOR_DARKGREEN);
			sprintf((char*)lcd_string, "rMASTJL");
			BSP_LCD_DisplayStringAt(130, 15, lcd_string, LEFT_MODE,0);
			
			sprintf((char*)lcd_string, "tTFLMP");
			BSP_LCD_DisplayStringAt(330, 15, lcd_string, LEFT_MODE,0);
			
			sprintf((char*)lcd_string, "oFTAMM");
			BSP_LCD_DisplayStringAt(503, 15, lcd_string, LEFT_MODE,0);
			//sprintf((char*)lcd_string, "QRSTUVWXYZ[\]^_`a");
			//BSP_LCD_DisplayStringAt(0, 45, lcd_string, LEFT_MODE,0);
			}
			
			else if(msg==SETTINGS)
			{
				//settings screen
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
				
				osDelay(10);
				CurrentScreen = SETTINGS;
				
				//BSP_LCD_DrawCircle(750,10,40);
			}
			
			else if(msg==CALL)
			{
				//call screen
				LCD_DrawBitmap(0,0,(uint8_t *)images[0].location);
				DMA2D_DrawImage((uint32_t)images[4].location, LeftButtonX, TopButtonY, 120, 120);
				
				uint8_t lcd_string[10] = "";
				sprintf((char*)lcd_string,"rPEEFRHLA");
				BSP_LCD_SetTextColor(LCD_COLOR_DARKGREEN);
				BSP_LCD_SetFont(&rus48);
				BSP_LCD_DisplayStringAt(0, 10, lcd_string, CENTER_MODE, 0);
				osDelay(10);
				CurrentScreen = CALL;
			}
			else if(msg==HELP)
			{
				//help screen
				LCD_DrawBitmap(0,0,(uint8_t *)images[0].location);
				DMA2D_DrawImage((uint32_t)images[4].location, LeftButtonX, TopButtonY, 120, 120);
				
				uint8_t lcd_string[10] = "";
				sprintf((char*)lcd_string,"rPNP[^");
				BSP_LCD_SetTextColor(LCD_COLOR_DARKGREEN);
				BSP_LCD_SetFont(&rus48);
				BSP_LCD_DisplayStringAt(0, 10, lcd_string, CENTER_MODE, 0);
				osDelay(10);
				CurrentScreen = HELP;
			}
			else if(msg==DOOR_OPEN)
			{
				//help screen
				LCD_DrawBitmap(0,0,(uint8_t *)images[0].location);
				DMA2D_DrawImage((uint32_t)images[4].location, LeftButtonX, TopButtonY, 120, 120);
				
				uint8_t lcd_string[10] = "";
				sprintf((char*)lcd_string,"qTLR]TA ECFRXA");
				BSP_LCD_SetTextColor(LCD_COLOR_DARKGREEN);
				BSP_LCD_SetFont(&rus48);
				BSP_LCD_DisplayStringAt(0, 50, lcd_string, CENTER_MODE, 0);
				
				sprintf((char*)lcd_string,"{TRJWLPE:");
				BSP_LCD_DisplayStringAt(0, 96, lcd_string, CENTER_MODE, 0);
				osDelay(10);
				CurrentScreen = DOOR_OPEN;
			}
			
		}
    osDelay(100);
		EXTI->IMR=(1<<EXTI_EMR_MR13_Pos)|(1<<EXTI_EMR_MR0_Pos);
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
			EXTI->IMR=(0<<EXTI_EMR_MR13_Pos)|(1<<EXTI_EMR_MR0_Pos);
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
						break;
					default:
						break;
				}
			EXTI->IMR=(1<<EXTI_EMR_MR13_Pos)|(1<<EXTI_EMR_MR0_Pos);
	}
  osDelay(40);
  }
}
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
