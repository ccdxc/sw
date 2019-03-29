/*
 * Copyright (c) 2019 Pensando Systems. All rights reserved.
 */

/*
 * ionic_types.h --
 *
 * Definitions of all types that used by ionic drivers(en/rdma)
 */

#ifndef _IONIC_TYPES_H_
#define _IONIC_TYPES_H_

#include <vmkapi.h>

/*
 *****************************************************************************
 * Data type redefinitions (generic)
 *****************************************************************************
*/

#define BIT(n)  (1UL << (n))
#define ARRAY_SIZE(a) (sizeof(a) / sizeof *(a))
#define  __iomem
#define true 1
#define HZ vmk_TimerCyclesPerSecond()

VMK_ASSERT_LIST(ionic_types,
        VMK_ASSERT_ON_COMPILE(true == VMK_TRUE);
)


typedef vmk_uint8 u8, uint8_t;
typedef vmk_uint16 u16, uint16_t;
typedef vmk_uint32 u32, uint32_t;
typedef vmk_uint64 u64, size_t, uint64_t;
typedef vmk_IOA dma_addr_t;
typedef vmk_Bool bool;
typedef vmk_Lock spinlock_t;

#define ETH_ALEN            VMK_ETH_ADDR_LENGTH
#define DIV_ROUND_UP(n, d)  (((n) + (d) - 1) / (d))
#define BITS_TO_LONGS(nr)   DIV_ROUND_UP(nr, VMK_BITS_PER_BYTE * sizeof(long))
#define DECLARE_BITMAP(name,bits)               \
        unsigned long name[BITS_TO_LONGS(bits)]

#define IONIC_ARRAY_SIZE(a) (sizeof(a) / sizeof *(a))

#endif /* End of _IONIC_TYPES_H_ */

