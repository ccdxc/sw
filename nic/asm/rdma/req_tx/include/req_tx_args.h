#ifndef __REQ_TX_ARGS_H
#define __REQ_TX_ARGS_H

struct req_tx_sge_to_lkey_info_t {
    sge_va                        : 64;
    sge_bytes                     : 16;
    dma_cmd_start_index           : 8;
    sge_index                     : 8;
    pad                           : 64; 
};

struct req_tx_lkey_to_ptseg_info_t {
    pt_offset                    : 32;
    log_page_size                : 5;
    pt_bytes                     : 16;
    dma_cmd_start_index          : 8;
    sge_index                    : 8;
    pad                          : 91;
};

#endif //__REQ_TX_ARGS_H
