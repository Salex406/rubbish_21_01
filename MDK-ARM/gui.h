#include "main.h"
#include "fatfs.h"
#include "stdlib.h"
#define swap(a,b) {int16_t t=a;a=b;b=t;}
#define FRAMEBUFER_ADDR 0xC0000000

#define PROGRESSBAR_HORIZONTAL 0
#define PROGRESSBAR_VERTICAL 1
#define LCD_COLOR_GREENCONTAINER 0xFF00b800
#define LCD_COLOR_ORANGEBUTTON 0xFFFF7F00

#define ABS(X)                 ((X) > 0 ? (X) : -(X))

void TFT_FillRectangle(uint16_t x1, uint16_t y1,uint16_t x2, uint16_t y2, uint32_t color);
void placePrBar(uint16_t x, uint16_t y, uint16_t length, uint16_t width, uint8_t progress,uint8_t type,uint32_t color);
void TFT_FillScreen(uint32_t color);
void DMA2D_DrawImage(uint32_t data, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
void DMA2D_DrawImage_simple(uint32_t data, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
void LCD_DrawBitmap(uint32_t Xpos, uint32_t Ypos, uint8_t *pbmp);
void LCD_DrawBitmap_bl(uint32_t Xpos, uint32_t Ypos, uint8_t *pbmp);
void ConvertBitmap(uint32_t Xpos, uint32_t Ypos, uint8_t *pbmp);
void DrawNumOnContainer(uint8_t num, uint8_t pos);
uint32_t ReadImage(uint32_t *ptr, const char* fname, uint8_t i);
void DrawPercents(void);
void PrintFullness(uint8_t container,uint8_t perc);
void get_touch_pos(uint16_t *x, uint16_t *y);
void print_touch_pos(uint16_t x, uint16_t y);
uint8_t LoadImagesFromSdToRAM(void);
uint8_t processCode(uint8_t* code);
void drawArrow(uint16_t x, uint16_t y,uint16_t length);
void DMA2D_LayersAlphaReconfig(uint32_t alpha1, uint32_t alpha2);
void TFT_DrawBitmapToMem(uint32_t Xpos, uint32_t Ypos, uint8_t *pbmp, uint8_t *pdst);
void HandleDebugTouch(uint16_t x, uint16_t y);
