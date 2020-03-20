// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
#ifndef __CAPRI_COMMON_HPP_
#define __CAPRI_COMMON_HPP_

#ifdef __cplusplus
namespace sdk {
namespace platform {
namespace capri {
#endif
/*
 * #defines shared by ASM and C code
 */

#define CAPRI_HBM_BASE          0x0C0000000
#define CAPRI_HBM_OFFSET(x)     (CAPRI_HBM_BASE + (x))

// Memory regions
#define CAPRI_MEM_SECURE_RAM_START          0
#define CAPRI_MEM_FLASH_START               0x20000
#define CAPRI_MEM_INTR_START                0x40000
#define CAPRI_MEM_SEMA_START                0x40000000

// Semaphores
#define CAPRI_SEM_RAW_OFFSET                0x1000
#define CAPRI_SEM_INC_OFFSET                0x2000
#define CAPRI_SEM_INC_NOT_FULL_OFFSET       0x4000
#define CAPRI_SEM_ATOMIC_ADD_BYTE_OFFSET    0x8000000
#define CAPRI_SEM_INF_OFFSET                CAPRI_SEM_INC_NOT_FULL_OFFSET
#define CAPRI_SEM_INC_NOT_FULL_SIZE         8
#define CAPRI_SEM_INC_NOT_FULL_PI_OFFSET    0
#define CAPRI_SEM_INC_NOT_FULL_CI_OFFSET    4

#define CAPRI_MEM_SEM_RAW_START \
                    (CAPRI_MEM_SEMA_START + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_MEM_SEM_INC_START \
                    (CAPRI_MEM_SEMA_START + CAPRI_SEM_INC_OFFSET)
#define CAPRI_MEM_SEM_INF_START \
                    (CAPRI_MEM_SEMA_START + CAPRI_SEM_INC_NOT_FULL_OFFSET)
#define CAPRI_MEM_SEM_ATOMIC_ADD_START \
                    (CAPRI_MEM_SEMA_START + CAPRI_SEM_ATOMIC_ADD_BYTE_OFFSET)

/*
 *  HW supports 1024 32 bit semaphores or 512 PI/CI (inc not full) pairs
 */

// PI/CI pairs
#define CAPRI_SEM_RNMDR_ALLOC_ADDR          (CAPRI_MEM_SEMA_START + 8 * 0)
#define CAPRI_SEM_RNMPR_ALLOC_ADDR          (CAPRI_MEM_SEMA_START + 8 * 1)
#define CAPRI_SEM_TNMDR_ALLOC_ADDR          (CAPRI_MEM_SEMA_START + 8 * 2)
#define CAPRI_SEM_TNMPR_ALLOC_ADDR          (CAPRI_MEM_SEMA_START + 8 * 3)
#define CAPRI_SEM_RNMPR_SMALL_ALLOC_ADDR    (CAPRI_MEM_SEMA_START + 8 * 4)
#define CAPRI_SEM_TNMPR_SMALL_ALLOC_ADDR    (CAPRI_MEM_SEMA_START + 8 * 5)
#define CAPRI_SEM_ASCQ_0_ADDR               (CAPRI_MEM_SEMA_START + 8 * 6)
#define CAPRI_SEM_ASCQ_1_ADDR               (CAPRI_MEM_SEMA_START + 8 * 7)
#define CAPRI_SEM_ASCQ_2_ADDR               (CAPRI_MEM_SEMA_START + 8 * 8)
#define CAPRI_SEM_ARQ_0_ADDR                (CAPRI_MEM_SEMA_START + 8 * 9)
#define CAPRI_SEM_ARQ_1_ADDR                (CAPRI_MEM_SEMA_START + 8 * 10)
#define CAPRI_SEM_ARQ_2_ADDR                (CAPRI_MEM_SEMA_START + 8 * 11)

#define CAPRI_SEM_IPSEC_RNMDR_ALLOC_ADDR          (CAPRI_MEM_SEMA_START + 8 * 12)
#define CAPRI_SEM_IPSEC_RNMPR_ALLOC_ADDR          (CAPRI_MEM_SEMA_START + 8 * 13)
#define CAPRI_SEM_IPSEC_TNMDR_ALLOC_ADDR          (CAPRI_MEM_SEMA_START + 8 * 14)
#define CAPRI_SEM_IPSEC_TNMPR_ALLOC_ADDR          (CAPRI_MEM_SEMA_START + 8 * 15)

#define CAPRI_SEM_IPSEC_BIG_RNMDR_ALLOC_ADDR          (CAPRI_MEM_SEMA_START + 8 * 16)
#define CAPRI_SEM_IPSEC_BIG_RNMPR_ALLOC_ADDR          (CAPRI_MEM_SEMA_START + 8 * 17)
#define CAPRI_SEM_IPSEC_BIG_TNMDR_ALLOC_ADDR          (CAPRI_MEM_SEMA_START + 8 * 18)
#define CAPRI_SEM_IPSEC_BIG_TNMPR_ALLOC_ADDR          (CAPRI_MEM_SEMA_START + 8 * 19)

#define CAPRI_SEM_TNMDPR_SMALL_ALLOC_ADDR   (CAPRI_MEM_SEMA_START + 8 * 20)
#define CAPRI_SEM_RNMDPR_SMALL_ALLOC_ADDR   (CAPRI_MEM_SEMA_START + 8 * 21)
#define CAPRI_SEM_TNMDPR_BIG_ALLOC_ADDR     (CAPRI_MEM_SEMA_START + 8 * 22)
#define CAPRI_SEM_RNMDPR_BIG_ALLOC_ADDR     (CAPRI_MEM_SEMA_START + 8 * 23)
#define CAPRI_SEM_CPU_RX_DPR_ALLOC_ADDR     (CAPRI_MEM_SEMA_START + 8 * 24)

#define CAPRI_SEM_TCP_OOQ_ALLOC_ADDR        (CAPRI_MEM_SEMA_START + 8 * 25)

// These need to be deleted but retaining them because proxyc code uses it
#define CAPRI_SEM_RNMPR_SMALL_FREE_ADDR     (CAPRI_MEM_SEMA_START + 8 * 26)
#define CAPRI_SEM_RNMPR_FREE_ADDR           (CAPRI_MEM_SEMA_START + 8 * 27)
#define CAPRI_SEM_RNMDR_SMALL_FREE_ADDR     (CAPRI_MEM_SEMA_START + 8 * 28)
#define CAPRI_SEM_RNMDR_FREE_ADDR           (CAPRI_MEM_SEMA_START + 8 * 29)

#define CAPRI_SEM_TCP_ACTL_Q_0_ADDR       (CAPRI_MEM_SEMA_START + 8 * 30)
#define CAPRI_SEM_TCP_ACTL_Q_1_ADDR       (CAPRI_MEM_SEMA_START + 8 * 31)
#define CAPRI_SEM_TCP_ACTL_Q_2_ADDR       (CAPRI_MEM_SEMA_START + 8 * 32)

#define CAPRI_SEM_LAST_PI_CI_SEMAPHORE      (CAPRI_MEM_SEMA_START + 8 * 63)

#define CAPRI_SEM_RAW_IS_PI_CI(_sem_addr) \
                        ((_sem_addr - CAPRI_SEM_RAW_OFFSET) <= \
                         CAPRI_SEM_LAST_PI_CI_SEMAPHORE)

#define CAPRI_MEM_SINGLE_SEMA_START         CAPRI_SEM_LAST_PI_CI_SEMAPHORE

// Semaphores (not PI/CI pairs)
// Define 32 bit (4 byte) semaphores here
#define CAPRI_SEM_TLS_RNMDR_IDX_ADDR         (CAPRI_MEM_SINGLE_SEMA_START + 8 * 0)

#define CAPRI_RNMDR_RING_SHIFT              12
#define CAPRI_RNMDR_RING_SIZE               (1 << CAPRI_RNMDR_RING_SHIFT)
#define CAPRI_SEM_RNMDR_ALLOC_RAW_ADDR \
                            (CAPRI_SEM_RNMDR_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_RNMDR_ALLOC_CI_RAW_ADDR \
                            (CAPRI_SEM_RNMDR_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET + \
                             CAPRI_SEM_INC_NOT_FULL_CI_OFFSET)
#define CAPRI_SEM_RNMDR_ALLOC_INC_ADDR \
                            (CAPRI_SEM_RNMDR_ALLOC_ADDR + CAPRI_SEM_INC_OFFSET)
#define CAPRI_SEM_RNMDR_ALLOC_INF_ADDR \
                            (CAPRI_SEM_RNMDR_ALLOC_ADDR + CAPRI_SEM_INF_OFFSET)

#define CAPRI_RNMDR_IPSEC_RING_SHIFT              12
#define CAPRI_RNMDR_IPSEC_RING_SIZE               (1 << CAPRI_RNMDR_IPSEC_RING_SHIFT)
#define CAPRI_SEM_IPSEC_RNMDR_ALLOC_RAW_ADDR \
                            (CAPRI_SEM_IPSEC_RNMDR_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_IPSEC_RNMDR_ALLOC_CI_RAW_ADDR \
                            (CAPRI_SEM_IPSEC_RNMDR_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET + \
                             CAPRI_SEM_INC_NOT_FULL_CI_OFFSET)
#define CAPRI_SEM_IPSEC_RNMDR_ALLOC_INC_ADDR \
                            (CAPRI_SEM_IPSEC_RNMDR_ALLOC_ADDR + CAPRI_SEM_INC_OFFSET)
#define CAPRI_SEM_IPSEC_RNMDR_ALLOC_INF_ADDR \
                            (CAPRI_SEM_IPSEC_RNMDR_ALLOC_ADDR + CAPRI_SEM_INF_OFFSET)

#define CAPRI_RNMDR_IPSEC_BIG_RING_SHIFT              12
#define CAPRI_RNMDR_IPSEC_BIG_RING_SIZE               (1 << CAPRI_RNMDR_IPSEC_BIG_RING_SHIFT)
#define CAPRI_SEM_IPSEC_BIG_RNMDR_ALLOC_RAW_ADDR \
                            (CAPRI_SEM_IPSEC_BIG_RNMDR_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_IPSEC_BIG_RNMDR_ALLOC_CI_RAW_ADDR \
                            (CAPRI_SEM_IPSEC_BIG_RNMDR_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET + \
                             CAPRI_SEM_INC_NOT_FULL_CI_OFFSET)
#define CAPRI_SEM_IPSEC_BIG_RNMDR_ALLOC_INC_ADDR \
                            (CAPRI_SEM_IPSEC_BIG_RNMDR_ALLOC_ADDR + CAPRI_SEM_INC_OFFSET)
#define CAPRI_SEM_IPSEC_BIG_RNMDR_ALLOC_INF_ADDR \
                            (CAPRI_SEM_IPSEC_BIG_RNMDR_ALLOC_ADDR + CAPRI_SEM_INF_OFFSET)

#define CAPRI_RNMPR_RING_SHIFT              12
#define CAPRI_RNMPR_RING_SIZE               (1 << CAPRI_RNMPR_RING_SHIFT)
#define CAPRI_SEM_RNMPR_ALLOC_RAW_ADDR \
                            (CAPRI_SEM_RNMPR_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_RNMPR_ALLOC_CI_RAW_ADDR \
                            (CAPRI_SEM_RNMPR_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET + \
                             CAPRI_SEM_INC_NOT_FULL_CI_OFFSET)
#define CAPRI_SEM_RNMPR_ALLOC_INC_ADDR \
                            (CAPRI_SEM_RNMPR_ALLOC_ADDR + CAPRI_SEM_INC_OFFSET)
#define CAPRI_SEM_RNMPR_ALLOC_INF_ADDR \
                            (CAPRI_SEM_RNMPR_ALLOC_ADDR + CAPRI_SEM_INF_OFFSET)

#define CAPRI_RNMPR_IPSEC_RING_SHIFT              12
#define CAPRI_RNMPR_IPSEC_RING_SIZE               (1 << CAPRI_RNMPR_IPSEC_RING_SHIFT)
#define CAPRI_SEM_IPSEC_RNMPR_ALLOC_RAW_ADDR \
                            (CAPRI_SEM_IPSEC_RNMPR_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_IPSEC_RNMPR_ALLOC_CI_RAW_ADDR \
                            (CAPRI_SEM_IPSEC_RNMPR_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET + \
                             CAPRI_SEM_INC_NOT_FULL_CI_OFFSET)
#define CAPRI_SEM_IPSEC_RNMPR_ALLOC_INC_ADDR \
                            (CAPRI_SEM_IPSEC_RNMPR_ALLOC_ADDR + CAPRI_SEM_INC_OFFSET)
#define CAPRI_SEM_IPSEC_RNMPR_ALLOC_INF_ADDR \
                            (CAPRI_SEM_IPSEC_RNMPR_ALLOC_ADDR + CAPRI_SEM_INF_OFFSET)

#define CAPRI_RNMPR_IPSEC_BIG_RING_SHIFT              12
#define CAPRI_RNMPR_IPSEC_BIG_RING_SIZE               (1 << CAPRI_RNMPR_IPSEC_BIG_RING_SHIFT)
#define CAPRI_SEM_IPSEC_BIG_RNMPR_ALLOC_RAW_ADDR \
                            (CAPRI_SEM_IPSEC_BIG_RNMPR_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_IPSEC_BIG_RNMPR_ALLOC_CI_RAW_ADDR \
                            (CAPRI_SEM_IPSEC_BIG_RNMPR_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET + \
                             CAPRI_SEM_INC_NOT_FULL_CI_OFFSET)
#define CAPRI_SEM_IPSEC_BIG_RNMPR_ALLOC_INC_ADDR \
                            (CAPRI_SEM_IPSEC_BIG_RNMPR_ALLOC_ADDR + CAPRI_SEM_INC_OFFSET)
#define CAPRI_SEM_IPSEC_BIG_RNMPR_ALLOC_INF_ADDR \
                            (CAPRI_SEM_IPSEC_BIG_RNMPR_ALLOC_ADDR + CAPRI_SEM_INF_OFFSET)

#define CAPRI_TNMDR_RING_SHIFT              12
#define CAPRI_TNMDR_RING_SIZE               (1 << CAPRI_TNMDR_RING_SHIFT)
#define CAPRI_SEM_TNMDR_ALLOC_RAW_ADDR \
                            (CAPRI_SEM_TNMDR_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_TNMDR_ALLOC_CI_RAW_ADDR \
                            (CAPRI_SEM_TNMDR_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET + \
                             CAPRI_SEM_INC_NOT_FULL_CI_OFFSET)
#define CAPRI_SEM_TNMDR_ALLOC_INF_ADDR \
                            (CAPRI_SEM_TNMDR_ALLOC_ADDR + CAPRI_SEM_INF_OFFSET)
#define CAPRI_SEM_TNMDR_ALLOC_INC_ADDR \
                            (CAPRI_SEM_TNMDR_ALLOC_ADDR + CAPRI_SEM_INC_OFFSET)

#define CAPRI_TNMDR_IPSEC_RING_SHIFT              12
#define CAPRI_TNMDR_IPSEC_RING_SIZE               (1 << CAPRI_TNMDR_IPSEC_RING_SHIFT)
#define CAPRI_SEM_IPSEC_TNMDR_ALLOC_RAW_ADDR \
                            (CAPRI_SEM_IPSEC_TNMDR_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_IPSEC_TNMDR_ALLOC_CI_RAW_ADDR \
                            (CAPRI_SEM_IPSEC_TNMDR_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET + \
                             CAPRI_SEM_INC_NOT_FULL_CI_OFFSET)
#define CAPRI_SEM_IPSEC_TNMDR_ALLOC_INF_ADDR \
                            (CAPRI_SEM_IPSEC_TNMDR_ALLOC_ADDR + CAPRI_SEM_INF_OFFSET)
#define CAPRI_SEM_IPSEC_TNMDR_ALLOC_INC_ADDR \
                            (CAPRI_SEM_IPSEC_TNMDR_ALLOC_ADDR + CAPRI_SEM_INC_OFFSET)

#define CAPRI_TNMDR_IPSEC_BIG_RING_SHIFT              12
#define CAPRI_TNMDR_IPSEC_BIG_RING_SIZE               (1 << CAPRI_TNMDR_IPSEC_BIG_RING_SHIFT)
#define CAPRI_SEM_IPSEC_BIG_TNMDR_ALLOC_RAW_ADDR \
                            (CAPRI_SEM_IPSEC_BIG_TNMDR_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_IPSEC_BIG_TNMDR_ALLOC_CI_RAW_ADDR \
                            (CAPRI_SEM_IPSEC_BIG_TNMDR_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET + \
                             CAPRI_SEM_INC_NOT_FULL_CI_OFFSET)
#define CAPRI_SEM_IPSEC_BIG_TNMDR_ALLOC_INF_ADDR \
                            (CAPRI_SEM_IPSEC_BIG_TNMDR_ALLOC_ADDR + CAPRI_SEM_INF_OFFSET)
#define CAPRI_SEM_IPSEC_BIG_TNMDR_ALLOC_INC_ADDR \
                            (CAPRI_SEM_IPSEC_BIG_TNMDR_ALLOC_ADDR + CAPRI_SEM_INC_OFFSET)

#define CAPRI_TNMPR_RING_SHIFT              12
#define CAPRI_TNMPR_RING_SIZE               (1 << CAPRI_TNMPR_RING_SHIFT)
#define CAPRI_SEM_TNMPR_ALLOC_RAW_ADDR \
                            (CAPRI_SEM_TNMPR_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_TNMPR_ALLOC_CI_RAW_ADDR \
                            (CAPRI_SEM_TNMPR_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET + \
                             CAPRI_SEM_INC_NOT_FULL_CI_OFFSET)
#define CAPRI_SEM_TNMPR_ALLOC_INF_ADDR \
                            (CAPRI_SEM_TNMPR_ALLOC_ADDR + CAPRI_SEM_INF_OFFSET)
#define CAPRI_SEM_TNMPR_ALLOC_INC_ADDR \
                            (CAPRI_SEM_TNMPR_ALLOC_ADDR + CAPRI_SEM_INC_OFFSET)

#define CAPRI_TNMPR_IPSEC_RING_SHIFT              12
#define CAPRI_TNMPR_IPSEC_RING_SIZE               (1 << CAPRI_TNMPR_IPSEC_RING_SHIFT)
#define CAPRI_SEM_IPSEC_TNMPR_ALLOC_RAW_ADDR \
                            (CAPRI_SEM_IPSEC_TNMPR_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_IPSEC_TNMPR_ALLOC_CI_RAW_ADDR \
                            (CAPRI_SEM_IPSEC_TNMPR_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET + \
                             CAPRI_SEM_INC_NOT_FULL_CI_OFFSET)
#define CAPRI_SEM_IPSEC_TNMPR_ALLOC_INF_ADDR \
                            (CAPRI_SEM_IPSEC_TNMPR_ALLOC_ADDR + CAPRI_SEM_INF_OFFSET)
#define CAPRI_SEM_IPSEC_TNMPR_ALLOC_INC_ADDR \
                            (CAPRI_SEM_IPSEC_TNMPR_ALLOC_ADDR + CAPRI_SEM_INC_OFFSET)

#define CAPRI_TNMPR_IPSEC_BIG_RING_SHIFT              12
#define CAPRI_TNMPR_IPSEC_BIG_RING_SIZE               (1 << CAPRI_TNMPR_IPSEC_RING_SHIFT)
#define CAPRI_SEM_IPSEC_BIG_TNMPR_ALLOC_RAW_ADDR \
                            (CAPRI_SEM_IPSEC_BIG_TNMPR_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_IPSEC_BIG_TNMPR_ALLOC_CI_RAW_ADDR \
                            (CAPRI_SEM_IPSEC_BIG_TNMPR_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET + \
                             CAPRI_SEM_INC_NOT_FULL_CI_OFFSET)
#define CAPRI_SEM_IPSEC_BIG_TNMPR_ALLOC_INF_ADDR \
                            (CAPRI_SEM_IPSEC_BIG_TNMPR_ALLOC_ADDR + CAPRI_SEM_INF_OFFSET)
#define CAPRI_SEM_IPSEC_BIG_TNMPR_ALLOC_INC_ADDR \
                            (CAPRI_SEM_IPSEC_BIG_TNMPR_ALLOC_ADDR + CAPRI_SEM_INC_OFFSET)

#define CAPRI_RNMPR_SMALL_RING_SIZE         16384
#define CAPRI_RNMPR_SMALL_RING_SHIFT        14
#define CAPRI_SEM_RNMPR_SMALL_ALLOC_RAW_ADDR \
                            (CAPRI_SEM_RNMPR_SMALL_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_RNMPR_SMALL_ALLOC_CI_RAW_ADDR \
                            (CAPRI_SEM_RNMPR_SMALL_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET + \
                             CAPRI_SEM_INC_NOT_FULL_CI_OFFSET)
#define CAPRI_SEM_RNMPR_SMALL_ALLOC_INF_ADDR \
                            (CAPRI_SEM_RNMPR_SMALL_ALLOC_ADDR + CAPRI_SEM_INF_OFFSET)

#define CAPRI_TNMPR_SMALL_RING_SIZE         16384

#define CAPRI_SEM_TNMPR_SMALL_ALLOC_RAW_ADDR \
                            (CAPRI_SEM_TNMPR_SMALL_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_TNMPR_SMALL_ALLOC_INF_ADDR \
                            (CAPRI_SEM_TNMPR_SMALL_ALLOC_ADDR + CAPRI_SEM_INF_OFFSET)

#define CAPRI_TNMDPR_SMALL_RING_SHIFT       12
#define CAPRI_TNMDPR_SMALL_RING_SIZE        (1 << CAPRI_RNMDPR_SMALL_RING_SHIFT)

#define CAPRI_SEM_TNMDPR_SMALL_ALLOC_RAW_ADDR   \
                            (CAPRI_SEM_TNMDPR_SMALL_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_TNMDPR_SMALL_ALLOC_INF_ADDR   \
                            (CAPRI_SEM_TNMDPR_SMALL_ALLOC_ADDR + CAPRI_SEM_INF_OFFSET)

#define CAPRI_RNMDPR_SMALL_RING_SHIFT       12
#define CAPRI_RNMDPR_SMALL_RING_SIZE        (1 << CAPRI_RNMDPR_SMALL_RING_SHIFT)

#define CAPRI_SEM_RNMDPR_SMALL_ALLOC_RAW_ADDR   \
                            (CAPRI_SEM_RNMDPR_SMALL_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_RNMDPR_SMALL_ALLOC_INF_ADDR   \
                            (CAPRI_SEM_RNMDPR_SMALL_ALLOC_ADDR + CAPRI_SEM_INF_OFFSET)

#define CAPRI_TNMDPR_BIG_RING_SHIFT         13
#define CAPRI_TNMDPR_BIG_RING_SIZE          (1 << CAPRI_TNMDPR_BIG_RING_SHIFT)

#define CAPRI_SEM_TNMDPR_BIG_ALLOC_RAW_ADDR     \
                            (CAPRI_SEM_TNMDPR_BIG_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_TNMDPR_BIG_ALLOC_CI_RAW_ADDR     \
                            (CAPRI_SEM_TNMDPR_BIG_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET + \
                             CAPRI_SEM_INC_NOT_FULL_CI_OFFSET)
#define CAPRI_SEM_TNMDPR_BIG_ALLOC_INC_ADDR     \
                            (CAPRI_SEM_TNMDPR_BIG_ALLOC_ADDR + CAPRI_SEM_INC_OFFSET)
#define CAPRI_SEM_TNMDPR_BIG_ALLOC_INF_ADDR     \
                            (CAPRI_SEM_TNMDPR_BIG_ALLOC_ADDR + CAPRI_SEM_INF_OFFSET)

#define CAPRI_RNMDPR_BIG_RING_SHIFT         13
#define CAPRI_RNMDPR_BIG_RING_SIZE          (1 << CAPRI_RNMDPR_BIG_RING_SHIFT)
#define CAPRI_RNMDPR_BIG_RING_MASK          ((1 << CAPRI_RNMDPR_BIG_RING_SHIFT) - 1)

#define CAPRI_SEM_RNMDPR_BIG_ALLOC_RAW_ADDR     \
                            (CAPRI_SEM_RNMDPR_BIG_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_RNMDPR_BIG_ALLOC_CI_RAW_ADDR     \
                            (CAPRI_SEM_RNMDPR_BIG_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET + \
                             CAPRI_SEM_INC_NOT_FULL_CI_OFFSET)
#define CAPRI_SEM_RNMDPR_BIG_ALLOC_INC_ADDR     \
                            (CAPRI_SEM_RNMDPR_BIG_ALLOC_ADDR + CAPRI_SEM_INC_OFFSET)
#define CAPRI_SEM_RNMDPR_BIG_ALLOC_INF_ADDR     \
                            (CAPRI_SEM_RNMDPR_BIG_ALLOC_ADDR + CAPRI_SEM_INF_OFFSET)

#define CAPRI_CPU_RX_DPR_RING_SHIFT         11
#define CAPRI_CPU_RX_DPR_RING_SIZE          (1 << CAPRI_CPU_RX_DPR_RING_SHIFT)
#define CAPRI_CPU_RX_DPR_RING_MASK          ((1 << CAPRI_CPU_RX_DPR_RING_SHIFT) - 1)
#define CAPRI_CPU_RX_DPR_OBJ_OVERHEADS      (128) /* Descriptor size for now, page metadata overheads TBD */
#define CAPRI_CPU_RX_DPR_OBJ_TOTAL_SIZE     (10 * 1024)
                                                // 10K for now, actually (9216 + CAPRI_CPU_RX_DPR_OBJ_OVERHEADS)
#define CAPRI_CPU_TX_PR_OBJ_TOTAL_SIZE      (10 * 1024)
                                                // 10K for now, actually (9216 + CAPRI_CPU_RX_DPR_OBJ_OVERHEADS)
#define CAPRI_CPU_RX_DPR_PAGE_OFFSET        CAPRI_CPU_RX_DPR_OBJ_OVERHEADS

#define CAPRI_SEM_CPU_RX_DPR_ALLOC_RAW_ADDR     \
                            (CAPRI_SEM_CPU_RX_DPR_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_CPU_RX_DPR_ALLOC_CI_RAW_ADDR     \
                            (CAPRI_SEM_CPU_RX_DPR_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET + \
                             CAPRI_SEM_INC_NOT_FULL_CI_OFFSET)
#define CAPRI_SEM_CPU_RX_DPR_ALLOC_INC_ADDR     \
                            (CAPRI_SEM_CPU_RX_DPR_ALLOC_ADDR + CAPRI_SEM_INC_OFFSET)
#define CAPRI_SEM_CPU_RX_DPR_ALLOC_INF_ADDR     \
                            (CAPRI_SEM_CPU_RX_DPR_ALLOC_ADDR + CAPRI_SEM_INF_OFFSET)

#define CAPRI_NMDPR_OBJ_OVERHEADS           (128) /* Descriptor size for now, page metadata overheads TBD */
#define CAPRI_NMDPR_SMALL_OBJ_TOTAL_SIZE    (2048 + CAPRI_NMDPR_OBJ_OVERHEADS)
#define CAPRI_NMDPR_BIG_OBJ_TOTAL_SIZE      (9216 + CAPRI_NMDPR_OBJ_OVERHEADS)
#define CAPRI_NMDPR_PAGE_OFFSET             CAPRI_NMDPR_OBJ_OVERHEADS

/*
 * OOQ
 */

// Size (num entries) of Allocator ring
#define CAPRI_TCP_ALLOC_OOQ_RING_SHIFT          12
#define CAPRI_TCP_ALLOC_OOQ_RING_SIZE           (1 << CAPRI_TCP_ALLOC_OOQ_RING_SHIFT)
#define CAPRI_TCP_ALLOC_OOQ_RING_MASK           ((1 << CAPRI_TCP_ALLOC_OOQ_RING_SHIFT) - 1)

// Size (num entries) of each object pointed by allocator ring
#define TCP_OOO_QUEUE_SHIFT                 8
#define TCP_OOO_QUEUE_NUM_ENTRIES           (1 << TCP_OOO_QUEUE_SHIFT)
#define TCP_OOO_QUEUE_ALLOC_SIZE            (TCP_OOO_QUEUE_NUM_ENTRIES * 8)

#define CAPRI_SEM_TCP_OOQ_ALLOC_RAW_ADDR \
                            (CAPRI_SEM_TCP_OOQ_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_TCP_OOQ_ALLOC_CI_RAW_ADDR     \
                            (CAPRI_SEM_TCP_OOQ_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET + \
                             CAPRI_SEM_INC_NOT_FULL_CI_OFFSET)
#define CAPRI_SEM_TCP_OOQ_ALLOC_INF_ADDR \
                            (CAPRI_SEM_TCP_OOQ_ALLOC_ADDR + CAPRI_SEM_INF_OFFSET)

// These need to be removed, but proxyc code seems to be using them
#define CAPRI_SEM_RNMDR_FREE_INF_ADDR \
                            (CAPRI_SEM_RNMDR_FREE_ADDR + CAPRI_SEM_INF_OFFSET)
#define CAPRI_SEM_RNMPR_FREE_INF_ADDR \
                            (CAPRI_SEM_RNMPR_FREE_ADDR + CAPRI_SEM_INF_OFFSET)
#define CAPRI_SEM_RNMDR_SMALL_FREE_INF_ADDR \
                            (CAPRI_SEM_RNMDR_SMALL_FREE_ADDR + CAPRI_SEM_INF_OFFSET)
#define CAPRI_SEM_RNMPR_SMALL_FREE_INF_ADDR \
                            (CAPRI_SEM_RNMPR_SMALL_FREE_ADDR + CAPRI_SEM_INF_OFFSET)


/*
 * 32 bit semaphores
 */

// ARQ Semaphores
#define CAPRI_SEM_ARQ_ADDR(_queue_id)                                  \
                            (CAPRI_SEM_ARQ_0_ADDR + 8 * _queue_id)  
#define CAPRI_SEM_ARQ_RAW_ADDR(_queue_id)                              \
                            (CAPRI_SEM_ARQ_ADDR(_queue_id) + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_ARQ_CI_RAW_ADDR(_queue_id)                           \
                            (CAPRI_SEM_ARQ_RAW_ADDR(_queue_id) + CAPRI_SEM_INC_NOT_FULL_CI_OFFSET)
#define CAPRI_SEM_ARQ_INF_ADDR(_queue_id)                              \
                            (CAPRI_SEM_ARQ_ADDR(_queue_id) + CAPRI_SEM_INF_OFFSET)

// TCP ACTL RXQ Semaphores
#define CAPRI_SEM_TCP_ACTL_Q_ADDR(_queue_id)                                  \
                            (CAPRI_SEM_TCP_ACTL_Q_0_ADDR + 8 * _queue_id)
#define CAPRI_SEM_TCP_ACTL_Q_RAW_ADDR(_queue_id)                              \
                            (CAPRI_SEM_TCP_ACTL_Q_ADDR(_queue_id) + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_TCP_ACTL_Q_CI_RAW_ADDR(_queue_id)                           \
                            (CAPRI_SEM_TCP_ACTL_Q_RAW_ADDR(_queue_id) + CAPRI_SEM_INC_NOT_FULL_CI_OFFSET)
#define CAPRI_SEM_TCP_ACTL_Q_INF_ADDR(_queue_id)                              \
                            (CAPRI_SEM_TCP_ACTL_Q_ADDR(_queue_id) + CAPRI_SEM_INF_OFFSET)

// ASCQ Semaphores
#define CAPRI_SEM_ASCQ_ADDR(_queue_id)                                  \
                            (CAPRI_SEM_ASCQ_0_ADDR + 8 * _queue_id)  
#define CAPRI_SEM_ASCQ_RAW_ADDR(_queue_id)                              \
                            (CAPRI_SEM_ASCQ_ADDR(_queue_id) + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_ASCQ_CI_RAW_ADDR(_queue_id)                           \
                            (CAPRI_SEM_ASCQ_RAW_ADDR(_queue_id) + CAPRI_SEM_INC_NOT_FULL_CI_OFFSET)
#define CAPRI_SEM_ASCQ_INF_ADDR(_queue_id)                              \
                            (CAPRI_SEM_ASCQ_ADDR(_queue_id) + CAPRI_SEM_INF_OFFSET)

#define CAPRI_HBM_CPU_TX_DR_RING_SIZE          128
#define CAPRI_HBM_CPU_TX_PR_RING_SIZE          128

// TLS Semaphores
#define CAPRI_SEM_TLS_RNMDR_IDX_INC_ADDR    \
        (CAPRI_SEM_TLS_RNMDR_IDX_ADDR + CAPRI_SEM_INC_OFFSET)

/*
 * Garbage collector queueu and ring defines
 */
#define CAPRI_RNMDR_GC_RING_SIZE                1024
#define CAPRI_TNMDR_GC_RING_SIZE                1024

#define CAPRI_HBM_GC_RNMDR_QTYPE                0
#define CAPRI_HBM_GC_TNMDR_QTYPE                1
#define CAPRI_HBM_GC_NUM_QTYPE                  2

#define CAPRI_HBM_GC_PER_PRODUCER_RING_SHIFT    12
#define CAPRI_HBM_GC_PER_PRODUCER_RING_SIZE     (1 << CAPRI_HBM_GC_PER_PRODUCER_RING_SHIFT)
#define CAPRI_HBM_GC_PER_PRODUCER_RING_MASK     (CAPRI_HBM_GC_PER_PRODUCER_RING_SIZE - 1)
#define CAPRI_HBM_RNMDR_ENTRY_SIZE              8

#define CAPRI_RNMDR_GC_TCP_RING_PRODUCER        0
#define CAPRI_RNMDR_GC_TLS_RING_PRODUCER        1
#define CAPRI_RNMDR_GC_IPSEC_RING_PRODUCER      2
#define CAPRI_RNMDR_GC_CPU_ARM_RING_PRODUCER    3
#define CAPRI_RNMDR_GC_CPU_TXDMA_RING_PRODUCER  4

#define CAPRI_TNMDR_GC_TCP_RING_PRODUCER        0
#define CAPRI_TNMDR_GC_TLS_RING_PRODUCER        1
#define CAPRI_TNMDR_GC_IPSEC_RING_PRODUCER      2

// Timers
#define CAPRI_MEM_TIMER_START           0x20000000
#define CAPRI_MEM_FAST_TIMER_START      (CAPRI_MEM_TIMER_START + 0x4000)
#define CAPRI_MEM_SLOW_TIMER_START      (CAPRI_MEM_TIMER_START + 0x10000)

#define CAPRI_FAST_TIMER_RESOLUTION     1
#define CAPRI_SLOW_TIMER_RESOLUTION     100

#define CAPRI_FAST_TIMER_TICKS(_millisecs) \
                            (_millisecs / CAPRI_FAST_TIMER_RESOLUTION)
#define CAPRI_SLOW_TIMER_TICKS(_millisecs) \
                            (_millisecs / CAPRI_SLOW_TIMER_RESOLUTION)

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

#define _CAPRI_ENCODE_DB_ADDR_UPD_BITS(__ring_upd, __sched_wr) \
   (DB_LOCAL_ADDR_BASE + ((__ring_upd + __sched_wr) << DB_UPD_SHIFT))

#define _CAPRI_SETUP_DB_ADDR(_ring_upd, _sched_wr, _lif, _qtype) \
   (_lif  << DB_LIF_SHIFT) | \
   (_qtype << DB_QTYPE_SHIFT) | \
   _CAPRI_ENCODE_DB_ADDR_UPD_BITS(_ring_upd, _sched_wr);

#define _CAPRI_SETUP_DB_DATA(_qid, _ring_id, _index) \
   (_qid << DB_QID_SHIFT) | \
   (_ring_id << DB_RING_SHIFT) | \
   (_index)

/*
 * Producer/Consumer ring sizes
 */
#define CAPRI_SERQ_RING_SLOTS_SHIFT     10
#define CAPRI_SERQ_RING_SLOTS_MASK      ((1 << CAPRI_SERQ_RING_SLOTS_SHIFT) - 1)
#define CAPRI_SERQ_RING_SLOTS           (1 << CAPRI_SERQ_RING_SLOTS_SHIFT)
#define CAPRI_SESQ_RING_SLOTS_SHIFT     10
#define CAPRI_SESQ_RING_SLOTS           (1 << CAPRI_SESQ_RING_SLOTS_SHIFT)
#define CAPRI_SESQ_RING_SLOTS_MASK      ((1 << CAPRI_SESQ_RING_SLOTS_SHIFT) - 1)
#define CAPRI_ASESQ_RING_SLOTS_SHIFT    10
#define CAPRI_ASESQ_RING_SLOTS          (1 << CAPRI_ASESQ_RING_SLOTS_SHIFT)
#define CAPRI_BSQ_RING_SLOTS_SHIFT      10
#define CAPRI_BSQ_RING_SLOTS            (1 << CAPRI_BSQ_RING_SLOTS_SHIFT)
#define CAPRI_BSQ_RING_SLOT_SIZE        4
#define CAPRI_BSQ_RING_SLOT_SIZE_SHFT   2
#define CAPRI_BARCO_RING_SLOTS_SHIFT    10
#define CAPRI_BARCO_RING_SLOTS_MASK     ((1 << CAPRI_BARCO_RING_SLOTS_SHIFT) - 1)
#define CAPRI_BARCO_RING_SLOTS          (1 << CAPRI_BARCO_RING_SLOTS_SHIFT)
#define CAPRI_BARCO_XTS_RING_SLOTS_SHIFT 12
#define CAPRI_BARCO_XTS_RING_SLOTS_MASK ((1 << CAPRI_BARCO_XTS_RING_SLOTS_SHIFT) - 1)
#define CAPRI_BARCO_XTS_RING_SLOTS      (1 << CAPRI_BARCO_XTS_RING_SLOTS_SHIFT)
#define CAPRI_OOO_RX2TX_RING_SLOTS_SHIFT    5
#define CAPRI_OOO_RX2TX_RING_SLOTS          (1 << CAPRI_OOO_RX2TX_RING_SLOTS_SHIFT)
#define CAPRI_OOO_RX2TX_RING_SLOTS_MASK     ((1 << CAPRI_OOO_RX2TX_RING_SLOTS_SHIFT) - 1)

#define CAPRI_SERQ_ENTRY_SIZE           32
#define CAPRI_SERQ_ENTRY_SIZE_SHIFT     5
#define CAPRI_SERQ_DESC_OFFSET          0
#define CAPRI_SESQ_ENTRY_SIZE           8
#define CAPRI_SESQ_ENTRY_SIZE_SHIFT     3
#define CAPRI_SESQ_DESC_OFFSET          64

// Capri core-clock frequencies
#define CORECLK_FREQ_ASIC_00        1093750000  /* 1093.750MHz (precise) */
#define CORECLK_FREQ_ASIC_01         957031250  /*  957.031MHz (precise) */
#define CORECLK_FREQ_ASIC_10         833333333  /*  833.333MHz (precise) */
#define CORECLK_FREQ_ASIC_11         750250000  /*  750.250MHz (precise) */


#ifdef __cplusplus
} // namespace capri
} // namespace platform
} // namespace sdk
#endif
#endif
