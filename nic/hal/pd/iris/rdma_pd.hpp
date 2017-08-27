#ifndef __HAL_PD_RDMA_HPP__
#define __HAL_PD_RDMA_HPP__

#include <base.h>
#include <ht.hpp>
#include <pd.hpp>
#include <hal_state_pd.hpp>
#include <common_rxdma_actions_p4pd.h>

using hal::utils::ht_ctxt_t;

namespace hal {
namespace pd {

#define MAX_LIFS 2048

extern hal_ret_t p4pd_common_p4plus_rxdma_stage0_lif_table1_entry_add (uint32_t idx,
                                                      uint32_t pt_base_addr_page_id,
                                                      uint8_t log_num_pt_entries,
                                                      uint32_t cqcb_base_addr_page_id,
                                                      uint8_t log_num_cq_entries,
                                                      uint32_t prefetch_pool_base_addr_page_id,
                                                      uint8_t log_num_prefetch_pool_entries);
extern hal_ret_t p4pd_common_p4plus_rxdma_stage0_lif_table1_entry_get(
       uint32_t idx, common_p4plus_stage0_lif_table1_actiondata *data);

extern hal_ret_t p4pd_common_p4plus_txdma_stage0_lif_table1_entry_get(
       uint32_t idx, common_p4plus_stage0_lif_table1_actiondata *data);


}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_RDMA_HPP__

