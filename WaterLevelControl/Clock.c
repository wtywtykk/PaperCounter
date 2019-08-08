#include "Global.h"
#include "Clock.h"

#define REFO_Freq 32768
#define VLO_Freq 10000

ulong fSMCLK, fMCLK, fACLK;

void Clock_SetVcore(unsigned int level)
{
	// Open PMM registers for write
	PMMCTL0_H = PMMPW_H;
	// Set SVS/SVM high side new level
	SVSMHCTL = SVSHE + SVSHRVL0 * level + SVMHE + SVSMHRRL0 * level;
	// Set SVM low side to new level
	SVSMLCTL = SVSLE + SVMLE + SVSMLRRL0 * level;
	// Wait till SVM is settled
	while ((PMMIFG & SVSMLDLYIFG) == 0);
	// Clear already set flags
	PMMIFG &= ~(SVMLVLRIFG + SVMLIFG);
	// Set VCore to new level
	PMMCTL0_L = PMMCOREV0 * level;
	// Wait till new level reached
	if ((PMMIFG & SVMLIFG))
		while ((PMMIFG & SVMLVLRIFG) == 0);
	// Set SVS/SVM low side to new level
	SVSMLCTL = SVSLE + SVSLRVL0 * level + SVMLE + SVSMLRRL0 * level;
	// Lock PMM registers for write access
	PMMCTL0_H = 0x00;
}

void Clock_SetVcoreForMCLK(ulong MCLK)
{
	uchar TargetVcore = 0;
	uchar i = 0;
	if (MCLK <= 8000000)
	{
		TargetVcore = 0;
	}
	else if (MCLK <= 12000000)
	{
		TargetVcore = 1;
	}
	else if (MCLK <= 20000000)
	{
		TargetVcore = 2;
	}
	else
	{
		TargetVcore = 3;
	}
	for (i = 0; i <= TargetVcore; i++)
	{
		Clock_SetVcore(i);
	}
}

void Clock_SetClockToLowest(void)
{
	UCSCTL4 = SELM__REFOCLK | SELS__REFOCLK | SELA__REFOCLK;
	UCSCTL5 = DIVM__1 | DIVS__1 | DIVA__1 | DIVPA__1;
	UCSCTL3 = SELREF__REFOCLK;
}

void Clock_SetClockSources(Clock* Ctrl)
{
	if (Ctrl->XT1Freq != 0)
	{
		P5SEL |= BIT4 | BIT5;
		UCSCTL6 |= XCAP_3;//12pF
		UCSCTL6 |= XT1OFF;
		while (SFRIFG1&OFIFG)
		{
			UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
			SFRIFG1 &= ~OFIFG;
		}
	}
	else
	{
		P5SEL &= ~(BIT4 | BIT5);
		UCSCTL6 |= XT1OFF;
	}
	if (Ctrl->XT2Freq != 0)
	{
		P5SEL |= BIT2 | BIT3;
		UCSCTL6 |= XT2OFF;
		while (SFRIFG1&OFIFG)
		{
			UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
			SFRIFG1 &= ~OFIFG;
		}
	}
	else
	{
		P5SEL &= ~(BIT2 | BIT3);
		UCSCTL6 |= XT2OFF;
	}
	if (Ctrl->DCOFreq != 0)
	{
		ushort CTL2 = 0;
		ushort CTL3 = 0;
		__bis_SR_register(SCG0);
		UCSCTL0 = 0;
		if (Ctrl->DCOFreq <= 630000)		//           DCOFreq < 0.63MHz
		{
			UCSCTL1 = DCORSEL_0;
		}
		else if (Ctrl->DCOFreq < 1250000)	// 0.63MHz < DCOFreq < 1.25MHz
		{
			UCSCTL1 = DCORSEL_1;
		}
		else if (Ctrl->DCOFreq < 2500000)	// 1.25MHz < DCOFreq <  2.5MHz
		{
			UCSCTL1 = DCORSEL_2;
		}
		else if (Ctrl->DCOFreq < 5000000)	// 2.5MHz  < DCOFreq <    5MHz
		{
			UCSCTL1 = DCORSEL_3;
		}
		else if (Ctrl->DCOFreq < 10000000)	// 5MHz    < DCOFreq <   10MHz
		{
			UCSCTL1 = DCORSEL_4;
		}
		else if (Ctrl->DCOFreq < 20000000)	// 10MHz   < DCOFreq <   20MHz
		{
			UCSCTL1 = DCORSEL_5;
		}
		else if (Ctrl->DCOFreq < 40000000)	// 20MHz   < DCOFreq <   40MHz
		{
			UCSCTL1 = DCORSEL_6;
		}
		else
		{
			UCSCTL1 = DCORSEL_7;
		}
		switch (Ctrl->DCODivider)
		{
		default:
			assert(0);
			Ctrl->DCODivider = 1;
		case 1:
			CTL2 = FLLD__1;
			break;
		case 2:
			CTL2 = FLLD__2;
			break;
		case 4:
			CTL2 = FLLD__4;
			break;
		case 8:
			CTL2 = FLLD__8;
			break;
		case 16:
			CTL2 = FLLD__16;
			break;
		}
		if (Ctrl->XT2Freq)
		{
			CTL2 |= (16 * Ctrl->DCOFreq / Ctrl->XT2Freq / Ctrl->DCODivider) - 1;
			CTL3 = SELREF__XT2CLK | FLLREFDIV__16;
			Ctrl->DCOFreq = ((ulong)(Ctrl->DCOFreq / Ctrl->XT2Freq)) * Ctrl->XT2Freq;
		}
		else if (Ctrl->XT1Freq)
		{
			CTL2 |= (8 * Ctrl->DCOFreq / Ctrl->XT1Freq / Ctrl->DCODivider) - 1;
			CTL3 = SELREF__XT1CLK | FLLREFDIV__8;
			Ctrl->DCOFreq = ((ulong)(Ctrl->DCOFreq / Ctrl->XT2Freq)) * Ctrl->XT1Freq;
		}
		else
		{
			CTL2 |= (4 * Ctrl->DCOFreq / REFO_Freq / Ctrl->DCODivider) - 1;
			CTL3 = SELREF__REFOCLK | FLLREFDIV__4;
			Ctrl->DCOFreq = ((ulong)(Ctrl->DCOFreq / Ctrl->XT2Freq)) * REFO_Freq;
		}
		UCSCTL2 = CTL2;
		UCSCTL3 = CTL3;
		__bic_SR_register(SCG0);
		while (SFRIFG1 & OFIFG)                                // Check OFIFG fault flag
		{
			UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);         // Clear OSC flaut Flags
			SFRIFG1 &= ~OFIFG;                                  // Clear OFIFG fault flag
		}
	}
}

void Clock_SetLines(Clock* Ctrl)
{
	ushort CTL4 = 0;
	ushort CTL5 = 0;

	switch (Ctrl->MCLKSource)
	{
	default:
		assert(0);
		Ctrl->MCLKSource = REFO;
	case REFO:
		CTL4 |= SELM__REFOCLK;
		fMCLK = REFO_Freq;
		break;
	case VLO:
		CTL4 |= SELM__VLOCLK;
		fMCLK = VLO_Freq;
		break;
	case XT1:
		CTL4 |= SELM__XT1CLK;
		fMCLK = Ctrl->XT1Freq;
		break;
	case XT2:
		CTL4 |= SELM__XT2CLK;
		fMCLK = Ctrl->XT2Freq;
		break;
	case DCO:
		CTL4 |= SELM__DCOCLK;
		fMCLK = Ctrl->DCOFreq;
		break;
	case DCODIV:
		CTL4 |= SELM__DCOCLKDIV;
		fMCLK = Ctrl->DCOFreq / Ctrl->DCODivider;
		break;
	}
	switch (Ctrl->SMCLKSource)
	{
	default:
		assert(0);
		Ctrl->SMCLKSource = REFO;
	case REFO:
		CTL4 |= SELS__REFOCLK;
		fSMCLK = REFO_Freq;
		break;
	case VLO:
		CTL4 |= SELS__VLOCLK;
		fSMCLK = VLO_Freq;
		break;
	case XT1:
		CTL4 |= SELS__XT1CLK;
		fSMCLK = Ctrl->XT1Freq;
		break;
	case XT2:
		CTL4 |= SELS__XT2CLK;
		fSMCLK = Ctrl->XT2Freq;
		break;
	case DCO:
		CTL4 |= SELS__DCOCLK;
		fSMCLK = Ctrl->DCOFreq;
		break;
	case DCODIV:
		CTL4 |= SELS__DCOCLKDIV;
		fSMCLK = Ctrl->DCOFreq / Ctrl->DCODivider;
		break;
	}
	switch (Ctrl->ACLKSource)
	{
	default:
		assert(0);
		Ctrl->ACLKSource = REFO;
	case REFO:
		CTL4 |= SELA__REFOCLK;
		fACLK = REFO_Freq;
		break;
	case VLO:
		CTL4 |= SELA__VLOCLK;
		fACLK = VLO_Freq;
		break;
	case XT1:
		CTL4 |= SELA__XT1CLK;
		fACLK = Ctrl->XT1Freq;
		break;
	case XT2:
		CTL4 |= SELA__XT2CLK;
		fACLK = Ctrl->XT2Freq;
		break;
	case DCO:
		CTL4 |= SELA__DCOCLK;
		fACLK = Ctrl->DCOFreq;
		break;
	case DCODIV:
		CTL4 |= SELA__DCOCLKDIV;
		fACLK = Ctrl->DCOFreq / Ctrl->DCODivider;
		break;
	}
	switch (Ctrl->ACLKDivide)
	{
	default:
		assert(0);
		Ctrl->ACLKDivide = 1;
	case 1:
		CTL5 |= DIVA__1;
		break;
	case 2:
		CTL5 |= DIVA__2;
		fACLK = fACLK / 2;
		break;
	case 4:
		CTL5 |= DIVA__4;
		fACLK = fACLK / 4;
		break;
	case 8:
		CTL5 |= DIVA__8;
		fACLK = fACLK / 8;
		break;
	case 16:
		CTL5 |= DIVA__16;
		fACLK = fACLK / 16;
		break;
	case 32:
		CTL5 |= DIVA__32;
		fACLK = fACLK / 32;
		break;
	}
	switch (Ctrl->MCLKDivide)
	{
	default:
		assert(0);
		Ctrl->MCLKDivide = 1;
	case 1:
		CTL5 |= DIVM__1;
		break;
	case 2:
		CTL5 |= DIVM__2;
		fMCLK = fMCLK / 2;
		break;
	case 4:
		CTL5 |= DIVM__4;
		fMCLK = fMCLK / 4;
		break;
	case 8:
		CTL5 |= DIVM__8;
		fMCLK = fMCLK / 8;
		break;
	case 16:
		CTL5 |= DIVM__16;
		fMCLK = fMCLK / 16;
		break;
	case 32:
		CTL5 |= DIVM__32;
		fMCLK = fMCLK / 32;
		break;
	}
	switch (Ctrl->SMCLKDivide)
	{
	default:
		assert(0);
		Ctrl->SMCLKDivide = 1;
	case 1:
		CTL5 |= DIVS__1;
		break;
	case 2:
		CTL5 |= DIVS__2;
		fSMCLK = fSMCLK / 2;
		break;
	case 4:
		CTL5 |= DIVS__4;
		fSMCLK = fSMCLK / 4;
		break;
	case 8:
		CTL5 |= DIVS__8;
		fSMCLK = fSMCLK / 8;
		break;
	case 16:
		CTL5 |= DIVS__16;
		fSMCLK = fSMCLK / 16;
		break;
	case 32:
		CTL5 |= DIVS__32;
		fSMCLK = fSMCLK / 32;
		break;
	}

	Clock_SetVcoreForMCLK(fMCLK);

	UCSCTL5 = CTL5;
	UCSCTL4 = CTL4;
}

uchar Clock_Init(Clock* Ctrl)
{
	Clock_SetClockToLowest();
	Clock_SetClockSources(Ctrl);
	Clock_SetLines(Ctrl);
}

ulong Clock_Query(Clocks QueryType)
{
	switch (QueryType)
	{
	case SMCLK:
		return fSMCLK;
		break;
	case MCLK:
		return fMCLK;
		break;
	case ACLK:
		return fACLK;
		break;
	default:
		return 0;
		break;
	}
}
