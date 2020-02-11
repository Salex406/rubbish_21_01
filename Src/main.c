/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "cmsis_os.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32f769i_discovery_lcd.h"
#include "stm32f769i_discovery.h"
#include "stm32f769i_discovery_ts.h"
#include "string.h"
#include "gui.h"
#include "stdlib.h"

xQueueHandle gui_msg_q;
osMessageQId TOUCH_Queue;

FATFS SDFatFs;  /* File system object for SD card logical drive */
char SD_Path[4]; /* SD card logical drive path */
const uint8_t NumberOfOdjectsToLoadFromSD = 17;
uint8_t sect[4096];
uint32_t ts_status = TS_OK;
#define LCD_FRAME_BUFFER SDRAM_DEVICE_ADDR


struct Screen
{
	char  name[15];
	char  filename[15];
	uint16_t width;
	uint16_t height;
	uint32_t* location;
};

struct Screen screens[17];
//0 - main
//1 - help icon
//2 - settings icon
//3 - call icon
//4 - back icon
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

DMA2D_HandleTypeDef hdma2d;

DSI_HandleTypeDef hdsi;

I2C_HandleTypeDef hi2c1;

LTDC_HandleTypeDef hltdc;

SD_HandleTypeDef hsd2;

SDRAM_HandleTypeDef hsdram1;

osThreadId defaultTaskHandle;
/* USER CODE BEGIN PV */
osThreadId blinkTaskHandle;
osThreadId drawTaskHandle;
osThreadId touchTaskHandle;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA2D_Init(void);
static void MX_DSIHOST_DSI_Init(void);
static void MX_FMC_Init(void);
static void MX_I2C1_Init(void);
static void MX_LTDC_Init(void);
static void MX_SDMMC2_SD_Init(void);
static void MX_ADC1_Init(void);
extern void StartDefaultTask(void const * argument);

/* USER CODE BEGIN PFP */
extern void StartBlinkTask(void const * argument);
extern void StartDrawTask(void const * argument);
extern void StartTouchTask(void const * argument);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint32_t OpenBMP(uint8_t *ptr, const char* fname)
{
	uint32_t ind=0,sz=0,i1=0,ind1=0;
	uint32_t bytesread = 0;
	FRESULT a = 217;
	FIL MyFile; 
  static uint32_t bmp_addr;
	a = f_open(&MyFile,fname,FA_READ);
  if(a!=FR_OK)
  {
		return 1;
  }
  else
  {
    if(f_read(&MyFile,sect,30,(UINT *)&bytesread)!=FR_OK)
    {
      return 1;
    }
    else
    {
      bmp_addr=(uint32_t)sect;
      /*Get bitmap size*/
      sz=*(uint16_t*)(bmp_addr + 2);
      sz|=(*(uint16_t*)(bmp_addr + 4))<<16;
      /*Get bitmap data address offset*/
      ind=*(uint16_t*)(bmp_addr + 0x000A);
      ind|=(*(uint16_t*)(bmp_addr + 12))<<16;
      f_close(&MyFile);
      f_open(&MyFile,fname,FA_READ);
      ind=0;
      do
      {
        if(sz<4096)
        {
          i1=sz;
        }
        else
        {
          i1=4096;
        }
        sz-=i1;
        f_lseek(&MyFile,ind1);
        f_read(&MyFile,sect,i1,(UINT *)&bytesread);
        memcpy((void*)(ptr+ind1),(void*)sect,i1);
        ind1+=i1;
      }
      while(sz>0);
      f_close(&MyFile);
    }
    ind1=0;
		return 0;
  }
}


/* USERCODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */
  

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA2D_Init();
  MX_DSIHOST_DSI_Init();
  MX_FMC_Init();
  MX_I2C1_Init();
  MX_LTDC_Init();
  MX_SDMMC2_SD_Init();
  MX_ADC1_Init();
  MX_FATFS_Init();
  /* USER CODE BEGIN 2 */
	strcpy(screens[0].filename, "m_scr_v2.bmp");
	strcpy(screens[1].filename, "help.h");//"bg_R.bmp"
	strcpy(screens[2].filename, "settings.h");
	strcpy(screens[3].filename, "call.h");
	strcpy(screens[4].filename, "back.h");
	strcpy(screens[5].filename, "0.h");
	strcpy(screens[6].filename, "1.h");
	strcpy(screens[7].filename, "2.h");
	strcpy(screens[8].filename, "3.h");
	strcpy(screens[9].filename, "4.h");
	strcpy(screens[10].filename, "5.h");
	strcpy(screens[11].filename, "6.h");
	strcpy(screens[12].filename, "7.h");
	strcpy(screens[13].filename, "8.h");
	strcpy(screens[14].filename, "9.h");
	strcpy(screens[15].filename, "perc.h");
	strcpy(screens[16].filename, "cont.h");
	
	strcpy(screens[0].name, "main");
	strcpy(screens[1].name, "i_help");
	strcpy(screens[2].name, "i_settings");
	strcpy(screens[3].name, "i_call");
	strcpy(screens[4].name, "i_back");
	strcpy(screens[5].name, "n_0");
	strcpy(screens[6].name, "n_1");
	strcpy(screens[7].name, "n_2");
	strcpy(screens[8].name, "n_3");
	strcpy(screens[9].name, "n_4");
	strcpy(screens[10].name, "n_5");
	strcpy(screens[11].name, "n_6");
	strcpy(screens[12].name, "n_7");
	strcpy(screens[13].name, "n_5");
	strcpy(screens[14].name, "n_9");
	strcpy(screens[15].name, "n_perc");
	strcpy(screens[16].name, "i_cont");
	//1 screen is 0x119800
	//1 icon is 1fc00
	//1 big_num is 6c00
	
	//0 - main
	//1 - help icon
	//2 - settings icon
	//3 - call icon
	//4 - back icon
	screens[0].location = (uint32_t *)0xC0300000;
	screens[1].location = (uint32_t *)0xC0419800;//0xC0410800;
	screens[2].location = (uint32_t *)0xC0439400;
	screens[3].location = (uint32_t *)0xC0459000;
	screens[4].location = (uint32_t *)0xC0478C00;
	
	screens[5].location = (uint32_t *)0xC0498800; //0
	screens[6].location = (uint32_t *)0xC049F400; //1
	screens[7].location = (uint32_t *)0xC04ACC00; //2
	screens[8].location = (uint32_t *)0xC04B3800; //3
	screens[9].location = (uint32_t *)0xC04BA400; //4
	screens[10].location = (uint32_t *)0xC04C1000; //5
	screens[11].location = (uint32_t *)0xC04C7C00; //6
	screens[12].location = (uint32_t *)0xC04CE800; //7
	screens[13].location = (uint32_t *)0xC04D5400;
	screens[14].location = (uint32_t *)0xC04DC000;
	screens[15].location = (uint32_t *)0xC04E2C00;
	screens[16].location = (uint32_t *)0xC04E9800; //containers
	
																	//0xC1000000
	BSP_LCD_Init();
	BSP_LCD_LayerDefaultInit(0, LCD_FB_START_ADDRESS);
	BSP_LCD_SelectLayer(LTDC_ACTIVE_LAYER_BACKGROUND);
	TFT_FillScreen(LCD_COLOR_WHITE);
	//BSP_LCD_Clear(LCD_COLOR_WHITE);
	BSP_LCD_DisplayStringAt(0,240,(uint8_t*)"Loading..",CENTER_MODE);
	
	
	//Loading bitmaps to RAM
	uint8_t pr = 0;
	uint8_t step = 100/NumberOfOdjectsToLoadFromSD;
	FATFS_LinkDriver(&SD_Driver, SD_Path);
	FRESULT a = 218;
	if(HAL_GPIO_ReadPin(GPIOI,GPIO_PIN_15)==0)
	{
		a = f_mount(&SDFatFs, (TCHAR const*)SDPath, 0);
		if(a==FR_OK)
		{		
				placePrBar(280,266,200,30,pr,PROGRESSBAR_HORIZONTAL,LCD_COLOR_BLUE);
			
				uint8_t res = OpenBMP((uint8_t *)screens[0].location,screens[0].filename);
				if(res==1){ BSP_LCD_DisplayStringAt(0,240,(uint8_t*)"File 1 open error",CENTER_MODE); Error_Handler();}
				pr+=step;
				placePrBar(280,266,200,30,pr,PROGRESSBAR_HORIZONTAL,LCD_COLOR_BLUE);
			
				res = ReadImage(screens[1].location,screens[1].filename);
				if(res==1) { BSP_LCD_DisplayStringAt(0,240,(uint8_t*)"File 2 open error",CENTER_MODE); Error_Handler();}
				pr+=step;
				placePrBar(280,266,200,30,pr,PROGRESSBAR_HORIZONTAL,LCD_COLOR_BLUE);
			
				res = ReadImage(screens[2].location,screens[2].filename);
				if(res==1) { BSP_LCD_DisplayStringAt(0,240,(uint8_t*)"File 3 open error",CENTER_MODE); Error_Handler();}
				pr+=step;
				placePrBar(280,266,200,30,pr,PROGRESSBAR_HORIZONTAL,LCD_COLOR_BLUE);
				
				res = ReadImage(screens[3].location,screens[3].filename);
				if(res==1) { BSP_LCD_DisplayStringAt(0,240,(uint8_t*)"File 4 open error",CENTER_MODE); Error_Handler();}
				pr+=step;
				placePrBar(280,266,200,30,pr,PROGRESSBAR_HORIZONTAL,LCD_COLOR_BLUE);
				
				res = ReadImage(screens[4].location,screens[4].filename);
				if(res==1) { BSP_LCD_DisplayStringAt(0,240,(uint8_t*)"File 5 open error",CENTER_MODE); Error_Handler();}
				pr+=step;
				placePrBar(280,266,200,30,pr,PROGRESSBAR_HORIZONTAL,LCD_COLOR_BLUE);
				
				res = ReadImage(screens[5].location,screens[5].filename);
				if(res==1) { BSP_LCD_DisplayStringAt(0,240,(uint8_t*)"File 6 open error",CENTER_MODE); Error_Handler();}
				pr+=step;
				placePrBar(280,266,200,30,pr,PROGRESSBAR_HORIZONTAL,LCD_COLOR_BLUE);
				
				res = ReadImage(screens[6].location,screens[6].filename);
				if(res==1) { BSP_LCD_DisplayStringAt(0,240,(uint8_t*)"File 7 open error",CENTER_MODE); Error_Handler();}
				pr+=step;
				placePrBar(280,266,200,30,pr,PROGRESSBAR_HORIZONTAL,LCD_COLOR_BLUE);
				
				res = ReadImage(screens[7].location,screens[7].filename);
				if(res==1) { BSP_LCD_DisplayStringAt(0,240,(uint8_t*)"File 8 open error",CENTER_MODE); Error_Handler();}
				pr+=step;
				placePrBar(280,266,200,30,pr,PROGRESSBAR_HORIZONTAL,LCD_COLOR_BLUE);
				
				res = ReadImage(screens[8].location,screens[8].filename);
				if(res==1) { BSP_LCD_DisplayStringAt(0,240,(uint8_t*)"File 9 open error",CENTER_MODE); Error_Handler();}
				pr+=step;
				placePrBar(280,266,200,30,pr,PROGRESSBAR_HORIZONTAL,LCD_COLOR_BLUE);
				
				res = ReadImage(screens[9].location,screens[9].filename);
				if(res==1) { BSP_LCD_DisplayStringAt(0,240,(uint8_t*)"File 10 open error",CENTER_MODE); Error_Handler();}
				pr+=step;
				placePrBar(280,266,200,30,pr,PROGRESSBAR_HORIZONTAL,LCD_COLOR_BLUE);
				
				res = ReadImage(screens[10].location,screens[10].filename);
				if(res==1) { BSP_LCD_DisplayStringAt(0,240,(uint8_t*)"File 11 open error",CENTER_MODE); Error_Handler();}
				pr+=step;
				placePrBar(280,266,200,30,pr,PROGRESSBAR_HORIZONTAL,LCD_COLOR_BLUE);
				
				res = ReadImage(screens[11].location,screens[11].filename);
				if(res==1) { BSP_LCD_DisplayStringAt(0,240,(uint8_t*)"File 12 open error",CENTER_MODE); Error_Handler();}
				pr+=step;
				placePrBar(280,266,200,30,pr,PROGRESSBAR_HORIZONTAL,LCD_COLOR_BLUE);
				
				res = ReadImage(screens[12].location,screens[12].filename);
				if(res==1) { BSP_LCD_DisplayStringAt(0,240,(uint8_t*)"File 13 open error",CENTER_MODE); Error_Handler();}
				pr+=step;
				placePrBar(280,266,200,30,pr,PROGRESSBAR_HORIZONTAL,LCD_COLOR_BLUE);
				
				res = ReadImage(screens[13].location,screens[13].filename);
				if(res==1) { BSP_LCD_DisplayStringAt(0,240,(uint8_t*)"File 14 open error",CENTER_MODE); Error_Handler();}
				pr+=step;
				placePrBar(280,266,200,30,pr,PROGRESSBAR_HORIZONTAL,LCD_COLOR_BLUE);
				
				res = ReadImage(screens[14].location,screens[14].filename);
				if(res==1) { BSP_LCD_DisplayStringAt(0,240,(uint8_t*)"File 15 open error",CENTER_MODE); Error_Handler();}
				pr+=step;
				placePrBar(280,266,200,30,pr,PROGRESSBAR_HORIZONTAL,LCD_COLOR_BLUE);
				
				res = ReadImage(screens[15].location,screens[15].filename);
				if(res==1) { BSP_LCD_DisplayStringAt(0,240,(uint8_t*)"File 16 open error",CENTER_MODE); Error_Handler();}
				pr+=step;
				placePrBar(280,266,200,30,pr,PROGRESSBAR_HORIZONTAL,LCD_COLOR_BLUE);
				
				res = ReadImage(screens[16].location,screens[16].filename);
				if(res==1) { BSP_LCD_DisplayStringAt(0,240,(uint8_t*)"File 17 open error",CENTER_MODE); Error_Handler();}
				pr+=step;
				
				placePrBar(280,266,200,30,100,PROGRESSBAR_HORIZONTAL,LCD_COLOR_BLUE);
		}
		else { BSP_LCD_DisplayStringAt(0,240,(uint8_t*)"SD mount error",CENTER_MODE); Error_Handler();}
	}
	else { BSP_LCD_DisplayStringAt(0,240,(uint8_t*)"SD is not connected",CENTER_MODE); Error_Handler();}
	uint8_t msg=0;
	
	gui_msg_q = xQueueGenericCreate(1, 1, 0);
	//uint8_t* location = (uint8_t *)0xC0419094;
	//ConvertBitmap(0,0,location);
	xQueueSend(gui_msg_q, &msg, 0);
	ts_status = BSP_TS_Init(800,480);
	ts_status = BSP_TS_ITConfig();
	osMessageQDef(usart_Queue, 1, uint8_t);
  TOUCH_Queue = osMessageCreate(osMessageQ(usart_Queue), NULL);
	
	//BSP_LCD_DrawBitmap(0,0,(uint8_t *)main_screen);
	//placePrBar(520,20,100,40,0,0,LCD_COLOR_BLUE);
	//TFT_FillRectangle(20, 20, 40, 70, LCD_COLOR_BLUE);
	//a = f_mount(&SDFatFs, (TCHAR const*)SDPath, 0);
	//a = f_open(&JPEG_File, "im.bmp", FA_READ);
  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 1280);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
	
	/* definition and creation of blinkTask */
  osThreadDef(blinkTask, StartBlinkTask, osPriorityNormal, 0, 16);
  blinkTaskHandle = osThreadCreate(osThread(blinkTask), NULL);
	
	/* definition and creation of drawTask */
  osThreadDef(drawTask, StartDrawTask, osPriorityNormal, 0, 1280);
  drawTaskHandle = osThreadCreate(osThread(drawTask), NULL);
	
	/* definition and creation of touchTask */
  osThreadDef(touchTask, StartTouchTask, osPriorityBelowNormal, 0, 128);
  touchTaskHandle = osThreadCreate(osThread(touchTask), NULL);
  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();
 
  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 400;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 15;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Activate the Over-Drive mode 
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC|RCC_PERIPHCLK_I2C1
                              |RCC_PERIPHCLK_SDMMC2|RCC_PERIPHCLK_CLK48;
  PeriphClkInitStruct.PLLSAI.PLLSAIN = 192;
  PeriphClkInitStruct.PLLSAI.PLLSAIR = 2;
  PeriphClkInitStruct.PLLSAI.PLLSAIQ = 2;
  PeriphClkInitStruct.PLLSAI.PLLSAIP = RCC_PLLSAIP_DIV2;
  PeriphClkInitStruct.PLLSAIDivQ = 1;
  PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_2;
  PeriphClkInitStruct.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48SOURCE_PLL;
  PeriphClkInitStruct.Sdmmc2ClockSelection = RCC_SDMMC2CLKSOURCE_CLK48;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
  */
  sConfig.Channel = ADC_CHANNEL_6;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief DMA2D Initialization Function
  * @param None
  * @retval None
  */
static void MX_DMA2D_Init(void)
{

  /* USER CODE BEGIN DMA2D_Init 0 */

  /* USER CODE END DMA2D_Init 0 */

  /* USER CODE BEGIN DMA2D_Init 1 */

  /* USER CODE END DMA2D_Init 1 */
  hdma2d.Instance = DMA2D;
  hdma2d.Init.Mode = DMA2D_M2M;
  hdma2d.Init.ColorMode = DMA2D_OUTPUT_ARGB8888;
  hdma2d.Init.OutputOffset = 0;
  hdma2d.LayerCfg[1].InputOffset = 0;
  hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB8888;
  hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  hdma2d.LayerCfg[1].InputAlpha = 0;
  hdma2d.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA;
  hdma2d.LayerCfg[1].RedBlueSwap = DMA2D_RB_REGULAR;
  if (HAL_DMA2D_Init(&hdma2d) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DMA2D_ConfigLayer(&hdma2d, 1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DMA2D_Init 2 */

  /* USER CODE END DMA2D_Init 2 */

}

/**
  * @brief DSIHOST Initialization Function
  * @param None
  * @retval None
  */
static void MX_DSIHOST_DSI_Init(void)
{

  /* USER CODE BEGIN DSIHOST_Init 0 */

  /* USER CODE END DSIHOST_Init 0 */

  DSI_PLLInitTypeDef PLLInit = {0};
  DSI_HOST_TimeoutTypeDef HostTimeouts = {0};
  DSI_PHY_TimerTypeDef PhyTimings = {0};
  DSI_VidCfgTypeDef VidCfg = {0};

  /* USER CODE BEGIN DSIHOST_Init 1 */

  /* USER CODE END DSIHOST_Init 1 */
  hdsi.Instance = DSI;
  hdsi.Init.AutomaticClockLaneControl = DSI_AUTO_CLK_LANE_CTRL_DISABLE;
  hdsi.Init.TXEscapeCkdiv = 4;
  hdsi.Init.NumberOfLanes = DSI_ONE_DATA_LANE;
  PLLInit.PLLNDIV = 20;
  PLLInit.PLLIDF = DSI_PLL_IN_DIV1;
  PLLInit.PLLODF = DSI_PLL_OUT_DIV1;
  if (HAL_DSI_Init(&hdsi, &PLLInit) != HAL_OK)
  {
    Error_Handler();
  }
  HostTimeouts.TimeoutCkdiv = 1;
  HostTimeouts.HighSpeedTransmissionTimeout = 0;
  HostTimeouts.LowPowerReceptionTimeout = 0;
  HostTimeouts.HighSpeedReadTimeout = 0;
  HostTimeouts.LowPowerReadTimeout = 0;
  HostTimeouts.HighSpeedWriteTimeout = 0;
  HostTimeouts.HighSpeedWritePrespMode = DSI_HS_PM_DISABLE;
  HostTimeouts.LowPowerWriteTimeout = 0;
  HostTimeouts.BTATimeout = 0;
  if (HAL_DSI_ConfigHostTimeouts(&hdsi, &HostTimeouts) != HAL_OK)
  {
    Error_Handler();
  }
  PhyTimings.ClockLaneHS2LPTime = 28;
  PhyTimings.ClockLaneLP2HSTime = 33;
  PhyTimings.DataLaneHS2LPTime = 15;
  PhyTimings.DataLaneLP2HSTime = 25;
  PhyTimings.DataLaneMaxReadTime = 0;
  PhyTimings.StopWaitTime = 0;
  if (HAL_DSI_ConfigPhyTimer(&hdsi, &PhyTimings) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DSI_ConfigFlowControl(&hdsi, DSI_FLOW_CONTROL_BTA) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DSI_SetLowPowerRXFilter(&hdsi, 10000) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DSI_ConfigErrorMonitor(&hdsi, HAL_DSI_ERROR_NONE) != HAL_OK)
  {
    Error_Handler();
  }
  VidCfg.VirtualChannelID = 0;
  VidCfg.ColorCoding = DSI_RGB888;
  VidCfg.LooselyPacked = DSI_LOOSELY_PACKED_DISABLE;
  VidCfg.Mode = DSI_VID_MODE_BURST;
  VidCfg.PacketSize = 800;
  VidCfg.NumberOfChunks = 0;
  VidCfg.NullPacketSize = 0;
  VidCfg.HSPolarity = DSI_HSYNC_ACTIVE_HIGH;
  VidCfg.VSPolarity = DSI_VSYNC_ACTIVE_HIGH;
  VidCfg.DEPolarity = DSI_DATA_ENABLE_ACTIVE_HIGH;
  VidCfg.HorizontalSyncActive = 273;
  VidCfg.HorizontalBackPorch = 273;
  VidCfg.HorizontalLine = 2643;
  VidCfg.VerticalSyncActive = 12;
  VidCfg.VerticalBackPorch = 12;
  VidCfg.VerticalFrontPorch = 12;
  VidCfg.VerticalActive = 480;
  VidCfg.LPCommandEnable = DSI_LP_COMMAND_DISABLE;
  VidCfg.LPLargestPacketSize = 0;
  VidCfg.LPVACTLargestPacketSize = 0;
  VidCfg.LPHorizontalFrontPorchEnable = DSI_LP_HFP_ENABLE;
  VidCfg.LPHorizontalBackPorchEnable = DSI_LP_HBP_ENABLE;
  VidCfg.LPVerticalActiveEnable = DSI_LP_VACT_ENABLE;
  VidCfg.LPVerticalFrontPorchEnable = DSI_LP_VFP_ENABLE;
  VidCfg.LPVerticalBackPorchEnable = DSI_LP_VBP_ENABLE;
  VidCfg.LPVerticalSyncActiveEnable = DSI_LP_VSYNC_ENABLE;
  VidCfg.FrameBTAAcknowledgeEnable = DSI_FBTAA_DISABLE;
  if (HAL_DSI_ConfigVideoMode(&hdsi, &VidCfg) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DSI_SetGenericVCID(&hdsi, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DSIHOST_Init 2 */

  /* USER CODE END DSIHOST_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00401959;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Analogue filter 
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Digital filter 
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief LTDC Initialization Function
  * @param None
  * @retval None
  */
static void MX_LTDC_Init(void)
{

  /* USER CODE BEGIN LTDC_Init 0 */

  /* USER CODE END LTDC_Init 0 */

  LTDC_LayerCfgTypeDef pLayerCfg = {0};

  /* USER CODE BEGIN LTDC_Init 1 */

  /* USER CODE END LTDC_Init 1 */
  hltdc.Instance = LTDC;
  hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AH;
  hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AH;
  hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
  hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
  hltdc.Init.HorizontalSync = 119;
  hltdc.Init.VerticalSync = 11;
  hltdc.Init.AccumulatedHBP = 239;
  hltdc.Init.AccumulatedVBP = 23;
  hltdc.Init.AccumulatedActiveW = 1039;
  hltdc.Init.AccumulatedActiveH = 503;
  hltdc.Init.TotalWidth = 1159;
  hltdc.Init.TotalHeigh = 515;
  hltdc.Init.Backcolor.Blue = 0;
  hltdc.Init.Backcolor.Green = 0;
  hltdc.Init.Backcolor.Red = 0;
  if (HAL_LTDC_Init(&hltdc) != HAL_OK)
  {
    Error_Handler();
  }
  pLayerCfg.WindowX0 = 0;
  pLayerCfg.WindowX1 = 800;
  pLayerCfg.WindowY0 = 0;
  pLayerCfg.WindowY1 = 480;
  pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_ARGB8888;
  pLayerCfg.Alpha = 255;
  pLayerCfg.Alpha0 = 0;
  pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
  pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
  pLayerCfg.FBStartAdress = 0;
  pLayerCfg.ImageWidth = 800;
  pLayerCfg.ImageHeight = 480;
  pLayerCfg.Backcolor.Blue = 0;
  pLayerCfg.Backcolor.Green = 0;
  pLayerCfg.Backcolor.Red = 0;
  if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LTDC_Init 2 */

  /* USER CODE END LTDC_Init 2 */

}

/**
  * @brief SDMMC2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SDMMC2_SD_Init(void)
{

  /* USER CODE BEGIN SDMMC2_Init 0 */

  /* USER CODE END SDMMC2_Init 0 */

  /* USER CODE BEGIN SDMMC2_Init 1 */

  /* USER CODE END SDMMC2_Init 1 */
  hsd2.Instance = SDMMC2;
  hsd2.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  hsd2.Init.ClockBypass = SDMMC_CLOCK_BYPASS_DISABLE;
  hsd2.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  hsd2.Init.BusWide = SDMMC_BUS_WIDE_1B;
  hsd2.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd2.Init.ClockDiv = 0;
  /* USER CODE BEGIN SDMMC2_Init 2 */
	HAL_SD_Init(&hsd2);
  /* USER CODE END SDMMC2_Init 2 */

}

/* FMC initialization function */
static void MX_FMC_Init(void)
{

  /* USER CODE BEGIN FMC_Init 0 */

  /* USER CODE END FMC_Init 0 */

  FMC_SDRAM_TimingTypeDef SdramTiming = {0};

  /* USER CODE BEGIN FMC_Init 1 */

  /* USER CODE END FMC_Init 1 */

  /** Perform the SDRAM1 memory initialization sequence
  */
  hsdram1.Instance = FMC_SDRAM_DEVICE;
  /* hsdram1.Init */
  hsdram1.Init.SDBank = FMC_SDRAM_BANK1;
  hsdram1.Init.ColumnBitsNumber = FMC_SDRAM_COLUMN_BITS_NUM_8;
  hsdram1.Init.RowBitsNumber = FMC_SDRAM_ROW_BITS_NUM_12;
  hsdram1.Init.MemoryDataWidth = FMC_SDRAM_MEM_BUS_WIDTH_32;
  hsdram1.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
  hsdram1.Init.CASLatency = FMC_SDRAM_CAS_LATENCY_3;
  hsdram1.Init.WriteProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
  hsdram1.Init.SDClockPeriod = FMC_SDRAM_CLOCK_PERIOD_2;
  hsdram1.Init.ReadBurst = FMC_SDRAM_RBURST_ENABLE;
  hsdram1.Init.ReadPipeDelay = FMC_SDRAM_RPIPE_DELAY_0;
  /* SdramTiming */
  SdramTiming.LoadToActiveDelay = 2;
  SdramTiming.ExitSelfRefreshDelay = 7;
  SdramTiming.SelfRefreshTime = 4;
  SdramTiming.RowCycleDelay = 7;
  SdramTiming.WriteRecoveryTime = 3;
  SdramTiming.RPDelay = 2;
  SdramTiming.RCDDelay = 2;

  if (HAL_SDRAM_Init(&hsdram1, &SdramTiming) != HAL_OK)
  {
    Error_Handler( );
  }

  /* USER CODE BEGIN FMC_Init 2 */

  /* USER CODE END FMC_Init 2 */
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOJ_CLK_ENABLE();
  __HAL_RCC_GPIOI_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOJ, GPIO_PIN_13|GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pins : PJ13 PJ5 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOJ, &GPIO_InitStruct);

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

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */


/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */



/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
	HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_13,GPIO_PIN_SET);
	while(1){}
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
