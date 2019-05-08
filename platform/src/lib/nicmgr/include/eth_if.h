/*
* Copyright (c) 2019, Pensando Systems Inc.
*/

#ifndef __ETH_IF_H__
#define __ETH_IF_H__

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

/* Supply these for ionic_if.h */
#define BIT(n)                  (1 << n)
#define BIT_ULL(nr)             (1ULL << (nr))
#define TEST_BIT(x, n)          ((x) & (1 << n))

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
#define __le16 uint16_t
#define __le32 uint32_t
#define __le64 uint64_t
typedef uint64_t dma_addr_t;

#include "platform/drivers/common/ionic_if.h"

#endif  /* __ETH_IF_H__ */
