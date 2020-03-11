#include "main.h"
#include "gui.h"
#include "stm32f7xx_hal.h"
#include "stdio.h"
#include "mechanics.h"
#include "string.h"

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
	
		HAL_GPIO_WritePin(PDOWN_PORT,PDOWN_Pin,GPIO_PIN_RESET);
	
	osDelay(50);
	while((k_pup_state == 0) && (counter < MaxCount))
	{
	
		HAL_GPIO_WritePin(PUP_PORT,PUP_Pin,GPIO_PIN_SET);
	
		osDelay(100);
		k_pup_state = HAL_GPIO_ReadPin(K_PUP_PORT,K_PUP_Pin);
		counter++;
	}
	
		HAL_GPIO_WritePin(PUP_PORT,PUP_Pin,GPIO_PIN_RESET);

	if(counter < MaxCount) return NO_ERR;
	else return OVERTIME_PRESSUP;
}

uint8_t driveCarriageToPos(CarriagePos destination)
{
	uint8_t uart_str[30];
	switch(destination)
	{
		case START_POS:
		{
			sprintf((char*)uart_str,"carriage -> start\n");
			HAL_UART_Transmit(&huart1,uart_str,strlen((char*)uart_str),100);
			
			break;
		}
		case GLASS_POS:
		{
			sprintf((char*)uart_str,"carriage -> glass\n");
			HAL_UART_Transmit(&huart1,uart_str,strlen((char*)uart_str),100);
			
			break;
		}
		case METAL_POS:
		{
			sprintf((char*)uart_str,"carriage -> metal\n");
			HAL_UART_Transmit(&huart1,uart_str,strlen((char*)uart_str),100);
			
			break;
		}
		case PLASTIC_POS:
		{
			sprintf((char*)uart_str,"carriage -> plastic\n");
			HAL_UART_Transmit(&huart1,uart_str,strlen((char*)uart_str),100);
			
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
			sprintf((char*)uart_str,"press -> up\n\r");
			HAL_UART_Transmit(&huart1,uart_str,strlen((char*)uart_str),100);
			Error s = PressUp();
			sprintf((char*)uart_str,"pressup result: %d \n\r", s);
			HAL_UART_Transmit(&huart1,uart_str,strlen((char*)uart_str),100);
			return s;
			break;
		}
		case DOWN:
		{
			sprintf((char*)uart_str,"press -> down \n\r");
			HAL_UART_Transmit(&huart1,uart_str,strlen((char*)uart_str),100);
			
			//main pressing
			while((adc_val < crit_adc) && (counter < MaxCount) && (k_pdn_state == 0))
			{
				
				HAL_GPIO_WritePin(PDOWN_PORT,PDOWN_Pin,GPIO_PIN_SET);
				
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
				counter = 0;

				HAL_GPIO_WritePin(PDOWN_PORT,PDOWN_Pin,GPIO_PIN_RESET);

				while((adc_val < crit_adc) && (counter < MaxCount))
				{
			
					HAL_GPIO_WritePin(PDOWN_PORT,PDOWN_Pin,GPIO_PIN_SET);
					
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
					sprintf((char*)uart_str,"afterpr_timeout err\n\r");
					HAL_UART_Transmit(&huart1,uart_str,strlen((char*)uart_str),100);
					return OVERTIME_AFTERPRESSING;
				}
			}
			else if((adc_val > crit_adc) && (k_pdn_state == 0))
			{
				//overcurrent error

				HAL_GPIO_WritePin(PDOWN_PORT,PDOWN_Pin,GPIO_PIN_RESET);
		
				sprintf((char*)uart_str,"overcurrent press down err\n\r");
				HAL_UART_Transmit(&huart1,uart_str,strlen((char*)uart_str),100);
				return OVERCURENT_PRESSING;
			}
			else if(counter >= MaxCount)
			{
				
				HAL_GPIO_WritePin(PDOWN_PORT,PDOWN_Pin,GPIO_PIN_RESET);
			
				sprintf((char*)uart_str,"overtime press down err\n\r");
				HAL_UART_Transmit(&huart1,uart_str,strlen((char*)uart_str),100);
				return OVERTIME_PRESSING;
			}
			break;
		}
	}
}
