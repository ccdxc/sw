#ifndef __COMMON_PHV_H
#define __COMMON_PHV_H

#include "types.h"
#include "capri.h"

#define S2S_DATA_WIDTH 160
#define GLOBAL_DATA_WIDTH 128
#define TO_STAGE_DATA_WIDTH 128

#define CQCB_BASE_ADDR_GET(_r, _cqcb_page_id) \
    sll     _r, _cqcb_page_id, HBM_PAGE_SIZE_SHIFT;

#define CQCB_ADDR_GET(_r, _cqid, _cqcb_page_id) \
    CQCB_BASE_ADDR_GET(_r, _cqcb_page_id);\
    add _r, _r, _cqid, LOG_SIZEOF_CQCB_T

#define PHV_GLOBAL_COMMON_T struct phv_global_common_t
struct phv_global_common_t {
    lif: 11;
    qid: 24;
    qtype: 3;
    cb_addr: 25;
    pad: 49;
    pad2: 16;
};

#endif //__COMMON_PHV_H
