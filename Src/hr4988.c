#include "hr4988.h"

static HR4988_DriverTypeDef ActiveDriversArr[MAX_DRIVERS_NUMBER];
static uint64_t DriverSteps[MAX_DRIVERS_NUMBER];
static uint64_t DriverStepcounts[MAX_DRIVERS_NUMBER];
static uint8_t StepcountEnabled[MAX_DRIVERS_NUMBER];
static uint8_t active_drivers=0x00U;




void HR4988_SetStepFreq(TIM_HandleTypeDef HalTimerHandle){
	
	
}

void HR4988_RunMotor(HR4988_DriverTypeDef * DriverStruct,HR4988_Direction Dir){
	
	DriverSteps[DriverStruct->Index]=0x00U;
	if (DriverStruct->Timer.State==HAL_TIM_STATE_READY){
		DriverStruct->Timer.Instance->ARR=DriverStruct->StartCounterPeriod;
	
		HAL_GPIO_WritePin(SHIELD_ENABLE_PORT,SHIELD_ENABLE_PIN,GPIO_PIN_RESET);
	
		HAL_GPIO_WritePin(DriverStruct->DIR_Port,DriverStruct->DIR,(GPIO_PinState)Dir);
	
		HAL_TIM_Base_Start_IT(&DriverStruct->Timer);
	}
	
		
}

void HR4988_StopMotor(HR4988_DriverTypeDef * DriverStruct){
	
	if (DriverStruct->DriverDisable){
		HAL_GPIO_WritePin(SHIELD_ENABLE_PORT,SHIELD_ENABLE_PIN,GPIO_PIN_SET);
	}
	HAL_TIM_Base_Stop_IT(&DriverStruct->Timer);
	
	
};

void HR4988_RunFixedSteps(HR4988_DriverTypeDef *DriverStruct, uint64_t Stepcount, HR4988_Direction Dir){
	
	DriverStepcounts[DriverStruct->Index]=Stepcount;
	StepcountEnabled[DriverStruct->Index]=0xFFU;
	HR4988_RunMotor(DriverStruct,Dir);


}


void HR4988_DriverInit(HR4988_DriverTypeDef * DriverStruct){
	
	DriverStruct->Index=active_drivers;
	ActiveDriversArr[DriverStruct->Index]=*DriverStruct;
	StepcountEnabled[DriverStruct->Index]=0x00U;
	active_drivers++;
			
		
}

void HR4988_DriverCallback(TIM_HandleTypeDef *htim)
{
	
	for (uint8_t i=0;i<active_drivers;i++){
		
		if (htim->Instance==ActiveDriversArr[i].Timer.Instance){
			 
			if (StepcountEnabled[i] && DriverSteps[i]==DriverStepcounts[i]){
				HR4988_StopMotor(&ActiveDriversArr[i]);
				StepcountEnabled[i]=0x00U;
			}
			else{
				HAL_GPIO_TogglePin(ActiveDriversArr[i].STEP_Port,ActiveDriversArr[i].STEP);
				DriverSteps[i]++;
				if (htim->Instance->ARR>ActiveDriversArr[i].StopCounterPeriod){
					htim->Instance->ARR=htim->Instance->ARR-ActiveDriversArr[i].PeriodStep;
				}
			}
			
		}
	}

  
}
