#ifndef CAP_QUIESCE_API_H
#define CAP_QUIESCE_API_H
#include "LogMsg.h" 
// quiescence
void cap_top_quiesce_continue_test(int chip_id);
void cap_top_quiesce_txs_start(int chip_id);
void cap_top_quiesce_pb_start(int chip_id);
void cap_top_quiesce_sw_phv_insert(int chip_id);
void cap_top_quiesce_txs_stop(int chip_id);
void cap_top_quiesce_pb_stop(int chip_id);
void cap_top_quiesce_pb_poll(int chip_id, const uint32_t* port_10_orig_credits, const uint32_t* port_11_orig_credits, int max_tries);
void cap_top_quiesce_p4p_prd_poll(int chip_id, int max_tries);
void cap_top_quiesce_txs_poll(int chip_id, int max_tries);
void cap_top_quiesce_psp_poll(int chip_id, int max_tries);
void cap_top_quiesce_p4p_tx_poll(int chip_id, int max_tries);
#endif
