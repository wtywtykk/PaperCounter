#ifndef _UARTTYPES_H_
#define _UARTTYPES_H_

typedef struct
{
	uchar EvenParity;
	uchar MSBFirst;
	uchar _7bit;
	uchar TwoStopBits;
}ParityInfo;

typedef enum
{
	NoError=0x00,
	FramingError=0x01,
	BufferOverrun=0x02,
	ParityError=0x04,
}UARTErrorType;

#endif