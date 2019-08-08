#include "Global.h"
#include "Clock.h"
#include "FDC2214_Reg.h"
#include "FDC2214.h"

#define WaitTime 10000

typedef struct
{
	ushort SensorDivider;
	ushort ClockDivider;


	float Freq;
	uchar AW;
	uchar WD;
}ChannelData;

static ChannelData Channels[4];
static uchar ChannelNewDataMask = 0;
static float ClkFreq = 40000000;
static uchar FailCount = 0;
static const float Calibration[4][4] = {
	//	Freq0	Cap0	Freq1	Cap1
	{ 5236724,		0,		3567490,		56 },
	{ 5452635,		0,		3959125,		47 },
	{ 5294127,		0,		3621911,		56 },
	{ 5284759,		0,		3581204,		56 },
};

void FDC2214_IICInit(void)
{
	ushort i = 0;
	ushort DeviceID = 0;

	P4SEL &= ~(BIT1 | BIT2);
	P4REN &= ~(BIT1 | BIT2);
	P4DIR &= ~(BIT1 | BIT2);
	P4OUT &= ~(BIT1 | BIT2);
	for (i = 0; i < 1000; i++)
	{
		__delay_cycles(100);
		P4DIR |= BIT2;
		__delay_cycles(100);
		P4DIR &= ~BIT2;
	}

	P4DIR &= ~(BIT1 | BIT2);
	P4OUT |= BIT1 | BIT2;
	P4REN |= BIT1 | BIT2;
	P4SEL |= BIT1 | BIT2;

	UCB1CTL1 |= UCSWRST;
	UCB1CTL0 = UCMST | UCMODE_3 | UCSYNC;
	UCB1CTL1 = UCSSEL__SMCLK | UCSWRST;
	UCB1BRW = Clock_Query(SMCLK) / 100000;
	UCB1I2COA = 0;
	UCB1I2CSA = 0x2A;
	UCB1IE = 0;
	UCB1IFG = 0;
	UCB1CTL1 &= ~UCSWRST;
}

void FDC2214_FailCheck(void)
{
	FailCount++;
	if (FailCount > 10)
	{
		FailCount = 0;
		FDC2214_IICInit();
	}
}

uchar FDC2214_WaitTx(void)
{
	ulong WaitCount = 0;
	do
	{
		WaitCount++;
		if ((WaitCount > WaitTime) || (UCB1IFG & (UCNACKIFG | UCALIFG)))
		{
			UCB1IFG = 0;
			FDC2214_FailCheck();
			return 0;
		}
	} while (!(UCB1IFG & UCTXIFG));
	return 1;
}

uchar FDC2214_Tx(uchar Val)
{
	ulong WaitCount = 0;
	do
	{
		WaitCount++;
		if ((WaitCount > WaitTime) || (UCB1IFG & (UCNACKIFG | UCALIFG)))
		{
			UCB1IFG = 0;
			FDC2214_FailCheck();
			return 0;
		}
	} while (!(UCB1IFG & UCTXIFG));
	UCB1TXBUF = Val;
	return 1;
}

uchar FDC2214_Rx(uchar* Val)
{
	ulong WaitCount = 0;
	do
	{
		WaitCount++;
		if ((WaitCount > WaitTime) || (UCB1IFG & (UCNACKIFG | UCALIFG)))
		{
			UCB1IFG = 0;
			FDC2214_FailCheck();
			return 0;
		}
	} while (!(UCB1IFG & UCRXIFG));
	*Val = UCB1RXBUF;
	return 1;
}

uchar FDC2214_Read(uchar RegAddr, ushort* Buf)
{
	uchar Ret = 0;
	uchar Recv[2] = { 0 };
	ulong WaitCount = 0;
	UCB1CTL1 |= UCTR;
	UCB1CTL1 |= UCTXSTT;
	if (FDC2214_Tx(RegAddr))
	{
		if (FDC2214_WaitTx())
		{
			UCB1CTL1 &= ~UCTR;
			UCB1CTL1 |= UCTXSTT;
			__disable_interrupt();
			if (FDC2214_Rx(&Recv[1]))
			{
				UCB1CTL1 |= UCTXSTP;
				if (FDC2214_Rx(&Recv[0]))
				{
					Ret = 1;
				}
			}
			else
			{
				UCB1CTL1 |= UCTXSTP;
			}
			__enable_interrupt();
		}
	}
	while (UCB1CTL1 & UCTXSTP)
	{
		WaitCount++;
		if (WaitCount > WaitTime)
		{
			Ret = 0;
			FDC2214_FailCheck();
		}
	}
	if (Ret)
	{
		*Buf = Recv[1] << 8 | Recv[0];
	}
	return Ret;
}

uchar FDC2214_Write(uchar RegAddr, ushort Buf)
{
	uchar Ret = 0;
	ulong WaitCount = 0;
	UCB1CTL1 |= UCTR;
	UCB1CTL1 |= UCTXSTT;
	if (FDC2214_Tx(RegAddr))
	{
		if (FDC2214_Tx(Buf >> 8))
		{
			if (FDC2214_Tx(Buf & 0xFF))
			{
				Ret = 1;
			}
		}
	}
	UCB1CTL1 |= UCTXSTP;
	while (UCB1CTL1 & UCTXSTP)
	{
		WaitCount++;
		if (WaitCount > WaitTime)
		{
			Ret = 0;
			FDC2214_FailCheck();
		}
	}
	return Ret;
}

uchar FDC2214_ModifyRegBit(uchar RegAddr, ushort Bit, uchar Set)
{
	ushort RegVal = 0;
	if (FDC2214_Read(RegAddr, &RegVal))
	{
		if (Set)
		{
			RegVal |= Bit;
		}
		else
		{
			RegVal &= ~Bit;
		}
		return FDC2214_Write(RegAddr, RegVal);
	}
	return 0;
}

uchar FDC2214_ModifyReg(uchar RegAddr, ushort Mask, ushort Shift, ushort Val)
{
	ushort RegVal = 0;
	if (FDC2214_Read(RegAddr, &RegVal))
	{
		RegVal = (RegVal & (~Mask)) | (Val << Shift);
		return FDC2214_Write(RegAddr, RegVal);
	}
	return 0;
}

void FDC2214_Init(void)
{
	ushort DeviceID = 0;

	FDC2214_IICInit();

	do
	{
		FDC2214_Read(FDC2214_DEVICE_ID, &DeviceID);
	} while (DeviceID != FDC2214_ID);

	FDC2214_Reset();

	FDC2214_Write(FDC2214_ERROR_CONFIG, FDC2214_ERROR_CONFIG_WD_ERR2OUT_MASK | FDC2214_ERROR_CONFIG_AH_WARN2OUT_MASK | FDC2214_ERROR_CONFIG_AL_WARN2OUT_MASK | FDC2214_ERROR_CONFIG_DRDY_2INT_MASK);

	ChannelNewDataMask = 0;
}

uchar FDC2214_UpdateChannelData(uchar Channel)
{
	uchar DataRegAddrH = FDC2214_DATA_CH0 + 2 * Channel;
	uchar DataRegAddrL = FDC2214_DATA_LSB_CH0 + 2 * Channel;
	ushort DataRegH = 0;
	ushort DataRegL = 0;
	ulong SensorCounter = 0;
	uchar Res = 1;

	assert(Channel < 4);

	Res &= FDC2214_Read(DataRegAddrH, &DataRegH);
	Res &= FDC2214_Read(DataRegAddrL, &DataRegL);

	SensorCounter = ((ulong)(DataRegH & FDC2214_DATA_MSB_MASK)) << 16 | DataRegL;
	Channels[Channel].Freq = (float)Channels[Channel].SensorDivider * SensorCounter / Channels[Channel].ClockDivider * ClkFreq / (1L << 28);

	if (DataRegH & FDC2214_DATA_ERR_AW_MASK)
	{
		Channels[Channel].AW = 1;
	}
	else
	{
		Channels[Channel].AW = 0;
	}
	if (DataRegH & FDC2214_DATA_ERR_WD_MASK)
	{
		Channels[Channel].WD = 1;
	}
	else
	{
		Channels[Channel].WD = 0;
	}

	ChannelNewDataMask |= 1 << Channel;

	return Res;
}

uchar FDC2214_GetData(void)
{
	uchar i = 0;
	uchar Res = 1;
	ushort StatusVal = 0;
	Res &= FDC2214_Read(FDC2214_STATUS, &StatusVal);
	if (Res)
	{
		uchar i = 0;
		for (i = 0; i < 4; i++)
		{
			if (StatusVal & (FDC2214_STATUS_CH0_UNREADCONV_MASK >> i))
			{
				Res &= FDC2214_UpdateChannelData(i);
			}
		}
	}
	return Res;
}

uchar FDC2214_IsChannelReady(uchar Channel)
{
	assert(Channel < 4);

	if (Channels[Channel].AW || Channels[Channel].WD)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

uchar FDC2214_GetChannelFreq(uchar Channel, float* Freq)
{
	assert(Channel < 4);

	*Freq = Channels[Channel].Freq;
	if (Channels[Channel].AW || Channels[Channel].WD)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

uchar FDC2214_GetChannelCapacitance(uchar Channel, float* Capacitance)
{
	float Freq = 0;
	uchar Ret;
	assert(Channel < 4);
	Ret = FDC2214_GetChannelFreq(Channel, &Freq);
	float Freq0 = Calibration[Channel][0];
	float Cap0 = Calibration[Channel][1];
	float Freq1 = Calibration[Channel][2];
	float Cap1 = Calibration[Channel][3];
	float x0 = 1.0 / (Freq0 * Freq0);
	float x1 = 1.0 / (Freq1 * Freq1);
	float x = 1.0 / (Freq * Freq);
	*Capacitance = (x - x0) * (Cap1 - Cap0) / (x1 - x0) + Cap0;
	return Ret;
}

void FDC2214_ClearNewDataMask(uchar Mask)
{
	ChannelNewDataMask &= ~Mask;
}

uchar FDC2214_GetNewDataMask(void)
{
	return ChannelNewDataMask;
}

/*
tCx=(CHx_RCOUNT*16)/fREFx
tSx= (CHx_SETTLECOUNT*16)/fREFx
CHx_SETTLECOUNT = 0x00, 0x01 ----> tSx = 32 ?fREFx
CHx_SETTLECOUNT > Vpk * fREFx * C * PI^2 / (32 * IDRIVEX)
DifferentalSensor:
0x01 - Sensor Freq 0.01MHz-8.75MHz
0x02 - Sensor Freq 5MHz-10MHz
SingleEndedSensor:
0x02 - Sensor Freq 0.01MHz-10MHz
fREFx = fCLK / CHx_FREF_DIVIDER
fREFx is > 4*fSENSOR
*/
uchar FDC2214_SetChannelConfig(uchar Channel, ushort RCount, ushort SettleCount, ushort RefDivider, FDC2214_CH_FREQ_SEL SensorFreq, FDC2214_IDRIVE Current)
{
	ushort DividerRegVal = 0;
	ushort DriveRegVal = 0;
	assert(Channel < 4);
	assert(RCount > 0x00FF);
	assert(RefDivider > 0);

	switch (SensorFreq)
	{
	default:
		assert(0);
	case MULTI_CH_0M01_8M75:
		DividerRegVal |= FDC2214_CLOCK_DIVIDERS_FIN_SEL_MASK & (0x1 << FDC2214_CLOCK_DIVIDERS_FIN_SEL_SHIFT);
		Channels[Channel].SensorDivider = 1;
		break;
	case MULTI_CH_5M_10M:
	case SINGLE_CH_0M01_10M:
		DividerRegVal |= FDC2214_CLOCK_DIVIDERS_FIN_SEL_MASK & (0x2 << FDC2214_CLOCK_DIVIDERS_FIN_SEL_SHIFT);
		Channels[Channel].SensorDivider = 2;
		break;

	}
	DividerRegVal |= FDC2214_CLOCK_DIVIDERS_FREF_DIVIDER_MASK & (RefDivider << FDC2214_CLOCK_DIVIDERS_FREF_DIVIDER_SHIFT);
	Channels[Channel].ClockDivider = RefDivider;
	DriveRegVal = FDC2214_DRIVE_CURRENT_IDRIVE_MASK & (Current << FDC2214_DRIVE_CURRENT_IDRIVE_SHIFT);

	FDC2214_Write(FDC2214_RCOUNT_CH0 + Channel, RCount);
	FDC2214_Write(FDC2214_SETTLECOUNT_CH0 + Channel, SettleCount);
	FDC2214_Write(FDC2214_CLOCK_DIVIDERS_CH0 + Channel, DividerRegVal);
	FDC2214_Write(FDC2214_DRIVE_CURRENT_CH0 + Channel, DriveRegVal);
}

uchar FDC2214_SetINTB(uchar Enable)
{
	return FDC2214_ModifyRegBit(FDC2214_CONFIG, FDC2214_CONFIG_INTB_DIS_MASK, !Enable);
}

uchar FDC2214_SetSleepMode(uchar Sleep)
{
	return FDC2214_ModifyRegBit(FDC2214_CONFIG, FDC2214_CONFIG_SLEEP_MODE_EN_MASK, Sleep);
}

uchar FDC2214_SetCurrentMode(uchar UserDefined, uchar SingleChannelHighCurrent)
{
	uchar Ret = 1;
	Ret &= FDC2214_ModifyRegBit(FDC2214_CONFIG, FDC2214_CONFIG_SENSOR_ACTIVATE_SEL_MASK, UserDefined);
	Ret &= FDC2214_ModifyRegBit(FDC2214_CONFIG, FDC2214_CONFIG_HIGH_CURRENT_DRV_MASK, SingleChannelHighCurrent);
	return  Ret;
}

uchar FDC2214_SetClockSource(uchar External, float ExtFreq)
{
	ClkFreq = External ? ExtFreq : 43400000.0;
	return FDC2214_ModifyRegBit(FDC2214_CONFIG, FDC2214_CONFIG_REF_CLK_SRC_MASK, External);
}

uchar FDC2214_SetConvertChannel(uchar AutoScan, uchar ChannelSel)
{
	uchar Ret = 1;
	assert(ChannelSel < 4);
	if (ChannelSel == 0 && AutoScan)
	{
		AutoScan = 0;
	}
	Ret &= FDC2214_ModifyRegBit(FDC2214_MUX_CONFIG, FDC2214_MUX_CONFIG_AUTOSCAN_EN_MASK, AutoScan);
	if (AutoScan)
	{
		Ret &= FDC2214_ModifyReg(FDC2214_MUX_CONFIG, FDC2214_MUX_CONFIG_RR_SEQUENCE_MASK, FDC2214_MUX_CONFIG_RR_SEQUENCE_SHIFT, ChannelSel - 1);
	}
	else
	{
		Ret &= FDC2214_ModifyReg(FDC2214_CONFIG, FDC2214_CONFIG_ACTIVE_CHAN_MASK, FDC2214_CONFIG_ACTIVE_CHAN_SHIFT, ChannelSel);
	}
	return Ret;
}

uchar FDC2214_SetDeglitch(FDC2214_DEGLITCH_SEL Val)
{
	return FDC2214_ModifyReg(FDC2214_MUX_CONFIG, FDC2214_MUX_CONFIG_DEGLITCH_MASK, FDC2214_MUX_CONFIG_DEGLITCH_SHIFT, Val);
}

uchar FDC2214_Reset(void)
{
	return FDC2214_ModifyRegBit(FDC2214_RESET_DEV, FDC2214_RESET_DEV_RESET_DEV_MASK, 1);
}




ulong fdc2214_get_channel_data(uchar Channel)
{
	ulong value = 0, value_lsb = 0;
	uchar DataRegAddrH = FDC2214_DATA_CH0 + 2 * Channel;
	uchar DataRegAddrL = FDC2214_DATA_LSB_CH0 + 2 * Channel;
	FDC2214_Read(DataRegAddrH, &value);
	FDC2214_Read(DataRegAddrL, &value_lsb);
	return (value << 16) | (value_lsb & 0x0000ffff);
}

#define PACKET_HEADER_F         0x5A
#define PACKET_HEADER_S         0xA5

static ushort serial_sum_check(uchar* p_buffer, uchar length)
{
	uchar index = length;
	ushort sum = 0;
	while (index--)
	{
		sum += *p_buffer++;
	}
	return sum;
}

static void serial_raw_handle(uchar* p_buffer, ulong data)
{
	p_buffer[0] = (data >> 24) & 0xff;
	p_buffer[1] = (data >> 16) & 0xff;
	p_buffer[2] = (data >> 8) & 0xff;
	p_buffer[3] = data & 0xff;
}

#include "UART.h"

void FDC2214_MonitorDebug(void)
{
	uchar buffer[20] = { 0 };
	buffer[0] = PACKET_HEADER_F;
	buffer[1] = PACKET_HEADER_S;
	serial_raw_handle(buffer + 2, fdc2214_get_channel_data(0));
	serial_raw_handle(buffer + 6, fdc2214_get_channel_data(1));
	serial_raw_handle(buffer + 10, fdc2214_get_channel_data(2));
	serial_raw_handle(buffer + 14, fdc2214_get_channel_data(3));

	ushort check = serial_sum_check(buffer + 2, 16);
	buffer[18] = (check >> 8) & 0xff;
	buffer[19] = check & 0xff;

	UART1_SendBytes(buffer, 20);
}
