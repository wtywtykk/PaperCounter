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

#define  MaxPaperCount 100
#define NumOfCalibration  15
#define rank_ 2


uint CaliIndex = 1;
ushort i_Calibration = 0;
static ushort CaliFlag = 0;
static ushort MeasFlag = 0;
static float OpenCap;
static float CapCali[NumOfCalibration];
float Factor[rank_+1];
static ushort TestNumPaper[NumOfCalibration] = { 1, 2, 3, 4, 5, 7, 9, 11, 14, 17, 20, 25, 27, 30, 31 };


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

	//switch (BoardKey_GetLastKey())
	//{
	//case BOARDKEY_RIGHT:
	//	FDC2214_Init();
	//	Control_Init();
	//	return;
	//	break;
	//}

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
		OLED_WriteStr(0, 7*8, sb, 1);
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

void DoEvents(void)  //循环执行的
{
	Control_Process();
	switch (ScreenStatus)
	{
	case Cali:
		break;
	case Run:
		break;
	}
}
void LSM(float* factor, ushort* zhangshu, float* CapData, float* OpenCap)
{
	int i, j, k;
	int maxn = NumOfCalibration;
	float atemp[2 * (rank_ + 1)] = { 0 }, b[rank_ + 1] = { 0 }, a[rank_ + 1][rank_ + 1];

	for (i = 0; i < maxn; i++) {  //
		atemp[1] += zhangshu[i];
		atemp[2] += powf(zhangshu[i], 2);
		atemp[3] += powf(zhangshu[i], 3);
		atemp[4] += powf(zhangshu[i], 4);
		atemp[5] += powf(zhangshu[i], 5);
		atemp[6] += powf(zhangshu[i], 6);
		b[0] += CapData[i];
		b[1] += zhangshu[i] * CapData[i];
		b[2] += pow(zhangshu[i], 2) * CapData[i];
	}

	atemp[0] = maxn;

	for (i = 0; i < rank_ + 1; i++) {  //构建线性方程组系数矩阵，b[]不变
		k = i;
		for (j = 0; j < rank_ + 1; j++)  a[i][j] = atemp[k++];
	}

	//以下为高斯列主元消去法解线性方程组
	for (k = 0; k < rank_ + 1 - 1; k++) {  //n - 1列
		int column = k;
		double mainelement = a[k][k];

		for (i = k; i < rank_ + 1; i++)  //找主元素
			if (fabs(a[i][k]) > mainelement) {
				mainelement = fabs(a[i][k]);
				column = i;
			}
		for (j = k; j < rank_ + 1; j++) {  //交换两行
			double atemp = a[k][j];
			a[k][j] = a[column][j];
			a[column][j] = atemp;
		}
		double btemp = b[k];
		b[k] = b[column];
		b[column] = btemp;

		for (i = k + 1; i < rank_ + 1; i++) {  //消元过程
			double Mik = a[i][k] / a[k][k];
			for (j = k; j < rank_ + 1; j++)  a[i][j] -= Mik * a[k][j];
			b[i] -= Mik * b[k];
		}
	}

	b[rank_ + 1 - 1] /= a[rank_ + 1 - 1][rank_ + 1 - 1];  //回代过程
	for (i = rank_ + 1 - 2; i >= 0; i--) {
		double sum = 0;
		for (j = i + 1; j < rank_ + 1; j++)  sum += a[i][j] * b[j];
		b[i] = (b[i] - sum) / a[i][i];
	}
	//高斯列主元消去法结束
	factor = b;
}

void Control_Key(void)
{
	uchar sb;
	float FinalData;

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
				OpenCap = FDC2214_GetFinalData();

			}
			else if (i_Calibration > NumOfCalibration*2)
			{
				sprintf(sb, "Calibration Over");
				OLED_Fill(0, 3 * 8, OLED_Width, 8, 0);
				OLED_Fill(0, 4 * 8, OLED_Width, 8, 0);
				OLED_WriteStr(10, 3 * 8, sb, 1);
				CaliFlag = 0;
				//最小二乘算法计算系数
				LSM(Factor, TestNumPaper, CapCali, &OpenCap);
			}
			else
			{
				if (i_Calibration % 2 == 1) 
				{
					sprintf(sb, "Please put %d paper!", TestNumPaper[(i_Calibration-1)/2]);
					OLED_Fill(0, 3 * 8, OLED_Width, 8, 0);
					OLED_Fill(0, 4 * 8, OLED_Width, 8, 0);

					OLED_WriteStr(5, 3 * 8, sb, 1);
				}
				else
				{
					CapCali[i_Calibration/2 - 1] = FDC2214_GetFinalData();
					printf("%d, %ld\r\n", TestNumPaper[i_Calibration / 2 - 1], (long)(CapCali[i_Calibration/2 - 1]));
					sprintf(sb, "%d, %ld", TestNumPaper[i_Calibration / 2 - 1], (long)(CapCali[i_Calibration/2 - 1]));
					OLED_WriteStr(5, 4 * 8, sb, 1);
				}
			}
			i_Calibration++;
		}
		else
		{
			//测量模式
			OLED_Fill(0, 0 * 8, OLED_Width, 8, 0);
			DrawKeyHintRight(0, 1);


			FinalData = FDC2214_GetFinalData();

			//换算成纸张数
			float zhangshu = powf(10, (2.5726 - log10f(FinalData)) / 0.8549);
			printf("%ld\r\n", (long)(1000 * FinalData));
			sprintf(sb, " %ld  ", (long)(zhangshu));
			OLED_WriteStr(0, 6 * 8, sb, 1);

			//sprintf(sb, "FinalData: %ld", (long)FinalData);
			OLED_Fill(0, 3 * 8, OLED_Width, 8, 0);
			OLED_WriteStr(0, 5 * 8, sb, 1);

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
	//Graphic_Init();
	BoardKey_Init();
	UART1_Init(115200, NULL);
	FDC2214_Init();
	Control_Init();

	//JumpToStatus(Cali);


	while (1)
	{
		UpdateScreen();
		//OLED_WriteStr(0, 7 * 8, "Calibrate", 1);
		Control_Key();
		//CheckKey();
		//DoEvents();
		//UpdateLED();
		//FDC2214_MonitorDebug();
		//Delay(500);
	}
}


