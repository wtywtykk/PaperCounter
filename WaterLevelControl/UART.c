#include "Global.h"
#include "UART.h"
#include "Clock.h"
#include <string.h>

/****************************************************************/
#define PORTSEL	P4SEL
#define PORTTXD	BIT4
#define PORTRXD	BIT5

#define CTL0	UCA1CTL0
#define CTL1	UCA1CTL1
#define BR		UCA1BRW
#define MCTL	UCA1MCTL
#define STAT	UCA1STAT
#define TXBUF	UCA1TXBUF
#define RXBUF	UCA1RXBUF
#define IE		UCA1IE
#define IFG		UCA1IFG
#define IV		UCA1IV

#define BufferSize (64)
/****************************************************************/

uchar UARTRecvBuffer[BufferSize];
ushort WriteCounter = 0;
ushort ReadCounter = 0;
UARTErrorType UARTError = NoError;

void UART1_InitPort(void)
{
	PORTSEL |= PORTTXD | PORTRXD;
}

void UART1_InitBuffer(void)
{
	WriteCounter = 0;
	ReadCounter = 0;
	UARTError = NoError;
}

void UART1_Init(ulong Baudrate, ParityInfo* Parity)
{
	ulong BRCLK = Clock_Query(SMCLK);
	double DivFactor = (double)BRCLK / Baudrate;
	uchar Oversampling = DivFactor > 64 ? 1 : 0;//TODO:Choose proper pred

	UART1_InitPort();
	UART1_InitBuffer();

	CTL1 |= UCSWRST;//**Put state machine in reset**
	CTL1 |= UCSSEL__SMCLK | UCRXEIE;//SMCLK,receive erroneous characters
	if (Oversampling)
	{
		uchar BRF;
		BR = (int)(DivFactor / 16);
		BRF = (int)((DivFactor / 16 - (int)(DivFactor / 16)) * 16);
		MCTL = UCOS16 | BRF << 4;
	}
	else
	{
		uchar BRS;
		BR = (int)DivFactor;
		BRS = (int)((DivFactor - (int)DivFactor) * 8);
		MCTL = BRS << 1;
	}

	CTL0 = 0;
	if (Parity)
	{
		CTL0 |= UCPEN;
		if (Parity->EvenParity)
		{
			CTL0 |= UCPAR;
		}
		if (Parity->MSBFirst)
		{
			CTL0 |= UCMSB;
		}
		if (Parity->_7bit)
		{
			CTL0 |= UC7BIT;
		}
		if (Parity->TwoStopBits)
		{
			CTL0 |= UCSPB;
		}
	}

	STAT = 0;
	CTL1 &= ~UCSWRST;//**Initialize USCI state machine**
	IE |= UCRXIE;//Enable RX interrupt
}

void UART1_SendByte(uchar Data)
{
	while (!(IFG & UCTXIFG));//TX buffer ready?
	TXBUF = Data;
}

void UART1_SendBytes(uchar* Data, uint DataSize)
{
	while (DataSize--)
	{
		UART1_SendByte(*(Data++));
	}
}

void UART1_SendString(uchar* Str)
{
	if (Str)
	{
		while (*Str)
		{
			UART1_SendByte(*(Str++));
		}
	}
}

ushort UART1_RecvBytes(uchar* Buf, ushort DataSize, UARTErrorType* Error)
{
	ushort BufferDataSize;
	if (DataSize > BufferSize - 1)
	{
		DataSize = BufferSize - 1;
	}
	__disable_interrupt();

	if (ReadCounter <= WriteCounter)
	{
		BufferDataSize = WriteCounter - ReadCounter;
	}
	else
	{
		BufferDataSize = BufferSize + WriteCounter - ReadCounter;
	}

	if (BufferDataSize < DataSize)
	{
		DataSize = BufferDataSize;
	}

	memcpy(Buf, &UARTRecvBuffer[ReadCounter], DataSize);
	if (Error)
	{
		*Error = UARTError;
	}
	UARTError = NoError;

	ReadCounter = (ReadCounter + DataSize) % BufferSize;

	__enable_interrupt();

	return DataSize;
}

ushort UART1_QueryDataSize(void)
{
	ushort Ret;
	__disable_interrupt();
	if (ReadCounter <= WriteCounter)
	{
		Ret = WriteCounter - ReadCounter;
	}
	else
	{
		Ret = BufferSize + WriteCounter - ReadCounter;
	}
	__enable_interrupt();
	return Ret;
}

/****************************************************************/
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
#elif defined(__GNUC__)
void __attribute__((interrupt(USCI_A1_VECTOR))) USCI_A1_ISR(void)
#else
#error Compiler not supported!
#endif
/****************************************************************/
{
	switch (IV)
	{
	case 0://Vector 0 - no interrupt
		break;
	case 2://Vector 2 - RXIFG
	{
		ushort NextWriteCounter = (WriteCounter + 1) % BufferSize;
		if (NextWriteCounter == ReadCounter)
		{
			UARTError |= BufferOverrun;
		}
		else
		{
			if (STAT&UCFE)
			{
				UARTError |= FramingError;
			}
			if (STAT&UCOE)
			{
				UARTError |= BufferOverrun;
			}
			if (STAT&UCPE)
			{
				UARTError |= ParityError;
			}
			UARTRecvBuffer[WriteCounter] = RXBUF;
			WriteCounter = NextWriteCounter;
		}
	}
	break;
	case 4://Vector 4 - TXIFG
		break;
	default:
		break;
	}
}
