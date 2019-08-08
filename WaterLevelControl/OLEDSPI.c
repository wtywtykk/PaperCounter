#include "OLEDSPI.h"

#define SCL_HIGH()    P6OUT|=BIT0
#define SCL_LOW()     P6OUT&=~BIT0
#define SDA_HIGH()    P6OUT|=BIT1
#define SDA_LOW()     P6OUT&=~BIT1
#define RES_HIGH()    P6OUT|=BIT2
#define RES_LOW()     P6OUT&=~BIT2
#define DC_HIGH()     P6OUT|=BIT3
#define DC_LOW()      P6OUT&=~BIT3
#define CS_HIGH()     P6OUT|=BIT4
#define CS_LOW()      P6OUT&=~BIT4

void OLEDSPI_delayus(unsigned int i)
{
	unsigned int j, k;
	for (k = 0; k < i; k++)
		for (j = 0; j < 4; j++);
}

void OLEDSPI_ioinit()
{
	P6DIR |= BIT0 | BIT1 | BIT2 | BIT3 | BIT4;
	SCL_LOW();
	SDA_LOW();
	RES_LOW();
	DC_LOW();
	CS_HIGH();
	RES_HIGH();
}

void OLEDSPI_writecmd(unsigned char OLED_command)
{
	unsigned char i;
	CS_LOW();
	DC_LOW();
	for (i = 0; i < 8; i++)
	{
		SCL_LOW();
		if (OLED_command & 0x80)
		{
			SDA_HIGH();
		}
		else
		{
			SDA_LOW();
		}
		OLED_command <<= 1;
		SCL_HIGH();
	}
	SCL_LOW();
	CS_HIGH();
}

void OLEDSPI_writedata(unsigned char OLED_data)
{
	unsigned char i;
	CS_LOW();
	DC_HIGH();
	for (i = 0; i < 8; i++)
	{
		SCL_LOW();
		if (OLED_data & 0x80)
		{
			SDA_HIGH();
		}
		else
		{
			SDA_LOW();
		}
		OLED_data <<= 1;
		SCL_HIGH();
	}
	SCL_LOW();
	CS_HIGH();
}
