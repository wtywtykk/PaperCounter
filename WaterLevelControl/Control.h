#pragma once

void Control_Init(void);
void Control_Process(void);
void Control_Disp(void);
void Control_Set(float NewLevel);
float Control_Get(void);
void Control_EnableCalibration(uchar Val);
void Control_SetForceMode(char Val);
