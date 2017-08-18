#ifndef __CAPRI_HBM_HPP__
#define __CAPRI_HBM_HPP__

#define CAPRI_HBM_MEM_REG_NAME_MAX_LEN 80
#define CARPI_HBM_MEM_NUM_MEM_REGS 50

#define JP4_ATOMIC_STATS         "atomic_stats" 

#define JKEY_REGIONS             "regions"
#define JKEY_REGION_NAME         "name"
#define JKEY_SIZE_KB             "size_kb"
#define JKEY_START_OFF           "start_offset"

#define JP4_PRGM                 "p4_program"
#define JP4PLUS_PRGM             "p4plus_program"
#define JLIF_QSTATE              "qstate"

#include <base.h>

typedef struct capri_hbm_region_s {
    char        mem_reg_name[CAPRI_HBM_MEM_REG_NAME_MAX_LEN];
    uint32_t    size_kb;
    uint32_t    start_offset;
} capri_hbm_region_t;

hal_ret_t capri_hbm_parse();
uint32_t get_start_offset(const char *reg_name);
uint32_t get_size_kb(const char *reg_name);

#define JKEY_REGIONS        "regions"
#define JKEY_REGION_NAME    "name"
#define JKEY_SIZE_KB        "size_kb"
#define JKEY_START_OFF      "start_offset"

#define JP4_PRGM            "p4_program"
#define JP4_SEMAPHORE       "semaphore"

#define CAPRI_HBM_REG_NMDR_RX            "nmdr-rx"
#define CAPRI_HBM_REG_NMDR_TX            "nmdr-tx"
#define CAPRI_HBM_REG_DESCRIPTOR_RX      "descriptor-rx"
#define CAPRI_HBM_REG_DESCRIPTOR_TX      "descriptor-tx"
#define CAPRI_HBM_REG_NMPR_BIG_RX        "nmpr-big-rx"
#define CAPRI_HBM_REG_NMPR_BIG_TX        "nmpr-big-tx"
#define CAPRI_HBM_REG_PAGE_BIG_RX        "page-big-rx"
#define CAPRI_HBM_REG_PAGE_BIG_TX        "page-big-tx"
#define CAPRI_HBM_REG_NMPR_SMALL_RX      "nmpr-small-rx"
#define CAPRI_HBM_REG_NMPR_SMALL_TX      "nmpr-small-tx"
#define CAPRI_HBM_REG_PAGE_SMALL_RX      "page-small-rx"
#define CAPRI_HBM_REG_PAGE_SMALL_TX      "page-small-tx"

#define CAPRI_NUM_SEMAPHORES            512

#define PAGE_SCRATCH_SIZE               512
#define PAGE_SCRATCH_SIZE_BYTES         8

#define    JUMBO_FRAME_SIZE             9216
#define    ETH_FRAME_SIZE               1536

typedef struct capri_descr_s {
        uint64_t        scratch[8];
        uint64_t        A0;
        uint32_t        O0;
        uint32_t        L0;
        uint64_t        A1;
        uint32_t        O1;
        uint32_t        L1;
        uint64_t        A2;
        uint32_t        O2;
        uint32_t        L2;
        uint64_t        next_addr;
        uint64_t        reserved;
} capri_descr_t;

typedef struct capri_big_page_s {
        uint64_t        scratch[PAGE_SCRATCH_SIZE_BYTES];
        char            data[JUMBO_FRAME_SIZE];
} capri_big_page_t;

typedef struct capri_small_page_s {
        uint64_t        scratch[PAGE_SCRATCH_SIZE_BYTES];
        char            data[ETH_FRAME_SIZE];
} capri_small_page_t;

typedef struct capri_semaphore_s {
        uint64_t        pidx;
        uint64_t        cidx;
} capri_semaphore_t;

// TODO: move defines to top level include that can be included by assembly
#define RNMDR_TABLE_BASE        "hbm_rnmdr_table_base"
#define RNMPR_TABLE_BASE        "hbm_rnmpr_table_base"

extern capri_semaphore_t           *g_hbm_semaphore;

#endif    // __CAPRI_HPP__
