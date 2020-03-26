// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
#ifndef __ASIC_COMMON_HPP__
#define __ASIC_COMMON_HPP__

#ifdef __cplusplus
namespace sdk  {
namespace asic {
#endif
/*
 * #defines shared by ASM and C code
 */

#define ASIC_HBM_BASE          0x0C0000000
#define ASIC_HBM_OFFSET(x)     (ASIC_HBM_BASE + (x))

// Memory regions
#define ASIC_MEM_SECURE_RAM_START          0
#define ASIC_MEM_FLASH_START               0x20000
#define ASIC_MEM_INTR_START                0x40000
#define ASIC_MEM_SEMA_START                0x40000000

// Semaphores
#define ASIC_SEM_RAW_OFFSET                0x1000
#define ASIC_SEM_INC_OFFSET                0x2000
#define ASIC_SEM_INC_NOT_FULL_OFFSET       0x4000
#define ASIC_SEM_ATOMIC_ADD_BYTE_OFFSET    0x8000000
#define ASIC_SEM_INF_OFFSET                ASIC_SEM_INC_NOT_FULL_OFFSET
#define ASIC_SEM_INC_NOT_FULL_SIZE         8
#define ASIC_SEM_INC_NOT_FULL_PI_OFFSET    0
#define ASIC_SEM_INC_NOT_FULL_CI_OFFSET    4

#define ASIC_MEM_SEM_RAW_START \
                    (ASIC_MEM_SEMA_START + ASIC_SEM_RAW_OFFSET)
#define ASIC_MEM_SEM_INC_START \
                    (ASIC_MEM_SEMA_START + ASIC_SEM_INC_OFFSET)
#define ASIC_MEM_SEM_INF_START \
                    (ASIC_MEM_SEMA_START + ASIC_SEM_INC_NOT_FULL_OFFSET)
#define ASIC_MEM_SEM_ATOMIC_ADD_START \
                    (ASIC_MEM_SEMA_START + ASIC_SEM_ATOMIC_ADD_BYTE_OFFSET)

/*
 *  HW supports 1024 32 bit semaphores or 512 PI/CI (inc not full) pairs
 */

// PI/CI pairs
#define ASIC_SEM_RNMDR_ALLOC_ADDR          (ASIC_MEM_SEMA_START + 8 * 0)
#define ASIC_SEM_RNMPR_ALLOC_ADDR          (ASIC_MEM_SEMA_START + 8 * 1)
#define ASIC_SEM_TNMDR_ALLOC_ADDR          (ASIC_MEM_SEMA_START + 8 * 2)
#define ASIC_SEM_TNMPR_ALLOC_ADDR          (ASIC_MEM_SEMA_START + 8 * 3)
#define ASIC_SEM_RNMPR_SMALL_ALLOC_ADDR    (ASIC_MEM_SEMA_START + 8 * 4)
#define ASIC_SEM_TNMPR_SMALL_ALLOC_ADDR    (ASIC_MEM_SEMA_START + 8 * 5)
#define ASIC_SEM_ASCQ_0_ADDR               (ASIC_MEM_SEMA_START + 8 * 6)
#define ASIC_SEM_ASCQ_1_ADDR               (ASIC_MEM_SEMA_START + 8 * 7)
#define ASIC_SEM_ASCQ_2_ADDR               (ASIC_MEM_SEMA_START + 8 * 8)
#define ASIC_SEM_ARQ_0_ADDR                (ASIC_MEM_SEMA_START + 8 * 9)
#define ASIC_SEM_ARQ_1_ADDR                (ASIC_MEM_SEMA_START + 8 * 10)
#define ASIC_SEM_ARQ_2_ADDR                (ASIC_MEM_SEMA_START + 8 * 11)

#define ASIC_SEM_IPSEC_RNMDR_ALLOC_ADDR          (ASIC_MEM_SEMA_START + 8 * 12)
#define ASIC_SEM_IPSEC_RNMPR_ALLOC_ADDR          (ASIC_MEM_SEMA_START + 8 * 13)
#define ASIC_SEM_IPSEC_TNMDR_ALLOC_ADDR          (ASIC_MEM_SEMA_START + 8 * 14)
#define ASIC_SEM_IPSEC_TNMPR_ALLOC_ADDR          (ASIC_MEM_SEMA_START + 8 * 15)

#define ASIC_SEM_IPSEC_BIG_RNMDR_ALLOC_ADDR          (ASIC_MEM_SEMA_START + 8 * 16)
#define ASIC_SEM_IPSEC_BIG_RNMPR_ALLOC_ADDR          (ASIC_MEM_SEMA_START + 8 * 17)
#define ASIC_SEM_IPSEC_BIG_TNMDR_ALLOC_ADDR          (ASIC_MEM_SEMA_START + 8 * 18)
#define ASIC_SEM_IPSEC_BIG_TNMPR_ALLOC_ADDR          (ASIC_MEM_SEMA_START + 8 * 19)

#define ASIC_SEM_TNMDPR_SMALL_ALLOC_ADDR   (ASIC_MEM_SEMA_START + 8 * 20)
#define ASIC_SEM_RNMDPR_SMALL_ALLOC_ADDR   (ASIC_MEM_SEMA_START + 8 * 21)
#define ASIC_SEM_TNMDPR_BIG_ALLOC_ADDR     (ASIC_MEM_SEMA_START + 8 * 22)
#define ASIC_SEM_RNMDPR_BIG_ALLOC_ADDR     (ASIC_MEM_SEMA_START + 8 * 23)
#define ASIC_SEM_CPU_RX_DPR_ALLOC_ADDR     (ASIC_MEM_SEMA_START + 8 * 24)

#define ASIC_SEM_TCP_OOQ_ALLOC_ADDR        (ASIC_MEM_SEMA_START + 8 * 25)

// These need to be deleted but retaining them because proxyc code uses it
#define ASIC_SEM_RNMPR_SMALL_FREE_ADDR     (ASIC_MEM_SEMA_START + 8 * 26)
#define ASIC_SEM_RNMPR_FREE_ADDR           (ASIC_MEM_SEMA_START + 8 * 27)
#define ASIC_SEM_RNMDR_SMALL_FREE_ADDR     (ASIC_MEM_SEMA_START + 8 * 28)
#define ASIC_SEM_RNMDR_FREE_ADDR           (ASIC_MEM_SEMA_START + 8 * 29)

#define ASIC_SEM_TCP_ACTL_Q_0_ADDR       (ASIC_MEM_SEMA_START + 8 * 30)
#define ASIC_SEM_TCP_ACTL_Q_1_ADDR       (ASIC_MEM_SEMA_START + 8 * 31)
#define ASIC_SEM_TCP_ACTL_Q_2_ADDR       (ASIC_MEM_SEMA_START + 8 * 32)

#define ASIC_SEM_LAST_PI_CI_SEMAPHORE      (ASIC_MEM_SEMA_START + 8 * 63)

#define ASIC_SEM_RAW_IS_PI_CI(_sem_addr) \
                        ((_sem_addr - ASIC_SEM_RAW_OFFSET) <= \
                         ASIC_SEM_LAST_PI_CI_SEMAPHORE)

#define ASIC_MEM_SINGLE_SEMA_START         ASIC_SEM_LAST_PI_CI_SEMAPHORE

// Semaphores (not PI/CI pairs)
// Define 32 bit (4 byte) semaphores here
#define ASIC_SEM_TLS_RNMDR_IDX_ADDR         (ASIC_MEM_SINGLE_SEMA_START + 8 * 0)

#define ASIC_RNMDR_RING_SHIFT              12
#define ASIC_RNMDR_RING_SIZE               (1 << ASIC_RNMDR_RING_SHIFT)
#define ASIC_SEM_RNMDR_ALLOC_RAW_ADDR \
                            (ASIC_SEM_RNMDR_ALLOC_ADDR + ASIC_SEM_RAW_OFFSET)
#define ASIC_SEM_RNMDR_ALLOC_CI_RAW_ADDR \
                            (ASIC_SEM_RNMDR_ALLOC_ADDR + ASIC_SEM_RAW_OFFSET + \
                             ASIC_SEM_INC_NOT_FULL_CI_OFFSET)
#define ASIC_SEM_RNMDR_ALLOC_INC_ADDR \
                            (ASIC_SEM_RNMDR_ALLOC_ADDR + ASIC_SEM_INC_OFFSET)
#define ASIC_SEM_RNMDR_ALLOC_INF_ADDR \
                            (ASIC_SEM_RNMDR_ALLOC_ADDR + ASIC_SEM_INF_OFFSET)

#define ASIC_RNMDR_IPSEC_RING_SHIFT              12
#define ASIC_RNMDR_IPSEC_RING_SIZE               (1 << ASIC_RNMDR_IPSEC_RING_SHIFT)
#define ASIC_SEM_IPSEC_RNMDR_ALLOC_RAW_ADDR \
                            (ASIC_SEM_IPSEC_RNMDR_ALLOC_ADDR + ASIC_SEM_RAW_OFFSET)
#define ASIC_SEM_IPSEC_RNMDR_ALLOC_CI_RAW_ADDR \
                            (ASIC_SEM_IPSEC_RNMDR_ALLOC_ADDR + ASIC_SEM_RAW_OFFSET + \
                             ASIC_SEM_INC_NOT_FULL_CI_OFFSET)
#define ASIC_SEM_IPSEC_RNMDR_ALLOC_INC_ADDR \
                            (ASIC_SEM_IPSEC_RNMDR_ALLOC_ADDR + ASIC_SEM_INC_OFFSET)
#define ASIC_SEM_IPSEC_RNMDR_ALLOC_INF_ADDR \
                            (ASIC_SEM_IPSEC_RNMDR_ALLOC_ADDR + ASIC_SEM_INF_OFFSET)

#define ASIC_RNMDR_IPSEC_BIG_RING_SHIFT              12
#define ASIC_RNMDR_IPSEC_BIG_RING_SIZE               (1 << ASIC_RNMDR_IPSEC_BIG_RING_SHIFT)
#define ASIC_SEM_IPSEC_BIG_RNMDR_ALLOC_RAW_ADDR \
                            (ASIC_SEM_IPSEC_BIG_RNMDR_ALLOC_ADDR + ASIC_SEM_RAW_OFFSET)
#define ASIC_SEM_IPSEC_BIG_RNMDR_ALLOC_CI_RAW_ADDR \
                            (ASIC_SEM_IPSEC_BIG_RNMDR_ALLOC_ADDR + ASIC_SEM_RAW_OFFSET + \
                             ASIC_SEM_INC_NOT_FULL_CI_OFFSET)
#define ASIC_SEM_IPSEC_BIG_RNMDR_ALLOC_INC_ADDR \
                            (ASIC_SEM_IPSEC_BIG_RNMDR_ALLOC_ADDR + ASIC_SEM_INC_OFFSET)
#define ASIC_SEM_IPSEC_BIG_RNMDR_ALLOC_INF_ADDR \
                            (ASIC_SEM_IPSEC_BIG_RNMDR_ALLOC_ADDR + ASIC_SEM_INF_OFFSET)

#define ASIC_RNMPR_RING_SHIFT              12
#define ASIC_RNMPR_RING_SIZE               (1 << ASIC_RNMPR_RING_SHIFT)
#define ASIC_SEM_RNMPR_ALLOC_RAW_ADDR \
                            (ASIC_SEM_RNMPR_ALLOC_ADDR + ASIC_SEM_RAW_OFFSET)
#define ASIC_SEM_RNMPR_ALLOC_CI_RAW_ADDR \
                            (ASIC_SEM_RNMPR_ALLOC_ADDR + ASIC_SEM_RAW_OFFSET + \
                             ASIC_SEM_INC_NOT_FULL_CI_OFFSET)
#define ASIC_SEM_RNMPR_ALLOC_INC_ADDR \
                            (ASIC_SEM_RNMPR_ALLOC_ADDR + ASIC_SEM_INC_OFFSET)
#define ASIC_SEM_RNMPR_ALLOC_INF_ADDR \
                            (ASIC_SEM_RNMPR_ALLOC_ADDR + ASIC_SEM_INF_OFFSET)

#define ASIC_RNMPR_IPSEC_RING_SHIFT              12
#define ASIC_RNMPR_IPSEC_RING_SIZE               (1 << ASIC_RNMPR_IPSEC_RING_SHIFT)
#define ASIC_SEM_IPSEC_RNMPR_ALLOC_RAW_ADDR \
                            (ASIC_SEM_IPSEC_RNMPR_ALLOC_ADDR + ASIC_SEM_RAW_OFFSET)
#define ASIC_SEM_IPSEC_RNMPR_ALLOC_CI_RAW_ADDR \
                            (ASIC_SEM_IPSEC_RNMPR_ALLOC_ADDR + ASIC_SEM_RAW_OFFSET + \
                             ASIC_SEM_INC_NOT_FULL_CI_OFFSET)
#define ASIC_SEM_IPSEC_RNMPR_ALLOC_INC_ADDR \
                            (ASIC_SEM_IPSEC_RNMPR_ALLOC_ADDR + ASIC_SEM_INC_OFFSET)
#define ASIC_SEM_IPSEC_RNMPR_ALLOC_INF_ADDR \
                            (ASIC_SEM_IPSEC_RNMPR_ALLOC_ADDR + ASIC_SEM_INF_OFFSET)

#define ASIC_RNMPR_IPSEC_BIG_RING_SHIFT              12
#define ASIC_RNMPR_IPSEC_BIG_RING_SIZE               (1 << ASIC_RNMPR_IPSEC_BIG_RING_SHIFT)
#define ASIC_SEM_IPSEC_BIG_RNMPR_ALLOC_RAW_ADDR \
                            (ASIC_SEM_IPSEC_BIG_RNMPR_ALLOC_ADDR + ASIC_SEM_RAW_OFFSET)
#define ASIC_SEM_IPSEC_BIG_RNMPR_ALLOC_CI_RAW_ADDR \
                            (ASIC_SEM_IPSEC_BIG_RNMPR_ALLOC_ADDR + ASIC_SEM_RAW_OFFSET + \
                             ASIC_SEM_INC_NOT_FULL_CI_OFFSET)
#define ASIC_SEM_IPSEC_BIG_RNMPR_ALLOC_INC_ADDR \
                            (ASIC_SEM_IPSEC_BIG_RNMPR_ALLOC_ADDR + ASIC_SEM_INC_OFFSET)
#define ASIC_SEM_IPSEC_BIG_RNMPR_ALLOC_INF_ADDR \
                            (ASIC_SEM_IPSEC_BIG_RNMPR_ALLOC_ADDR + ASIC_SEM_INF_OFFSET)

#define ASIC_TNMDR_RING_SHIFT              12
#define ASIC_TNMDR_RING_SIZE               (1 << ASIC_TNMDR_RING_SHIFT)
#define ASIC_SEM_TNMDR_ALLOC_RAW_ADDR \
                            (ASIC_SEM_TNMDR_ALLOC_ADDR + ASIC_SEM_RAW_OFFSET)
#define ASIC_SEM_TNMDR_ALLOC_CI_RAW_ADDR \
                            (ASIC_SEM_TNMDR_ALLOC_ADDR + ASIC_SEM_RAW_OFFSET + \
                             ASIC_SEM_INC_NOT_FULL_CI_OFFSET)
#define ASIC_SEM_TNMDR_ALLOC_INF_ADDR \
                            (ASIC_SEM_TNMDR_ALLOC_ADDR + ASIC_SEM_INF_OFFSET)
#define ASIC_SEM_TNMDR_ALLOC_INC_ADDR \
                            (ASIC_SEM_TNMDR_ALLOC_ADDR + ASIC_SEM_INC_OFFSET)

#define ASIC_TNMDR_IPSEC_RING_SHIFT              12
#define ASIC_TNMDR_IPSEC_RING_SIZE               (1 << ASIC_TNMDR_IPSEC_RING_SHIFT)
#define ASIC_SEM_IPSEC_TNMDR_ALLOC_RAW_ADDR \
                            (ASIC_SEM_IPSEC_TNMDR_ALLOC_ADDR + ASIC_SEM_RAW_OFFSET)
#define ASIC_SEM_IPSEC_TNMDR_ALLOC_CI_RAW_ADDR \
                            (ASIC_SEM_IPSEC_TNMDR_ALLOC_ADDR + ASIC_SEM_RAW_OFFSET + \
                             ASIC_SEM_INC_NOT_FULL_CI_OFFSET)
#define ASIC_SEM_IPSEC_TNMDR_ALLOC_INF_ADDR \
                            (ASIC_SEM_IPSEC_TNMDR_ALLOC_ADDR + ASIC_SEM_INF_OFFSET)
#define ASIC_SEM_IPSEC_TNMDR_ALLOC_INC_ADDR \
                            (ASIC_SEM_IPSEC_TNMDR_ALLOC_ADDR + ASIC_SEM_INC_OFFSET)

#define ASIC_TNMDR_IPSEC_BIG_RING_SHIFT              12
#define ASIC_TNMDR_IPSEC_BIG_RING_SIZE               (1 << ASIC_TNMDR_IPSEC_BIG_RING_SHIFT)
#define ASIC_SEM_IPSEC_BIG_TNMDR_ALLOC_RAW_ADDR \
                            (ASIC_SEM_IPSEC_BIG_TNMDR_ALLOC_ADDR + ASIC_SEM_RAW_OFFSET)
#define ASIC_SEM_IPSEC_BIG_TNMDR_ALLOC_CI_RAW_ADDR \
                            (ASIC_SEM_IPSEC_BIG_TNMDR_ALLOC_ADDR + ASIC_SEM_RAW_OFFSET + \
                             ASIC_SEM_INC_NOT_FULL_CI_OFFSET)
#define ASIC_SEM_IPSEC_BIG_TNMDR_ALLOC_INF_ADDR \
                            (ASIC_SEM_IPSEC_BIG_TNMDR_ALLOC_ADDR + ASIC_SEM_INF_OFFSET)
#define ASIC_SEM_IPSEC_BIG_TNMDR_ALLOC_INC_ADDR \
                            (ASIC_SEM_IPSEC_BIG_TNMDR_ALLOC_ADDR + ASIC_SEM_INC_OFFSET)

#define ASIC_TNMPR_RING_SHIFT              12
#define ASIC_TNMPR_RING_SIZE               (1 << ASIC_TNMPR_RING_SHIFT)
#define ASIC_SEM_TNMPR_ALLOC_RAW_ADDR \
                            (ASIC_SEM_TNMPR_ALLOC_ADDR + ASIC_SEM_RAW_OFFSET)
#define ASIC_SEM_TNMPR_ALLOC_CI_RAW_ADDR \
                            (ASIC_SEM_TNMPR_ALLOC_ADDR + ASIC_SEM_RAW_OFFSET + \
                             ASIC_SEM_INC_NOT_FULL_CI_OFFSET)
#define ASIC_SEM_TNMPR_ALLOC_INF_ADDR \
                            (ASIC_SEM_TNMPR_ALLOC_ADDR + ASIC_SEM_INF_OFFSET)
#define ASIC_SEM_TNMPR_ALLOC_INC_ADDR \
                            (ASIC_SEM_TNMPR_ALLOC_ADDR + ASIC_SEM_INC_OFFSET)

#define ASIC_TNMPR_IPSEC_RING_SHIFT              12
#define ASIC_TNMPR_IPSEC_RING_SIZE               (1 << ASIC_TNMPR_IPSEC_RING_SHIFT)
#define ASIC_SEM_IPSEC_TNMPR_ALLOC_RAW_ADDR \
                            (ASIC_SEM_IPSEC_TNMPR_ALLOC_ADDR + ASIC_SEM_RAW_OFFSET)
#define ASIC_SEM_IPSEC_TNMPR_ALLOC_CI_RAW_ADDR \
                            (ASIC_SEM_IPSEC_TNMPR_ALLOC_ADDR + ASIC_SEM_RAW_OFFSET + \
                             ASIC_SEM_INC_NOT_FULL_CI_OFFSET)
#define ASIC_SEM_IPSEC_TNMPR_ALLOC_INF_ADDR \
                            (ASIC_SEM_IPSEC_TNMPR_ALLOC_ADDR + ASIC_SEM_INF_OFFSET)
#define ASIC_SEM_IPSEC_TNMPR_ALLOC_INC_ADDR \
                            (ASIC_SEM_IPSEC_TNMPR_ALLOC_ADDR + ASIC_SEM_INC_OFFSET)

#define ASIC_TNMPR_IPSEC_BIG_RING_SHIFT              12
#define ASIC_TNMPR_IPSEC_BIG_RING_SIZE               (1 << ASIC_TNMPR_IPSEC_RING_SHIFT)
#define ASIC_SEM_IPSEC_BIG_TNMPR_ALLOC_RAW_ADDR \
                            (ASIC_SEM_IPSEC_BIG_TNMPR_ALLOC_ADDR + ASIC_SEM_RAW_OFFSET)
#define ASIC_SEM_IPSEC_BIG_TNMPR_ALLOC_CI_RAW_ADDR \
                            (ASIC_SEM_IPSEC_BIG_TNMPR_ALLOC_ADDR + ASIC_SEM_RAW_OFFSET + \
                             ASIC_SEM_INC_NOT_FULL_CI_OFFSET)
#define ASIC_SEM_IPSEC_BIG_TNMPR_ALLOC_INF_ADDR \
                            (ASIC_SEM_IPSEC_BIG_TNMPR_ALLOC_ADDR + ASIC_SEM_INF_OFFSET)
#define ASIC_SEM_IPSEC_BIG_TNMPR_ALLOC_INC_ADDR \
                            (ASIC_SEM_IPSEC_BIG_TNMPR_ALLOC_ADDR + ASIC_SEM_INC_OFFSET)

#define ASIC_RNMPR_SMALL_RING_SIZE         16384
#define ASIC_RNMPR_SMALL_RING_SHIFT        14
#define ASIC_SEM_RNMPR_SMALL_ALLOC_RAW_ADDR \
                            (ASIC_SEM_RNMPR_SMALL_ALLOC_ADDR + ASIC_SEM_RAW_OFFSET)
#define ASIC_SEM_RNMPR_SMALL_ALLOC_CI_RAW_ADDR \
                            (ASIC_SEM_RNMPR_SMALL_ALLOC_ADDR + ASIC_SEM_RAW_OFFSET + \
                             ASIC_SEM_INC_NOT_FULL_CI_OFFSET)
#define ASIC_SEM_RNMPR_SMALL_ALLOC_INF_ADDR \
                            (ASIC_SEM_RNMPR_SMALL_ALLOC_ADDR + ASIC_SEM_INF_OFFSET)

#define ASIC_TNMPR_SMALL_RING_SIZE         16384

#define ASIC_SEM_TNMPR_SMALL_ALLOC_RAW_ADDR \
                            (ASIC_SEM_TNMPR_SMALL_ALLOC_ADDR + ASIC_SEM_RAW_OFFSET)
#define ASIC_SEM_TNMPR_SMALL_ALLOC_INF_ADDR \
                            (ASIC_SEM_TNMPR_SMALL_ALLOC_ADDR + ASIC_SEM_INF_OFFSET)

#define ASIC_TNMDPR_SMALL_RING_SHIFT       12
#define ASIC_TNMDPR_SMALL_RING_SIZE        (1 << ASIC_RNMDPR_SMALL_RING_SHIFT)

#define ASIC_SEM_TNMDPR_SMALL_ALLOC_RAW_ADDR   \
                            (ASIC_SEM_TNMDPR_SMALL_ALLOC_ADDR + ASIC_SEM_RAW_OFFSET)
#define ASIC_SEM_TNMDPR_SMALL_ALLOC_INF_ADDR   \
                            (ASIC_SEM_TNMDPR_SMALL_ALLOC_ADDR + ASIC_SEM_INF_OFFSET)

#define ASIC_RNMDPR_SMALL_RING_SHIFT       12
#define ASIC_RNMDPR_SMALL_RING_SIZE        (1 << ASIC_RNMDPR_SMALL_RING_SHIFT)

#define ASIC_SEM_RNMDPR_SMALL_ALLOC_RAW_ADDR   \
                            (ASIC_SEM_RNMDPR_SMALL_ALLOC_ADDR + ASIC_SEM_RAW_OFFSET)
#define ASIC_SEM_RNMDPR_SMALL_ALLOC_INF_ADDR   \
                            (ASIC_SEM_RNMDPR_SMALL_ALLOC_ADDR + ASIC_SEM_INF_OFFSET)

#define ASIC_TNMDPR_BIG_RING_SHIFT         13
#define ASIC_TNMDPR_BIG_RING_SIZE          (1 << ASIC_TNMDPR_BIG_RING_SHIFT)

#define ASIC_SEM_TNMDPR_BIG_ALLOC_RAW_ADDR     \
                            (ASIC_SEM_TNMDPR_BIG_ALLOC_ADDR + ASIC_SEM_RAW_OFFSET)
#define ASIC_SEM_TNMDPR_BIG_ALLOC_CI_RAW_ADDR     \
                            (ASIC_SEM_TNMDPR_BIG_ALLOC_ADDR + ASIC_SEM_RAW_OFFSET + \
                             ASIC_SEM_INC_NOT_FULL_CI_OFFSET)
#define ASIC_SEM_TNMDPR_BIG_ALLOC_INC_ADDR     \
                            (ASIC_SEM_TNMDPR_BIG_ALLOC_ADDR + ASIC_SEM_INC_OFFSET)
#define ASIC_SEM_TNMDPR_BIG_ALLOC_INF_ADDR     \
                            (ASIC_SEM_TNMDPR_BIG_ALLOC_ADDR + ASIC_SEM_INF_OFFSET)

#define ASIC_RNMDPR_BIG_RING_SHIFT         13
#define ASIC_RNMDPR_BIG_RING_SIZE          (1 << ASIC_RNMDPR_BIG_RING_SHIFT)
#define ASIC_RNMDPR_BIG_RING_MASK          ((1 << ASIC_RNMDPR_BIG_RING_SHIFT) - 1)

#define ASIC_SEM_RNMDPR_BIG_ALLOC_RAW_ADDR     \
                            (ASIC_SEM_RNMDPR_BIG_ALLOC_ADDR + ASIC_SEM_RAW_OFFSET)
#define ASIC_SEM_RNMDPR_BIG_ALLOC_CI_RAW_ADDR     \
                            (ASIC_SEM_RNMDPR_BIG_ALLOC_ADDR + ASIC_SEM_RAW_OFFSET + \
                             ASIC_SEM_INC_NOT_FULL_CI_OFFSET)
#define ASIC_SEM_RNMDPR_BIG_ALLOC_INC_ADDR     \
                            (ASIC_SEM_RNMDPR_BIG_ALLOC_ADDR + ASIC_SEM_INC_OFFSET)
#define ASIC_SEM_RNMDPR_BIG_ALLOC_INF_ADDR     \
                            (ASIC_SEM_RNMDPR_BIG_ALLOC_ADDR + ASIC_SEM_INF_OFFSET)

#define ASIC_CPU_RX_DPR_RING_SHIFT         10
#define ASIC_CPU_RX_DPR_RING_SIZE          (1 << ASIC_CPU_RX_DPR_RING_SHIFT)
#define ASIC_CPU_RX_DPR_RING_MASK          ((1 << ASIC_CPU_RX_DPR_RING_SHIFT) - 1)
#define ASIC_CPU_RX_DPR_OBJ_OVERHEADS      (128) /* Descriptor size for now, page metadata overheads TBD */
#define ASIC_CPU_RX_DPR_OBJ_TOTAL_SIZE     (10 * 1024)
                                                // 10K for now, actually (9216 + ASIC_CPU_RX_DPR_OBJ_OVERHEADS)
#define ASIC_CPU_TX_PR_OBJ_TOTAL_SIZE      (10 * 1024)
                                                // 10K for now, actually (9216 + ASIC_CPU_RX_DPR_OBJ_OVERHEADS)
#define ASIC_CPU_RX_DPR_PAGE_OFFSET        ASIC_CPU_RX_DPR_OBJ_OVERHEADS

#define ASIC_SEM_CPU_RX_DPR_ALLOC_RAW_ADDR     \
                            (ASIC_SEM_CPU_RX_DPR_ALLOC_ADDR + ASIC_SEM_RAW_OFFSET)
#define ASIC_SEM_CPU_RX_DPR_ALLOC_CI_RAW_ADDR     \
                            (ASIC_SEM_CPU_RX_DPR_ALLOC_ADDR + ASIC_SEM_RAW_OFFSET + \
                             ASIC_SEM_INC_NOT_FULL_CI_OFFSET)
#define ASIC_SEM_CPU_RX_DPR_ALLOC_INC_ADDR     \
                            (ASIC_SEM_CPU_RX_DPR_ALLOC_ADDR + ASIC_SEM_INC_OFFSET)
#define ASIC_SEM_CPU_RX_DPR_ALLOC_INF_ADDR     \
                            (ASIC_SEM_CPU_RX_DPR_ALLOC_ADDR + ASIC_SEM_INF_OFFSET)

#define ASIC_NMDPR_OBJ_OVERHEADS           (128) /* Descriptor size for now, page metadata overheads TBD */
#define ASIC_NMDPR_SMALL_OBJ_TOTAL_SIZE    (2048 + ASIC_NMDPR_OBJ_OVERHEADS)
#define ASIC_NMDPR_BIG_OBJ_TOTAL_SIZE      (9216 + ASIC_NMDPR_OBJ_OVERHEADS)
#define ASIC_NMDPR_PAGE_OFFSET             ASIC_NMDPR_OBJ_OVERHEADS

/*
 * OOQ
 */

// Size (num entries) of Allocator ring
#define ASIC_TCP_ALLOC_OOQ_RING_SHIFT          12
#define ASIC_TCP_ALLOC_OOQ_RING_SIZE           (1 << ASIC_TCP_ALLOC_OOQ_RING_SHIFT)
#define ASIC_TCP_ALLOC_OOQ_RING_MASK           ((1 << ASIC_TCP_ALLOC_OOQ_RING_SHIFT) - 1)

// Size (num entries) of each object pointed by allocator ring
#define TCP_OOO_QUEUE_SHIFT                 8
#define TCP_OOO_QUEUE_NUM_ENTRIES           (1 << TCP_OOO_QUEUE_SHIFT)
#define TCP_OOO_QUEUE_ALLOC_SIZE            (TCP_OOO_QUEUE_NUM_ENTRIES * 8)

#define ASIC_SEM_TCP_OOQ_ALLOC_RAW_ADDR \
                            (ASIC_SEM_TCP_OOQ_ALLOC_ADDR + ASIC_SEM_RAW_OFFSET)
#define ASIC_SEM_TCP_OOQ_ALLOC_CI_RAW_ADDR     \
                            (ASIC_SEM_TCP_OOQ_ALLOC_ADDR + ASIC_SEM_RAW_OFFSET + \
                             ASIC_SEM_INC_NOT_FULL_CI_OFFSET)
#define ASIC_SEM_TCP_OOQ_ALLOC_INF_ADDR \
                            (ASIC_SEM_TCP_OOQ_ALLOC_ADDR + ASIC_SEM_INF_OFFSET)

// These need to be removed, but proxyc code seems to be using them
#define ASIC_SEM_RNMDR_FREE_INF_ADDR \
                            (ASIC_SEM_RNMDR_FREE_ADDR + ASIC_SEM_INF_OFFSET)
#define ASIC_SEM_RNMPR_FREE_INF_ADDR \
                            (ASIC_SEM_RNMPR_FREE_ADDR + ASIC_SEM_INF_OFFSET)
#define ASIC_SEM_RNMDR_SMALL_FREE_INF_ADDR \
                            (ASIC_SEM_RNMDR_SMALL_FREE_ADDR + ASIC_SEM_INF_OFFSET)
#define ASIC_SEM_RNMPR_SMALL_FREE_INF_ADDR \
                            (ASIC_SEM_RNMPR_SMALL_FREE_ADDR + ASIC_SEM_INF_OFFSET)


/*
 * 32 bit semaphores
 */

// ARQ Semaphores
#define ASIC_SEM_ARQ_ADDR(_queue_id)                                  \
                            (ASIC_SEM_ARQ_0_ADDR + 8 * _queue_id)  
#define ASIC_SEM_ARQ_RAW_ADDR(_queue_id)                              \
                            (ASIC_SEM_ARQ_ADDR(_queue_id) + ASIC_SEM_RAW_OFFSET)
#define ASIC_SEM_ARQ_CI_RAW_ADDR(_queue_id)                           \
                            (ASIC_SEM_ARQ_RAW_ADDR(_queue_id) + ASIC_SEM_INC_NOT_FULL_CI_OFFSET)
#define ASIC_SEM_ARQ_INF_ADDR(_queue_id)                              \
                            (ASIC_SEM_ARQ_ADDR(_queue_id) + ASIC_SEM_INF_OFFSET)

// TCP ACTL RXQ Semaphores
#define ASIC_SEM_TCP_ACTL_Q_ADDR(_queue_id)                                  \
                            (ASIC_SEM_TCP_ACTL_Q_0_ADDR + 8 * _queue_id)
#define ASIC_SEM_TCP_ACTL_Q_RAW_ADDR(_queue_id)                              \
                            (ASIC_SEM_TCP_ACTL_Q_ADDR(_queue_id) + ASIC_SEM_RAW_OFFSET)
#define ASIC_SEM_TCP_ACTL_Q_CI_RAW_ADDR(_queue_id)                           \
                            (ASIC_SEM_TCP_ACTL_Q_RAW_ADDR(_queue_id) + ASIC_SEM_INC_NOT_FULL_CI_OFFSET)
#define ASIC_SEM_TCP_ACTL_Q_INF_ADDR(_queue_id)                              \
                            (ASIC_SEM_TCP_ACTL_Q_ADDR(_queue_id) + ASIC_SEM_INF_OFFSET)

// ASCQ Semaphores
#define ASIC_SEM_ASCQ_ADDR(_queue_id)                                  \
                            (ASIC_SEM_ASCQ_0_ADDR + 8 * _queue_id)  
#define ASIC_SEM_ASCQ_RAW_ADDR(_queue_id)                              \
                            (ASIC_SEM_ASCQ_ADDR(_queue_id) + ASIC_SEM_RAW_OFFSET)
#define ASIC_SEM_ASCQ_CI_RAW_ADDR(_queue_id)                           \
                            (ASIC_SEM_ASCQ_RAW_ADDR(_queue_id) + ASIC_SEM_INC_NOT_FULL_CI_OFFSET)
#define ASIC_SEM_ASCQ_INF_ADDR(_queue_id)                              \
                            (ASIC_SEM_ASCQ_ADDR(_queue_id) + ASIC_SEM_INF_OFFSET)

#define ASIC_HBM_CPU_TX_DR_RING_SIZE          128
#define ASIC_HBM_CPU_TX_PR_RING_SIZE          128

// TLS Semaphores
#define ASIC_SEM_TLS_RNMDR_IDX_INC_ADDR    \
        (ASIC_SEM_TLS_RNMDR_IDX_ADDR + ASIC_SEM_INC_OFFSET)

/*
 * Garbage collector queueu and ring defines
 */
#define ASIC_RNMDR_GC_RING_SIZE                1024
#define ASIC_TNMDR_GC_RING_SIZE                1024

#define ASIC_HBM_GC_RNMDR_QTYPE                0
#define ASIC_HBM_GC_TNMDR_QTYPE                1
#define ASIC_HBM_GC_NUM_QTYPE                  2

#define ASIC_HBM_GC_PER_PRODUCER_RING_SHIFT    12
#define ASIC_HBM_GC_PER_PRODUCER_RING_SIZE     (1 << ASIC_HBM_GC_PER_PRODUCER_RING_SHIFT)
#define ASIC_HBM_GC_PER_PRODUCER_RING_MASK     (ASIC_HBM_GC_PER_PRODUCER_RING_SIZE - 1)
#define ASIC_HBM_RNMDR_ENTRY_SIZE              8

#define ASIC_RNMDR_GC_TCP_RING_PRODUCER        0
#define ASIC_RNMDR_GC_TLS_RING_PRODUCER        1
#define ASIC_RNMDR_GC_IPSEC_RING_PRODUCER      2
#define ASIC_RNMDR_GC_CPU_ARM_RING_PRODUCER    3
#define ASIC_RNMDR_GC_CPU_TXDMA_RING_PRODUCER  4

#define ASIC_TNMDR_GC_TCP_RING_PRODUCER        0
#define ASIC_TNMDR_GC_TLS_RING_PRODUCER        1
#define ASIC_TNMDR_GC_IPSEC_RING_PRODUCER      2

// Timers
#define ASIC_MEM_TIMER_START           0x20000000
#define ASIC_MEM_FAST_TIMER_START      (ASIC_MEM_TIMER_START + 0x4000)
#define ASIC_MEM_SLOW_TIMER_START      (ASIC_MEM_TIMER_START + 0x10000)

#define ASIC_FAST_TIMER_RESOLUTION     1
#define ASIC_SLOW_TIMER_RESOLUTION     100

#define ASIC_FAST_TIMER_TICKS(_millisecs) \
                            (_millisecs / ASIC_FAST_TIMER_RESOLUTION)
#define ASIC_SLOW_TIMER_TICKS(_millisecs) \
                            (_millisecs / ASIC_SLOW_TIMER_RESOLUTION)

// Doorbell
#define DB_LOCAL_ADDR_BASE             0x8800000
#define DB_IDX_UPD_NOP                 (0x0 << 2)
#define DB_IDX_UPD_CIDX_SET            (0x1 << 2)
#define DB_IDX_UPD_PIDX_SET            (0x2 << 2)
#define DB_IDX_UPD_PIDX_INC            (0x3 << 2)

#define DB_SCHED_UPD_NOP               (0x0)
#define DB_SCHED_UPD_EVAL              (0x1)
#define DB_SCHED_UPD_CLEAR             (0x2)
#define DB_SCHED_UPD_SET               (0x3)

#define DB_RING_UPD_SHIFT               2
#define DB_UPD_SHIFT                   17
#define DB_LIF_SHIFT                    6
#define DB_QTYPE_SHIFT                  3
#define DB_RING_SHIFT                  16
#define DB_QID_SHIFT                   24

#define _ASIC_ENCODE_DB_ADDR_UPD_BITS(__ring_upd, __sched_wr) \
   (DB_LOCAL_ADDR_BASE + ((__ring_upd + __sched_wr) << DB_UPD_SHIFT))

#define _ASIC_SETUP_DB_ADDR(_ring_upd, _sched_wr, _lif, _qtype) \
   (_lif  << DB_LIF_SHIFT) | \
   (_qtype << DB_QTYPE_SHIFT) | \
   _ASIC_ENCODE_DB_ADDR_UPD_BITS(_ring_upd, _sched_wr);

#define _ASIC_SETUP_DB_DATA(_qid, _ring_id, _index) \
   (_qid << DB_QID_SHIFT) | \
   (_ring_id << DB_RING_SHIFT) | \
   (_index)

/*
 * Producer/Consumer ring sizes
 */
#define ASIC_SERQ_RING_SLOTS_SHIFT     10
#define ASIC_SERQ_RING_SLOTS_MASK      ((1 << ASIC_SERQ_RING_SLOTS_SHIFT) - 1)
#define ASIC_SERQ_RING_SLOTS           (1 << ASIC_SERQ_RING_SLOTS_SHIFT)
#define ASIC_SESQ_RING_SLOTS_SHIFT     10
#define ASIC_SESQ_RING_SLOTS           (1 << ASIC_SESQ_RING_SLOTS_SHIFT)
#define ASIC_SESQ_RING_SLOTS_MASK      ((1 << ASIC_SESQ_RING_SLOTS_SHIFT) - 1)
#define ASIC_ASESQ_RING_SLOTS_SHIFT    10
#define ASIC_ASESQ_RING_SLOTS          (1 << ASIC_ASESQ_RING_SLOTS_SHIFT)
#define ASIC_BSQ_RING_SLOTS_SHIFT      10
#define ASIC_BSQ_RING_SLOTS            (1 << ASIC_BSQ_RING_SLOTS_SHIFT)
#define ASIC_BSQ_RING_SLOT_SIZE        4
#define ASIC_BSQ_RING_SLOT_SIZE_SHFT   2
#define ASIC_BARCO_RING_SLOTS_SHIFT    10
#define ASIC_BARCO_RING_SLOTS_MASK     ((1 << ASIC_BARCO_RING_SLOTS_SHIFT) - 1)
#define ASIC_BARCO_RING_SLOTS          (1 << ASIC_BARCO_RING_SLOTS_SHIFT)
#define ASIC_BARCO_XTS_RING_SLOTS_SHIFT 12
#define ASIC_BARCO_XTS_RING_SLOTS_MASK ((1 << ASIC_BARCO_XTS_RING_SLOTS_SHIFT) - 1)
#define ASIC_BARCO_XTS_RING_SLOTS      (1 << ASIC_BARCO_XTS_RING_SLOTS_SHIFT)
#define ASIC_OOO_RX2TX_RING_SLOTS_SHIFT    5
#define ASIC_OOO_RX2TX_RING_SLOTS          (1 << ASIC_OOO_RX2TX_RING_SLOTS_SHIFT)
#define ASIC_OOO_RX2TX_RING_SLOTS_MASK     ((1 << ASIC_OOO_RX2TX_RING_SLOTS_SHIFT) - 1)

#define ASIC_SERQ_ENTRY_SIZE           32
#define ASIC_SERQ_ENTRY_SIZE_SHIFT     5
#define ASIC_SERQ_DESC_OFFSET          0
#define ASIC_SESQ_ENTRY_SIZE           8
#define ASIC_SESQ_ENTRY_SIZE_SHIFT     3
#define ASIC_SESQ_DESC_OFFSET          64

// ASIC core-clock frequencies
#define CORECLK_FREQ_ASIC_00        1093750000  /* 1093.750MHz (precise) */
#define CORECLK_FREQ_ASIC_01         957031250  /*  957.031MHz (precise) */
#define CORECLK_FREQ_ASIC_10         833333333  /*  833.333MHz (precise) */
#define CORECLK_FREQ_ASIC_11         750250000  /*  750.250MHz (precise) */


#ifdef __cplusplus
}    // namespace asic 
}    // namespace sdk
#endif
#endif    // __ASIC_COMMON_HPP__
