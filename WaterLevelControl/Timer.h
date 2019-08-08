#pragma once
#include "Global.h"

typedef void(*TimerCallback)(void);

typedef enum
{
	Timer1ms = 0,
	Timer10ms,
	Timer100ms,
	Timer1s,
}TimerPeriod;

void InitTimer(void);
void PauseTimer(void);
void StartTimer(void);
void ClearTimer(void);
ulong GetTickSafe(void);
void Delay(ulong Interval);
uchar Timer_Register(TimerPeriod Period, TimerCallback Callback);
uchar Timer_Unregister(TimerPeriod Period, TimerCallback Callback);
