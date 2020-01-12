
/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#ifndef __CAP_WDT_H__
#define __CAP_WDT_H__

#define WDT_BASE        0x1400ULL
#define WDT_STRIDE      0x400
#define WDT_CTR_BASE(c) (WDT_BASE + WDT_STRIDE * (c))

#define WDT_CR          0x00
#define WDT_TORR        0x04
#define WDT_CRR         0x0c

#define WDT_CR_ENABLE   0x1
#define WDT_CR_PCLK_256 (0x7 << 2)

#define WDT_KICK_VAL    0x76

#endif

