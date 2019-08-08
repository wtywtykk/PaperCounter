#pragma once
#include "Global.h"
#include "OLED.h"

#define XLevelL           0x00
#define XLevelH           0x10
#define XLevel            ((XLevelH&0x0F)*16+XLevelL)
#define Brightness        0xCF

void OLEDHal_SetPos(unsigned char x, unsigned char y);
void OLEDHal_CLS(void);
void OLEDHal_FILL(unsigned char Color);
void OLEDHal_WriteDataArray(unsigned char* Buf, unsigned int size);
void OLEDHal_Init(void);
