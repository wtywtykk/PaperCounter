#pragma once
#include "Global.h"

#define OLED_HeightLine 8
#define OLED_Height (OLED_HeightLine*8)
#define OLED_Width 128

void OLED_Init(void);
void OLED_Clear(void);
void OLED_SetPos(uchar X, uchar YLine);
void OLED_WriteRaw(uchar Data);
void OLED_WriteChar(uchar X, uchar Y, char c, uchar Val);
void OLED_WriteStr(uchar X, uchar Y, const char *Str, uchar Val);
void OLED_EstimateStr(const char* Str, uchar* Width, uchar* Height);
void OLED_BufferFulsh(void);
void OLED_Fill(uchar X, uchar Y, uchar Width, uchar Height, uchar Val);
void OLED_Bitblt(uchar* Src, uchar DstX, uchar DstY, uchar Width, uchar Height);
void OLED_DrawLine(uchar X1, uchar Y1, uchar X2, uchar Y2, uchar Val);
void OLED_DrawCircle(uchar X0, uchar Y0, uchar Radius, uchar Val);
