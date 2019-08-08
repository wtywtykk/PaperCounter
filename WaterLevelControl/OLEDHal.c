#include "OLEDHal.h"

#define OLED_USESPI 0

#if OLED_USESPI
#include "OLEDSPI.h"
#define IOInit OLEDSPI_ioinit
#define WriteCmd OLEDSPI_writecmd
#define WriteData OLEDSPI_writedata
#else
#include "OLEDIIC.h"
#define IOInit OLEDIIC_ioinit
#define WriteCmd OLEDIIC_writecmd
#define WriteData OLEDIIC_writedata
#endif

void OLEDHal_SetPos(unsigned char x, unsigned char y)
{
	WriteCmd(0xb0 + y);
	//x = x + 2;
	WriteCmd(((x & 0xf0) >> 4) | 0x10);
	WriteCmd(x & 0x0f);
}

void OLEDHal_CLS(void)
{
	unsigned char y, x;
	for (y = 0; y < OLED_HeightLine; y++)
	{
		OLEDHal_SetPos(0, y);
		for (x = 0; x < OLED_Width; x++)
		{
			WriteData(0);
		}
	}
}

void OLEDHal_FILL(unsigned char Color)
{
	unsigned char y, x;
	for (y = 0; y < OLED_HeightLine; y++)
	{
		OLEDHal_SetPos(0, y);
		for (x = 0; x < OLED_Width; x++)
		{
			WriteData(Color ? 0xFF : 0);
		}
	}
}

void OLEDHal_WriteDataArray(unsigned char* Buf, unsigned int size)
{
	unsigned char y, x;
	for (y = 0; y < OLED_HeightLine; y++)
	{
		OLEDHal_SetPos(0, y);
		for (x = 0; x < OLED_Width; x++)
		{
			if (size == 0)
			{
				return;
			}
			WriteData(*Buf);
			size--;
			Buf++;
		}
	}
}

void OLEDHal_Init(void)
{
	IOInit();

	WriteCmd(0xAE);        //display off
	WriteCmd(0x20);      //Set Memory Addressing Mode
	WriteCmd(0x2);      //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	WriteCmd(0xb0);      //Set Page Start Address for Page Addressing Mode,0-7
	WriteCmd(0xc8);      //Set COM Output Scan Direction
	WriteCmd(0x00);     //---set low column address
	WriteCmd(0x10);     //---set high column address
	WriteCmd(0x40);     //--set start line address
	WriteCmd(0x81);     //--set contrast control register
	WriteCmd(0x7f);
	WriteCmd(0xa1);     //--set segment re-map 0 to 127
	WriteCmd(0xa6);     //--set normal display
	WriteCmd(0xa8);     //--set multiplex ratio(1 to 64)
	WriteCmd(0x3F);     //
	WriteCmd(0xa4);     //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	WriteCmd(0xd3);     //-set display offset
	WriteCmd(0x00);     //-not offset
	WriteCmd(0xd5);     //--set display clock divide ratio/oscillator frequency
	WriteCmd(0xf0);     //--set divide ratio
	WriteCmd(0xd9);     //--set pre-charge period
	WriteCmd(0x22);      //
	WriteCmd(0xda);     //--set com pins hardware configuration
	WriteCmd(0x12);
	WriteCmd(0xdb);     //--set vcomh
	WriteCmd(0x20);     //0x20,0.77xVcc
	WriteCmd(0x8d);     //--set DC-DC enable
	WriteCmd(0x14);     //
	WriteCmd(0xaf);     //--turn on oled panel
	OLEDHal_CLS();
	OLEDHal_SetPos(0, 0);
}
