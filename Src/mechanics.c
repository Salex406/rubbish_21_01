#include "main.h"
#include "gui.h"
#include "stm32f7xx_hal.h"
#include "stdio.h"
#include "mechanics.h"
#include "string.h"
#include "stm32f769i_discovery_lcd.h"

extern xQueueHandle gui_msg_q;
extern xQueueHandle err_msg_q;

void doorBlock(uint8_t state)
{
	if(state == 0) HAL_GPIO_WritePin(DOOR_PORT,DOOR_Pin,GPIO_PIN_RESET);
	else HAL_GPIO_WritePin(DOOR_PORT,DOOR_Pin,GPIO_PIN_SET);
}

void rotationCounter(FunctionalState state)
{
	if(state == ENABLE)
	{
		HAL_NVIC_ClearPendingIRQ(EXTI4_IRQn);
		HAL_NVIC_EnableIRQ(EXTI4_IRQn);
	}		
	else 
	{
		HAL_NVIC_DisableIRQ(EXTI4_IRQn);
	}
}

Error PressUp()
{
	uint16_t counter = 0;
	uint8_t k_pup_state = 0;
	uint16_t MaxCount = PresTimeout * 10;
	
	HAL_GPIO_WritePin(PDOWN_PORT,PDOWN_Pin,GPIO_PIN_SET);
	
	if(DebugMessages == 1)
	{
		uint8_t uart_str[20];
		sprintf((char*)uart_str,"press -> up\n\r");
		BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
		BSP_LCD_SetFont(&Font20);
		BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 15,(uint8_t*)uart_str, RIGHT_MODE, 1);
		HAL_UART_Transmit(&huart1,uart_str,strlen((char*)uart_str),100);
	}
	
	osDelay(50);
	while((k_pup_state == 0) && (counter < MaxCount))
	{
	
		HAL_GPIO_WritePin(PUP_PORT,PUP_Pin,GPIO_PIN_RESET);
	
		osDelay(100);
		k_pup_state = HAL_GPIO_ReadPin(K_PUP_PORT,K_PUP_Pin);
		counter++;
	}
	
		HAL_GPIO_WritePin(PUP_PORT,PUP_Pin,GPIO_PIN_SET);

	if(counter < MaxCount) return NO_ERR;
	else return OVERTIME_PRESSUP;
}

uint8_t driveCarriageToPos(CarriagePos destination)
{
	uint8_t uart_str[20];
	switch(destination)
	{
		case START_POS:
		{
			if(DebugMessages == 1)
			{
				sprintf((char*)uart_str,"carriage -> start\n");
				BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
				BSP_LCD_SetFont(&Font20);
				BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 15, (uint8_t*)uart_str, RIGHT_MODE, 1);
				HAL_UART_Transmit(&huart1,uart_str,strlen((char*)uart_str),100);
			}	
			
			break;
		}
		case GLASS_POS:
		{
			if(DebugMessages == 1)
			{
				sprintf((char*)uart_str,"carriage -> glass\n");
				BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
				BSP_LCD_SetFont(&Font20);
				BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 15, (uint8_t*)uart_str, RIGHT_MODE, 1);
				HAL_UART_Transmit(&huart1,uart_str,strlen((char*)uart_str),100);
			}	
		
			
			break;
		}
		case METAL_POS:
		{
			if(DebugMessages == 1)
			{
				sprintf((char*)uart_str,"carriage -> metal\n");
				BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
				BSP_LCD_SetFont(&Font20);
				BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 15, (uint8_t*)uart_str, RIGHT_MODE, 1);
				HAL_UART_Transmit(&huart1,uart_str,strlen((char*)uart_str),100);
			}	
			
			break;
		}
		case PLASTIC_POS:
		{
			if(DebugMessages == 1)
			{
				sprintf((char*)uart_str,"carriage -> plastic\n");
				BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
				BSP_LCD_SetFont(&Font20);
				BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 15, (uint8_t*)uart_str, RIGHT_MODE, 1);
				HAL_UART_Transmit(&huart1,uart_str,strlen((char*)uart_str),100);
			}	
			
			break;
		}
	}
	return 0;
}

Error drivePress(PressPos destination)
{
	uint8_t uart_str[80];
	float sensor_s = 0.125; // V/A 20A
	float crit_c = (uint16_t)(sensor_s*CriticalCurrentForPres + 2.5);
	uint16_t crit_adc = crit_c*4095/5;
	uint16_t MaxCount = PresTimeout * 10;
	uint16_t counter = 0;
	uint16_t adc_val = 0;
	uint8_t k_pdn_state = 0;

	switch(destination)
	{
		case UP:
		{
			Error s = PressUp();
			return s;
			break;
		}
		case DOWN:
		{
			HAL_GPIO_WritePin(PDOWN_PORT,PDOWN_Pin,GPIO_PIN_SET);
			HAL_GPIO_WritePin(PUP_PORT,PUP_Pin,GPIO_PIN_SET);
			
			if(DebugMessages == 1)
			{
				sprintf((char*)uart_str,"press -> down \n\r");
				BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
				BSP_LCD_SetFont(&Font20);
				BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 15, (uint8_t*)uart_str, RIGHT_MODE, 1);
				HAL_UART_Transmit(&huart1,uart_str,strlen((char*)uart_str),100);
			}
			
			//main pressing
			while((adc_val < crit_adc) && (counter < MaxCount) && (k_pdn_state == 0))
			{
				
				HAL_GPIO_WritePin(PDOWN_PORT,PDOWN_Pin,GPIO_PIN_RESET);
				
				HAL_ADC_Start(&hadc1);
				HAL_ADC_PollForConversion(&hadc1,100);

				placePrBar(20,400,760,50,counter*100/MaxCount,PROGRESSBAR_HORIZONTAL,LCD_COLOR_ORANGEBUTTON);
				osDelay(100);
				sprintf((char*)uart_str," pressing.ADC: %d,cnt: %d,pin %d \n\r",adc_val,counter,k_pdn_state);
				HAL_UART_Transmit(&huart1,uart_str,strlen((char*)uart_str),100);
				sprintf((char*)uart_str,"\n");
				HAL_UART_Transmit(&huart1,uart_str,strlen((char*)uart_str),100);
				adc_val = HAL_ADC_GetValue(&hadc1);
				k_pdn_state = HAL_GPIO_ReadPin(K_PDN_PORT,K_PDN_Pin);
				counter++;
			}
			
			if(k_pdn_state == 1)
			{
				//success, afterpressing
				if(DebugMessages == 1)
				{
					BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
					BSP_LCD_SetFont(&Font20);
					BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 15,(uint8_t*)"success,afterpr", RIGHT_MODE, 1);
				}
				counter = 0;

				HAL_GPIO_WritePin(PDOWN_PORT,PDOWN_Pin,GPIO_PIN_SET);

				while((adc_val < crit_adc) && (counter < MaxCount))
				{
			
					HAL_GPIO_WritePin(PDOWN_PORT,PDOWN_Pin,GPIO_PIN_RESET);
					
					HAL_ADC_Start(&hadc1);
					HAL_ADC_PollForConversion(&hadc1,100);
					HAL_Delay(100);
					adc_val = HAL_ADC_GetValue(&hadc1);
					sprintf((char*)uart_str," afterpressing.ADC: %d,cnt: %d \n\r",adc_val,counter);
					HAL_UART_Transmit(&huart1,uart_str,strlen((char*)uart_str),100);
					sprintf((char*)uart_str,"\n");
					HAL_UART_Transmit(&huart1,uart_str,strlen((char*)uart_str),100);
					counter++;
				}
				HAL_GPIO_WritePin(PDOWN_PORT,PDOWN_Pin,GPIO_PIN_SET);
				if(adc_val > crit_adc)
				{
					//success, finishing
					sprintf((char*)uart_str," afterpr_success \n\r ");
					HAL_UART_Transmit(&huart1,uart_str,strlen((char*)uart_str),100);
					sprintf((char*)uart_str,"press->up\n\r");
					HAL_UART_Transmit(&huart1,uart_str,strlen((char*)uart_str),100);
					
					Error s = PressUp();
					
					sprintf((char*)uart_str," pressup_state: %d\n\r ",s);
					HAL_UART_Transmit(&huart1,uart_str,strlen((char*)uart_str),100);
					return s;
				}
				else if(counter >= MaxCount)
				{
					Error s = PressUp();
					sprintf((char*)uart_str,"afterpr_timeout err\n\r");
					HAL_UART_Transmit(&huart1,uart_str,strlen((char*)uart_str),100);
					return OVERTIME_AFTERPRESSING;
				}
			}
			else if((adc_val > crit_adc) && (k_pdn_state == 0))
			{
				//overcurrent error

				HAL_GPIO_WritePin(PDOWN_PORT,PDOWN_Pin,GPIO_PIN_SET);
		
				sprintf((char*)uart_str,"overcurrent press down err\n\r");
				HAL_UART_Transmit(&huart1,uart_str,strlen((char*)uart_str),100);
				Error s = PressUp();
				return OVERCURENT_PRESSING;
			}
			else if(counter >= MaxCount)
			{
				
				HAL_GPIO_WritePin(PDOWN_PORT,PDOWN_Pin,GPIO_PIN_SET);
			
				sprintf((char*)uart_str,"overtime press down err\n\r");
				HAL_UART_Transmit(&huart1,uart_str,strlen((char*)uart_str),100);
				Error s = PressUp();
				return OVERTIME_PRESSING;
			}
			break;
		}
	}
}

void MX_GPIO_Init_custom(void)
{
GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOJ_CLK_ENABLE();
  __HAL_RCC_GPIOI_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOJ, GPIO_PIN_13|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_1, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_7, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOJ, GPIO_PIN_3, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOH, GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pins : PJ13 PJ4 PJ5 PJ1 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOJ, &GPIO_InitStruct);

  /*Configure GPIO pins : PA12 PA11 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PI13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

  /*Configure GPIO pin : PI15 */
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

  /*Configure GPIO pin : PC8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PF7 */
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : PF6 PF9 PF8 */
  GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	/*Configure GPIO pin : PF6 */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pin : PA4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PJ3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOJ, &GPIO_InitStruct);

  /*Configure GPIO pin : PH7 */
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /*Configure GPIO pin : PH6 */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /*Configure GPIO pin : PJ0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOJ, &GPIO_InitStruct);

  /*Configure GPIO pins : PB14 PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI4_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}