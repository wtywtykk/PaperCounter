#pragma once
#include "Global.h"

void OLEDIIC_ioinit(void);
void OLEDIIC_writecmd(unsigned char OLED_command);
void OLEDIIC_writedata(unsigned char OLED_data);
