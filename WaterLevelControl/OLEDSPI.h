#pragma once
#include "Global.h"

void OLEDSPI_ioinit(void);
void OLEDSPI_writecmd(unsigned char OLED_command);
void OLEDSPI_writedata(unsigned char OLED_data);
