#include "Global.h"
#include "Timer.h"
#include "PortInterrupts.h"
#include "BoardKey.h"

#define AntiGlitchTime 500

ulong LeftBtnLastTick = 0;
ulong RightBtnLastTick = 0;

uchar KeyFlag = 0;

void BoardKey_LeftIntr(void);
void BoardKey_RightIntr(void);

void BoardKey_Init(void)
{
	P2DIR &= ~(1 << 1);
	P2OUT |= 1 << 1;
	P2REN |= 1 << 1;
	P2IES |= 1 << 1;
	P2IE |= 1 << 1;
	P2IFG = 0;
	P1DIR &= ~(1 << 1);
	P1OUT |= 1 << 1;
	P1REN |= 1 << 1;
	P1IES |= 1 << 1;
	P1IE |= 1 << 1;
	P1IFG = 0;

	PortInterrupts_Register(2, 1, &BoardKey_LeftIntr);
	PortInterrupts_Register(1, 1, &BoardKey_RightIntr);

	LeftBtnLastTick = GetTickSafe();
	RightBtnLastTick = LeftBtnLastTick;
	KeyFlag = 0;
}

uchar BoardKey_GetLastKey(void)
{
	uchar Flag = KeyFlag;
	KeyFlag = 0;
	return Flag;
}

void BoardKey_LeftIntr(void)
{
	ulong CurTick = GetTickSafe();
	if (CurTick > LeftBtnLastTick + AntiGlitchTime)
	{
		LeftBtnLastTick = CurTick;
		KeyFlag = BOARDKEY_LEFT;
	}
}

void BoardKey_RightIntr(void)
{
	ulong CurTick = GetTickSafe();
	if (CurTick > RightBtnLastTick + AntiGlitchTime)
	{
		RightBtnLastTick = CurTick;
		KeyFlag = BOARDKEY_RIGHT;
	}
}
