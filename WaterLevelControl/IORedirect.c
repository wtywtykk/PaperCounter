#include "Global.h"
#include "UART.h"
#include <string.h>

int putchar(int character)
{
	UART1_SendByte(character);
	return character;
}

int _write(int fd, char *pBuffer, int size)
{
	UART1_SendBytes(pBuffer, size);
	return 1;
}

int _isatty(int fd)
{
	return 1;
}

int _read(int fd, char *pBuffer, int size)
{
	char* UARTBuf = NULL;
	while (UART1_QueryDataSize() == 0);
	return UART1_RecvBytes(pBuffer, size, NULL);
}
