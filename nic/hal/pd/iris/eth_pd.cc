
#include <cstring>

#include "if_pd_utils.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/base.h"
#include "sdk/directmap.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/iris/eth_pd.hpp"
#include "nic/hal/pd/p4pd_api.hpp"
#include "nic/gen/iris/include/p4pd.h"
#include "nic/gen/common_rxdma_actions/include/common_rxdma_actions_p4pd.h"

#ifndef P4PD_CLI
#include "nic/hal/pd/capri/capri_loader.h"
#include "nic/hal/pd/capri/capri_tbl_rw.hpp"
#include "nic/model_sim/include/lib_model_client.h"
#include "nic/asic/capri/model/utils/cap_blk_reg_model.h"
#include "nic/asic/capri/model/cap_top/cap_top_csr.h"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#endif

namespace hal {
namespace pd {

EXTC hal_ret_t
pd_rss_params_table_entry_add(pd_rss_params_table_entry_add_args_t *args)
{
    return p4pd_common_p4plus_rxdma_rss_params_table_entry_add(args->hw_lif_id,
                                                               args->rss_type,
                                                               args->rss_key);
}

hal_ret_t
p4pd_common_p4plus_rxdma_rss_params_table_entry_add(
    uint32_t hw_lif_id, uint8_t rss_type, uint8_t *rss_key)
{
    hal_ret_t           ret = HAL_RET_OK;
    p4pd_error_t        pd_err;
    eth_rx_rss_params_actiondata data = { 0 };

    HAL_ASSERT(hw_lif_id < MAX_LIFS);
    HAL_ASSERT(rss_key != NULL);

    data.eth_rx_rss_params_action_u.eth_rx_rss_params_eth_rx_rss_params.rss_type = rss_type;
    memcpy(&data.eth_rx_rss_params_action_u.eth_rx_rss_params_eth_rx_rss_params.rss_key,
           rss_key,
           sizeof(data.eth_rx_rss_params_action_u.eth_rx_rss_params_eth_rx_rss_params.rss_key));
    memrev((uint8_t *)&data.eth_rx_rss_params_action_u.eth_rx_rss_params_eth_rx_rss_params.rss_key,
           sizeof(data.eth_rx_rss_params_action_u.eth_rx_rss_params_eth_rx_rss_params.rss_key));

    pd_err = p4pd_global_entry_write(P4_COMMON_RXDMA_ACTIONS_TBL_ID_ETH_RX_RSS_PARAMS,
                                     hw_lif_id, NULL, NULL, &data);
    if (pd_err != P4PD_SUCCESS) {
        ret = HAL_RET_HW_FAIL;
        HAL_ASSERT(0);
    }

    return ret;
}

hal_ret_t
p4pd_common_p4plus_rxdma_rss_params_table_entry_get(
    uint32_t hw_lif_id, eth_rx_rss_params_actiondata *data)
{
    hal_ret_t           ret = HAL_RET_OK;
    p4pd_error_t        pd_err;

    HAL_ASSERT(hw_lif_id < MAX_LIFS);
    HAL_ASSERT(data != NULL);

    pd_err = p4pd_global_entry_read(P4_COMMON_RXDMA_ACTIONS_TBL_ID_ETH_RX_RSS_PARAMS,
                                    hw_lif_id, NULL, NULL, data);
    if (pd_err != P4PD_SUCCESS) {
        ret = HAL_RET_HW_FAIL;
        HAL_ASSERT(0);
    }

    memrev((uint8_t *)&data->eth_rx_rss_params_action_u.eth_rx_rss_params_eth_rx_rss_params.rss_key,
           sizeof(data->eth_rx_rss_params_action_u.eth_rx_rss_params_eth_rx_rss_params.rss_key));

    return ret;
}

EXTC hal_ret_t
pd_rss_indir_table_entry_add(pd_rss_indir_table_entry_add_args_t *args)
{
    return p4pd_common_p4plus_rxdma_rss_indir_table_entry_add(args->hw_lif_id,
                                                              args->index, 
                                                              args->enable,
                                                              args->qid);
}


hal_ret_t
p4pd_common_p4plus_rxdma_rss_indir_table_entry_add(
    uint32_t hw_lif_id, uint8_t index, uint8_t enable, uint8_t qid)
{
    uint64_t tbl_base;
    uint64_t tbl_index;
    uint64_t addr;
    eth_rx_rss_indir_actiondata data = { 0 };

    HAL_ASSERT(hw_lif_id < MAX_LIFS);
    HAL_ASSERT(index < ETH_RSS_LIF_INDIR_TBL_SZ);
    HAL_ASSERT(qid < ETH_RSS_MAX_QUEUES);

    data.eth_rx_rss_indir_action_u.eth_rx_rss_indir_eth_rx_rss_indir.enable = enable;
    data.eth_rx_rss_indir_action_u.eth_rx_rss_indir_eth_rx_rss_indir.qid = qid;

    tbl_index = (hw_lif_id * ETH_RSS_LIF_INDIR_TBL_SZ) +
                (index * ETH_RSS_LIF_INDIR_TBL_ENTRY_SZ);
    tbl_base = get_start_offset(CAPRI_HBM_REG_RSS_INDIR_TABLE);
    tbl_base = (tbl_base + ETH_RSS_INDIR_TBL_SZ) & ~(ETH_RSS_INDIR_TBL_SZ - 1);
    addr = tbl_base + tbl_index;

    capri_hbm_write_mem(addr,
            (uint8_t *)&data.eth_rx_rss_indir_action_u,
            sizeof(data.eth_rx_rss_indir_action_u));

    HAL_TRACE_DEBUG("rss_indir_table add, hw_lif_id : {}, index : {}, addr : {}",
                    hw_lif_id, index, addr);

    return HAL_RET_OK;
}

hal_ret_t
p4pd_common_p4plus_rxdma_rss_indir_table_entry_get(
    uint32_t hw_lif_id, uint8_t index, eth_rx_rss_indir_actiondata *data)
{
    uint64_t tbl_base;
    uint64_t tbl_index;
    uint64_t addr;

    HAL_ASSERT(hw_lif_id < MAX_LIFS);
    HAL_ASSERT(index < ETH_RSS_LIF_INDIR_TBL_SZ);
    HAL_ASSERT(data != NULL);

    tbl_index = (hw_lif_id * ETH_RSS_LIF_INDIR_TBL_SZ) +
                (index * ETH_RSS_LIF_INDIR_TBL_ENTRY_SZ);
    tbl_base = get_start_offset(CAPRI_HBM_REG_RSS_INDIR_TABLE);
    tbl_base = (tbl_base + ETH_RSS_INDIR_TBL_SZ) & ~(ETH_RSS_INDIR_TBL_SZ - 1);
    addr = tbl_base + tbl_index;

    capri_hbm_read_mem(addr,
            (uint8_t *)&data->eth_rx_rss_indir_action_u,
             sizeof(data->eth_rx_rss_indir_action_u));

    HAL_TRACE_DEBUG("rss_indir_table get, hw_lif_id : {}, index : {}, addr : {}",
                    hw_lif_id, index, addr);

    return HAL_RET_OK;
}

}    // namespace pd
}    // namespace hal
