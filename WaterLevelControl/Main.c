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

typedef enum //邀请赛测评用
{
	Cali,
	Run,
}ScreenEnum;
ScreenEnum ScreenStatus = Cali;
uchar ScreenClearRequired = 1;

#define NumOfCalibration  10

uint CaliIndex = 1;
ushort i_Calibration = 0;
static ushort CaliFlag = 0;
static ushort MeasFlag = 0;
static float CapCali[NumOfCalibration] = { 0 };
static float TestNumPaper[NumOfCalibration] = { 1, 2, 3, 4, 5, 10, 15, 20, 25, 30 };
static ushort CalSegment[CalibrationSegmentCount] = { 3,7 };

void InitSysClock(void) //430时钟初始化
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

void InitLED(void)  //初始化LED IO口
{
	P1DIR |= 1 << 0;
	P4DIR |= 1 << 7;
	P4OUT &= ~(1 << 7);
	P1OUT &= ~(1 << 0);
}

void Buzzer()
{
	P2DIR |= (1 << 5);
	P2OUT |= (1 << 5);
	Delay(100);
	P2OUT &= ~(1 << 5);
}

void UpdateLED(void)
{
	if ((GetTickSafe() / 500) % 2)
	{
		P1OUT |= 1 << 0;  //P1.0 高电平
		P4OUT &= ~(1 << 7);//P4.7 低电平
	}
	else
	{
		P1OUT &= ~(1 << 0);
		P4OUT |= 1 << 7;
	}
}

void JumpToStatus(ScreenEnum NewStatus)//邀请赛测评用 切换模式
{
	ScreenStatus = NewStatus;
	switch (ScreenStatus)
	{
	case Cali:
		CaliIndex = 1;
		break;
	case Run:
		break;
	}
	ScreenClearRequired = 1;
}

void CheckKey(void)
{
	char sb[32];

	switch (ScreenStatus)  //先选定模式 再控制按键
	{
	case Cali:

		break;
	case Run:

		switch (BoardKey_GetLastKey())
		{
		case BOARDKEY_LEFT:
		{
		}
		break;
		case BOARDKEY_RIGHT:
			JumpToStatus(Cali);
			break;
		}
		break;
	}
}

void DrawKeyHint(void)   //显示按键底部提示
{
	const char* LeftTip = "--";
	const char* RightTip = "--";
	uchar TipWidth = 0;

	LeftTip = "Cali";
	RightTip = "Meas";

	OLED_Fill(0, 7 * 8, OLED_Width, 8, 0);
	OLED_WriteStr(0, 7 * 8, LeftTip, 0);
	OLED_EstimateStr(RightTip, &TipWidth, NULL);
	OLED_WriteStr(OLED_Width - TipWidth, 7 * 8, RightTip, 0);
}

void DrawKeyHintLeft(uchar Pos, uchar Val)   //显示按键底部提示
{
	const char* LeftTip = "--";
	uchar TipWidth = 0;
	LeftTip = "Cali";
	//OLED_Fill(0, Pos * 8, OLED_Width, 8, Val);
	OLED_WriteStr(0, Pos * 8, LeftTip, Val);
}

void DrawKeyHintRight(uchar Pos, uchar Val)   //显示按键底部提示
{
	const char* RightTip = "--";
	uchar TipWidth = 0;

	RightTip = "Meas";
	//OLED_Fill(0, Pos * 8, OLED_Width, 8, Val);
	OLED_EstimateStr(RightTip, &TipWidth, NULL);
	OLED_WriteStr(OLED_Width - TipWidth, Pos * 8, RightTip, Val);
}

void DrawInterface(void)  //显示主界面
{
	char sb[32];

	switch (ScreenStatus)
	{
	case Cali:
		sprintf(sb, "Calibrate count: %d ", CaliIndex); //打印数据
		OLED_WriteStr(0, 0, "Calibration", 1);
		OLED_WriteStr(0, 7 * 8, sb, 1);
		break;
	case Run:
		break;
	}
}

void UpdateScreen(void)
{
	if (ScreenClearRequired)
	{
		OLED_Clear();
		ScreenClearRequired = 0;
	}

	DrawKeyHint();
	//DrawInterface();
	//Control_Disp();
	OLED_BufferFulsh();
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

	for (i = 0; i < Count; i++) {  //
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
	//最小二乘算法计算系数
	uint i = 0;
	uint CalDataIndex = 0;
	float CapCaliLog[NumOfCalibration] = { 0 };
	float TestNumPaperLog[NumOfCalibration] = { 0 };
	float TestNumPaperrev[NumOfCalibration] = { 0 };

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
	for (i = 0; i < NumOfCalibration; i++)
	{
		TestNumPaperrev[i] = Info.k[1] * CapCaliLog[i] + Info.b[1];
	}
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
			return expf(Info.k[i] * MeasureDataLog + Info.b[i]);
		}
	}
	return -1;
}

void Control_Key(void)
{
	char sb[64];

	//DrawKeyHint();

	switch (BoardKey_GetLastKey())
	{
	case BOARDKEY_LEFT:
		OLED_Fill(0, 0 * 8, OLED_Width, 8, 0);
		DrawKeyHintLeft(0, 1);
		CaliFlag = 1;
		P1OUT |= 1 << 0;
		Delay(500);
		P1OUT &= ~(1 << 0);

		break;

	case BOARDKEY_RIGHT:

		P4OUT |= 1 << 7;
		Delay(500);
		P4OUT &= ~(1 << 7);//P4.7 低电平

		if (CaliFlag)//校准模式
		{
			if (i_Calibration == 0)
			{
				//开路检测
				sprintf(sb, "Open Calibration");
				OLED_Fill(0, 3 * 8, OLED_Width, 8, 0);
				OLED_WriteStr(10, 3 * 8, sb, 1);
				Info.OpenCap = 0;
				Info.OpenCap = FDC2214_GetFinalData();
				if (Info.OpenCap < 0)
				{
					Buzzer();
					Delay(100);
					Buzzer();
					OLED_WriteStr(10, 3 * 8, "PlateShort", 1);
					return;
				}
			}
			else if (i_Calibration > NumOfCalibration * 2)
			{
				sprintf(sb, "Calibration Done");
				OLED_Fill(0, 3 * 8, OLED_Width, 8, 0);
				OLED_Fill(0, 4 * 8, OLED_Width, 8, 0);
				OLED_WriteStr(10, 3 * 8, sb, 1);
				CaliFlag = 0;
				CalcCalData();
				Buzzer();
				Delay(50);
				Buzzer();
				Delay(50);
				Buzzer();
				Delay(50);
			}
			else
			{
				if (i_Calibration % 2 == 1)
				{
					sprintf(sb, "Please put %d paper!", (uint)TestNumPaper[(i_Calibration - 1) / 2]);
					OLED_Fill(0, 3 * 8, OLED_Width, 8, 0);
					OLED_Fill(0, 4 * 8, OLED_Width, 8, 0);

					OLED_WriteStr(5, 3 * 8, sb, 1);
				}
				else
				{
					CapCali[i_Calibration / 2 - 1] = FDC2214_GetFinalData();
					if (CapCali[i_Calibration / 2 - 1] < 0)
					{
						Buzzer();
						Delay(100);
						Buzzer();
						OLED_WriteStr(10, 3 * 8, "PlateShort", 1);
						return;
					}
					printf("%d, %ld\r\n", TestNumPaper[i_Calibration / 2 - 1], (long)(CapCali[i_Calibration / 2 - 1]));
					sprintf(sb, "%d, %ld", TestNumPaper[i_Calibration / 2 - 1], (long)(CapCali[i_Calibration / 2 - 1]));
					OLED_WriteStr(5, 4 * 8, sb, 1);
					Buzzer();
				}
			}
			i_Calibration++;
		}
		else
		{
			//测量模式
			OLED_Fill(0, 0 * 8, OLED_Width, 8, 0);
			DrawKeyHintRight(0, 1);

			float MeasVal = FDC2214_GetFinalData();
			if (MeasVal > 0)
			{
				float PaperCount = CalcPaperCount(MeasVal);
				long PaperCountInt = (PaperCount + 0.5);
				if (PaperCount > 0)
				{
					printf("%ld\r\n", (long)(1000 * PaperCount));
					sprintf(sb, " %ld       ", PaperCountInt);
					OLED_WriteStr(0, 6 * 8, sb, 1);
				}
				else
				{
					printf("Error\r\n");
					OLED_WriteStr(0, 6 * 8, "Error  ", 1);
				}
				sprintf(sb, "1kM %ld     ", (long)((MeasVal+Info.OpenCap)*1000));
				OLED_WriteStr(0, 2 * 8, sb, 1);
				sprintf(sb, "1kD %ld     ", (long)((PaperCount - PaperCountInt) * 1000));
				OLED_WriteStr(0, 3 * 8, sb, 1);
				Buzzer();
			}
			else
			{
				printf("PlateShort\r\n");
				OLED_WriteStr(0, 6 * 8, "PlateShort  ", 1);
				Buzzer();
				Delay(100);
				Buzzer();
			}
		}
		break;
	}
}
ushort Buf[1024];

int main()
{
	char sb[32];
	WDTCTL = WDTPW | WDTHOLD;//关看门狗
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
		Control_Key();
	}
}


