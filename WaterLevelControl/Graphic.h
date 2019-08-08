#pragma once

#define USEOLED 1

#if USEOLED
#define Graphic_ScreenWidth OLED_Width
#define Graphic_ScreenHeight OLED_Height
#else
#define Graphic_ScreenWidth EPD_Width
#define Graphic_ScreenHeight EPD_Height
#endif

void Graphic_Init(void);
void Graphic_Clear(void);
void Graphic_WriteChar(uchar X, uchar Y, char c, uchar CharVal, uchar BgVal);
void Graphic_WriteStr(uchar X, uchar Y, const char *Str, uchar CharVal, uchar BgVal);
void Graphic_EstimateStr(const char* Str, uchar* Width, uchar* Height);
void Graphic_BufferFulsh(void);
void Graphic_Fill(uchar X, uchar Y, uchar Width, uchar Height, uchar Val);
void Graphic_Bitblt(uchar* Src, uchar DstX, uchar DstY, uchar Width, uchar Height, uchar CharVal, uchar BgVal);
void Graphic_DrawLine(uchar X1, uchar Y1, uchar X2, uchar Y2, uchar Val);
void Graphic_DrawCircle(uchar X0, uchar Y0, uchar Radius, uchar Val);
void Graphic_SetPower(uchar On);
