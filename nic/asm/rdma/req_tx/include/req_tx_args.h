#ifndef __REQ_TX_ARGS_H
#define __REQ_TX_ARGS_H

struct req_tx_sge_to_lkey_info_t {
    sge_va                        : 64;
    sge_bytes                     : 16;
    dma_cmd_start_index           : 8;
    sge_index                     : 8;
    lkey_invalidate               : 1;
    rsvd_key_err                  : 1; 
    pad                           : 62; 
};

struct req_tx_lkey_to_ptseg_info_t {
    pt_offset                    : 32;
    log_page_size                : 5;
    pt_bytes                     : 16;
    dma_cmd_start_index          : 8;
    sge_index                    : 8;
    host_addr                    : 1;
    pad                          : 90;
};

struct req_tx_to_stage_2_t {
    wqe_addr: 64;
    header_template_addr: 32;
    ah_base_addr_page_id: 22;
    pad: 10;
};

#if 0
struct req_tx_sqwqe_to_lkey_mw_info_t {
    va                           : 64;
    len                          : 32;
    r_key                        : 32;
    new_r_key_key                : 8;
    acc_ctrl                     : 8;
    mw_type                      : 2;
    zbva                         : 1;
    index                        : 1;
    pad                          : 12;
};

struct req_tx_sqlkey_to_rkey_mw_info_t {
    va                           : 64;
    len                          : 32;
    mw_pt_base                   : 32;
    new_r_key_key                : 8;
    acc_ctrl                     : 8;
    mw_type                      : 2;
    zbva                         : 1;
    index                        : 1;
    log_page_size                : 5;
    pad                          : 7;
};
#endif

#endif //__REQ_TX_ARGS_H
