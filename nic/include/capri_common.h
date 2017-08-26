#ifndef __CAPRI_COMMON_H
#define __CAPRI_COMMON_H

/*
 * #defines shared by ASM and C code
 */

#define CAPRI_HBM_OFFSET(x)     (0x80000000 + (x))

// Memory regions
#define CAPRI_MEM_SECURE_RAM_START          0
#define CAPRI_MEM_FLASH_START               0x20000
#define CAPRI_MEM_INTR_START                0x40000
#define CAPRI_MEM_SEMA_START                0x40000000

// Semaphores
#define CAPRI_SEM_RAW_OFFSET                0x1000
#define CAPRI_SEM_INC_OFFSET                0x2000
#define CAPRI_SEM_INC_NOT_FULL_OFFSET       0x4000
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

// 1K 32 bit semaphores or 512 PI/CI (inc not full) pairs
#define CAPRI_SEM_RNMDR_ADDR                (CAPRI_MEM_SEMA_START + 8 * 0)
#define CAPRI_RNMDR_RING_SIZE               16384
#define CAPRI_RNMDR_RING_SHIFT              14
#define CAPRI_SEM_RNMDR_RAW_ADDR \
                            (CAPRI_SEM_RNMDR_ADDR + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_RNMDR_INF_ADDR \
                            (CAPRI_SEM_RNMDR_ADDR + CAPRI_SEM_INF_OFFSET)


#define CAPRI_SEM_RNMPR_ADDR                (CAPRI_MEM_SEMA_START + 8 * 1)
#define CAPRI_RNMPR_RING_SIZE               16384
#define CAPRI_RNMPR_RING_SHIFT              14
#define CAPRI_SEM_RNMPR_RAW_ADDR \
                            (CAPRI_SEM_RNMPR_ADDR + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_RNMPR_INF_ADDR \
                            (CAPRI_SEM_RNMPR_ADDR + CAPRI_SEM_INF_OFFSET)


#define CAPRI_SEM_TNMDR_ADDR                (CAPRI_MEM_SEMA_START + 8 * 2)
#define CAPRI_TNMDR_RING_SIZE               16384
#define CAPRI_TNMDR_RING_SHIFT              14
#define CAPRI_SEM_TNMDR_RAW_ADDR \
                            (CAPRI_SEM_TNMDR_ADDR + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_TNMDR_INF_ADDR \
                            (CAPRI_SEM_TNMDR_ADDR + CAPRI_SEM_INF_OFFSET)


#define CAPRI_SEM_TNMPR_ADDR                (CAPRI_MEM_SEMA_START + 8 * 3)
#define CAPRI_TNMPR_RING_SIZE               16384
#define CAPRI_TNMPR_RING_SHIFT              14
#define CAPRI_SEM_TNMPR_RAW_ADDR \
                            (CAPRI_SEM_TNMPR_ADDR + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_TNMPR_INF_ADDR \
                            (CAPRI_SEM_TNMPR_ADDR + CAPRI_SEM_INF_OFFSET)


#define CAPRI_SEM_RNMPR_SMALL_ADDR          (CAPRI_MEM_SEMA_START + 8 * 4)
#define CAPRI_RNMPR_SMALL_RING_SIZE         16384
#define CAPRI_SEM_RNMPR_SMALL_RAW_ADDR \
                            (CAPRI_SEM_RNMPR_SMALL_ADDR + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_RNMPR_SMALL_INF_ADDR \
                            (CAPRI_SEM_RNMPR_SMALL_ADDR + CAPRI_SEM_INF_OFFSET)


#define CAPRI_SEM_TNMPR_SMALL_ADDR          (CAPRI_MEM_SEMA_START + 8 * 5)
#define CAPRI_TNMPR_SMALL_RING_SIZE         16384
#define CAPRI_SEM_TNMPR_SMALL_RAW_ADDR \
                            (CAPRI_SEM_TNMPR_SMALL_ADDR + CAPRI_SEM_RAW_OFFSET)
#define CAPRI_SEM_TNMPR_SMALL_INF_ADDR \
                            (CAPRI_SEM_TNMPR_SMALL_ADDR + CAPRI_SEM_INF_OFFSET)

#endif
