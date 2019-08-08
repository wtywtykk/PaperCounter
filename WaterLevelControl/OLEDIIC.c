#include "OLEDIIC.h"

#define SCL_HIGH()    P6OUT|=BIT0
#define SCL_LOW()     P6OUT&=~BIT0
#define SDA_HIGH()    P6OUT|=BIT5
#define SDA_LOW()     P6OUT&=~BIT5

void OLEDIIC_delayus(unsigned int i)
{
	unsigned int j, k;
	for (k = 0; k < i; k++)
		for (j = 0; j < 4; j++);
}

void OLEDIIC_ioinit()
{
	P6DIR |= BIT0 | BIT5;
	P6REN |= BIT0 | BIT5;
}

void OLEDIIC_start()
{
	SCL_HIGH();
	SDA_HIGH();
	//OLEDIIC_delayus(2);
	SDA_LOW();
	//OLEDIIC_delayus(2);
	SCL_LOW();
}

void OLEDIIC_stop()
{
	SCL_LOW();
	SDA_LOW();
	//IIC_delayus(2);
	SCL_HIGH();
	SDA_HIGH();
	//IIC_delayus(2);
}

void OLEDIIC_writebyte(unsigned char OLED_byte)
{
	unsigned char i;
	for (i = 0; i < 8; i++)
	{
		if (OLED_byte & 0x80)
			SDA_HIGH();
		else
			SDA_LOW();
		SCL_HIGH();
		SCL_LOW();
		OLED_byte <<= 1;
	}
	SDA_HIGH();
	SCL_HIGH();
	SCL_LOW();
}

void OLEDIIC_writecmd(unsigned char OLED_command)
{
	OLEDIIC_start();
	OLEDIIC_writebyte(0x78);    //Slave address,SA0=0
	OLEDIIC_writebyte(0x00);    //write command
	OLEDIIC_writebyte(OLED_command);
	OLEDIIC_stop();
}

void OLEDIIC_writedata(unsigned char OLED_data)
{
	OLEDIIC_start();
	OLEDIIC_writebyte(0x78);
	OLEDIIC_writebyte(0x40);    //write data
	OLEDIIC_writebyte(OLED_data);
	OLEDIIC_stop();
}
