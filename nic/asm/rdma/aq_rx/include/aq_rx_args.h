#ifndef __AQ_RX_ARGS_H
#define __AQ_RX_ARGS_H

struct aq_rx_to_stage_wqe_t {
    rqcb_base_addr_hi                :   24;
    pad                              :  104;
};

struct aq_rx_to_stage_sqcb1_t {
    sqcb_base_addr_hi        :  24;
    rqcb_base_addr_hi        :  24;
    rrq_base_addr            :  32;
    rrq_depth_log2           :   5;
    rrq_valid                :   1;
    err_retry_count          :   3;
    err_retry_count_valid    :   1;
    tx_psn_valid             :   1;
    tx_psn                   :  24;
    pad                      :  13;
};

struct aq_rx_to_stage_rqcb1_t {
    cqcb_base_addr_hi        :    24;
    cq_id                    :    24;
    rsq_base_addr            :    32;
    rsq_depth_log2           :     5;
    rsq_valid                :     1;
    q_key                    :    32;
    q_key_valid              :     1;
    pad                      :     9;
};

struct aq_rx_to_stage_cqcb_t {
    cqcb_base_addr_hi                :   24;
    wqe_id                           :   16;
    log_num_cq_entries               :    4;
    aqcb_addr                        :   28;
    cq_id                            :   24;
    status                           :    8;
    op                               :    8;
    error                            :    1;
    pad                              :   15;
};

#endif //__AQ_RX_ARGS_H
