#include "Global.h"

typedef enum
{
	DCO,
	XT1,
	XT2,
	REFO,
	VLO,
	DCODIV
}ClockSource;

typedef enum
{
	SMCLK,
	MCLK,
	ACLK
}Clocks;

typedef struct
{
	ulong DCOFreq;
	uint DCODivider;
	ulong XT1Freq;
	ulong XT2Freq;
	ClockSource MCLKSource;
	uint MCLKDivide;
	ClockSource SMCLKSource;
	uint SMCLKDivide;
	ClockSource ACLKSource;
	uint ACLKDivide;

}Clock;

uchar Clock_Init(Clock* Ctrl);
ulong Clock_Query(Clocks QueryType);
