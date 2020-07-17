//#include "main.h"
#include "stm32f7xx_hal.h"

#define MAX_DRIVERS_NUMBER 1
#define SHIELD_ENABLE_PORT GPIOJ
#define SHIELD_ENABLE_PIN GPIO_PIN_4



typedef struct{
	GPIO_TypeDef * STEP_Port, * DIR_Port;
	uint16_t STEP, DIR;
	TIM_HandleTypeDef Timer;
	uint8_t Index, PeriodStep;
	uint16_t StartCounterPeriod, StopCounterPeriod;
	uint8_t DriverDisable;
	
}HR4988_DriverTypeDef;

typedef enum dir_enum{
	CW_DIR=0x00U,
	CCW_DIR
	
}HR4988_Direction;





void HR4988_DriverInit(HR4988_DriverTypeDef * DriverStruct);

void HR4988_RunMotor(HR4988_DriverTypeDef * DriverStruct,HR4988_Direction Dir);

void HR4988_StopMotor(HR4988_DriverTypeDef * DriverStruct);

void HR4988_RunFixedSteps(HR4988_DriverTypeDef *DriverStruct, uint64_t Stepcount, HR4988_Direction Dir);

void HR4988_DriverCallback(TIM_HandleTypeDef *htim); //insert this into HAL timer callback function

