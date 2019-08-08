#include "Global.h"
#include <string.h>

#define DMA_Channels 3

uchar DMA_Interrupt[DMA_Channels] = { 0 };

void DMA_Init(void)
{
	DMACTL4 = DMARMWDIS;
	memset(DMA_Interrupt, 0, sizeof(DMA_Interrupt));
}

uchar DMA_QueryInterrupt(uchar Index)
{
	assert(Index < DMA_Channels);
	return DMA_Interrupt[Index];
}

void DMA_ClearInterrupt(uchar Index)
{
	assert(Index < DMA_Channels);
	DMA_Interrupt[Index] = 0;
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=DMA_VECTOR
__interrupt void DMA_ISR(void)
#elif defined(__GNUC__)
void __attribute__((interrupt(DMA_VECTOR))) DMA_ISR(void)
#else
#error Compiler not supported!
#endif
{
	uchar Index = DMAIV / 2 - 1;
	assert(Index < DMA_Channels);
	DMA_Interrupt[Index] = 1;
}
