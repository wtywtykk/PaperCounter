#include "Global.h"
#include "OLEDBuffered.h"
#include <string.h>

static uchar ImageBuffer[OLED_HeightLine][OLED_Width];
static uint BufferX;
static uint BufferYLine;

#define PixelYAddrFromY(Y) ((Y)/8)
#define PixelBitMaskFromY(Y) (1<<(Y)%8)

uchar OLEDBuffered_BufferRead(uchar X, uchar YLine)
{
	assert(X < OLED_Width && YLine < OLED_HeightLine);
	return ImageBuffer[YLine][X];
}

void OLEDBuffered_BufferWrite(uchar X, uchar YLine, uchar Val)
{
	assert(X < OLED_Width && YLine < OLED_HeightLine);
	ImageBuffer[YLine][X] = Val;
}

uchar OLEDBuffered_BufferReadBit(uchar X, uchar Y)
{
	assert(X < OLED_Width && Y < OLED_Height);
	return ImageBuffer[PixelYAddrFromY(Y)][X] & PixelBitMaskFromY(Y);
}

void OLEDBuffered_BufferWriteBit(uchar X, uchar Y, uchar Val)
{
	uchar Mask = PixelBitMaskFromY(Y);
	assert(X < OLED_Width && Y < OLED_Height);
	Val = Val ? Mask : 0;
	ImageBuffer[PixelYAddrFromY(Y)][X] = (ImageBuffer[PixelYAddrFromY(Y)][X] & ~Mask) | Val;
}

void OLEDBuffered_BufferFulsh(void)
{
	OLEDHal_SetPos(0, 0);
	OLEDHal_WriteDataArray(ImageBuffer[0], sizeof(ImageBuffer));
}

void OLEDBuffered_Init(void)
{
	OLEDHal_Init();

	memset(ImageBuffer, 0, sizeof(ImageBuffer));
	BufferX = 0;
	BufferYLine = 0;
}

void OLEDBuffered_Clear(void)
{
	memset(ImageBuffer, 0, sizeof(ImageBuffer));
}

void OLEDBuffered_SetPos(uchar X, uchar YLine)
{
	assert(X < OLED_Width && YLine < OLED_HeightLine);

	BufferX = X;
	BufferYLine = YLine;
}

void OLEDBuffered_WriteData(uchar Data)
{
	OLEDBuffered_BufferWrite(BufferX, BufferYLine, Data);
	BufferX++;
	if (BufferX == OLED_Width)
	{
		BufferX = 0;
		BufferYLine++;
		if (BufferYLine == OLED_HeightLine)
		{
			BufferYLine = 0;
		}
	}
}
