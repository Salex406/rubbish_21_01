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
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
struct Screen
{
	char  name[15];
	char  filename[15];
	uint16_t width;
	uint16_t height;
	uint32_t* location;
};

extern DMA2D_HandleTypeDef hdma2d;
extern LTDC_HandleTypeDef hltdc;
extern struct Screen screens[16];

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
void StartDrawTask(void const * argument);
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
			DMA2D_DrawImage((uint32_t)screens[16].location, 126, 70, 540, 400);
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

void StartDrawTask(void const * argument)
{
  /* Infinite loop */
  for(;;)
  {
		uint8_t msg = 15;
		if (xQueueReceive(gui_msg_q, &msg, 0) == pdTRUE)
		{
			if(msg==0)
			{
				//main screen
				LCD_DrawBitmap(0,0,(uint8_t *)screens[msg].location);
				DMA2D_DrawImage((uint32_t)screens[1].location, 670, 342, 120, 120);
				DMA2D_DrawImage((uint32_t)screens[2].location, 13, 342, 120, 120);
				DMA2D_DrawImage((uint32_t)screens[3].location, 670, 13, 120, 120);
				DMA2D_DrawImage((uint32_t)screens[16].location, 126, 70, 540, 400);
				osDelay(10);
				PrintFullness(1,0);
				osDelay(5);
				PrintFullness(2,0);
				osDelay(5);
				PrintFullness(3,0);
				osDelay(5);
				DrawPercents();
				
				/*DrawNumOnContainer(9, 1);
				DrawNumOnContainer(8, 2);
				DrawNumOnContainer(7, 3);
				DrawNumOnContainer(6, 4);
				DrawNumOnContainer(5, 5);
				DrawNumOnContainer(4, 6);*/
			}
			
			//DMA2D_DrawImage(0xC0700000, 50, 50, 120, 120);
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
			//Touchscreen_Handle_NewTouch();
		}
    osDelay(10);
  }
}
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
