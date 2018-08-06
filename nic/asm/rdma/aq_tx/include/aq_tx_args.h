#ifndef __AQ_TX_ARGS_H
#define __AQ_TX_ARGS_H

struct aq_tx_to_stage_t {
    cqcb_base_addr_hi       : 24;
    log_num_cq_entries      : 4;
    pad                     : 100;
};

#endif //__AQ_TX_ARGS_H
