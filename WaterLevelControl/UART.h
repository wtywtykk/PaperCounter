#ifndef _UART1_H_
#define _UART1_H_

#include "UARTTypes.h"

void UART1_Init(ulong Baudrate, ParityInfo* Parity);
void UART1_SendByte(uchar Data);
void UART1_SendBytes(uchar* Data, uint DataSize);
void UART1_SendString(uchar* Str);
ushort UART1_RecvBytes(uchar* Buf, ushort DataSize, UARTErrorType* Error);
ushort UART1_QueryDataSize(void);

#endif