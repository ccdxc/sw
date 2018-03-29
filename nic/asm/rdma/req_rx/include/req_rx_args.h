#ifndef __REQ_RX_ARGS_H
#define __REQ_RX_ARGS_H

#include "capri.h"
#include "common_phv.h"

struct req_rx_to_stage_t {
    msn                     : 24;
    bth_psn                 : 24; 
    syndrome                :  8;
    cqcb_base_addr_page_id  : 22;
    log_num_cq_entries      : 4;
    pad                     : 46;
};

struct req_rx_rrqsge_to_lkey_info_t {
    sge_va                  : 64;
    sge_bytes               : 16;
    log_page_size           : 5;
    is_atomic               : 1;
    dma_cmd_eop             : 1;
    dma_cmd_start_index     : 4;
    sge_index               : 8;
    cq_dma_cmd_index        : 8;
    cq_id                   : 24;
    pad                     : 29;
};

struct req_rx_rrqlkey_to_ptseg_info_t {
    pt_offset               : 32;
    pt_bytes                : 16;
    log_page_size           : 5;
    dma_cmd_start_index     : 4;
    is_atomic               : 1;
    dma_cmd_eop             : 1;
    sge_index               : 8;
    pad                     : 93;
};


#endif //__REQ_RX_ARGS_H
