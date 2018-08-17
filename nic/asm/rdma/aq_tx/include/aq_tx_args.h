#ifndef __AQ_TX_ARGS_H
#define __AQ_TX_ARGS_H

struct aq_tx_to_stage_t {
    cqcb_base_addr_hi       :  24;
    log_num_cq_entries      :   4;
    ah_base_addr_page_id    :  22;
    rrq_base_addr_page_id   :  22;
    rsq_base_addr_page_id   :  22;
    pad                     :  34;
};

#endif //__AQ_TX_ARGS_H
