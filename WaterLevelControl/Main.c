#include "Global.h"
#include "Clock.h"
#include "PortInterrupts.h"
#include "DMA.h"
#include "Timer.h"
#include "OLED.h"
#include "BoardKey.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "FDC2214.h"
#include "Control.h"
#include "Graphic.h"
#include "UART.h"
#include "DMAADC.h"
#include "InfoStore.h"
#include "DigitDisp.h"

typedef enum
{
	Cali,
	Run,
}ScreenEnum;
ScreenEnum ScreenStatus = Run;

#define NumOfCalibration  10

ushort CaliProcessCounter = 0;
static float CapCali[NumOfCalibration] = { 0 };
static float TestNumPaper[NumOfCalibration] = { 1, 2, 3, 4, 5, 10, 15, 20, 25, 30 };
static ushort CalSegment[CalibrationSegmentCount] = { 3,7 };

static uchar LastDataShort = 0;
static float LastCap = 0;
static float MeasurePaperCount = 0;

void InitSysClock(void)
{
	Clock clk;
	clk.ACLKDivide = 1;
	clk.ACLKSource = DCODIV;
	clk.DCOFreq = 50000000;
	clk.DCODivider = 2;
	clk.MCLKDivide = 1;
	clk.MCLKSource = DCODIV;
	clk.SMCLKDivide = 1;
	clk.SMCLKSource = DCODIV;
	clk.XT1Freq = 32767;
	clk.XT2Freq = 4000000;
	Clock_Init(&clk);
}

void InitLED(void)
{
	P1DIR |= 1 << 0;
	P4DIR |= 1 << 7;
	P4OUT &= ~(1 << 7);
	P1OUT &= ~(1 << 0);
}

void Buzzer()
{
	P2DIR |= (1 << 5);
	P2OUT &= ~(1 << 5);
	Delay(100);
	P2OUT |= (1 << 5);
}

void JumpToStatus(ScreenEnum NewStatus)
{
	ScreenStatus = NewStatus;
	switch (ScreenStatus)
	{
	case Cali:
		CaliProcessCounter = 0;
		break;
	case Run:
		break;
	}
}

void DrawKeyHint(void)
{
	const char* LeftTip = "--";
	const char* RightTip = "--";
	uchar TipWidth = 0;

	switch (ScreenStatus)
	{
	case Cali:
		RightTip = "Meas";
		break;
	case Run:
		LeftTip = "Cali";
		RightTip = "Meas";
		break;
	}

	OLED_Fill(0, 7 * 8, OLED_Width, 8, 0);
	OLED_WriteStr(0, 7 * 8, LeftTip, 0);
	OLED_EstimateStr(RightTip, &TipWidth, NULL);
	OLED_WriteStr(OLED_Width - TipWidth, 7 * 8, RightTip, 0);
}

void DrawInterface(void)
{
	char sb[32];

	if (LastDataShort)
	{
		OLED_WriteStr(0, 6 * 8, "!!!PlateShort!!!", 1);
	}

	switch (ScreenStatus)
	{
	case Cali:
	{
		ushort CalibrationIndex = CaliProcessCounter / 2;
		uchar IsDone = CaliProcessCounter % 2;
		OLED_WriteStr(0, 0, "Calibration", 0);

		if (CalibrationIndex == 0)
		{
			OLED_WriteStr(0, 3 * 8, "Open Calibration", 1);
		}
		else if (CalibrationIndex > NumOfCalibration)
		{
			OLED_WriteStr(0, 3 * 8, "Calibration Done", 1);
			OLED_WriteStr(0, 4 * 8, "Press ""Meas"" to save.", 1);
		}
		else
		{
			sprintf(sb, "Put %d paper(s)", (uint)TestNumPaper[CalibrationIndex - 1]);
			OLED_WriteStr(5, 3 * 8, sb, 1);
		}
		if (IsDone)
		{
			OLED_WriteStr(0, 4 * 8, "...Sampled", 1);
		}
		sprintf(sb, "UCC%ld", (long)((LastCap + Info.OpenCap) * 1000));
		OLED_WriteStr(0, 5 * 8, sb, 1);
	}
	break;
	case Run:
	{
		OLED_WriteStr(0, 0, "Measure", 0);
		long PaperCountInt = (MeasurePaperCount + 0.5);
		if (MeasurePaperCount > 0 && !LastDataShort)
		{
			sprintf(sb, "%ld", PaperCountInt);
			DigitDisp_DispStr(60, 0, 16, 32, 3, sb);
		}
		else
		{
			DigitDisp_DispStr(60, 0, 16, 32, 3, "EEE");
		}
		sprintf(sb, "UCC%ld D%ld", (long)((LastCap + Info.OpenCap) * 1000), (long)((MeasurePaperCount - PaperCountInt) * 1000));
		OLED_WriteStr(0, 5 * 8, sb, 1);
	}
	break;
	}
}

void UpdateScreen(void)
{
	OLED_Clear();
	DrawKeyHint();
	DrawInterface();
	OLED_BufferFulsh();
}

void PlayVoice(uchar Str[])//GB2312 encoding
{
	uchar Command[256];
	uint16_t Len = strlen(Str);
	if (Len > sizeof(Command) - 5)
	{
		Len = sizeof(Command) - 5;
	}
	Command[0] = 0xFD;
	Command[1] = (Len + 2) >> 8;
	Command[2] = (Len + 2) & 0xFF;
	Command[3] = 0x1;
	Command[4] = 0x0;
	strncpy(&Command[5], Str, Len);
	UART1_SendBytes(Command, Len + 5);
}

void VoiceReportCount(void)
{
	char Buf[128];
	if (!LastDataShort)
	{
		if (MeasurePaperCount > 0)
		{
			long PaperCountInt = (MeasurePaperCount + 0.5);
			sprintf(Buf, "[n2][m3][g1]测到%d张纸[m20][g2]%d papers detected", PaperCountInt, PaperCountInt);
		}
		else
		{
			strcpy(Buf, "[m3]测量失败[m20]measurement failed");
		}
	}
	else
	{
		strcpy(Buf, "[m3]极板短路[m20]plate short");
	}
	PlayVoice(Buf);
}

void LSM(ushort Count, float x[], float y[], float* k, float* b)
{
	int i;
	double SumXY = 0;
	double SumX = 0;
	double SumY = 0;
	double SumSqrX = 0;
	double AverageX = 0;
	double AverageY = 0;

	for (i = 0; i < Count; i++)
	{
		SumXY += x[i] * y[i];
		SumX += x[i];
		SumY += y[i];
		SumSqrX += x[i] * x[i];
	}
	AverageX = SumX / Count;
	AverageY = SumY / Count;

	*k = (Count * SumXY - SumX * SumY) / (Count * SumSqrX - SumX * SumX);
	*b = AverageY - *k * AverageX;
}

void CalcCalData()
{
	uint i = 0;
	uint CalDataIndex = 0;
	float CapCaliLog[NumOfCalibration] = { 0 };
	float TestNumPaperLog[NumOfCalibration] = { 0 };

	for (i = 0; i < NumOfCalibration; i++)
	{
		CapCaliLog[i] = logf(CapCali[i]);
		TestNumPaperLog[i] = logf(TestNumPaper[i]);
	}

	for (i = 0; i < CalibrationSegmentCount; i++)
	{
		LSM(CalSegment[i], &CapCaliLog[CalDataIndex], &TestNumPaperLog[CalDataIndex], &Info.k[i], &Info.b[i]);
		CalDataIndex += CalSegment[i];
		if (i < CalibrationSegmentCount - 1)
		{
			Info.Boundary[i] = (CapCali[CalDataIndex - 1] + CapCali[CalDataIndex]) / 2;
		}
	}
	assert(CalDataIndex == NumOfCalibration);
	SaveInfo();
}

float CalcPaperCount(float MeasureData)
{
	float MeasureDataLog = logf(MeasureData);
	uint i = 0;
	uint CalDataIndex = 0;
	for (i = 0; i < CalibrationSegmentCount; i++)
	{
		if ((i == CalibrationSegmentCount - 1) || (MeasureData >= Info.Boundary[i]))
		{
			float Ret = expf(Info.k[i] * MeasureDataLog + Info.b[i]);
			if (Ret > 999)
			{
				Ret = -1;
			}
			return Ret;
		}
	}
	return -1;
}

float DoMeasurementAndReport(void)
{
	float Ret = 0;
	P4OUT &= ~(1 << 7);
	P1OUT &= ~(1 << 0);
	Delay(1000);
	P4OUT |= (1 << 7);
	Ret = FDC2214_GetFinalData();
	LastCap = Ret;
	if (Ret < 0)
	{
		LastDataShort = 1;
		Buzzer();
		Delay(100);
		Buzzer();
	}
	else
	{
		Buzzer();
		LastDataShort = 0;
	}
	P1OUT |= (1 << 0);
	return Ret;
}

void CheckKey(void)
{
	char sb[64];

	switch (ScreenStatus)
	{
	case Cali:
		switch (BoardKey_GetLastKey())
		{
		case BOARDKEY_LEFT:
			break;
		case BOARDKEY_RIGHT:
		{
			ushort CalibrationIndex = CaliProcessCounter / 2;
			uchar IsDone = CaliProcessCounter % 2;
			if (IsDone)
			{
				if (CalibrationIndex >= NumOfCalibration)
				{
					CalcCalData();
					Buzzer();
					Delay(50);
					Buzzer();
					Delay(50);
					Buzzer();
					Delay(50);
					JumpToStatus(Run);
				}
			}
			else
			{
				if (CalibrationIndex == 0)
				{
					Info.OpenCap = 0;
					Info.OpenCap = DoMeasurementAndReport();
					if (Info.OpenCap < 0)
					{
						return;
					}
				}
				else
				{
					CapCali[CalibrationIndex - 1] = DoMeasurementAndReport();
					if (CapCali[CalibrationIndex - 1] < 0)
					{
						return;
					}
				}
			}
			CaliProcessCounter++;
		}
		break;
		}
		break;
	case Run:
		switch (BoardKey_GetLastKey())
		{
		case BOARDKEY_LEFT:
			JumpToStatus(Cali);
			break;
		case BOARDKEY_RIGHT:
		{
			float MeasVal = DoMeasurementAndReport();
			if (MeasVal > 0)
			{
				MeasurePaperCount = CalcPaperCount(MeasVal);
			}
			else
			{
				MeasurePaperCount = -1;
			}
			VoiceReportCount();
		}
		break;
		}
		break;
	}
}

int main()
{
	WDTCTL = WDTPW | WDTHOLD;
	__enable_interrupt();
	InitSysClock();
	PortInterrupts_Init();
	LoadInfo();
	DMA_Init();
	InitTimer();
	StartTimer();
	InitLED();
	OLED_Init();
	BoardKey_Init();
	UART1_Init(115200, NULL);

	OLED_WriteStr(0, 0, "FDCInit...  ", 1);
	OLED_BufferFulsh();

	FDC2214_Init();
	Control_Init();

	while (1)
	{
		UpdateScreen();
		CheckKey();
	}
}


