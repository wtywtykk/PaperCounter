#include "Global.h"
#include "OLEDBuffered.h"
#include "code_table.h"
#include <string.h>

void OLED_Init(void)
{
	OLEDBuffered_Init();
	OLEDBuffered_Clear();
}

void OLED_Clear(void)
{
	OLEDBuffered_Clear();
}

void OLED_SetPos(uchar X, uchar YLine)
{
	assert(X < OLED_Width && YLine < OLED_HeightLine);
	if (X < OLED_Width && YLine < OLED_HeightLine)
	{
		OLEDBuffered_SetPos(X, YLine);
	}
}

void OLED_WriteRaw(uchar Data)
{
	OLEDBuffered_WriteData(Data);
}

void OLED_WriteCharAligned(char c, uchar Val)
{
	uchar i;
	c -= 32;
	for (i = 0; i < 6; i++)
	{
		OLEDBuffered_WriteData(Val ? Asc6x8_OLED[c][i] : ~Asc6x8_OLED[c][i]);
	}
}

void OLED_WriteCharUnaligned(uchar X, uchar Y, char c, uchar Val)
{
	uchar i;
	c -= 32;
	for (i = 0; i < 6; i++)
	{
		uchar ColData = Val ? Asc6x8_OLED[c][i] : ~Asc6x8_OLED[c][i];
		uchar j;
		for (j = 0; j < 8; j++)
		{
			OLEDBuffered_BufferWriteBit(X + i, Y + j, ColData & 0x1);
			ColData >>= 1;
		}
	}
}

void OLED_WriteChar(uchar X, uchar Y, char c, uchar Val)
{
	assert(X < OLED_Width && Y < OLED_Height);
	if (X < OLED_Width && Y < OLED_Height)
	{
		if (Y % 8)
		{
			OLED_WriteCharUnaligned(X, Y, c, Val);
		}
		else
		{
			OLEDBuffered_SetPos(X, Y / 8);
			OLED_WriteCharAligned(c, Val);
		}
	}
}

void OLED_WriteStrAligned(uchar X, uchar YLine, const char *Str, uchar Val)
{
	assert(X < OLED_Width && YLine < OLED_HeightLine);
	OLEDBuffered_SetPos(X, YLine);
	while (*Str)
	{
		OLED_WriteCharAligned(*Str, Val);
		Str++;
	}
}

void OLED_WriteStrUnaligned(uchar X, uchar Y, const char *Str, uchar Val)
{
	assert(X < OLED_Width && Y < OLED_Height);
	while (*Str)
	{
		if (X > OLED_Width - 6)
		{
			Y += 8;
			X = 0;
		}
		if (Y > OLED_Height - 8)
		{
			Y = 0;
		}
		OLED_WriteCharUnaligned(X, Y, *Str, Val);
		Str++;
		X += 6;
	}
}

void OLED_WriteStr(uchar X, uchar Y, const char *Str, uchar Val)
{
	assert(X < OLED_Width && Y < OLED_Height);
	if (X < OLED_Width && Y < OLED_Height && Str)
	{
		if (Y % 8)
		{
			OLED_WriteStrUnaligned(X, Y, Str, Val);
		}
		else
		{
			OLED_WriteStrAligned(X, Y / 8, Str, Val);
		}
	}
}

void OLED_EstimateStr(const char* Str, uchar* Width, uchar* Height)
{
	if (Width)
	{
		*Width = strlen((char*)Str) * 6;
	}
	if (Height)
	{
		*Height = 8;
	}
}

void OLED_BufferFulsh(void)
{
	OLEDBuffered_BufferFulsh();
}

void OLED_Fill(uchar X, uchar Y, uchar Width, uchar Height, uchar Val)
{
	uchar CurY, DestY, CntX, DestX;
	CurY = Y;
	DestX = X + Width;
	DestY = Y + Height;
	if (X > OLED_Width || Y > OLED_Height)
	{
		return;
	}
	if (DestX > OLED_Width)
	{
		DestX = OLED_Width;
	}
	if (DestY > OLED_Height)
	{
		DestY = OLED_Height;
	}
	while (CurY % 8 != 0 && CurY < DestY)
	{
		for (CntX = X; CntX < DestX; CntX++)
		{
			OLEDBuffered_BufferWriteBit(CntX, CurY, Val);
		}
		CurY++;
	}
	while (DestY - CurY > 8)
	{
		for (CntX = X; CntX < DestX; CntX++)
		{
			OLEDBuffered_BufferWrite(CntX, CurY / 8, Val ? 0xFF : 0x00);
		}
		CurY += 8;
	}
	while (CurY < DestY)
	{
		for (CntX = X; CntX < DestX; CntX++)
		{
			OLEDBuffered_BufferWriteBit(CntX, CurY, Val);
		}
		CurY++;
	}
}

void OLED_Bitblt(uchar* Src, uchar DstX, uchar DstY, uchar Width, uchar Height)
{
	uint X, Y;
	uchar* SrcPtr = Src;
	for (Y = 0; Y < Height; Y++)
	{
		uint PixelCounter = 0;
		for (X = 0; X < Width; X++)
		{
			ushort PixelX = (ushort)X + DstX;
			ushort PixelY = (ushort)Y + DstY;
			if (PixelX < OLED_Width && PixelY < OLED_Height)
			{
				OLEDBuffered_BufferWriteBit(PixelX, PixelY, 0x80 & (*SrcPtr << PixelCounter));
			}
			PixelCounter++;
			if (PixelCounter == 8)
			{
				PixelCounter = 0;
				SrcPtr++;
			}
		}
		if (PixelCounter)
		{
			SrcPtr++;
		}
	}
}

void OLED_DrawLine(uchar X1, uchar Y1, uchar X2, uchar Y2, uchar Val)
{
	short xerr = 0, yerr = 0, delta_x, delta_y, distance;
	short incx, incy, uRow, uCol;
	ushort i = 0;
	delta_x = X2 - X1;
	delta_y = Y2 - Y1;
	uRow = X1;
	uCol = Y1;
	if (delta_x > 0)
	{
		incx = 1;
	}
	else if (delta_x == 0)
	{
		incx = 0;
	}
	else
	{
		incx = -1;
		delta_x = -delta_x;
	}
	if (delta_y > 0)
	{
		incy = 1;
	}
	else if (delta_y == 0)
	{
		incy = 0;
	}
	else
	{
		incy = -1;
		delta_y = -delta_y;
	}
	if (delta_x > delta_y)
	{
		distance = delta_x;
	}
	else
	{
		distance = delta_y;
	}
	for (i = 0; i <= distance + 1; i++)
	{
		OLEDBuffered_BufferWriteBit(uRow, uCol, Val);
		xerr += delta_x;
		yerr += delta_y;
		if (xerr > distance)
		{
			xerr -= distance;
			uRow += incx;
		}
		if (yerr > distance)
		{
			yerr -= distance;
			uCol += incy;
		}
	}
}

void OLED_DrawCircle(uchar X0, uchar Y0, uchar Radius, uchar Val)
{
	//Midpoint circle algorithm
	//https://en.wikipedia.org/wiki/Midpoint_circle_algorithm
	short X = Radius;
	short Y = 0;
	short err = 0;

	while (X >= Y)
	{
		OLEDBuffered_BufferWriteBit(X0 + X, Y0 + Y, Val);
		OLEDBuffered_BufferWriteBit(X0 + Y, Y0 + X, Val);
		OLEDBuffered_BufferWriteBit(X0 - Y, Y0 + X, Val);
		OLEDBuffered_BufferWriteBit(X0 - X, Y0 + Y, Val);
		OLEDBuffered_BufferWriteBit(X0 - X, Y0 - Y, Val);
		OLEDBuffered_BufferWriteBit(X0 - Y, Y0 - X, Val);
		OLEDBuffered_BufferWriteBit(X0 + Y, Y0 - X, Val);
		OLEDBuffered_BufferWriteBit(X0 + X, Y0 - Y, Val);

		if (err <= 0)
		{
			Y += 1;
			err += 2 * Y + 1;
		}
		if (err > 0)
		{
			X -= 1;
			err -= 2 * X + 1;
		}
	}
}
