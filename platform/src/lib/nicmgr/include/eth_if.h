/*
* Copyright (c) 2019, Pensando Systems Inc.
*/

#ifndef __ETH_IF_H__
#define __ETH_IF_H__

#include <stddef.h>
#include <stdint.h>

/* Supply these for ionic_if.h */
#define BIT(n)                  (1 << n)
#define BIT_ULL(nr)             (1ULL << (nr))
#define TEST_BIT(x, n)          ((x) & (1 << n))
#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t
#define __le16 uint16_t
#define __le32 uint32_t
#define __le64 uint64_t
#define dma_addr_t uint64_t

#include "platform/drivers/common/ionic_if.h"

#endif  /* __ETH_IF_H__ */
