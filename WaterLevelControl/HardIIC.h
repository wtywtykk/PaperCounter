#pragma once

void HardIIC_Init(void);
uchar HardIIC_WaitTx(void);
uchar HardIIC_Tx(uchar Val);
uchar HardIIC_Rx(uchar* Val);
uchar HardIIC_Read(uchar RegAddr, uchar* Buf, ushort Len);
uchar HardIIC_Write(uchar RegAddr, uchar* Buf, ushort Len);
