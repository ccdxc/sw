#ifndef __CAPRI_HBM_HPP__
#define __CAPRI_HBM_HPP__

#define CAPRI_HBM_MEM_REG_NAME_MAX_LEN 80
#define CARPI_HBM_MEM_NUM_MEM_REGS 20

#define JKEY_REGIONS             "regions"
#define JKEY_REGION_NAME         "name"
#define JKEY_SIZE_KB             "size_kb"
#define JKEY_START_OFF           "start_offset"

#define JP4_PRGM                 "p4_program"

#include <base.h>

typedef struct capri_hbm_region_s {
    char        mem_reg_name[CAPRI_HBM_MEM_REG_NAME_MAX_LEN];
    uint32_t    size_kb;
    uint32_t    start_offset;
} capri_hbm_region_t;

hal_ret_t capri_hbm_parse();
uint32_t get_start_offset(const char *reg_name);
uint32_t get_size_kb(const char *reg_name);
#endif    // __CAPRI_HPP__
