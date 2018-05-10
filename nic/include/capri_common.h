#ifndef __CAPRI_COMMON_H
#define __CAPRI_COMMON_H

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
#define CAPRI_SEM_RNMDR_FREE_ADDR           (CAPRI_MEM_SEMA_START + 8 * 1)
#define CAPRI_SEM_RNMPR_ALLOC_ADDR          (CAPRI_MEM_SEMA_START + 8 * 2)
#define CAPRI_SEM_RNMPR_FREE_ADDR           (CAPRI_MEM_SEMA_START + 8 * 3)
#define CAPRI_SEM_TNMDR_ALLOC_ADDR          (CAPRI_MEM_SEMA_START + 8 * 4)
#define CAPRI_SEM_TNMDR_FREE_ADDR           (CAPRI_MEM_SEMA_START + 8 * 5)
#define CAPRI_SEM_TNMPR_ALLOC_ADDR          (CAPRI_MEM_SEMA_START + 8 * 6)
#define CAPRI_SEM_TNMPR_FREE_ADDR           (CAPRI_MEM_SEMA_START + 8 * 7)
#define CAPRI_SEM_RNMPR_SMALL_ALLOC_ADDR    (CAPRI_MEM_SEMA_START + 8 * 8)
#define CAPRI_SEM_RNMPR_SMALL_FREE_ADDR     (CAPRI_MEM_SEMA_START + 8 * 9)
#define CAPRI_SEM_TNMPR_SMALL_ALLOC_ADDR    (CAPRI_MEM_SEMA_START + 8 * 10)
#define CAPRI_SEM_TNMPR_SMALL_FREE_ADDR     (CAPRI_MEM_SEMA_START + 8 * 11)

#define CAPRI_SEM_LAST_PI_CI_SEMAPHORE      (CAPRI_MEM_SEMA_START + 8 * 63)

#define CAPRI_SEM_RAW_IS_PI_CI(_sem_addr) \
                        ((_sem_addr - CAPRI_SEM_RAW_OFFSET) <= \
                         CAPRI_SEM_LAST_PI_CI_SEMAPHORE)

#define CAPRI_MEM_SINGLE_SEMA_START         CAPRI_SEM_LAST_PI_CI_SEMAPHORE

// Semaphores (not PI/CI pairs)
// Define 32 bit (4 byte) semaphores here
#define CAPRI_SEM_TCP_NMDR_IDX_ADDR         (CAPRI_MEM_SINGLE_SEMA_START + 8 * 0)
#define CAPRI_SEM_ARQ_RX_0_IDX_ADDR         (CAPRI_MEM_SINGLE_SEMA_START + 8 * 1)
#define CAPRI_SEM_ARQ_RX_1_IDX_ADDR         (CAPRI_MEM_SINGLE_SEMA_START + 8 * 2)
#define CAPRI_SEM_ARQ_RX_2_IDX_ADDR         (CAPRI_MEM_SINGLE_SEMA_START + 8 * 3)
#define CAPRI_SEM_ARQ_TX_0_IDX_ADDR         (CAPRI_MEM_SINGLE_SEMA_START + 8 * 4)
#define CAPRI_SEM_ARQ_TX_1_IDX_ADDR         (CAPRI_MEM_SINGLE_SEMA_START + 8 * 5)
#define CAPRI_SEM_ARQ_TX_2_IDX_ADDR         (CAPRI_MEM_SINGLE_SEMA_START + 8 * 6)
#define CAPRI_SEM_TLS_RNMDR_IDX_ADDR        (CAPRI_MEM_SINGLE_SEMA_START + 8 * 7)

//#define CAPRI_RNMDR_RING_SIZE               16384
//#define CAPRI_RNMDR_RING_SHIFT              14
#define CAPRI_RNMDR_RING_SIZE               1024
#define CAPRI_RNMDR_RING_SHIFT              10
#define CAPRI_SEM_RNMDR_ALLOC_RAW_ADDR \
                            (CAPRI_SEM_RNMDR_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_RNMDR_ALLOC_CI_RAW_ADDR \
                            (CAPRI_SEM_RNMDR_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET + \
                             CAPRI_SEM_INC_NOT_FULL_CI_OFFSET)
#define CAPRI_SEM_RNMDR_ALLOC_INF_ADDR \
                            (CAPRI_SEM_RNMDR_ALLOC_ADDR + CAPRI_SEM_INF_OFFSET)

#define CAPRI_SEM_RNMDR_FREE_RAW_ADDR \
                            (CAPRI_SEM_RNMDR_FREE_ADDR + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_RNMDR_FREE_INF_ADDR \
                            (CAPRI_SEM_RNMDR_FREE_ADDR + CAPRI_SEM_INF_OFFSET)
#define CAPRI_SEM_RNMDR_FREE_INC_ADDR \
                            (CAPRI_SEM_RNMDR_FREE_ADDR + CAPRI_SEM_INC_OFFSET)

//#define CAPRI_RNMPR_RING_SIZE               16384
//#define CAPRI_RNMPR_RING_SHIFT              14
#define CAPRI_RNMPR_RING_SIZE               1024
#define CAPRI_RNMPR_RING_SHIFT              10
#define CAPRI_SEM_RNMPR_ALLOC_RAW_ADDR \
                            (CAPRI_SEM_RNMPR_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_RNMPR_ALLOC_CI_RAW_ADDR \
                            (CAPRI_SEM_RNMPR_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET + \
                             CAPRI_SEM_INC_NOT_FULL_CI_OFFSET)
#define CAPRI_SEM_RNMPR_ALLOC_INF_ADDR \
                            (CAPRI_SEM_RNMPR_ALLOC_ADDR + CAPRI_SEM_INF_OFFSET)

#define CAPRI_SEM_RNMPR_FREE_RAW_ADDR \
                            (CAPRI_SEM_RNMPR_FREE_ADDR + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_RNMPR_FREE_CI_RAW_ADDR \
                            (CAPRI_SEM_RNMPR_FREE_ADDR + CAPRI_SEM_RAW_OFFSET + \
                             CAPRI_SEM_INC_NOT_FULL_CI_OFFSET)
#define CAPRI_SEM_RNMPR_FREE_INF_ADDR \
                            (CAPRI_SEM_RNMPR_FREE_ADDR + CAPRI_SEM_INF_OFFSET)
#define CAPRI_SEM_RNMPR_FREE_INC_ADDR \
                            (CAPRI_SEM_RNMPR_FREE_ADDR + CAPRI_SEM_INC_OFFSET)


//#define CAPRI_TNMDR_RING_SIZE               16384
//#define CAPRI_TNMDR_RING_SHIFT              14
#define CAPRI_TNMDR_RING_SIZE               1024
#define CAPRI_TNMDR_RING_SHIFT              10
#define CAPRI_SEM_TNMDR_ALLOC_RAW_ADDR \
                            (CAPRI_SEM_TNMDR_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_TNMDR_ALLOC_CI_RAW_ADDR \
                            (CAPRI_SEM_TNMDR_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET + \
                             CAPRI_SEM_INC_NOT_FULL_CI_OFFSET)
#define CAPRI_SEM_TNMDR_ALLOC_INF_ADDR \
                            (CAPRI_SEM_TNMDR_ALLOC_ADDR + CAPRI_SEM_INF_OFFSET)

#define CAPRI_SEM_TNMDR_FREE_RAW_ADDR \
                            (CAPRI_SEM_TNMDR_FREE_ADDR + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_TNMDR_FREE_CI_RAW_ADDR \
                            (CAPRI_SEM_TNMDR_FREE_ADDR + CAPRI_SEM_RAW_OFFSET + \
                             CAPRI_SEM_INC_NOT_FULL_CI_OFFSET)
#define CAPRI_SEM_TNMDR_FREE_INF_ADDR \
                            (CAPRI_SEM_TNMDR_FREE_ADDR + CAPRI_SEM_INF_OFFSET)
#define CAPRI_SEM_TNMDR_FREE_INC_ADDR \
                            (CAPRI_SEM_TNMDR_FREE_ADDR + CAPRI_SEM_INC_OFFSET)


//#define CAPRI_TNMPR_RING_SIZE               16384
//#define CAPRI_TNMPR_RING_SHIFT              14
#define CAPRI_TNMPR_RING_SIZE               1024
#define CAPRI_TNMPR_RING_SHIFT              10
#define CAPRI_SEM_TNMPR_ALLOC_RAW_ADDR \
                            (CAPRI_SEM_TNMPR_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_TNMPR_ALLOC_CI_RAW_ADDR \
                            (CAPRI_SEM_TNMPR_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET + \
                             CAPRI_SEM_INC_NOT_FULL_CI_OFFSET)
#define CAPRI_SEM_TNMPR_ALLOC_INF_ADDR \
                            (CAPRI_SEM_TNMPR_ALLOC_ADDR + CAPRI_SEM_INF_OFFSET)

#define CAPRI_SEM_TNMPR_FREE_RAW_ADDR \
                            (CAPRI_SEM_TNMPR_FREE_ADDR + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_TNMPR_FREE_CI_RAW_ADDR \
                            (CAPRI_SEM_TNMPR_FREE_ADDR + CAPRI_SEM_RAW_OFFSET + \
                             CAPRI_SEM_INC_NOT_FULL_CI_OFFSET)
#define CAPRI_SEM_TNMPR_FREE_INF_ADDR \
                            (CAPRI_SEM_TNMPR_FREE_ADDR + CAPRI_SEM_INF_OFFSET)
#define CAPRI_SEM_TNMPR_FREE_INC_ADDR \
                            (CAPRI_SEM_TNMPR_FREE_ADDR + CAPRI_SEM_INC_OFFSET)


#define CAPRI_RNMPR_SMALL_RING_SIZE         16384
#define CAPRI_RNMPR_SMALL_RING_SHIFT        14
#define CAPRI_SEM_RNMPR_SMALL_ALLOC_RAW_ADDR \
                            (CAPRI_SEM_RNMPR_SMALL_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_RNMPR_SMALL_ALLOC_CI_RAW_ADDR \
                            (CAPRI_SEM_RNMPR_SMALL_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET + \
                             CAPRI_SEM_INC_NOT_FULL_CI_OFFSET)
#define CAPRI_SEM_RNMPR_SMALL_ALLOC_INF_ADDR \
                            (CAPRI_SEM_RNMPR_SMALL_ALLOC_ADDR + CAPRI_SEM_INF_OFFSET)

#define CAPRI_SEM_RNMPR_SMALL_FREE_RAW_ADDR \
                            (CAPRI_SEM_RNMPR_SMALL_FREE_ADDR + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_RNMPR_SMALL_FREE_INF_ADDR \
                            (CAPRI_SEM_RNMPR_SMALL_FREE_ADDR + CAPRI_SEM_INF_OFFSET)

#define CAPRI_TNMPR_SMALL_RING_SIZE         16384

#define CAPRI_SEM_TNMPR_SMALL_ALLOC_RAW_ADDR \
                            (CAPRI_SEM_TNMPR_SMALL_ALLOC_ADDR + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_TNMPR_SMALL_ALLOC_INF_ADDR \
                            (CAPRI_SEM_TNMPR_SMALL_ALLOC_ADDR + CAPRI_SEM_INF_OFFSET)
#define CAPRI_SEM_TNMPR_SMALL_FREE_RAW_ADDR \
                            (CAPRI_SEM_TNMPR_SMALL_FREE_ADDR + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_TNMPR_SMALL_FREE_INF_ADDR \
                            (CAPRI_SEM_TNMPR_SMALL_FREE_ADDR + CAPRI_SEM_INF_OFFSET)

/*
 * 32 bit semaphores
 */
#define CAPRI_SEM_TCP_NMDR_GC_IDX_RAW_ADDR \
                            (CAPRI_SEM_TCP_NMDR_IDX_ADDR + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_TCP_NMDR_GC_IDX_INC_ADDR \
                            (CAPRI_SEM_TCP_NMDR_IDX_ADDR + CAPRI_SEM_INC_OFFSET)
// ARQ Semaphores
#define CAPRI_SEM_ARQ_RX_IDX_ADDR(_queue_id) \
        (CAPRI_SEM_ARQ_RX_0_IDX_ADDR + 8 * _queue_id)
#define CAPRI_SEM_ARQ_RX_IDX_RAW_ADDR(_queue_id) \
                (CAPRI_SEM_ARQ_RX_IDX_ADDR(_queue_id) + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_ARQ_RX_IDX_INC_ADDR(_queue_id) \
                (CAPRI_SEM_ARQ_RX_IDX_ADDR(_queue_id) + CAPRI_SEM_INC_OFFSET)

// TLS Semaphores
#define CAPRI_SEM_TLS_RNMDR_IDX_INC_ADDR    \
        (CAPRI_SEM_TLS_RNMDR_IDX_ADDR + CAPRI_SEM_INC_OFFSET)

// TxDMA
#define CAPRI_SEM_ARQ_TX_IDX_ADDR(_queue_id) \
        (CAPRI_SEM_ARQ_TX_0_IDX_ADDR + 8 * _queue_id)
#define CAPRI_SEM_ARQ_TX_IDX_RAW_ADDR(_queue_id) \
                (CAPRI_SEM_ARQ_TX_IDX_ADDR(_queue_id) + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_ARQ_TX_IDX_INC_ADDR(_queue_id) \
                (CAPRI_SEM_ARQ_TX_IDX_ADDR(_queue_id) + CAPRI_SEM_INC_OFFSET)

/*
 * Garbage collector queueu and ring defines
 */
#define CAPRI_RNMDR_GC_RING_SIZE                1024
#define CAPRI_TNMDR_GC_RING_SIZE                1024

#define CAPRI_HBM_GC_RNMDR_QTYPE                0
#define CAPRI_HBM_GC_TNMDR_QTYPE                1
#define CAPRI_HBM_GC_NUM_QTYPE                  2

#define CAPRI_HBM_GC_PER_PRODUCER_RING_SIZE     256
#define CAPRI_HBM_GC_PER_PRODUCER_RING_SHIFT    8
#define CAPRI_HBM_GC_PER_PRODUCER_RING_MASK     255
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
#define DB_IDX_UPD_NOP                 (0x0 << 2)
#define DB_IDX_UPD_CIDX_SET            (0x1 << 2)
#define DB_IDX_UPD_PIDX_SET            (0x2 << 2)
#define DB_IDX_UPD_PIDX_INC            (0x3 << 2)

#define DB_SCHED_UPD_NOP               (0x0)
#define DB_SCHED_UPD_EVAL              (0x1)
#define DB_SCHED_UPD_CLEAR             (0x2)
#define DB_SCHED_UPD_SET               (0x3)

/*
 * Producer/Consumer ring sizes
 */
#define CAPRI_SERQ_RING_SLOTS_SHIFT     10
#define CAPRI_SERQ_RING_SLOTS_MASK      ((1 << CAPRI_SERQ_RING_SLOTS_SHIFT) - 1)
#define CAPRI_SERQ_RING_SLOTS           (1 << CAPRI_SERQ_RING_SLOTS_SHIFT)
#define CAPRI_SESQ_RING_SLOTS_SHIFT     10
#define CAPRI_SESQ_RING_SLOTS           (1 << CAPRI_SESQ_RING_SLOTS_SHIFT)
#define CAPRI_ASESQ_RING_SLOTS_SHIFT    10
#define CAPRI_ASESQ_RING_SLOTS          (1 << CAPRI_ASESQ_RING_SLOTS_SHIFT)
#define CAPRI_BSQ_RING_SLOTS_SHIFT      10
#define CAPRI_BSQ_RING_SLOTS            (1 << CAPRI_BSQ_RING_SLOTS_SHIFT)
#define CAPRI_BSQ_RING_SLOT_SIZE        4
#define CAPRI_BSQ_RING_SLOT_SIZE_SHFT   2
#define CAPRI_BARCO_RING_SLOTS_SHIFT    10
#define CAPRI_BARCO_RING_SLOTS_MASK     ((1 << CAPRI_BARCO_RING_SLOTS_SHIFT) - 1)

#endif
