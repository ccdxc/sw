#ifndef __AQ_TX_ARGS_H
#define __AQ_TX_ARGS_H

struct aq_tx_to_stage_wqe_info_t {
    cqcb_base_addr_hi       :  24;
    sqcb_base_addr_hi       :  24;
    rqcb_base_addr_hi       :  24;
    log_num_cq_entries      :   4;
    log_num_kt_entries      :   5;
    log_num_dcqcn_profiles  :   5;
    ah_base_addr_page_id    :  22;
    barmap_base             :  10;
    barmap_size             :   8;
    pad                     :   2;
};

struct aq_tx_to_stage_wqe2_info_t {
    ah_base_addr_page_id             :   22;
    sqcb_base_addr_hi                :   24;
    rqcb_base_addr_hi                :   24;
    log_num_dcqcn_profiles           :    4;
    log_num_kt_entries               :    5;
    pad                              :   49;
};

struct aq_tx_to_stage_sqcb_info_t {
    rqcb_base_addr_hi                :   24;
    pad                              :   104;
};

struct aq_tx_to_stage_fb_info_t {
    wqe_id                  :  16;
    pad                     :  112;
};

struct aq_tx_aqcb_to_wqe_t {
    cb_addr                 :  34;
    rsvd                    :   6;
    map_count_completed     :  32;
    log_num_ah_entries      :   4;
    log_num_eq_entries      :   4;
    log_num_sq_entries      :   4;
    log_num_rq_entries      :   4;
    pad                     :  72;
};

#endif //__AQ_TX_ARGS_H
