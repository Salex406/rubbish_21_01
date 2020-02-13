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
    FIRST_BIN,      //4
		SECOND_BIN,//5
		THIRD_BIN//6
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
		if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0) == GPIO_PIN_SET)
		{
			//Touchscreen_Handle_NewTouch();
			//LCD_DrawBitmap(0,0,(uint8_t *)screens[0].location);
			DMA2D_DrawImage((uint32_t)images[16].location, 126, 70, 540, 400);
			osDelay(10);
			placePrBar(166,446,168,98,i,PROGRESSBAR_VERTICAL,0xFF00b800);
			placePrBar(350,446,168,98,j,PROGRESSBAR_VERTICAL,LCD_COLOR_YELLOW);
			placePrBar(536,446,168,98,k,PROGRESSBAR_VERTICAL,LCD_COLOR_BLUE);
			osDelay(10);
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
			if(msg==MAIN)
			{
				//main screen
				LCD_DrawBitmap(0,0,(uint8_t *)images[0].location);
				DMA2D_DrawImage((uint32_t)images[1].location, RightButtonX, BottomButtonY, 120, 120);
				DMA2D_DrawImage((uint32_t)images[2].location, LeftButtonX, BottomButtonY, 120, 120);
				DMA2D_DrawImage((uint32_t)images[3].location, RightButtonX, TopButtonY, 120, 120);
				DMA2D_DrawImage((uint32_t)images[16].location, 126, 70, 540, 400);
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
			sprintf((char*)lcd_string, "ABCDEFGHIJKLMNOP");
			BSP_LCD_DisplayStringAt(0, 10, lcd_string, LEFT_MODE,0);
			sprintf((char*)lcd_string, "QRSTUVWXYZ[\]^_`a");
			BSP_LCD_DisplayStringAt(0, 45, lcd_string, LEFT_MODE,0);
			}
			
			if(msg==SETTINGS)
			{
				//settings screen
				LCD_DrawBitmap(0,0,(uint8_t *)images[0].location);
				DMA2D_DrawImage((uint32_t)images[4].location, LeftButtonX, TopButtonY, 120, 120);
				
				uint8_t lcd_string[40] = "";
				sprintf((char*)lcd_string,"pASTRPKLJ");
				BSP_LCD_SetTextColor(LCD_COLOR_DARKGREEN);
				BSP_LCD_SetFont(&rus48);
				BSP_LCD_DisplayStringAt(0, 10, lcd_string, CENTER_MODE, 0);
				osDelay(10);
				CurrentScreen = SETTINGS;
			}
			
		}
    osDelay(400);
  }
}

void StartTouchTask(void const * argument)
{
  /* Infinite loop */
	osEvent event;
  for(;;)
  {
		event = osMessageGet(TOUCH_Queue, 100);
  	if (event.status == osEventMessage)
		{
			//HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_13,GPIO_PIN_SET);
			uint16_t x = -1, y = -1;
			get_touch_pos(&x, &y);
			print_touch_pos(x, y);
			uint8_t lcd_string[60] = "";
			BSP_LCD_SetFont(&Font20);
			if(x > 0 && x < 115 && y > 0 && y < 115)
			{
				//left top button
				sprintf((char*)lcd_string, "left top");
				BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 45, lcd_string, LEFT_MODE, 1);
				Screen screenToLoad;
				switch(CurrentScreen){
					case SETTINGS: 
						screenToLoad = MAIN;
						xQueueSend(gui_msg_q, &screenToLoad, 0);
						break;
					default:
						break;
			}
			}
			else if(x > 700 && y < 115)
			{
				//right top button
				sprintf((char*)lcd_string, "right top");
				BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 45, lcd_string, LEFT_MODE, 1);
			}
			else if(x < 115 && y > 320)
			{
				//left bottom button
				sprintf((char*)lcd_string, "left bottom");
				BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 45, lcd_string, LEFT_MODE, 1);
				Screen screenToLoad;
				switch(CurrentScreen){
					case MAIN: 
						screenToLoad = SETTINGS;
						xQueueSend(gui_msg_q, &screenToLoad, 0);
						break;
					default:
						break;
			}
			}
			else if(x > 700 && y > 320)
			{
				//right bottom button
				sprintf((char*)lcd_string, "right bottom");
				BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 45, lcd_string, LEFT_MODE, 1);
			
		}
	}
    osDelay(50);
  }
}
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
