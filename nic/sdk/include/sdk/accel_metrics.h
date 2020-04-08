// {C} Copyright 2020 Pensando Systems Inc. All rights reserved

#ifndef _ACCEL_METRICS_
#define _ACCEL_METRICS_

/*
 * Accelerator sub-rings:
 *  Capri: single sub-ring per ring ID
 *  Post-Capri: multiple sub-rings per
 */
enum {
    ACCEL_SUB_RING0     = 0,
    ACCEL_SUB_RING_MAX,
    ACCEL_SUB_RING_ALL  = 0xffffffff,
};

/**
 * HW ring info metrics layout for Delphi
 */
typedef struct {
    uint64_t    pndx;
    uint64_t    cndx;
    uint64_t    input_bytes;
    uint64_t    output_bytes;
    uint64_t    soft_resets;
} __PACK__ accel_ring_metrics_t;

/**
 * Ring miscellaneous register info, identified by register name
 */
#define ACCEL_RING_REG_NAME_MAX         32
#define ACCEL_RING_NUM_REGS_MAX         32

typedef struct {
    char        name[ACCEL_RING_REG_NAME_MAX];
    uint32_t    val;
} accel_ring_reg_val_t;

#endif /* _ACCEL_METRICS_ */
