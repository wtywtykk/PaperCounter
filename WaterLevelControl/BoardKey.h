#pragma once

#define BOARDKEY_LEFT 0x1
#define BOARDKEY_RIGHT 0x2

void BoardKey_Init(void);
uchar BoardKey_GetLastKey(void);
