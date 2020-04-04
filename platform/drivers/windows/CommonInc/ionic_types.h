#ifndef _IONIC_TYPES_H
#define _IONIC_TYPES_H

/*****************************************************************************
 * Data type redefinitions (generic)
 *****************************************************************************/
typedef UCHAR uint8_t, u_int8_t, u_char, u8;
typedef USHORT uint16_t, u_int16_t, u_short, ushort, u16, __le16;

#ifdef KERNEL_MODE
typedef ULONG32 uint32_t, u_int32_t, u_long, ulong, uintmax_t, u32, __le32, atomic_t;
#else
typedef ULONG32 uint32_t, u_int32_t, ulong, uintmax_t, u32, __le32, atomic_t;
#endif
typedef ULONG64 uint64_t, u_int64_t, u_quad_t, u64, __le64, dma_addr_t;
typedef LONG64 quad_t;
typedef unsigned int u_int, uint;
typedef int intmax_t;
typedef LONG64 phys_addr_t;
typedef ULONG cpumask_t;

typedef u64 netdev_features_t;

#endif