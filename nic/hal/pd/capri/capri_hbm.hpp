#ifndef __CAPRI_HBM_HPP__
#define __CAPRI_HBM_HPP__

#define CAPRI_HBM_MEM_REG_NAME_MAX_LEN 80
#define CARPI_HBM_MEM_NUM_MEM_REGS 20

#define JKEY_REGIONS             "regions"
#define JKEY_REGION_NAME         "name"
#define JKEY_SIZE_KB             "size_kb"
#define JKEY_START_OFF           "start_offset"

#define JP4_PRGM                 "p4_program"
#define JP4_ATOMIC_STATS         "atomic_stats"
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

// TODO: move defines to top level include
#define CAPRI_NUM_SEMAPHORES            512
#define CAPRI_NUM_DESCRIPTORS           9216
#define CAPRI_NUM_BIG_PAGES             3072
#define CAPRI_NUM_SMALL_PAGES           6144

#define CAPRI_DESCRIPTOR_SIZE           128
#define CAPRI_BIG_PAGE_SIZE             9728
#define CAPRI_SMALL_PAGE_SIZE           2048

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

#endif    // __CAPRI_HPP__
