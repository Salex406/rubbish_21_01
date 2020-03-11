/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
typedef enum {
    MAIN,      
    SETTINGS,      
    CALL,    
    HELP,    
    DOOR_OPEN,
		PRESSING,
		ERR
}Screen;

struct Image
{
	char  name[15];
	char  filename[15];
	uint16_t width;
	uint16_t height;
	uint32_t* location;
};

typedef enum {
		NO_ERR,
    OVERCURENT_PRESSING,      
    OVERTIME_PRESSING,      
    OVERTIME_AFTERPRESSING,    
    OVERTIME_PRESSUP
}Error;


typedef enum {
		PLASTIC,
		GLASS,
		METAL
}BinSelection;

static const uint8_t NumberOfOdjectsToLoadFromSD = 20;
static const uint16_t LoadProgressBarPosY = 285;
static const uint16_t LoadProgressBarPosX = 228;
static const uint16_t LoadProgressBarLength = 260;
static const uint8_t LoadProgressBarWidth = 35;

static const uint16_t ContainerBottomX = 126;
static const uint16_t ContainerBottomY = 210;
static const uint16_t ContainerFlapsY = ContainerBottomY - 143;
static const uint16_t ContainerYellowFlapX = 325;
static const uint16_t ContainerGreenFlapX = 146;
static const uint16_t ContainerBlueFlapX = 502;
static const uint16_t NumOnContainerY = 120;

static const uint8_t DistanceBetweenStrings48 = 35;
static const uint8_t MaxCharactersOnString = 25;

static const uint16_t RightButtonX = 670;
static const uint16_t LeftButtonX = 13;
static const uint16_t TopButtonY = 13;
static const uint16_t BottomButtonY = 342;

static const uint16_t PresTimeout = 40;
static const uint16_t CriticalCurrentForPres = 15;

static const uint8_t* StringLoad = (uint8_t*)"hADQTHKA...";
static const uint8_t* StringMountError = (uint8_t*)"oYIBKA KAQS\\ ";
static const uint8_t* StringNoSD = (uint8_t*)"oSRTSRSCTFS KAQSA PAM`SI";
static const uint8_t* StringPlastic = (uint8_t*)"pLARSIK";
static const uint8_t* StringGlass = (uint8_t*)"rSFKLO";
static const uint8_t* StringMetal = (uint8_t*)"mFSALL";
static const uint8_t* StringSettings = (uint8_t*)"nARSQOJKI";
static const uint8_t* StringHelp = (uint8_t*)"pOEEFQGKA";
static const uint8_t* StringRemoveCap = (uint8_t*)"tEALISF KQ\\YKT R BTS\\LKI";
static const uint8_t* StringScanBarcodeOr = (uint8_t*)"rKANIQTJSF YSQIVKOE ILI";
static const uint8_t* StringChooseBinManually = (uint8_t*)"c\\BFQISF BAK CQTXNT_";
static const uint8_t* StringPlaceRubbishIntoBin	= (uint8_t*)"pOMFRSISF MTROQ C KAMFQT";
static const uint8_t* StringOrChooseAnotherBin	= (uint8_t*)"iLI C\\BFQISF EQTDOJ BAK";
static const uint8_t* StringRubbishIsPressing	= (uint8_t*)"mTROQ PQFRRTFSR`,";
static const uint8_t* StringWait	= (uint8_t*)"pOEOGEISF";
static const uint8_t* StringError	= (uint8_t*)"oYIBKA!";
static const uint8_t* StringPressOverload	= (uint8_t*)"pFQFDQTHKA PQFRRA";
static const uint8_t* StringPressTimeout	= (uint8_t*)"sAJMATS PQFRRA";
static const uint8_t* StringPressUpTimeout	= (uint8_t*)"sAJMATS POEN`SI` PQFRRA";
static const uint8_t* StringBarcodeNotFound	= (uint8_t*)"ySQIVKOE NF NAJEFN";

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
