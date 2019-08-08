#pragma once

typedef enum
{
	ADC_Ch0 = 0,
	ADC_Ch1,
	ADC_Ch2,
	ADC_Ch3,
	ADC_Ch4,
	ADC_Ch5,
	ADC_Ch6,
	ADC_Ch7,
	ADC_VeREFP,
	ADC_VeREFN,
	ADC_Temp,
	ADC_Power,
	ADC_Ch8,
	ADC_Ch9,
	ADC_Ch10,
	ADC_Ch11
}ADC_Channels;

typedef enum
{
	AVCC_AVSS = 0,
	VREFP_AVSS = 1,
	VeREFP_AVSS = 2,
	AVCC_VREFN = 4,
	VREFP_VREFN = 5,
	VeREFP_VeREFN = 6
}ADC_RefSel;

typedef struct
{
	ADC_Channels Source;
	ADC_RefSel Ref;
}ADC_ChannelConfig;

typedef enum
{
	V15,
	V20,
	V25
}ADC_RefVolt;

typedef struct
{
	uint ClkDiv;
	uchar SlowMode;//clock divide by 4
	ADC_RefVolt RefVolt;
	uchar RefAlwaysOn;
	uchar RefOutput;
	ADC_ChannelConfig Channel0;
	uchar TimerTrigger;
	float TimerFreq;
	ushort SampleTime;
}ADC_InitInfo;

void DMAADC_Init(ADC_InitInfo* InitInfo);
void DMAADC_Sample(ushort* Buf, ushort Len);
uchar DMAADC_IsBusy(void);
