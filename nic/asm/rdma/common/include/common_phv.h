#ifndef __COMMON_PHV_H
#define __COMMON_PHV_H

#include "types.h"

#define S2S_DATA_WIDTH 160
#define GLOBAL_DATA_WIDTH 128

#define PT_BASE_ADDR_GET(_r) \
    sll     _r, k.global.pt_base_addr_page_id, HBM_PAGE_SIZE_SHIFT;

#define KT_BASE_ADDR_GET(_r) \
    add     _r, k.global.log_num_pt_entries, CAPRI_LOG_SIZEOF_U64; \
    add     _r, _r, HBM_PAGE_SIZE_SHIFT; \
    sllv    _r, k.global.pt_base_addr_page_id, _r;

#define CQCB_BASE_ADDR_GET(_r) \
    sll     _r, k.global.cqcb_base_addr_page_id, HBM_PAGE_SIZE_SHIFT;

#define CQCB_ADDR_GET(_r, _cqid) \
    CQCB_BASE_ADDR_GET(_r);\
    add _r, _r, _cqid, LOG_SIZEOF_CQCB_T

#define EQCB_BASE_ADDR_GET(_r) \
    add     _r, k.global.log_num_cq_entries, LOG_SIZEOF_CQCB_T; \
    add     _r, _r, HBM_PAGE_SIZE_SHIFT; \
    sllv    _r, k.global.cqcb_base_addr_page_id, _r; 

#define EQCB_ADDR_GET(_r, _eqid) \
    EQCB_BASE_ADDR_GET(_r); \
    add _r, _r, _eqid, LOG_SIZEOF_EQCB_T

#define PHV_GLOBAL_COMMON_T struct phv_global_common_t
struct phv_global_common_t {
    lif: 11;
    qtype: 3;
    qid: 24;
    cb_addr: 25;
    pt_base_addr_page_id: 20;
    log_num_pt_entries: 7;
    cqcb_base_addr_page_id: 20;
    log_num_cq_entries: 5;
    union roce_opcode_flags_t flags;
    //prefetch_pool_base_addr_page_id: 20;
    //log_num_prefetch_pool_entries: 5;
};

struct sram_lif_entry_t {
    pt_base_addr_page_id: 20;
    log_num_pt_entries: 7;
    cqcb_base_addr_page_id: 20;
    log_num_cq_entries: 5;
    //prefetch_pool_base_addr_page_id: 20;
    //log_num_prefetch_pool_entries: 5;
};

#endif //__COMMON_PHV_H
