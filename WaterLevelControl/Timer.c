#include "Global.h"
#include "Clock.h"
#include "Timer.h"
#include <string.h>

#define MaxCallbackCount 10

volatile ulong Ticks = 0;
TimerCallback CallbackList[4][MaxCallbackCount + 1] = { 0 };

void InitTimer(void)
{
	ulong MaxCount = Clock_Query(SMCLK) / 1000 - 1;//1/1000s=1ms
	TA0CTL = MC__STOP | ID__1 | TASSEL__SMCLK | TAIE;
	TA0CCTL0 = 0;
	TA0CCTL1 = 0;
	TA0CCTL2 = 0;
	TA0CCTL3 = 0;
	TA0CCTL4 = 0;
	TA0CCR0 = MaxCount;
	TA0EX0 = TAIDEX_0;
	TA0CTL |= TACLR;
	Ticks = 0;
	memset(CallbackList, 0, sizeof(CallbackList));
}

void PauseTimer(void)
{
	TA0CTL &= ~MC__UP;
}

void StartTimer(void)
{
	TA0CTL |= MC__UP;
}

void ClearTimer(void)
{
	TA0CTL |= TACLR;
	Ticks = 0;
}

ulong GetTickSafe(void)
{
	ulong CurTick;
	do
	{
		CurTick = Ticks;
	} while (CurTick != Ticks);
	return CurTick;
}

void Delay(ulong Interval)
{
	ulong EndTick = GetTickSafe() + Interval;
	while (GetTickSafe() < EndTick);
}

uchar Timer_Register(TimerPeriod Period, TimerCallback Callback)
{
	uchar i = 0;
	assert(Period < 4);
	__disable_interrupt();
	for (i = 0; i < MaxCallbackCount; i++)
	{
		if (CallbackList[Period][i] == Callback)
		{
			return 1;
		}
		if (CallbackList[Period][i] == NULL)
		{
			CallbackList[Period][i] = Callback;
			CallbackList[Period][i + 1] = 0;
			__enable_interrupt();
			return 1;
		}
	}
	__enable_interrupt();
	return 0;
}

uchar Timer_Unregister(TimerPeriod Period, TimerCallback Callback)
{
	uchar i = 0;
	assert(Period < 4);
	__disable_interrupt();
	for (i = 0; i < MaxCallbackCount; i++)
	{
		if (CallbackList[Period][i] == Callback)
		{
			uchar j = i;
			for (j = 0; j < MaxCallbackCount + 1; j++)
			{
				CallbackList[Period][i] = CallbackList[Period][i + 1];
			}
			__enable_interrupt();
			return 1;
		}
	}
	__enable_interrupt();
	return 0;
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER0_A1_VECTOR
__interrupt void TIMERA0_ISR(void)
#elif defined(__GNUC__)
void __attribute__((interrupt(TIMER0_A1_VECTOR))) TIMERA0_ISR(void)
#else
#error Compiler not supported!
#endif
{
	uchar i = 0;
	Ticks++;
	for (i = 0; CallbackList[0][i] != NULL; i++)
	{
		CallbackList[0][i]();
	}
	if (Ticks % 10 == 1)
	{
		for (i = 0; CallbackList[1][i] != NULL; i++)
		{
			CallbackList[1][i]();
		}
	}
	if (Ticks % 100 == 10)
	{
		for (i = 0; CallbackList[2][i] != NULL; i++)
		{
			CallbackList[2][i]();
		}
	}
	if (Ticks % 1000 == 100)
	{
		for (i = 0; CallbackList[3][i] != NULL; i++)
		{
			CallbackList[3][i]();
		}
	}
	TA0IV = 0;
}
