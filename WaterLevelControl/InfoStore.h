#pragma once

#include "Global.h"

#define CalibrationSegmentCount 2

typedef struct
{
	float OpenCap;
	float Boundary[CalibrationSegmentCount-1];
	float k[CalibrationSegmentCount];
	float b[CalibrationSegmentCount];
	ushort Key;
}InfoBlock;

extern InfoBlock Info;

void LoadInfo(void); 
void EraseInfo(void);
void SaveInfo();
