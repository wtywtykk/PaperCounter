#include "Global.h"
#include "Timer.h"
#include "OLED.h"
#include "BoardKey.h"
#include "FDC2214.h"
#include <stdio.h>
#include <math.h>
#include "InfoStore.h"

//#define FDC_Debug 1

#define PulseDuty 8000
#define PulseDutyCompensate 9000
#define ChannelMask (0x0F)
#define TankCalibPoints 20
#define TankCalibMaxHeight 15
#define NumMeasure 100

float Level = 0;
ulong TankCalibVal[TankCalibPoints] = { 0 };
uchar UseCalibration = 0;
char ForceMode = 0;
float Duty = 0;

float Control_GetRawMeasureValue(void)
{
	if ((FDC2214_GetNewDataMask() & ChannelMask) == ChannelMask)
	{
		float CapPlate;
		float CapWire;
		FDC2214_ClearNewDataMask(0xFF);
		FDC2214_GetChannelCapacitance(0, &CapPlate);
		FDC2214_GetChannelCapacitance(1, &CapWire);
		CapPlate -= CapWire;
		return CapPlate;
	}
	return -1;
}

void Control_DebugDisp(void)//显示FDC 调试值
{
	uchar i = 0;
#ifdef FDC_Debug
	FDC2214_GetData();
#endif

	for (i = 0; i < 4; i++)
	{
		float vF;
		float vC;
		char sb[32];
		uchar Ret = FDC2214_GetChannelFreq(i, &vF);
		FDC2214_GetChannelCapacitance(i, &vC);
		//printf("%d %dF %ld C %ld\r\n", i, Ret, (long)(vF), (long)(1000 * vC));
		sprintf(sb,"%d %dF %ld C %ld  ", i, Ret, (long)(vF), (long)(1000 * vC));
		OLED_WriteStr(0, i*8, sb, 1);
	}
}

void Data_processing(float* FinalData, float* RawData)
{
	ushort i,j = 0;
	ushort start = NumMeasure / 4;
	ushort end = NumMeasure / 4 * 3;
	ushort length = end - start;
	float FinalData_temp = 0;
	float temp = 0;
	
	for (i = 0; i < NumMeasure - 1; ++i)//n个数,总共需要进行n-1次    
	{                 
		//n-1个数排完,第一个数一定已经归位        
		//每次会将最大(升序)或最小(降序)放到最后面        
		for(j = 0 ; j < NumMeasure - i - 1 ; ++j)        
		{            
			if (RawData[j] > RawData[j + 1])//每次冒泡,进行交换            
			{                
				temp = RawData[j];                
				RawData[j] = RawData[j + 1];                
				RawData[j + 1] = temp;            
			}        
		}
	}
	
	//TODO 数据处理
	for(i = start ; i < end ; i++)
	{
		FinalData_temp += RawData[i];
	}
	*FinalData = FinalData_temp / length;
	
}

float FDC2214_GetFinalData(void)
{
	ushort i = 0;
	float vF;
	float vC[NumMeasure];
	float FinalData;
	char sb[32];
	
	for (i = 0; i < NumMeasure; i++)
	{	
		FDC2214_GetData();
		Delay(2);
		FDC2214_GetChannelCapacitance(1, &vC[i]);
		Delay(2);
	}
	Data_processing(&FinalData, vC);

	//float zhangshu = powf(10, (2.5726 - log10f(FinalData)) / 0.8549);
	//printf("%ld\r\n", (long)(1000 * FinalData));
	//sprintf(sb, " %ld  ", (long)(zhangshu));
	//OLED_WriteStr(0, 6 * 8, sb, 1);
	//sprintf(sb, " %ld  ", (long)(1000 * FinalData));
	//OLED_WriteStr(0, 5 * 8, sb, 1);
	return FinalData;
}
void Control_Init(void)
{
	FDC2214_SetDeglitch(DEGLITCH_BW_10M);
	FDC2214_SetClockSource(1, 40000000);
	FDC2214_SetINTB(0);
	FDC2214_SetCurrentMode(0, 0);
	FDC2214_SetChannelConfig(0, 0x1000, 0x00A0, 1, MULTI_CH_0M01_8M75, DRIVE_1p571mA);
	FDC2214_SetChannelConfig(1, 0x1000, 0x00A0, 1, MULTI_CH_0M01_8M75, DRIVE_1p571mA);
	FDC2214_SetChannelConfig(2, 0x1000, 0x00A0, 1, MULTI_CH_0M01_8M75, DRIVE_1p571mA);
	FDC2214_SetChannelConfig(3, 0x1000, 0x00A0, 1, MULTI_CH_0M01_8M75, DRIVE_1p571mA);
	FDC2214_SetConvertChannel(1, 3);
	FDC2214_SetSleepMode(0);
}

void Control_Process(void)
{
	//FDC2214_GetData();
	FDC2214_GetFinalData();
}

void lftoa(char* Buf, double Val)//把val的小数转换到字符串
{
	long IntPart;
	long FloatPart;
	if (Val < 0)
	{
		Buf[0] = '-';
		Val = -Val;
	}
	else
	{
		Buf[0] = ' ';
	}
	Buf++;
	IntPart = Val;
	FloatPart = 10.0 * (Val - IntPart);
	sprintf(Buf, "%3ld.%01ld", IntPart, FloatPart);
}

void Control_Disp(void)
{
	char sb[32];
	char nsb[32];
	//Control_DebugDisp();
	//lftoa(nsb, Level);
	//sprintf(sb, "Level: %s mm  ", nsb);
	//OLED_WriteStr(0, 40, sb, 1);
	//printf("%ld\r\n", (long)(Level * 1000));//printf 直接重定向到串口
}
