// Copyright (c) 2019, Pensando Systems Inc.

#ifndef __MPART_RSVD_HPP__
#define __MPART_RSVD_HPP__

#ifdef __cplusplus
namespace sdk {
namespace platform {
namespace utils {
#endif

// TODO , Move to a function.
#define MREGION_BASE_ADDR       0x0C0000000ULL

// Common mem region start offset from base.
#define MREGION_RSVD_START_OFFSET (64 * 1024 * 1024) // 64MB

// Common memory region size
#define MREGION_PCIEMGR_NAME "pciemgr"
#define MREGION_PCIEMGR_START_OFFSET (MREGION_RSVD_START_OFFSET)
#define MREGION_PCIEMGR_SIZE ( 32 * 1024 * 1024) // 32MB

#define MREGION_MPU_TRACE_NAME  "mpu-trace"
#define MREGION_MPU_TRACE_START_OFFSET (MREGION_PCIEMGR_START_OFFSET + MREGION_PCIEMGR_SIZE)
#define MREGION_MPU_TRACE_SIZE (31 * 1024 * 1024 + 768 * 1024) // 31MB+768K

#define MREGION_MEMTUN_NAME "memtun"
#define MREGION_MEMTUN_START_OFFSET (MREGION_MPU_TRACE_START_OFFSET + MREGION_MPU_TRACE_SIZE)
#define MREGION_MEMTUN_SIZE (256 * 1024) // 256KB

// Size used by common regions
#define MREGION_RSVD_END_OFFSET (MREGION_MEMTUN_START_OFFSET + MREGION_MEMTUN_SIZE)

#define MREGIONS_RSVD_COUNT 3

#ifdef __cplusplus
}   // namespace utils
}   // namespace platform
}   // namespace sdk
#endif
#endif   // __MPART_RSVD_HPP__
