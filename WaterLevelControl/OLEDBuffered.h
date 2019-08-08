#include "OLEDHal.h"
uchar OLEDBuffered_BufferRead(uchar X,uchar YLine);
void OLEDBuffered_BufferWrite(uchar X,uchar YLine,uchar Val);
uchar OLEDBuffered_BufferReadBit(uchar X,uchar Y);
void OLEDBuffered_BufferWriteBit(uchar X,uchar Y,uchar Val);
void OLEDBuffered_BufferFulsh(void);
void OLEDBuffered_Init(void);
void OLEDBuffered_Clear(void);
void OLEDBuffered_SetPos(uchar X, uchar YLine);
void OLEDBuffered_WriteData(uchar Data);
