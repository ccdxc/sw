
#include <cstring>

#include "nic/hal/pd/iris/nw/if_pd_utils.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/base.hpp"
#include "nic/sdk/include/sdk/directmap.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/iris/lif/eth_pd.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "gen/p4gen/p4/include/p4pd.h"
#include "gen/p4gen/common_rxdma_actions/include/common_rxdma_actions_p4pd.h"

#include "include/sdk/platform/capri/capri_tbl_rw.hpp"
#include "nic/model_sim/include/lib_model_client.h"
#include "nic/asic/capri/model/utils/cap_blk_reg_model.h"
#include "nic/asic/capri/model/cap_top/cap_top_csr.h"
#include "nic/hal/pd/capri/capri_hbm.hpp"

namespace hal {
namespace pd {

hal_ret_t
pd_rss_params_table_entry_add(pd_func_args_t *pd_func_args)
{
    pd_rss_params_table_entry_add_args_t *args = pd_func_args->pd_rss_params_table_entry_add;
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
    eth_rx_rss_params_actiondata_t data = { 0 };

    HAL_ASSERT(hw_lif_id < MAX_LIFS);
    HAL_ASSERT(rss_key != NULL);

    data.action_u.eth_rx_rss_params_eth_rx_rss_params.rss_type = rss_type;
    memcpy(&data.action_u.eth_rx_rss_params_eth_rx_rss_params.rss_key,
           rss_key,
           sizeof(data.action_u.eth_rx_rss_params_eth_rx_rss_params.rss_key));
    memrev((uint8_t *)&data.action_u.eth_rx_rss_params_eth_rx_rss_params.rss_key,
           sizeof(data.action_u.eth_rx_rss_params_eth_rx_rss_params.rss_key));

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
    uint32_t hw_lif_id, eth_rx_rss_params_actiondata_t *data)
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

    memrev((uint8_t *)&data->action_u.eth_rx_rss_params_eth_rx_rss_params.rss_key,
           sizeof(data->action_u.eth_rx_rss_params_eth_rx_rss_params.rss_key));

    return ret;
}

hal_ret_t
pd_rss_indir_table_entry_add(pd_func_args_t *pd_func_args)
{
    pd_rss_indir_table_entry_add_args_t *args = pd_func_args->pd_rss_indir_table_entry_add;
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
    eth_rx_rss_indir_actiondata_t data = { 0 };

    if (hw_lif_id >= MAX_LIFS ||
        index >= ETH_RSS_LIF_INDIR_TBL_SZ ||
        qid >= ETH_RSS_MAX_QUEUES) {
        HAL_TRACE_ERR("{}: {}, index : {}, qid : {}",
                      __FUNCTION__, hw_lif_id, index, qid);
        return HAL_RET_ERR;
    };

    data.action_u.eth_rx_rss_indir_eth_rx_rss_indir.enable = enable;
    data.action_u.eth_rx_rss_indir_eth_rx_rss_indir.qid = qid;

    tbl_index = (hw_lif_id * ETH_RSS_LIF_INDIR_TBL_SZ) +
                (index * ETH_RSS_LIF_INDIR_TBL_ENTRY_SZ);
    tbl_base = get_start_offset(CAPRI_HBM_REG_RSS_INDIR_TABLE);
    tbl_base = (tbl_base + ETH_RSS_INDIR_TBL_SZ) & ~(ETH_RSS_INDIR_TBL_SZ - 1);
    addr = tbl_base + tbl_index;

    HAL_TRACE_DEBUG("{}: hw_lif_id : {}, index : {}, addr : {:x}, enable : {}, qid : {}",
                    __FUNCTION__, hw_lif_id, index, addr, enable, qid);

    capri_hbm_write_mem(addr,
            (uint8_t *)&data.action_u,
            sizeof(data.action_u));
    p4plus_invalidate_cache(addr, sizeof(data.action_u),
        P4PLUS_CACHE_INVALIDATE_RXDMA);

    return HAL_RET_OK;
}

hal_ret_t
p4pd_common_p4plus_rxdma_rss_indir_table_entry_get(
    uint32_t hw_lif_id, uint8_t index, eth_rx_rss_indir_actiondata_t *data)
{
    uint64_t tbl_base;
    uint64_t tbl_index;
    uint64_t addr;

    if (hw_lif_id >= MAX_LIFS ||
        index >= ETH_RSS_LIF_INDIR_TBL_SZ) {
        HAL_TRACE_ERR("{}: hw_lif_id : {} index : {}",
            __FUNCTION__, hw_lif_id, index);
        return HAL_RET_ERR;
    };

    tbl_index = (hw_lif_id * ETH_RSS_LIF_INDIR_TBL_SZ) +
                (index * ETH_RSS_LIF_INDIR_TBL_ENTRY_SZ);
    tbl_base = get_start_offset(CAPRI_HBM_REG_RSS_INDIR_TABLE);
    tbl_base = (tbl_base + ETH_RSS_INDIR_TBL_SZ) & ~(ETH_RSS_INDIR_TBL_SZ - 1);
    addr = tbl_base + tbl_index;

    capri_hbm_read_mem(addr,
            (uint8_t *)&data->action_u,
             sizeof(data->action_u));

    return HAL_RET_OK;
}

}    // namespace pd
}    // namespace hal
