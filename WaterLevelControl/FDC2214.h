#pragma once

typedef enum
{
	DEGLITCH_BW_1M = 0,
	DEGLITCH_BW_3M3 = 4,
	DEGLITCH_BW_10M = 5,
	DEGLITCH_BW_33M = 7,
}FDC2214_DEGLITCH_SEL;

typedef enum
{
	MULTI_CH_0M01_8M75,
	MULTI_CH_5M_10M,
	SINGLE_CH_0M01_10M,
}FDC2214_CH_FREQ_SEL;

typedef enum
{
	DRIVE_0p016mA = 0,
	DRIVE_0p018mA,
	DRIVE_0p021mA,
	DRIVE_0p025mA,
	DRIVE_0p028mA,
	DRIVE_0p033mA,
	DRIVE_0p038mA,
	DRIVE_0p044mA,
	DRIVE_0p052mA,
	DRIVE_0p060mA,
	DRIVE_0p069mA,
	DRIVE_0p081mA,
	DRIVE_0p093mA,
	DRIVE_0p108mA,
	DRIVE_0p126mA,
	DRIVE_0p146mA,
	DRIVE_0p169mA,
	DRIVE_0p196mA,
	DRIVE_0p228mA,
	DRIVE_0p264mA,
	DRIVE_0p307mA,
	DRIVE_0p356mA,
	DRIVE_0p413mA,
	DRIVE_0p479mA,
	DRIVE_0p555mA,
	DRIVE_0p644mA,
	DRIVE_0p747mA,
	DRIVE_0p867mA,
	DRIVE_1p006mA,
	DRIVE_1p167mA,
	DRIVE_1p354mA,
	DRIVE_1p571mA,
}FDC2214_IDRIVE;

void FDC2214_Init(void);
uchar FDC2214_GetData(void);
uchar FDC2214_IsChannelReady(uchar Channel);
uchar FDC2214_GetChannelFreq(uchar Channel, float* Freq);
uchar FDC2214_GetChannelCapacitance(uchar Channel, float* Capacitance);
void FDC2214_ClearNewDataMask(uchar Mask);
uchar FDC2214_GetNewDataMask(void);
uchar FDC2214_SetChannelConfig(uchar Channel, ushort RCount, ushort SettleCount, ushort RefDivider, FDC2214_CH_FREQ_SEL SensorFreq, FDC2214_IDRIVE Current);
uchar FDC2214_SetINTB(uchar Enable);
uchar FDC2214_SetSleepMode(uchar Sleep);
uchar FDC2214_SetCurrentMode(uchar UserDefined, uchar SingleChannelHighCurrent);
uchar FDC2214_SetClockSource(uchar External, float ExtFreq);
uchar FDC2214_SetConvertChannel(uchar AutoScan, uchar ChannelSel);
uchar FDC2214_SetDeglitch(FDC2214_DEGLITCH_SEL Val);
uchar FDC2214_Reset(void);




void FDC2214_MonitorDebug(void);
