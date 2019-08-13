#include "Global.h"
#include "OLED.h"

static uchar NumberSeg[13] = { 0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x80,0x40,0x79 };//0123456789.-E
/*
 --      0
|  |   5   1
 --      6
|  |   4   2
 -- .    3  7
*/

uchar DigitDisp_DispChar(uchar x, uchar y, uchar width, uchar height, uchar stroke, char Chr)
{
	uchar Mask = 0;
	uchar CharWidth = 0;
	if (Chr >= '0' && Chr <= '9')
	{
		Mask = NumberSeg[Chr - '0'];
		CharWidth = width + stroke;
	}
	else if (Chr == '.')
	{
		Mask = NumberSeg[10];
		CharWidth = stroke + stroke;
	}
	else if (Chr == '-')
	{
		Mask = NumberSeg[11];
		CharWidth = width + stroke;
	}
	else if (Chr == 'E')
	{
		Mask = NumberSeg[12];
		CharWidth = width + stroke;
	}
	OLED_Fill(x + stroke, y, width - 2 * stroke, stroke, Mask & (1 << 0));
	OLED_Fill(x + width - stroke, y + stroke, stroke, (height - 3 * stroke) / 2, Mask & (1 << 1));
	OLED_Fill(x + width - stroke, y + stroke + (height - 3 * stroke) / 2 + stroke, stroke, (height - 3 * stroke) / 2, Mask & (1 << 2));
	OLED_Fill(x + stroke, y + height - stroke, width - 2 * stroke, stroke, Mask & (1 << 3));
	OLED_Fill(x, y + stroke + (height - 3 * stroke) / 2 + stroke, stroke, (height - 3 * stroke) / 2, Mask & (1 << 4));
	OLED_Fill(x, y + stroke, stroke, (height - 3 * stroke) / 2, Mask & (1 << 5));
	OLED_Fill(x + stroke, y + (height - 3 * stroke) / 2 + stroke, width - 2 * stroke, stroke, Mask & (1 << 6));
	OLED_Fill(x, y + height - stroke, stroke, stroke, Mask & (1 << 7));

	return CharWidth;
}

void DigitDisp_DispStr(uchar x, uchar y, uchar width, uchar height, uchar stroke, const char* Str)
{
	while (*Str)
	{
		x += DigitDisp_DispChar(x, y, width, height, stroke, *Str);
		Str++;
	}
}
