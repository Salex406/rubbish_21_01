#include "gui.h"
#include "stm32f7xx_hal.h"
#include "stm32f769i_discovery_lcd.h"
#include "stm32f769i_discovery_ts.h"
#include "stdio.h"
#include "string.h"
#include "mechanics.h"

#include "../Components/ft6x06/ft6x06.h"

extern DMA2D_HandleTypeDef hdma2d;
extern LTDC_HandleTypeDef hltdc;
extern struct Image images[20];
extern const uint16_t NumOnContainerY;

extern uint8_t* dma2d_in1;
extern uint8_t* dma2d_in2;

void TFT_DrawBitmapToMem(uint32_t Xpos, uint32_t Ypos, uint8_t *pbmp, uint8_t *pdst)
{
	uint32_t index=0, width=0, height=0, bit_pixel=0;
	/* Get bitmap data addres offset*/
	index = *(__IO uint16_t *) (pbmp+10);
	index |= (*(__IO uint16_t *) (pbmp+12)) << 16;
	/* Read bitmap width */
	width = *(__IO uint16_t *) (pbmp+18);
	width |= (*(__IO uint16_t *) (pbmp+20)) << 16;
	/* Read bitmap height */
	height = *(__IO uint16_t *) (pbmp+22);
	height |= (*(__IO uint16_t *) (pbmp+24)) << 16;
	/* Read bit/pixel */
	bit_pixel = *(__IO uint16_t *) (pbmp+28);
	/* Bypass the bitmap header */
	pbmp += (index + (width * (height-1) * (bit_pixel/8)));
	if((bit_pixel/8) == 4)
	{
		TFT_FillScreen(0xFFFF0000);
	}
	else if((bit_pixel/8) == 2)
	{
		TFT_FillScreen(0xFF00FF00);
	}
	else
	{
		/* Convert picture to ARGB8888 pixel format */
		for(index=0; index < height; index++)
		{
			hdma2d.Init.Mode = DMA2D_M2M_PFC;
			hdma2d.Init.ColorMode = DMA2D_ARGB8888;
			hdma2d.Init.OutputOffset = 0;
			hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
			hdma2d.LayerCfg[1].InputAlpha = 0xFF;
			hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB888;
			hdma2d.LayerCfg[1].InputOffset = 0;
			if(HAL_DMA2D_Init(&hdma2d) == HAL_OK)
			{
				if(HAL_DMA2D_ConfigLayer(&hdma2d, 1) == HAL_OK)
				{
					if(HAL_DMA2D_Start(&hdma2d, (uint32_t) pbmp, (uint32_t) pdst, width, 1) == HAL_OK)
					{
						HAL_DMA2D_PollForTransfer(&hdma2d, 10);
					}
				}
			}
			pdst += 800*4;
			pbmp -= width*(bit_pixel/8);
		}
	}
  hdma2d.Init.Mode = DMA2D_M2M_BLEND;
  hdma2d.Init.ColorMode = DMA2D_OUTPUT_ARGB8888;
  hdma2d.Init.OutputOffset = 0;
  hdma2d.LayerCfg[1].InputOffset = 0;
  hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB8888;
  hdma2d.LayerCfg[1].AlphaMode = DMA2D_REPLACE_ALPHA;
  hdma2d.LayerCfg[1].InputAlpha = 0;
  hdma2d.LayerCfg[0].InputOffset = 0;
  hdma2d.LayerCfg[0].InputColorMode = DMA2D_INPUT_ARGB8888;
  hdma2d.LayerCfg[0].AlphaMode = DMA2D_REPLACE_ALPHA;
  hdma2d.LayerCfg[0].InputAlpha = 0;
  if (HAL_DMA2D_Init(&hdma2d) == HAL_OK)
	{
		HAL_DMA2D_ConfigLayer(&hdma2d, 0);
		HAL_DMA2D_ConfigLayer(&hdma2d, 1);
	}
	bit_pixel = 0;
}

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

void FillTriangle(uint16_t x1, uint16_t x2, uint16_t x3, uint16_t y1, uint16_t y2, uint16_t y3)
{
  int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0,
  yinc1 = 0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0,
  curpixel = 0;

  deltax = ABS(x2 - x1);        /* The difference between the x's */
  deltay = ABS(y2 - y1);        /* The difference between the y's */
  x = x1;                       /* Start x off at the first pixel */
  y = y1;                       /* Start y off at the first pixel */

  if (x2 >= x1)                 /* The x-values are increasing */
  {
    xinc1 = 1;
    xinc2 = 1;
  }
  else                          /* The x-values are decreasing */
  {
    xinc1 = -1;
    xinc2 = -1;
  }

  if (y2 >= y1)                 /* The y-values are increasing */
  {
    yinc1 = 1;
    yinc2 = 1;
  }
  else                          /* The y-values are decreasing */
  {
    yinc1 = -1;
    yinc2 = -1;
  }

  if (deltax >= deltay)         /* There is at least one x-value for every y-value */
  {
    xinc1 = 0;                  /* Don't change the x when numerator >= denominator */
    yinc2 = 0;                  /* Don't change the y for every iteration */
    den = deltax;
    num = deltax / 2;
    numadd = deltay;
    numpixels = deltax;         /* There are more x-values than y-values */
  }
  else                          /* There is at least one y-value for every x-value */
  {
    xinc2 = 0;                  /* Don't change the x for every iteration */
    yinc1 = 0;                  /* Don't change the y when numerator >= denominator */
    den = deltay;
    num = deltay / 2;
    numadd = deltax;
    numpixels = deltay;         /* There are more y-values than x-values */
  }

  for (curpixel = 0; curpixel <= numpixels; curpixel++)
  {
    BSP_LCD_DrawLine(x, y, x3, y3);

    num += numadd;              /* Increase the numerator by the top of the fraction */
    if (num >= den)             /* Check if numerator >= denominator */
    {
      num -= den;               /* Calculate the new numerator value */
      x += xinc1;               /* Change the x as appropriate */
      y += yinc1;               /* Change the y as appropriate */
    }
    x += xinc2;                 /* Change the x as appropriate */
    y += yinc2;                 /* Change the y as appropriate */
  }
}

uint8_t LoadImagesFromSdToRAM()
{
	FATFS SDFatFs;  /* File system object for SD card logical drive */
	char SD_Path[4]; /* SD card logical drive path */
	//from main.c
	/*extern uint8_t NumberOfOdjectsToLoadFromSD;
	extern uint16_t LoadProgressBarPosX;
	extern uint16_t LoadProgressBarPosY;
	extern uint16_t LoadProgressBarLength;
	extern uint8_t LoadProgressBarWidth;*/
	
	BSP_LCD_SetFont(&rus48);
	BSP_LCD_DisplayStringAt(0, 235, (uint8_t*)StringLoad, CENTER_MODE, 1); //"Loading.."
	BSP_LCD_SetFont(&Font20);
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
				placePrBar(LoadProgressBarPosX,LoadProgressBarPosY,LoadProgressBarLength,LoadProgressBarWidth,pr,PROGRESSBAR_HORIZONTAL,LCD_COLOR_ORANGEBUTTON);
			
				//load image to RAM from .bmp file
				uint8_t res = OpenBMP((uint8_t *)images[0].location,images[0].filename);
				images[0].width = 800;
				images[0].height = 480;
			
				if(res==1){ TFT_FillScreen(LCD_COLOR_WHITE);BSP_LCD_DisplayStringAt(0,240,(uint8_t*)"File 1 open error",CENTER_MODE, 1); Error_Handler();}
				pr+=step;
				placePrBar(LoadProgressBarPosX,LoadProgressBarPosY,LoadProgressBarLength,LoadProgressBarWidth,pr,PROGRESSBAR_HORIZONTAL,LCD_COLOR_ORANGEBUTTON);
				
				//load images to RAM from .h files
				for(uint8_t i=1;i<NumberOfOdjectsToLoadFromSD;i++)
				{
					if(i==17){
						i=17;
					}
					
					res = ReadImage(images[i].location,images[i].filename, i);
					if(res==1)
					{
						TFT_FillScreen(LCD_COLOR_WHITE);
						char* error_str;
						sprintf(error_str,"File %d open error. Pls reboot and check SD card.",i+1);
						Error_Handler();
					}
						pr+=step;
						placePrBar(LoadProgressBarPosX,LoadProgressBarPosY,LoadProgressBarLength,LoadProgressBarWidth,pr,PROGRESSBAR_HORIZONTAL,LCD_COLOR_ORANGEBUTTON);
				}
		}
		else { TFT_FillScreen(LCD_COLOR_WHITE);BSP_LCD_SetFont(&rus48);BSP_LCD_DisplayStringAt(0,240,(uint8_t*)StringMountError,CENTER_MODE, 1); Error_Handler();}
	}
	else { TFT_FillScreen(LCD_COLOR_WHITE);BSP_LCD_SetFont(&rus48);BSP_LCD_DisplayStringAt(30,240,(uint8_t*)StringNoSD,CENTER_MODE, 1); Error_Handler();}
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
uint32_t ex = 0;
FRESULT a;
uint32_t bytesread = 0;

//read image from .h from SD card
uint32_t ReadImage(uint32_t *ptr, const char* fname, uint8_t i)
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
			//width = (uint16_t)strtol((char*)b+1, NULL, 10);
			width= 100*(b[1] - '0') + 10*(b[2] - '0') + (b[3] - '0');
			f_read(&MyFile,b,5,(UINT *)&bytesread);
			images[i].width = width;
			//height = (uint16_t)strtol((char*)b+1, NULL, 10);
			height= 100*(b[1] - '0') + 10*(b[2] - '0') + (b[3] - '0');
			images[i].height = height;
			f_read(&MyFile,b,8,(UINT *)&bytesread);

			size = (uint32_t)strtoll((char*)b+1, NULL, 10);
			//size= 10000*(b[1] - '0') + 1000*(b[2] - '0') + 100*(b[3] - '0') + 10*(b[4] - '0') + (b[5] - '0');
			f_read(&MyFile,b,2,(UINT *)&bytesread);
			for(uint32_t i=0;i<size;i++)
			{
				f_read(&MyFile,b,9,(UINT *)&bytesread);
				ex = (uint32_t)strtoul((char*)b, NULL, 16);
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
			case 0: DMA2D_DrawImage((uint32_t)images[5].location, 163, NumOnContainerY, 44, 66);break;
			case 1: DMA2D_DrawImage((uint32_t)images[6].location, 163, NumOnContainerY, 41, 66);break;
			case 2: DMA2D_DrawImage((uint32_t)images[7].location, 163, NumOnContainerY, 41, 66);break;
			case 3: DMA2D_DrawImage((uint32_t)images[8].location, 163, NumOnContainerY, 41, 66);break;
			case 4: DMA2D_DrawImage((uint32_t)images[9].location, 163, NumOnContainerY, 46, 66);break;
			case 5: DMA2D_DrawImage((uint32_t)images[10].location, 163, NumOnContainerY, 42, 66);break;
			case 6: DMA2D_DrawImage((uint32_t)images[11].location, 163, NumOnContainerY, 42, 66);break;
			case 7: DMA2D_DrawImage((uint32_t)images[12].location, 163, NumOnContainerY, 42, 66);break;
			case 8: DMA2D_DrawImage((uint32_t)images[13].location, 163, NumOnContainerY, 42, 66);break;
			case 9: DMA2D_DrawImage((uint32_t)images[14].location, 163, NumOnContainerY, 42, 66);break;
		}
	}
	else if(pos == 2){
		switch (num){
			case 0: DMA2D_DrawImage((uint32_t)images[5].location, 208, NumOnContainerY, 44, 66);break;
			case 1: DMA2D_DrawImage((uint32_t)images[6].location, 208, NumOnContainerY, 41, 66);break;
			case 2: DMA2D_DrawImage((uint32_t)images[7].location, 208, NumOnContainerY, 41, 66);break;
			case 3: DMA2D_DrawImage((uint32_t)images[8].location, 208, NumOnContainerY, 41, 66);break;
			case 4: DMA2D_DrawImage((uint32_t)images[9].location, 208, NumOnContainerY, 46, 66);break;
			case 5: DMA2D_DrawImage((uint32_t)images[10].location, 208, NumOnContainerY, 42, 66);break;
			case 6: DMA2D_DrawImage((uint32_t)images[11].location, 208, NumOnContainerY, 42, 66);break;
			case 7: DMA2D_DrawImage((uint32_t)images[12].location, 208, NumOnContainerY, 42, 66);break;
			case 8: DMA2D_DrawImage((uint32_t)images[13].location, 208, NumOnContainerY, 42, 66);break;
			case 9: DMA2D_DrawImage((uint32_t)images[14].location, 208, NumOnContainerY, 42, 66);break;
		}
		//DMA2D_DrawImage((uint32_t)screens[6].location, 216, 123, 41, 66);
		
	}
	else if(pos == 3){
		switch (num){
			case 0: DMA2D_DrawImage((uint32_t)images[5].location, 343, NumOnContainerY, 44, 66);break;
			case 1: DMA2D_DrawImage((uint32_t)images[6].location, 343, NumOnContainerY, 41, 66);break;
			case 2: DMA2D_DrawImage((uint32_t)images[7].location, 343, NumOnContainerY, 41, 66);break;
			case 3: DMA2D_DrawImage((uint32_t)images[8].location, 343, NumOnContainerY, 41, 66);break;
			case 4: DMA2D_DrawImage((uint32_t)images[9].location, 343, NumOnContainerY, 46, 66);break;
			case 5: DMA2D_DrawImage((uint32_t)images[10].location, 343, NumOnContainerY, 42, 66);break;
			case 6: DMA2D_DrawImage((uint32_t)images[11].location, 343, NumOnContainerY, 42, 66);break;
			case 7: DMA2D_DrawImage((uint32_t)images[12].location, 343, NumOnContainerY, 42, 66);break;
			case 8: DMA2D_DrawImage((uint32_t)images[13].location, 343, NumOnContainerY, 42, 66);break;
			case 9: DMA2D_DrawImage((uint32_t)images[14].location, 343, NumOnContainerY, 42, 66);break;
		}
		//DMA2D_DrawImage((uint32_t)screens[7].location, 340, 123, 41, 66);
	}
	else if(pos == 4){
		switch (num){
			case 0: DMA2D_DrawImage((uint32_t)images[5].location, 389, NumOnContainerY, 44, 66);break;
			case 1: DMA2D_DrawImage((uint32_t)images[6].location, 389, NumOnContainerY, 41, 66);break;
			case 2: DMA2D_DrawImage((uint32_t)images[7].location, 389, NumOnContainerY, 41, 66);break;
			case 3: DMA2D_DrawImage((uint32_t)images[8].location, 389, NumOnContainerY, 41, 66);break;
			case 4: DMA2D_DrawImage((uint32_t)images[9].location, 389, NumOnContainerY, 46, 66);break;
			case 5: DMA2D_DrawImage((uint32_t)images[10].location, 389, NumOnContainerY, 42, 66);break;
			case 6: DMA2D_DrawImage((uint32_t)images[11].location, 389, NumOnContainerY, 42, 66);break;
			case 7: DMA2D_DrawImage((uint32_t)images[12].location, 389, NumOnContainerY, 42, 66);break;
			case 8: DMA2D_DrawImage((uint32_t)images[13].location, 389, NumOnContainerY, 42, 66);break;
			case 9: DMA2D_DrawImage((uint32_t)images[14].location, 389, NumOnContainerY, 42, 66);break;
		}
		//DMA2D_DrawImage((uint32_t)screens[8].location, 386, 123, 41, 66);
	}
	else if(pos == 5){
		switch (num){
			case 0: DMA2D_DrawImage((uint32_t)images[5].location, 516, NumOnContainerY, 44, 66);break;
			case 1: DMA2D_DrawImage((uint32_t)images[6].location, 516, NumOnContainerY, 41, 66);break;
			case 2: DMA2D_DrawImage((uint32_t)images[7].location, 516, NumOnContainerY, 41, 66);break;
			case 3: DMA2D_DrawImage((uint32_t)images[8].location, 516, NumOnContainerY, 41, 66);break;
			case 4: DMA2D_DrawImage((uint32_t)images[9].location, 516, NumOnContainerY, 46, 66);break;
			case 5: DMA2D_DrawImage((uint32_t)images[10].location, 516, NumOnContainerY, 42, 66);break;
			case 6: DMA2D_DrawImage((uint32_t)images[11].location, 516, NumOnContainerY, 42, 66);break;
			case 7: DMA2D_DrawImage((uint32_t)images[12].location, 516, NumOnContainerY, 42, 66);break;
			case 8: DMA2D_DrawImage((uint32_t)images[13].location, 516, NumOnContainerY, 42, 66);break;
			case 9: DMA2D_DrawImage((uint32_t)images[14].location, 516, NumOnContainerY, 42, 66);break;
		}
		//DMA2D_DrawImage((uint32_t)screens[9].location, 513, 123, 46, 66);
	}
	else if(pos == 6){
		switch (num){
			case 0: DMA2D_DrawImage((uint32_t)images[5].location, 562, NumOnContainerY, 44, 66);break;
			case 1: DMA2D_DrawImage((uint32_t)images[6].location, 562, NumOnContainerY, 41, 66);break;
			case 2: DMA2D_DrawImage((uint32_t)images[7].location, 562, NumOnContainerY, 41, 66);break;
			case 3: DMA2D_DrawImage((uint32_t)images[8].location, 562, NumOnContainerY, 41, 66);break;
			case 4: DMA2D_DrawImage((uint32_t)images[9].location, 562, NumOnContainerY, 46, 66);break;
			case 5: DMA2D_DrawImage((uint32_t)images[10].location, 562, NumOnContainerY, 42, 66);break;
			case 6: DMA2D_DrawImage((uint32_t)images[11].location, 562, NumOnContainerY, 42, 66);break;
			case 7: DMA2D_DrawImage((uint32_t)images[12].location, 562, NumOnContainerY, 42, 66);break;
			case 8: DMA2D_DrawImage((uint32_t)images[13].location, 562, NumOnContainerY, 42, 66);break;
			case 9: DMA2D_DrawImage((uint32_t)images[14].location, 562, NumOnContainerY, 42, 66);break;
		}
		//DMA2D_DrawImage((uint32_t)screens[10].location, 559, 123, 42, 66);
	}
}

void drawArrow(uint16_t x, uint16_t y,uint16_t length)
{
	uint8_t tr_h = 20;
	uint8_t width  =50;
	BSP_LCD_SetTextColor(LCD_COLOR_ORANGEBUTTON);
	TFT_FillRectangle(x, y, x + length, y + width, LCD_COLOR_ORANGEBUTTON);
	FillTriangle(x + length, x + length, x+tr_h+length,y,y+width,y+width/2);
	
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
				DMA2D_DrawImage((uint32_t)images[15].location, 258, 158, 23, 26);
				osDelay(5);
				DMA2D_DrawImage((uint32_t)images[15].location, 433, 158, 23, 26);
				osDelay(5);	
				DMA2D_DrawImage((uint32_t)images[15].location, 609, 158, 23, 26);
} 

void get_touch_pos(uint16_t *x, uint16_t *y)
{
	TS_StateTypeDef  TS_State = {0};
	BSP_TS_GetState_mod(&TS_State);
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
char plastic[14];
char metal[14];
char glass[14];

uint8_t processCode(uint8_t* code)
{
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
  BSP_LCD_SetFont(&Font12);
  BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 25, code, RIGHT_MODE, 1);
	strcpy(plastic, "4606224000121");
	strcpy(metal, "4600823094017");
	strcpy(glass, "5060214370028");
	if(strcmp((char*)code,plastic)==0) return 1;
	else if(strcmp((char*)code,metal)==0) return 2;
	else if(strcmp((char*)code,glass)==0) return 3;
}

void DMA2D_LayersAlphaReconfig(uint32_t alpha1, uint32_t alpha2)
{
  hdma2d_discovery.LayerCfg[1].InputAlpha = alpha1;
  hdma2d_discovery.LayerCfg[0].InputAlpha = alpha2;
	HAL_DMA2D_ConfigLayer(&hdma2d, 1);
	HAL_DMA2D_ConfigLayer(&hdma2d, 0);
}

void HandleDebugTouch(uint16_t x, uint16_t y)
{
	extern HR4988_DriverTypeDef XDriver;
	uint8_t shift = 235;
	uint16_t shift_r = 430;
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	BSP_LCD_SetFont(&Font20);
	if(x > 200 && x < 280 && y > 0 && y < 60)
	{
		//press up
		BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 40, (uint8_t*)"press -> up     ", LEFT_MODE, 1);
		HAL_GPIO_WritePin(PDOWN_PORT,PDOWN_Pin,GPIO_PIN_SET);
		HAL_GPIO_WritePin(PUP_PORT,PUP_Pin,GPIO_PIN_RESET);
	}
	else if(x > 200 && x < 280 && y > 100 && y < 160)
	{
		//press down
		BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 40, (uint8_t*)"press -> down     ", LEFT_MODE, 1);
		HAL_GPIO_WritePin(PDOWN_PORT,PDOWN_Pin,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(PUP_PORT,PUP_Pin,GPIO_PIN_SET);
	}
	else if(x > 360 && x < 400 && y > 50 && y < 120)
	{
		//press stop
		HAL_GPIO_WritePin(PDOWN_PORT,PDOWN_Pin,GPIO_PIN_SET);
		HAL_GPIO_WritePin(PUP_PORT,PUP_Pin,GPIO_PIN_SET);
		BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 40, (uint8_t*)"press stop      ", LEFT_MODE, 1);
	}
	
	else if(x > 200 && x < 280 && y > 240 && y < 285)
	{
		//carriage left
		BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 40, (uint8_t*)"carriage -> left", LEFT_MODE, 1);
		HR4988_RunMotor(&XDriver,CW_DIR);
	}
	else if(x > 200 && x < 280 && y > 340 && y < 380)
	{
		//carriage right
		BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 40, (uint8_t*)"carriage -> right", LEFT_MODE, 1);
		HR4988_RunMotor(&XDriver,CW_DIR);
	}
	else if(x > 360 && x < 400 && y > 270 && y < 320)
	{
		//carriage stop
		BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 40, (uint8_t*)"carriage stop   ", LEFT_MODE, 1);
		HR4988_StopMotor(&XDriver );
	}
	
	else if(x > 640 && x < 800 && y > 240 && y < 280)
	{
		//stopor up
		BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 40, (uint8_t*)"stopor -> up    ", LEFT_MODE, 1);
		HAL_GPIO_WritePin(STOPOR_PORT,STOPOR_Pin,GPIO_PIN_SET);
	}
	else if(x > 640 && x < 800 && y > 325 && y < 380)
	{
		//stopor down
		BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 40, (uint8_t*)"stopor -> down  ", LEFT_MODE, 1);
		HAL_GPIO_WritePin(STOPOR_PORT,STOPOR_Pin,GPIO_PIN_RESET);
	}
}
