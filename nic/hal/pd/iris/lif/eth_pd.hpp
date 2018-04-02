#ifndef __HAL_PD_ETH_HPP__
#define __HAL_PD_ETH_HPP__

#include "nic/include/base.h"
#include "nic/include/pd.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/gen/common_rxdma_actions/include/common_rxdma_actions_p4pd.h"

namespace hal {
namespace pd {

// Maximum number of queue per LIF
#define ETH_RSS_MAX_QUEUES                  (128)
// Number of entries in a LIF's indirection table
#define ETH_RSS_LIF_INDIR_TBL_LEN           ETH_RSS_MAX_QUEUES
// Size of each LIF indirection table entry
#define ETH_RSS_LIF_INDIR_TBL_ENTRY_SZ      (sizeof(eth_rx_rss_indir_eth_rx_rss_indir_t))
// Size of a LIF's indirection table
#define ETH_RSS_LIF_INDIR_TBL_SZ            (ETH_RSS_LIF_INDIR_TBL_LEN * ETH_RSS_LIF_INDIR_TBL_ENTRY_SZ)
// Max number of LIFs supported
#define MAX_LIFS                            (2048)
// Size of the entire LIF indirection table
#define ETH_RSS_INDIR_TBL_SZ                (MAX_LIFS * ETH_RSS_LIF_INDIR_TBL_SZ)

extern hal_ret_t p4pd_common_p4plus_rxdma_rss_params_table_entry_add(
    uint32_t hw_lif_id, uint8_t rss_type, uint8_t *rss_key);
extern hal_ret_t p4pd_common_p4plus_rxdma_rdma_params_table_entry_get(
    uint32_t hw_lif_id, eth_rx_rss_params_actiondata *data);

extern hal_ret_t p4pd_common_p4plus_rxdma_rss_indir_table_entry_add(
    uint32_t hw_lif_id, uint8_t index, uint8_t enable, uint8_t qid);
extern hal_ret_t p4pd_common_p4plus_rxdma_rss_indir_table_entry_get(
    uint32_t hw_lif_id, uint8_t index, eth_rx_rss_indir_actiondata *data);

}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_ETH_HPP__
