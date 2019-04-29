/*
 * Copyright 2018-2019 Pensando Systems, Inc.  All rights reserved.
 */
 
#ifndef __NVME_COMMON_H__
#define __NVME_COMMON_H__

/**
 * NVME HBM handle as defined in nic/conf/iris/hbm_mem.json
 */
#define NVME_HBM_HANDLE                      "nvme"
#define NVME_QINFO_HBM_HANDLE                "nvme-qinfo"

#ifndef __NVME_ASM_DEFINES_H__

/**
 * nvme_qtype_t - NVME qtype
 */
typedef enum nvme_qtype {
    NVME_QTYPE_SQ        = 0,
    NVME_QTYPE_CQ        = 1,
    NVME_QTYPE_ARMQ      = 2,
    NVME_QTYPE_MAX
} nvme_qtype_t;


/*
 * HW control block (i.e., qstate) definitions
 */
#define HW_CB_SINGLE_SIZE                       64
#define HW_CB_SINGLE_SHFT                       6

#define HW_CB_MULTIPLE(cb_size_shft)            \
    ((cb_size_shft) - HW_CB_SINGLE_SHFT + 1)

#define NVME_SQ_CB_SIZE_SHIFT            6 
#define NVME_CQ_CB_SIZE_SHIFT            6 
#define NVME_ARMQ_CB_SIZE_SHIFT          6 

#endif

#endif    // __NVME_COMMON_H__
