#include "Global.h"
#include "PortInterrupts.h"
#include <string.h>

void DefaultPortISR(void);

PORT_ISR ISRs[2][9] = { &DefaultPortISR };

void DefaultPortISR(void)
{
}

void PortInterrupts_Init(void)
{
	uchar i = 0;
	for (i = 0; i < 2; i++)
	{
		uchar j = 0;
		for (j = 0; j < 9; j++)
		{
			ISRs[i][j] = &DefaultPortISR;
		}
	}
}

void PortInterrupts_Register(uchar Port, uchar Pin, PORT_ISR NewISR)
{
	assert(Port >= 1 && Port <= 2 && Pin <= 7);
	if (NewISR)
	{
		ISRs[Port - 1][Pin + 1] = NewISR;
	}
	else
	{
		ISRs[Port - 1][Pin + 1] = &DefaultPortISR;
	}
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=PORT1_VECTOR
__interrupt void PORT1_ISR(void)
#elif defined(__GNUC__)
void __attribute__((interrupt(PORT1_VECTOR))) PORT1_ISR(void)
#else
#error Compiler not supported!
#endif
{
	ISRs[0][P1IV / 2]();
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=PORT2_VECTOR
__interrupt void PORT2_ISR(void)
#elif defined(__GNUC__)
void __attribute__((interrupt(PORT2_VECTOR))) PORT2_ISR(void)
#else
#error Compiler not supported!
#endif
{
	ISRs[1][P2IV / 2]();
}
