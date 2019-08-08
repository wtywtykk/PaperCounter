#include "Global.h"
#include "Clock.h"
#include "DMAADC.h"

#define SampleTime0 ADC12SHT0_4
#define SampleTime1 ADC12SHT1_4

void DMAADC_InitTimer(float Freq, ushort SampleTime)
{
	TB0CCR0 = Clock_Query(ACLK) / Freq - 1;
	TB0CCR1 = SampleTime;
	TB0CCTL1 = OUTMOD_7;
	TB0CTL = TBSSEL__ACLK + MC__UP + TBCLR;
}

void DMAADC_InitADC(ADC_InitInfo* InitInfo)
{
	assert(InitInfo->ClkDiv <= 8 && InitInfo->ClkDiv >= 1);

	REFCTL0 = REFMSTR;
	switch (InitInfo->RefVolt)
	{
	case V15:
		REFCTL0 |= REFVSEL_0;
		break;
	case V20:
		REFCTL0 |= REFVSEL_1;
		break;
	case V25:
		REFCTL0 |= REFVSEL_2;
		break;
	default:
	{
		uchar InvalidRef = 0;
		assert(InvalidRef);
	}
	}
	if (InitInfo->RefAlwaysOn)
	{
		REFCTL0 |= REFON;
	}
	if (InitInfo->RefOutput)
	{
		REFCTL0 |= REFOUT;
	}

	ADC12CTL0 &= ~ADC12ENC;

	if (InitInfo->TimerTrigger)
	{
		ADC12CTL0 = SampleTime0 | SampleTime1 | ADC12MSC | ADC12ON;
		ADC12CTL1 = ADC12CSTARTADD_0 | ADC12SHS_3 | ((InitInfo->ClkDiv - 1) * 0x20u) | ADC12SSEL_1 | ADC12CONSEQ_2;//Sample-and-hold source = CCI0B = TBCCR1 output
	}
	else
	{
		ADC12CTL0 = SampleTime0 | SampleTime1 | ADC12ON;
		ADC12CTL1 = ADC12CSTARTADD_0 | ADC12SHS_0 | ADC12SHP | ((InitInfo->ClkDiv - 1) * 0x20u) | ADC12SSEL_1 | ADC12CONSEQ_0;
	}

	if (InitInfo->SlowMode)
	{
		//预分频4，12bit，Ref驱动能力采样50ksps
		ADC12CTL2 = ADC12PDIV | ADC12RES_2 | ADC12SR;
	}
	else
	{
		//12bit
		ADC12CTL2 = ADC12RES_2;
	}
	if (!InitInfo->RefAlwaysOn)
	{
		//Ref驱动Burst mode
		//Burst mode下REF只在转换期间有输出
		ADC12CTL2 |= ADC12REFBURST;
	}

	P6SEL |= 0x1 << InitInfo->Channel0.Source;
	ADC12MCTL0 = (InitInfo->Channel0.Source | InitInfo->Channel0.Ref << 4) | ADC12EOS;

	ADC12IE = 0;
	ADC12CTL0 |= ADC12ENC;
}

void DMAADC_InitDMA(void)
{
	DMA0CTL &= ~DMAEN;
	DMACTL0_L = DMA0TSEL__ADC12IFG;
	DMACTL4 = DMARMWDIS;
	DMA0CTL = DMADT_0 | DMADSTINCR_3 | DMASRCINCR_0 | DMASWDW;
	DMA0SA = (ulong)&ADC12MEM0;
}

void DMAADC_Init(ADC_InitInfo* InitInfo)
{
	DMAADC_InitADC(InitInfo);
	if (InitInfo->TimerTrigger)
	{
		DMAADC_InitTimer(InitInfo->TimerFreq, InitInfo->SampleTime);
	}
	DMAADC_InitDMA();
}

void DMAADC_Sample(ushort* Buf, ushort Len)
{
	ADC12CTL0 &= ~ADC12ENC;
	ADC12CTL0 |= ADC12ENC;

	DMA0DA = (ulong)Buf;
	DMA0SZ = Len;
	DMA0CTL |= DMAEN;
}

uchar DMAADC_IsBusy(void)
{
	return DMA0CTL & DMAEN ? 1 : 0;
}
