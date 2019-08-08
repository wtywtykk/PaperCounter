#pragma once

#include "Global.h"

#define CalibrationCount 10

typedef struct
{
	float LevelCalibration[CalibrationCount];
	ushort Key;
}InfoBlock;

extern InfoBlock Info;

void LoadInfo(void); 
void EraseInfo(void);
void SaveInfo();
