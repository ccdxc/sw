#ifndef __AQ_RX_ARGS_H
#define __AQ_RX_ARGS_H

struct aq_rx_to_stage_t {
    cqcb_base_addr_hi                :   24;
    log_num_cq_entries               :    4;
    bth_se                           :    1;
    sqcb_base_addr_hi                :   24;
    rqcb_base_addr_hi                :   24;
    pad                              :   51;
};

#endif //__AQ_RX_ARGS_H
