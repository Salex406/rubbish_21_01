#include "main.h"
#include "gui.h"
#include "stm32f7xx_hal.h"
#include "stdio.h"



//D10
#define K_K2_PORT GPIOA
#define K_K2_Pin GPIO_PIN_11

//D10
#define K_K3_PORT GPIOA
#define K_K3_Pin GPIO_PIN_11

//D9
#define K_K1_PORT GPIOH
#define K_K1_Pin GPIO_PIN_6

//D9
#define K_K4_PORT GPIOH
#define K_K4_Pin GPIO_PIN_6

//D3
#define K_PDN_PORT GPIOF
#define K_PDN_Pin GPIO_PIN_6

//D4
#define K_PUP_PORT GPIOJ
#define K_PUP_Pin GPIO_PIN_0

//D6
#define PDOWN_PORT GPIOF
#define PDOWN_Pin GPIO_PIN_7

//D7
#define PUP_PORT GPIOJ
#define PUP_Pin GPIO_PIN_3

//D13
#define DOOR_PORT GPIOA
#define DOOR_Pin GPIO_PIN_12

typedef enum {
    START,      
    GLASS,       
    METAL,   
    PLASTIC,   
}CarriagePos;

typedef enum {
    UP,      
    DOWN,       
}PressPos;

typedef enum {
    OK,      
    TIMEOUT_ERR,
		OVERCURRENT_ERR,       
}PressState;

extern const uint16_t PresTimeout;
extern const uint16_t CriticalCurrentForPres;
extern UART_HandleTypeDef huart1;
extern ADC_HandleTypeDef hadc1;

uint8_t driveCarriageToPos(CarriagePos destination);
PressState drivePress(PressPos destination);