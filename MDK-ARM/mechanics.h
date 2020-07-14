#include "main.h"
#include "gui.h"
#include "stm32f7xx_hal.h"
#include "stdio.h"

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

//D9
#define K_K1_PORT GPIOH
#define K_K1_Pin GPIO_PIN_6

//D10
#define K_K4_PORT GPIOA
#define K_K4_Pin GPIO_PIN_11

//D11
#define STOPOR_PORT GPIOB
#define STOPOR_Pin GPIO_PIN_15

//D12
#define DOORBLOCK_PORT GPIOB
#define DOORBLOCK_Pin GPIO_PIN_14

//D13
#define DOOR_PORT GPIOA
#define DOOR_Pin GPIO_PIN_12

//A4
#define K_K2_PORT GPIOF
#define K_K2_Pin GPIO_PIN_8

//A5
#define K_K3_PORT GPIOF
#define K_K3_Pin GPIO_PIN_9

typedef enum {
    START_POS,      
    GLASS_POS,       
    METAL_POS,   
    PLASTIC_POS,   
}CarriagePos;

typedef enum {
    UP,      
    DOWN,       
}PressPos;

/*
typedef enum {
    OK,      
    TIMEOUT_ERR,
		OVERCURRENT_ERR,       
}PressState;
*/

extern const uint16_t PresTimeout;
extern const uint16_t CriticalCurrentForPres;
extern UART_HandleTypeDef huart1;
extern ADC_HandleTypeDef hadc1;

uint8_t driveCarriageToPos(CarriagePos destination);
Error drivePress(PressPos destination);
void doorBlock(uint8_t state);
void rotationCounter(FunctionalState state);
void MX_GPIO_Init_custom(void);

