#pragma once

typedef void(*PORT_ISR)(void);

void PortInterrupts_Init(void);
void PortInterrupts_Register(uchar Port, uchar Pin, PORT_ISR NewISR);
