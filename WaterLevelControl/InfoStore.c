#include "Global.h"
#include "InfoStore.h"
#include <string.h>

#define Info_Key 0x5AA5

InfoBlock Info;   //全部校准数据

uchar* const InfoFlashAddr[4] = { (uchar*)0x1800, (uchar*)0x1880, (uchar*)0x1900, (uchar*)0x1980 };

void LoadInfo(void)
{
	memcpy(&Info, InfoFlashAddr[0], sizeof(Info));
	if (Info.Key != Info_Key)
	{
		ushort i = 0;
		for (i = 0; i < CalibrationCount; i++)
		{
			//Info.LevelCalibration[i] = CalibrationLower + (CalibrationUpper - CalibrationLower) / (CalibrationCount - 1) * i;
			//TODO: 载入默认校准数据
		}
	}
}

void EraseInfo(void)
{
	ushort i = 0;
	uchar* EndAddr = InfoFlashAddr[0] + sizeof(Info);
	__disable_interrupt();
	FCTL3 = FWKEY;
	FCTL1 = FWKEY + ERASE;
	for (i = 0; i < sizeof(InfoFlashAddr) / sizeof(InfoFlashAddr[0]); i++)
	{
		if (EndAddr > InfoFlashAddr[i])
		{
			*InfoFlashAddr[i] = 0;
		}
	}
	FCTL1 = FWKEY;
	FCTL3 = FWKEY + LOCK;
	__enable_interrupt();
}

void SaveInfo()
{
	ushort i;
	uchar* ReadPtr = (uchar*)&Info;
	uchar* WritePtr = InfoFlashAddr[0];
	EraseInfo();
	Info.Key = Info_Key;
	FCTL3 = FWKEY;
	//FCTL1 = FWKEY + BLKWRT;
	FCTL1 = FWKEY + WRT;
	for (i = 0; i < sizeof(Info); i++)
	{
		*WritePtr = *ReadPtr;
		WritePtr++;
		ReadPtr++;
	}
	FCTL1 = FWKEY;
	FCTL3 = FWKEY + LOCK;
}
