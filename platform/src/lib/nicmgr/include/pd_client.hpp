/*
* Copyright (c) 2018, Pensando Systems Inc.
*/

#ifndef __PD_STATE_HPP__
#define __PD_STATE_HPP__

#include "hal_client.hpp"
#include "lif_manager.hpp"
#include "include/sdk/platform/utils/mpartition.hpp"
#include "nic/sdk/include/sdk/directmap.hpp"
#include "common_rxdma_actions_p4pd.h"
#include "capri_hbm.hpp"

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

class PdClient {
public:
    static PdClient* factory(platform_mode_t platform);
    void destroy(PdClient *pdc);

    char *hal_cfg_path_;
    platform_mode_t platform_;
    class sdk::platform::program_info *pinfo_;
    class sdk::platform::utils::mpartition *mp_;
    class NicLIFManager *lm_;
    directmap    **p4plus_rxdma_dm_tables_;

    int lif_qstate_map_init(uint64_t hw_lif_id,
                            struct queue_info* queue_info,
                            uint8_t coses);

    int lif_qstate_init(uint64_t hw_lif_id, struct queue_info* queue_info);

    int program_qstate(struct queue_info* queue_info,
                       struct lif_info *lif_info,
                       uint8_t coses);

    int p4plus_rxdma_init_tables();
    int pd_state_init();

    int p4pd_common_p4plus_rxdma_rss_params_table_entry_add(
            uint32_t hw_lif_id, uint8_t rss_type, uint8_t *rss_key);
    int p4pd_common_p4plus_rxdma_rdma_params_table_entry_get(
            uint32_t hw_lif_id, eth_rx_rss_params_actiondata *data);

    int p4pd_common_p4plus_rxdma_rss_indir_table_entry_add(
            uint32_t hw_lif_id, uint8_t index, uint8_t enable, uint8_t qid);
    int p4pd_common_p4plus_rxdma_rss_indir_table_entry_get(
            uint32_t hw_lif_id, uint8_t index, eth_rx_rss_indir_actiondata *data);

    int p4pd_common_p4plus_rxdma_rss_params_table_entry_get(
            uint32_t hw_lif_id, eth_rx_rss_params_actiondata *data);

    int eth_program_rss(uint32_t hw_lif_id, uint16_t rss_type,
                        uint8_t *rss_key, uint8_t *rss_indir,
                        uint16_t num_queues);

    sdk::platform::utils::mem_addr_t mem_start_addr(const char *region);

private:
    PdClient(){}
    ~PdClient(){}

};


#endif //__PD_STATE_HPP__
