#include "Global.h"
#include "Clock.h"

#define WaitTime 10000

static uchar FailCount = 0;

void HardIIC_Init(void)
{
	ushort i = 0;
	ushort DeviceID = 0;

	P4SEL &= ~(BIT1 | BIT2);
	P4REN &= ~(BIT1 | BIT2);
	P4DIR &= ~(BIT1 | BIT2);
	P4OUT &= ~(BIT1 | BIT2);
	for (i = 0; i < 1000; i++)
	{
		__delay_cycles(100);
		P4DIR |= BIT2;
		__delay_cycles(100);
		P4DIR &= ~BIT2;
	}

	P4DIR &= ~(BIT1 | BIT2);
	P4OUT |= BIT1 | BIT2;
	P4REN |= BIT1 | BIT2;
	P4SEL |= BIT1 | BIT2;

	UCB1CTL1 |= UCSWRST;
	UCB1CTL0 = UCMST | UCMODE_3 | UCSYNC;
	UCB1CTL1 = UCSSEL__SMCLK | UCSWRST;
	UCB1BRW = Clock_Query(SMCLK) / 100000;
	UCB1I2COA = 0;
	UCB1I2CSA = 0x2A;
	UCB1IE = 0;
	UCB1IFG = 0;
	UCB1CTL1 &= ~UCSWRST;
}

void HardIIC_FailCheck(void)
{
	FailCount++;
	if (FailCount > 10)
	{
		FailCount = 0;
		HardIIC_Init();
	}
}

uchar HardIIC_WaitTx(void)
{
	ulong WaitCount = 0;
	do
	{
		WaitCount++;
		if ((WaitCount > WaitTime) || (UCB1IFG & (UCNACKIFG | UCALIFG)))
		{
			UCB1IFG = 0;
			HardIIC_FailCheck();
			return 0;
		}
	} while (!(UCB1IFG & UCTXIFG));
	return 1;
}

uchar HardIIC_Tx(uchar Val)
{
	ulong WaitCount = 0;
	do
	{
		WaitCount++;
		if ((WaitCount > WaitTime) || (UCB1IFG & (UCNACKIFG | UCALIFG)))
		{
			UCB1IFG = 0;
			HardIIC_FailCheck();
			return 0;
		}
	} while (!(UCB1IFG & UCTXIFG));
	UCB1TXBUF = Val;
	return 1;
}

uchar HardIIC_Rx(uchar* Val)
{
	ulong WaitCount = 0;
	do
	{
		WaitCount++;
		if ((WaitCount > WaitTime) || (UCB1IFG & (UCNACKIFG | UCALIFG)))
		{
			UCB1IFG = 0;
			HardIIC_FailCheck();
			return 0;
		}
	} while (!(UCB1IFG & UCRXIFG));
	*Val = UCB1RXBUF;
	return 1;
}

uchar HardIIC_Read(uchar RegAddr, uchar* Buf, ushort Len)
{
	uchar Ret = 0;
	ulong WaitCount = 0;
	UCB1CTL1 |= UCTR;
	UCB1CTL1 |= UCTXSTT;
	if (HardIIC_Tx(RegAddr))
	{
		if (HardIIC_WaitTx())
		{
			ushort i = 0;
			Ret = 1;
			UCB1CTL1 &= ~UCTR;
			UCB1CTL1 |= UCTXSTT;
			__disable_interrupt();
			while (i < Len - 1 && Ret)
			{
				Ret &= HardIIC_Rx(&Buf[i]);
				i++;
			}
			if (Ret)
			{
				UCB1CTL1 |= UCTXSTP;
				Ret &= HardIIC_Rx(&Buf[i]);
			}
			else
			{
				UCB1CTL1 |= UCTXSTP;
			}
			__enable_interrupt();
		}
	}
	while (UCB1CTL1 & UCTXSTP)
	{
		WaitCount++;
		if (WaitCount > WaitTime)
		{
			Ret = 0;
			HardIIC_FailCheck();
		}
	}
	return Ret;
}

uchar HardIIC_Write(uchar RegAddr, uchar* Buf, ushort Len)
{
	uchar Ret = 0;
	ulong WaitCount = 0;
	UCB1CTL1 |= UCTR;
	UCB1CTL1 |= UCTXSTT;
	if (HardIIC_Tx(RegAddr))
	{
		ushort i = 0;
		Ret = 1;
		while (i < Len && Ret)
		{
			Ret &= HardIIC_Tx(Buf[i]);
			i++;
		}
	}
	UCB1CTL1 |= UCTXSTP;
	while (UCB1CTL1 & UCTXSTP)
	{
		WaitCount++;
		if (WaitCount > WaitTime)
		{
			Ret = 0;
			HardIIC_FailCheck();
		}
	}
	return Ret;
}
