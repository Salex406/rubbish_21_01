#include <esp.h>
#include <string.h>
#include <main.h>
#include <gui.h>
#include "stm32f769i_discovery_lcd.h"

extern const uint8_t MaxCharactersOnString;
extern UART_HandleTypeDef huart5;

void espPower(FunctionalState state)
{
	if(state == ENABLE)
	{
		HAL_GPIO_WritePin(GPIOH, GPIO_PIN_7, GPIO_PIN_SET);
	}		
	else 
	{
		HAL_GPIO_WritePin(GPIOH, GPIO_PIN_7, GPIO_PIN_RESET);
	}
}

uint8_t dat[300];
uint8_t item_name[300];
uint8_t lang[300];
uint8_t* ter = item_name;
uint16_t length;
uint8_t ch;
uint16_t i = 2;
uint16_t i_n = 0;
uint8_t findCode(char* code)
{
		memset(dat, 0x00, 300);
		memset(item_name, 0x00, 300);
		UART5->ICR |= (1 << USART_ICR_ORECF_Pos);
		HAL_UART_Transmit(&huart5,(uint8_t*)code,13, 100); 
		HAL_UART_Receive(&huart5, (uint8_t*)dat, 300, 3000);
		
		char* istr = strstr((char*)dat, "B");
			
		if(dat[0] == 'n' || dat[1] == 'n') {BSP_LCD_DisplayStringAt(20, 150, (uint8_t*)StringBarcodeNotFound, LEFT_MODE, 0); return 4;}
		
		i = (uint8_t*)istr - dat + 1;
		i_n = 0;
		ch = dat[i];
		while((ch != 0x0D) && i_n <= MaxCharactersOnString)
		{
			if((ch & 0x80) == 0)
			{
				item_name[i_n] = ch;
				lang[i_n] = 0;
				i_n++;
				i++;
			}
			else if((ch & 0x80) == 0x80)
			{
				uint16_t ch16 = (dat[i]<<8) | dat[i+1];
				lang[i_n] = 1;
				if(ch16 <= 0xD0AF)
				{
					//big characters
					item_name[i_n] = ch16 - 0xD04F;
				}
				else if(ch16 >= 0xD180)
				{
					item_name[i_n] = ch16 - 0xD12F;
				}
				else
				{
					item_name[i_n] = ch16 - 0xD06F;
				}
				i+=2;
				i_n++;
			}
			else i_n++;
			ch = dat[i];
		}

		BSP_LCD_DisplayStringAt_m(20, 150, item_name, lang, LEFT_MODE, 0);
		osDelay(40);
		return  4;
}
