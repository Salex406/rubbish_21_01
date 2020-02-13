#include "gui.h"
#include "stm32f7xx_hal.h"
#include "stm32f769i_discovery_lcd.h"
#include "stm32f769i_discovery_ts.h"
#include "stdio.h"



struct Image
{
	char  name[15];
	char  filename[15];
	uint16_t width;
	uint16_t height;
	uint32_t* location;
};

extern DMA2D_HandleTypeDef hdma2d;
extern LTDC_HandleTypeDef hltdc;
extern struct Image images[17];

//load .bmp to RAM
uint32_t OpenBMP(uint8_t *ptr, const char* fname)
{
	uint8_t sect[4096];
	uint32_t ind=0,sz=0,i1=0,ind1=0;
	uint32_t bytesread = 0;
	//FRESULT a = 217;
	FRESULT a;
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

uint8_t LoadImagesFromSdToRAM()
{
	FATFS SDFatFs;  /* File system object for SD card logical drive */
	char SD_Path[4]; /* SD card logical drive path */
	//from main.c
	extern uint8_t NumberOfOdjectsToLoadFromSD;
	extern uint16_t LoadProgressBarPosX;
	extern uint16_t LoadProgressBarPosY;
	extern uint16_t LoadProgressBarLength;
	extern uint8_t LoadProgressBarWidth;
	
	BSP_LCD_SetFont(&rus48);
	BSP_LCD_DisplayStringAt(0,235,(uint8_t*)"jADRUILA...",CENTER_MODE, 1); //"Loading.."
	BSP_LCD_SetFont(&Font20)
	;
	//Loading bitmaps to RAM
	uint8_t pr = 0;
	uint8_t step = 100/NumberOfOdjectsToLoadFromSD;
	FATFS_LinkDriver(&SD_Driver, SD_Path);
	//FRESULT a = 218;
	FRESULT a;
	if(HAL_GPIO_ReadPin(GPIOI,GPIO_PIN_15)==0)
	{
		a = f_mount(&SDFatFs, (TCHAR const*)SDPath, 0);
		if(a==FR_OK)
		{		
				placePrBar(LoadProgressBarPosX,LoadProgressBarPosY,LoadProgressBarLength,LoadProgressBarWidth,pr,PROGRESSBAR_HORIZONTAL,LCD_COLOR_BLUE);
			
				//load image to RAM from .bmp file
				uint8_t res = OpenBMP((uint8_t *)images[0].location,images[0].filename);
				if(res==1){ BSP_LCD_DisplayStringAt(0,240,(uint8_t*)"File 1 open error",CENTER_MODE, 1); Error_Handler();}
				pr+=step;
				placePrBar(LoadProgressBarPosX,LoadProgressBarPosY,LoadProgressBarLength,LoadProgressBarWidth,pr,PROGRESSBAR_HORIZONTAL,LCD_COLOR_BLUE);
				
				//load images to RAM from .h files
				for(uint8_t i=1;i<NumberOfOdjectsToLoadFromSD;i++)
				{
					res = ReadImage(images[i].location,images[i].filename);
					if(res==1)
					{
						char* error_str;
						sprintf(error_str,"File %d open error. Pls reboot and check SD card.",i+1);
						Error_Handler();
					}
						pr+=step;
						placePrBar(LoadProgressBarPosX,LoadProgressBarPosY,LoadProgressBarLength,LoadProgressBarWidth,pr,PROGRESSBAR_HORIZONTAL,LCD_COLOR_BLUE);
				}
		}
		else { BSP_LCD_DisplayStringAt(0,240,(uint8_t*)"SD mount error",CENTER_MODE, 1); Error_Handler();}
	}
	else { BSP_LCD_DisplayStringAt(0,240,(uint8_t*)"SD is not connected",CENTER_MODE, 1); Error_Handler();}
	return 0;
}

void TFT_FillRectangle(uint16_t x1, uint16_t y1,uint16_t x2, uint16_t y2, uint32_t color)
{
  if(x1>x2) swap(x1,x2);
  if(y1>y2) swap(y1,y2);
  uint32_t addr = 0;
	addr = FRAMEBUFER_ADDR + 4*(y1*hltdc.LayerCfg[0].ImageWidth + x1);
  hdma2d.Init.Mode = DMA2D_R2M;
  hdma2d.Init.OutputOffset = hltdc.LayerCfg[0].ImageWidth-(x2-x1);
  if(HAL_DMA2D_Init(&hdma2d) == HAL_OK)
  {
    if (HAL_DMA2D_Start(&hdma2d, color, addr, x2-x1, y2-y1) == HAL_OK)
    {
      HAL_DMA2D_PollForTransfer(&hdma2d, 10);
    }
  }
}


void TFT_FillScreen(uint32_t color)
{
  hdma2d.Init.Mode = DMA2D_R2M;
  hdma2d.Init.OutputOffset = 0;
  if(HAL_DMA2D_Init(&hdma2d) == HAL_OK)
  {
    if (HAL_DMA2D_Start(&hdma2d, color, FRAMEBUFER_ADDR,
    hltdc.LayerCfg[0].ImageWidth, hltdc.LayerCfg[0].ImageHeight) == HAL_OK)
    {
      HAL_DMA2D_PollForTransfer(&hdma2d, 10);
    }
  }
}

//place progress bar (horisontal or vertical)
void placePrBar(uint16_t x, uint16_t y, uint16_t length, uint16_t width, uint8_t progress,uint8_t type,uint32_t color)
{
	if(type==PROGRESSBAR_HORIZONTAL)
	{
		uint16_t filledLength = length*progress/100;
		TFT_FillRectangle(x, y, x + filledLength, y + width, color);
		TFT_FillRectangle(x + filledLength, y, x + length, y + width, 0xFFFFFFFF); 
	}
	else
	{
		uint16_t filledLength = length*progress/100;
		TFT_FillRectangle(x, y, x + width, y - filledLength, color);
		TFT_FillRectangle(x, y - length, x + width, y - filledLength, 0xFFFFFFFF); 
	}
}

//func from ST. Is not used now
uint32_t Touchscreen_Handle_NewTouch(void)
{
TS_StateTypeDef  TS_State = {0};
static uint32_t touchscreen_color_idx = 0;
#define TS_MULTITOUCH_FOOTPRINT_CIRCLE_RADIUS 15
#define TOUCH_INFO_STRING_SIZE                70
  uint16_t x1 = 0;
  uint16_t y1 = 0;
  uint16_t x2 = 0;
  uint16_t y2 = 0;
  uint32_t drawTouch1 = 0; /* activate/deactivate draw of footprint of touch 1 */
  uint32_t drawTouch2 = 0; /* activate/deactivate draw of footprint of touch 2 */
  uint32_t colors[24] = {LCD_COLOR_BLUE, LCD_COLOR_GREEN, LCD_COLOR_RED, LCD_COLOR_CYAN, LCD_COLOR_MAGENTA, LCD_COLOR_YELLOW,
                         LCD_COLOR_LIGHTBLUE, LCD_COLOR_LIGHTGREEN, LCD_COLOR_LIGHTRED, LCD_COLOR_LIGHTCYAN, LCD_COLOR_LIGHTMAGENTA,
                         LCD_COLOR_LIGHTYELLOW, LCD_COLOR_DARKBLUE, LCD_COLOR_DARKGREEN, LCD_COLOR_DARKRED, LCD_COLOR_DARKCYAN,
                         LCD_COLOR_DARKMAGENTA, LCD_COLOR_DARKYELLOW, LCD_COLOR_LIGHTGRAY, LCD_COLOR_GRAY, LCD_COLOR_DARKGRAY,
                         LCD_COLOR_BLACK, LCD_COLOR_BROWN, LCD_COLOR_ORANGE };
  uint32_t ts_status = TS_OK;
  uint8_t lcd_string[TOUCH_INFO_STRING_SIZE] = "";

  /* Check in polling mode in touch screen the touch status and coordinates */
  /* of touches if touch occurred                                           */
  ts_status = BSP_TS_GetState(&TS_State);
  if(TS_State.touchDetected)
  {
    /* One or dual touch have been detected  */

    /* Erase previous information on touchscreen play pad area */
			BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
			//BSP_LCD_SetTextColor(colors[(touchscreen_color_idx++ % 24)]);
      //BSP_LCD_FillCircle(x1, y1, TS_MULTITOUCH_FOOTPRINT_CIRCLE_RADIUS);

      BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
      //BSP_LCD_SetFont(&Font16);
      //BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 70, (uint8_t *)"TOUCH INFO : ", CENTER_MODE);
			
			x1 = TS_State.touchX[0];
			y1 = TS_State.touchY[0];
      BSP_LCD_SetFont(&Font20);
      sprintf((char*)lcd_string, "x1 = %d, y1 = %d",
              x1,
              y1);
      BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 15, lcd_string, RIGHT_MODE, 1);
    //BSP_LCD_FillRect(0, 80, BSP_LCD_GetXSize(), BSP_LCD_GetYSize() - 160);

    /* Re-Draw touch screen play area on LCD */
    /*BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
    BSP_LCD_DrawRect(10, 90, BSP_LCD_GetXSize() - 20, BSP_LCD_GetYSize() - 180);
    BSP_LCD_DrawRect(11, 91, BSP_LCD_GetXSize() - 22, BSP_LCD_GetYSize() - 182);*/

    /* Erase previous information on bottom text bar */
   // BSP_LCD_FillRect(0, BSP_LCD_GetYSize() - 80, BSP_LCD_GetXSize(), 80);

    /* Desactivate drawing footprint of touch 1 and touch 2 until validated against boundaries of touch pad values */
    //drawTouch1 = drawTouch2 = 0;

    /* Get X and Y position of the first touch post calibrated */
   // x1 = TS_State.touchX[0];
    //y1 = TS_State.touchY[0];
    
    /*if((y1 > (90 + TS_MULTITOUCH_FOOTPRINT_CIRCLE_RADIUS)) &&
       (y1 < (BSP_LCD_GetYSize() - 90 - TS_MULTITOUCH_FOOTPRINT_CIRCLE_RADIUS)))
    {
      drawTouch1 = 1;
    }*/

    /* If valid touch 1 position : inside the reserved area for the use case : draw the touch */
    /*if(drawTouch1 == 1)
    {
      BSP_LCD_SetTextColor(colors[(touchscreen_color_idx++ % 24)]);
      BSP_LCD_FillCircle(x1, y1, TS_MULTITOUCH_FOOTPRINT_CIRCLE_RADIUS);

      BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
      //BSP_LCD_SetFont(&Font16);
      //BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 70, (uint8_t *)"TOUCH INFO : ", CENTER_MODE);

      BSP_LCD_SetFont(&Font12);
      sprintf((char*)lcd_string, "x1 = %d, y1 = %d",
              x1,
              y1);
      BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 45, lcd_string, CENTER_MODE);
    }*/ /* of if(drawTouch1 == 1) */

    /*if(TS_State.touchDetected > 1)
    {*/
      /* Get X and Y position of the second touch post calibrated */
      /*x2 = TS_State.touchX[1];
      y2 = TS_State.touchY[1];
      
      if((y2 > (90 + TS_MULTITOUCH_FOOTPRINT_CIRCLE_RADIUS)) &&
         (y2 < (BSP_LCD_GetYSize() - 90 - TS_MULTITOUCH_FOOTPRINT_CIRCLE_RADIUS)))
      {
        drawTouch2 = 1;
      }

     
      if(drawTouch2 == 1)
      {
        sprintf((char*)lcd_string, "x2 = %d, y2 = %d",
                x2,
                y2);
        BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 35, lcd_string, CENTER_MODE);

        //BSP_LCD_SetTextColor(colors[(touchscreen_color_idx++ % 24)]);
        //BSP_LCD_FillCircle(x2, y2, TS_MULTITOUCH_FOOTPRINT_CIRCLE_RADIUS);
      //} *//* of if(drawTouch2 == 1) */

//} /* of if(TS_State.touchDetected > 1) */
    /*if((drawTouch1 == 1) || (drawTouch2 == 1))
    {
      
      ts_status = BSP_TS_Get_GestureId(&TS_State);

      sprintf((char*)lcd_string, "Gesture Id = %s", ts_gesture_id_string_tab[TS_State.gestureId]);
      BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
      BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 15, lcd_string, CENTER_MODE);
    }
    else
    {
      BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
      BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 70, (uint8_t *)"Invalid touch position : use drawn touch area : ", CENTER_MODE);
    }*/
  } /* of if(TS_State.TouchDetected) */

  return(ts_status);
}

extern DMA2D_HandleTypeDef hdma2d_discovery;
extern LTDC_HandleTypeDef  hltdc_discovery;

//func used in LCD_DrawBitmap, from ST libs
static void ConvertLineToARGB8888(void *pSrc, void *pDst, uint32_t xSize, uint32_t ColorMode)
{
  /* Configure the DMA2D Mode, Color Mode and output offset */
  hdma2d_discovery.Init.Mode         = DMA2D_M2M_PFC;
  hdma2d_discovery.Init.ColorMode    = DMA2D_OUTPUT_ARGB8888;
  hdma2d_discovery.Init.OutputOffset = 0;

  /* Foreground Configuration */
  hdma2d_discovery.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  hdma2d_discovery.LayerCfg[1].InputAlpha = 0xFF;
  hdma2d_discovery.LayerCfg[1].InputColorMode = ColorMode;
  hdma2d_discovery.LayerCfg[1].InputOffset = 0;

  hdma2d_discovery.Instance = DMA2D;

  /* DMA2D Initialization */
  if(HAL_DMA2D_Init(&hdma2d_discovery) == HAL_OK)
  {
    if(HAL_DMA2D_ConfigLayer(&hdma2d_discovery, 1) == HAL_OK)
    {
      if (HAL_DMA2D_Start(&hdma2d_discovery, (uint32_t)pSrc, (uint32_t)pDst, xSize, 1) == HAL_OK)
      {
        /* Polling For DMA transfer */
        HAL_DMA2D_PollForTransfer(&hdma2d_discovery, 10);
      }
    }
  }
}


//Draw bitmap file (bmp)
void LCD_DrawBitmap(uint32_t Xpos, uint32_t Ypos, uint8_t *pbmp)
{
  uint32_t index = 0, width = 0, height = 0, bit_pixel = 0;
  uint32_t Address;
  uint32_t InputColorMode = 0;

  /* Get bitmap data address offset */
  index = pbmp[10] + (pbmp[11] << 8) + (pbmp[12] << 16)  + (pbmp[13] << 24);

  /* Read bitmap width */
  width = pbmp[18] + (pbmp[19] << 8) + (pbmp[20] << 16)  + (pbmp[21] << 24);

  /* Read bitmap height */
  height = pbmp[22] + (pbmp[23] << 8) + (pbmp[24] << 16)  + (pbmp[25] << 24);

  /* Read bit/pixel */
  bit_pixel = pbmp[28] + (pbmp[29] << 8);

  /* Set the address */
  Address = hltdc_discovery.LayerCfg[0].FBStartAdress + (((BSP_LCD_GetXSize()*Ypos) + Xpos)*(4));

  /* Get the layer pixel format */
  if ((bit_pixel/8) == 4)
  {
    InputColorMode = DMA2D_INPUT_ARGB8888;
  }
  else if ((bit_pixel/8) == 2)
  {
    InputColorMode = DMA2D_INPUT_RGB565;
  }
  else
  {
    InputColorMode = DMA2D_INPUT_RGB888;
  }

  /* Bypass the bitmap header */
  pbmp += (index + (width * (height - 1) * (bit_pixel/8)));

  /* Convert picture to ARGB8888 pixel format */
  for(index=0; index < height; index++)
  {
    /* Pixel format conversion */
    ConvertLineToARGB8888((uint32_t *)pbmp, (uint32_t *)Address, width, InputColorMode);

    /* Increment the source and destination buffers */
    Address+=  (BSP_LCD_GetXSize()*4);
    pbmp -= width*(bit_pixel/8);
  }
}


void ConvertBitmap(uint32_t Xpos, uint32_t Ypos, uint8_t *pbmp)
{
  uint32_t index = 0, width = 0, height = 0, bit_pixel = 0;
  uint32_t Address;
  uint32_t InputColorMode = 0;

  /* Get bitmap data address offset */
  index = pbmp[10] + (pbmp[11] << 8) + (pbmp[12] << 16)  + (pbmp[13] << 24);

  /* Read bitmap width */
  width = pbmp[18] + (pbmp[19] << 8) + (pbmp[20] << 16)  + (pbmp[21] << 24);

  /* Read bitmap height */
  height = pbmp[22] + (pbmp[23] << 8) + (pbmp[24] << 16)  + (pbmp[25] << 24);

  /* Read bit/pixel */
  bit_pixel = pbmp[28] + (pbmp[29] << 8);

  /* Set the address */
  Address = 0xC0700000;

  /* Get the layer pixel format */
  if ((bit_pixel/8) == 4)
  {
    InputColorMode = DMA2D_INPUT_ARGB8888;
  }
  else if ((bit_pixel/8) == 2)
  {
    InputColorMode = DMA2D_INPUT_RGB565;
  }
  else
  {
    InputColorMode = DMA2D_INPUT_RGB888;
  }

  /* Bypass the bitmap header */
  pbmp += (index + (width * (height - 1) * (bit_pixel/8)));

  /* Convert picture to ARGB8888 pixel format */
  for(index=0; index < height; index++)
  {
    /* Pixel format conversion */
    ConvertLineToARGB8888((uint32_t *)pbmp, (uint32_t *)Address, width, InputColorMode);

    /* Increment the source and destination buffers */
    Address+=  (BSP_LCD_GetXSize()*4);
    pbmp -= width*(bit_pixel/8);
  }
}

//Draw image with blending 
void DMA2D_DrawImage(uint32_t data, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
  uint32_t destination = LCD_FB_START_ADDRESS + (x + y * BSP_LCD_GetXSize()) * 4;

  DMA2D_HandleTypeDef hdma2d;
  hdma2d.Instance = DMA2D;

  hdma2d_discovery.Init.Mode = DMA2D_M2M_BLEND;
  hdma2d_discovery.Init.ColorMode = DMA2D_OUTPUT_ARGB8888;
  hdma2d_discovery.Init.OutputOffset = BSP_LCD_GetXSize() - width;

  // Foreground
  hdma2d_discovery.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  hdma2d_discovery.LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB8888;
  hdma2d_discovery.LayerCfg[1].InputOffset = 0;
  hdma2d_discovery.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA;
  //hdma2d_discovery.LayerCfg[1].InputAlpha = 0x80;
	
  // Background
  hdma2d_discovery.LayerCfg[0].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  hdma2d_discovery.LayerCfg[0].InputColorMode = DMA2D_INPUT_ARGB8888;
  hdma2d_discovery.LayerCfg[0].InputOffset = BSP_LCD_GetXSize() - width;

  HAL_DMA2D_Init(&hdma2d_discovery);
  HAL_DMA2D_ConfigLayer(&hdma2d_discovery, 1);
  HAL_DMA2D_ConfigLayer(&hdma2d_discovery, 0);
  HAL_DMA2D_BlendingStart(&hdma2d_discovery, data, destination, destination, width, height);
  HAL_DMA2D_PollForTransfer(&hdma2d_discovery, 10);
}


uint8_t b[16];
uint32_t b1[16];
uint32_t size=0;
uint16_t width=0;
uint16_t height=0;
unsigned long ex = 0;
FRESULT a;
uint32_t bytesread = 0;

//read image from .h from SD card
uint32_t ReadImage(uint32_t *ptr, const char* fname)
{
	FIL MyFile; 
	uint32_t ind=0,sz=0,i1=0,ind1=0;
	a = f_open(&MyFile,fname,FA_READ);
  if(a!=FR_OK)
  {
		return 1;
  }
  else
  {
		
      f_open(&MyFile,fname,FA_READ);
			f_read(&MyFile,b,5,(UINT *)&bytesread);
			width = (uint16_t)strtol((char*)b+1, NULL, 10);
			//width= 100*(b[1] - '0') + 10*(b[2] - '0') + (b[3] - '0');
			f_read(&MyFile,b,5,(UINT *)&bytesread);
			height = (uint16_t)strtol((char*)b+1, NULL, 10);
			//height= 100*(b[1] - '0') + 10*(b[2] - '0') + (b[3] - '0');
			f_read(&MyFile,b,8,(UINT *)&bytesread);

			size = (uint32_t)strtoll((char*)b+1, NULL, 10);
			//size= 10000*(b[1] - '0') + 1000*(b[2] - '0') + 100*(b[3] - '0') + 10*(b[4] - '0') + (b[5] - '0');
			f_read(&MyFile,b,2,(UINT *)&bytesread);
			for(uint32_t i=0;i<size;i++)
			{
				f_read(&MyFile,b,9,(UINT *)&bytesread);
				ex = strtoll((char*)b, NULL, 16);
				ptr[i]=ex;
				//f_read(&MyFile,b,1,(UINT *)&bytesread);
			}
			a = f_close(&MyFile);
		
		return 0;
  }
}



void DrawNumOnContainer(uint8_t num, uint8_t pos)
{
	if(pos == 1){
		switch (num){
			case 0: DMA2D_DrawImage((uint32_t)images[5].location, 173, 138, 44, 66);break;
			case 1: DMA2D_DrawImage((uint32_t)images[6].location, 173, 138, 41, 66);break;
			case 2: DMA2D_DrawImage((uint32_t)images[7].location, 173, 138, 41, 66);break;
			case 3: DMA2D_DrawImage((uint32_t)images[8].location, 173, 138, 41, 66);break;
			case 4: DMA2D_DrawImage((uint32_t)images[9].location, 173, 138, 46, 66);break;
			case 5: DMA2D_DrawImage((uint32_t)images[10].location, 173, 138, 42, 66);break;
			case 6: DMA2D_DrawImage((uint32_t)images[11].location, 173, 138, 42, 66);break;
			case 7: DMA2D_DrawImage((uint32_t)images[12].location, 173, 138, 42, 66);break;
			case 8: DMA2D_DrawImage((uint32_t)images[13].location, 173, 138, 42, 66);break;
			case 9: DMA2D_DrawImage((uint32_t)images[14].location, 173, 138, 42, 66);break;
		}
	}
	else if(pos == 2){
		switch (num){
			case 0: DMA2D_DrawImage((uint32_t)images[5].location, 219, 138, 44, 66);break;
			case 1: DMA2D_DrawImage((uint32_t)images[6].location, 219, 138, 41, 66);break;
			case 2: DMA2D_DrawImage((uint32_t)images[7].location, 219, 138, 41, 66);break;
			case 3: DMA2D_DrawImage((uint32_t)images[8].location, 219, 138, 41, 66);break;
			case 4: DMA2D_DrawImage((uint32_t)images[9].location, 219, 138, 46, 66);break;
			case 5: DMA2D_DrawImage((uint32_t)images[10].location, 219, 138, 42, 66);break;
			case 6: DMA2D_DrawImage((uint32_t)images[11].location, 219, 138, 42, 66);break;
			case 7: DMA2D_DrawImage((uint32_t)images[12].location, 219, 138, 42, 66);break;
			case 8: DMA2D_DrawImage((uint32_t)images[13].location, 219, 138, 42, 66);break;
			case 9: DMA2D_DrawImage((uint32_t)images[14].location, 219, 138, 42, 66);break;
		}
		//DMA2D_DrawImage((uint32_t)screens[6].location, 216, 123, 41, 66);
		
	}
	else if(pos == 3){
		switch (num){
			case 0: DMA2D_DrawImage((uint32_t)images[5].location, 343, 138, 44, 66);break;
			case 1: DMA2D_DrawImage((uint32_t)images[6].location, 343, 138, 41, 66);break;
			case 2: DMA2D_DrawImage((uint32_t)images[7].location, 343, 138, 41, 66);break;
			case 3: DMA2D_DrawImage((uint32_t)images[8].location, 343, 138, 41, 66);break;
			case 4: DMA2D_DrawImage((uint32_t)images[9].location, 343, 138, 46, 66);break;
			case 5: DMA2D_DrawImage((uint32_t)images[10].location, 343, 138, 42, 66);break;
			case 6: DMA2D_DrawImage((uint32_t)images[11].location, 343, 138, 42, 66);break;
			case 7: DMA2D_DrawImage((uint32_t)images[12].location, 343, 138, 42, 66);break;
			case 8: DMA2D_DrawImage((uint32_t)images[13].location, 343, 138, 42, 66);break;
			case 9: DMA2D_DrawImage((uint32_t)images[14].location, 343, 138, 42, 66);break;
		}
		//DMA2D_DrawImage((uint32_t)screens[7].location, 340, 123, 41, 66);
	}
	else if(pos == 4){
		switch (num){
			case 0: DMA2D_DrawImage((uint32_t)images[5].location, 389, 138, 44, 66);break;
			case 1: DMA2D_DrawImage((uint32_t)images[6].location, 389, 138, 41, 66);break;
			case 2: DMA2D_DrawImage((uint32_t)images[7].location, 389, 138, 41, 66);break;
			case 3: DMA2D_DrawImage((uint32_t)images[8].location, 389, 138, 41, 66);break;
			case 4: DMA2D_DrawImage((uint32_t)images[9].location, 389, 138, 46, 66);break;
			case 5: DMA2D_DrawImage((uint32_t)images[10].location, 389, 138, 42, 66);break;
			case 6: DMA2D_DrawImage((uint32_t)images[11].location, 389, 138, 42, 66);break;
			case 7: DMA2D_DrawImage((uint32_t)images[12].location, 389, 138, 42, 66);break;
			case 8: DMA2D_DrawImage((uint32_t)images[13].location, 389, 138, 42, 66);break;
			case 9: DMA2D_DrawImage((uint32_t)images[14].location, 389, 138, 42, 66);break;
		}
		//DMA2D_DrawImage((uint32_t)screens[8].location, 386, 123, 41, 66);
	}
	else if(pos == 5){
		switch (num){
			case 0: DMA2D_DrawImage((uint32_t)images[5].location, 516, 138, 44, 66);break;
			case 1: DMA2D_DrawImage((uint32_t)images[6].location, 516, 138, 41, 66);break;
			case 2: DMA2D_DrawImage((uint32_t)images[7].location, 516, 138, 41, 66);break;
			case 3: DMA2D_DrawImage((uint32_t)images[8].location, 516, 138, 41, 66);break;
			case 4: DMA2D_DrawImage((uint32_t)images[9].location, 516, 138, 46, 66);break;
			case 5: DMA2D_DrawImage((uint32_t)images[10].location, 516, 138, 42, 66);break;
			case 6: DMA2D_DrawImage((uint32_t)images[11].location, 516, 138, 42, 66);break;
			case 7: DMA2D_DrawImage((uint32_t)images[12].location, 516, 138, 42, 66);break;
			case 8: DMA2D_DrawImage((uint32_t)images[13].location, 516, 138, 42, 66);break;
			case 9: DMA2D_DrawImage((uint32_t)images[14].location, 516, 138, 42, 66);break;
		}
		//DMA2D_DrawImage((uint32_t)screens[9].location, 513, 123, 46, 66);
	}
	else if(pos == 6){
		switch (num){
			case 0: DMA2D_DrawImage((uint32_t)images[5].location, 562, 138, 44, 66);break;
			case 1: DMA2D_DrawImage((uint32_t)images[6].location, 562, 138, 41, 66);break;
			case 2: DMA2D_DrawImage((uint32_t)images[7].location, 562, 138, 41, 66);break;
			case 3: DMA2D_DrawImage((uint32_t)images[8].location, 562, 138, 41, 66);break;
			case 4: DMA2D_DrawImage((uint32_t)images[9].location, 562, 138, 46, 66);break;
			case 5: DMA2D_DrawImage((uint32_t)images[10].location, 562, 138, 42, 66);break;
			case 6: DMA2D_DrawImage((uint32_t)images[11].location, 562, 138, 42, 66);break;
			case 7: DMA2D_DrawImage((uint32_t)images[12].location, 562, 138, 42, 66);break;
			case 8: DMA2D_DrawImage((uint32_t)images[13].location, 562, 138, 42, 66);break;
			case 9: DMA2D_DrawImage((uint32_t)images[14].location, 562, 138, 42, 66);break;
		}
		//DMA2D_DrawImage((uint32_t)screens[10].location, 559, 123, 42, 66);
	}
}

void PrintFullness(uint8_t container,uint8_t perc)
{
	if(perc>=100)perc=99;
	uint8_t edin = perc%10;
	uint8_t dec = perc/10;
	switch (container){
		case 1:
			if(dec==0){DrawNumOnContainer(edin,2);break;}
			else{DrawNumOnContainer(dec,1);osDelay(3);DrawNumOnContainer(edin,2);break;}
		case 2:
			if(dec==0){DrawNumOnContainer(edin,4);break;}
			else{DrawNumOnContainer(dec,3);osDelay(3);DrawNumOnContainer(edin,4);break;}
		case 3:
			if(dec==0){DrawNumOnContainer(edin,6);break;}
			else{DrawNumOnContainer(dec,5);osDelay(3);DrawNumOnContainer(edin,6);break;}
	}
}

void DrawPercents()
{
				DMA2D_DrawImage((uint32_t)images[15].location, 263, 178, 23, 26);
				osDelay(5);
				DMA2D_DrawImage((uint32_t)images[15].location, 433, 178, 23, 26);
				osDelay(5);	
				DMA2D_DrawImage((uint32_t)images[15].location, 608, 178, 23, 26);
}

void get_touch_pos(uint16_t *x, uint16_t *y)
{
	TS_StateTypeDef  TS_State = {0};
	BSP_TS_GetState(&TS_State);
	*x = TS_State.touchX[0];
	*y = TS_State.touchY[0];
}

void print_touch_pos(uint16_t x, uint16_t y)
{
			uint8_t lcd_string[60] = "";
			sprintf((char*)lcd_string, "x1 = %d, y1 = %d",
              x,
              y);
			
			BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
      BSP_LCD_SetFont(&Font12);
      BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 15, lcd_string, RIGHT_MODE, 1);
}
