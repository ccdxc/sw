#ifndef __AQ_TX_ARGS_H
#define __AQ_TX_ARGS_H

struct aq_tx_to_stage_wqe_info_t {
    cqcb_base_addr_hi       :  24;
    sqcb_base_addr_hi       :  24;
    pad                     :  80;
};

struct aq_tx_to_stage_wqe2_info_t {
    ah_base_addr_page_id    :  22;
    rrq_base_addr_page_id   :  22;
    rsq_base_addr_page_id   :  22;
    pad                     :  62;
};

struct aq_tx_to_stage_fb_info_t {
    cq_num                  :  24;
    pad                     :  104;
};

#endif //__AQ_TX_ARGS_H
