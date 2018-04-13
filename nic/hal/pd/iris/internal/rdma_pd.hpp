#ifndef __HAL_PD_RDMA_HPP__
#define __HAL_PD_RDMA_HPP__

#include "nic/include/base.h"
#include "sdk/ht.hpp"
#include "nic/include/pd.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/gen/common_txdma_actions/include/common_txdma_actions_p4pd.h"
#include "nic/gen/common_rxdma_actions/include/common_rxdma_actions_p4pd.h"

using sdk::lib::ht_ctxt_t;

namespace hal {
namespace pd {

#define MAX_LIFS 2048

extern hal_ret_t p4pd_common_p4plus_rxdma_stage0_rdma_params_table_entry_add (uint32_t idx,
                                                      uint8_t rdma_en_qtype_mask,
                                                      uint32_t pt_base_addr_page_id,
                                                      uint8_t log_num_pt_entries,
                                                      uint32_t cqcb_base_addr_hi,
                                                      uint8_t log_num_cq_entries,
                                                      uint32_t prefetch_pool_base_addr_page_id,
                                                      uint8_t log_num_prefetch_pool_entries,
                                                      uint8_t sq_qtype,
                                                      uint8_t rq_qtype);
extern hal_ret_t p4pd_common_p4plus_rxdma_stage0_rdma_params_table_entry_get(
       uint32_t idx, rx_stage0_load_rdma_params_actiondata *data);

extern hal_ret_t p4pd_common_p4plus_txdma_stage0_rdma_params_table_entry_add (uint32_t idx,
                                                      uint8_t rdma_en_qtype_mask,
                                                      uint32_t pt_base_addr_page_id,
                                                      uint8_t log_num_pt_entries,
                                                      uint32_t cqcb_base_addr_hi,
                                                      uint8_t log_num_cq_entries,
                                                      uint32_t prefetch_pool_base_addr_page_id,
                                                      uint8_t log_num_prefetch_pool_entries,
                                                      uint8_t sq_qtype,
                                                      uint8_t rq_qtype);
extern hal_ret_t p4pd_common_p4plus_txdma_stage0_rdma_params_table_entry_get(
       uint32_t idx, tx_stage0_lif_params_table_actiondata *data);


}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_RDMA_HPP__

