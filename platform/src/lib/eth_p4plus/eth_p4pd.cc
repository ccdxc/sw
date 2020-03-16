//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This library provides p4 ethernet table programming API's
///
//----------------------------------------------------------------------------

#include "lib/p4/p4_api.hpp"
#include "platform/capri/capri_hbm_rw.hpp"
#include "platform/capri/capri_tbl_rw.hpp"
#include "platform/utils/mpartition.hpp"
#include "asic/rw/asicrw.hpp"

#include "eth_p4pd.hpp"

static inline uint8_t*
memrev (uint8_t *block, size_t elnum)
{
    uint8_t *s, *t, tmp;

    for (s = block, t = s + (elnum - 1); s < t; s++, t--) {
        tmp = *s;
        *s = *t;
        *t = tmp;
    }
    return block;
}

sdk_ret_t
p4plus_rxdma_rss_params_update (uint32_t hw_lif_id,
                                uint8_t rss_type,
                                uint8_t *rss_key,
                                bool debug)
{
    sdk_ret_t           ret = SDK_RET_OK;
    p4pd_error_t        pd_err;
    eth_rx_rss_params_actiondata_t data = { 0 };

    if (hw_lif_id >= MAX_LIFS || rss_key == NULL) {
        ret = SDK_RET_INVALID_ARG;
        return ret;
    }

    data.action_u.eth_rx_rss_params_eth_rx_rss_params.rss_type = rss_type;
    data.action_u.eth_rx_rss_params_eth_rx_rss_params.debug = (uint8_t)debug;
    memcpy(&data.action_u.eth_rx_rss_params_eth_rx_rss_params.rss_key,
           rss_key,
           sizeof(data.action_u.eth_rx_rss_params_eth_rx_rss_params.rss_key));
    memrev((uint8_t *)&data.action_u.eth_rx_rss_params_eth_rx_rss_params.rss_key,
           sizeof(data.action_u.eth_rx_rss_params_eth_rx_rss_params.rss_key));

    pd_err = p4pd_global_entry_write(P4_COMMON_RXDMA_ACTIONS_TBL_ID_ETH_RX_RSS_PARAMS,
                                     hw_lif_id, NULL, NULL, &data);
    if (pd_err != P4PD_SUCCESS) {
        ret = SDK_RET_HW_PROGRAM_ERR;
        return ret;
    }

    return ret;
}

sdk_ret_t
p4plus_rxdma_rss_params_get (uint32_t hw_lif_id,
                             eth_rx_rss_params_actiondata_t *data)
{
    sdk_ret_t           ret = SDK_RET_OK;
    p4pd_error_t        pd_err;

    if (hw_lif_id >= MAX_LIFS || data == NULL) {
        ret = SDK_RET_INVALID_ARG;
        return ret;
    }

    pd_err = p4pd_global_entry_read(P4_COMMON_RXDMA_ACTIONS_TBL_ID_ETH_RX_RSS_PARAMS,
                                    hw_lif_id, NULL, NULL, data);
    if (pd_err != P4PD_SUCCESS) {
        ret = SDK_RET_HW_PROGRAM_ERR;
        return ret;
    }

    memrev((uint8_t *)&data->action_u.eth_rx_rss_params_eth_rx_rss_params.rss_key,
           sizeof(data->action_u.eth_rx_rss_params_eth_rx_rss_params.rss_key));

    return ret;
}

sdk_ret_t
p4plus_rxdma_rss_indir_update (uint32_t hw_lif_id,
                               uint8_t index,
                               uint8_t enable,
                               uint8_t qid)
{
    uint64_t tbl_base;
    uint64_t tbl_index;
    uint64_t addr;
    mpartition *mp;
    sdk_ret_t  ret = SDK_RET_OK;
    eth_rx_rss_indir_actiondata_t data = { 0 };

    if (hw_lif_id >= MAX_LIFS ||
        index >= ETH_RSS_LIF_INDIR_TBL_SZ ||
        qid >= ETH_RSS_MAX_QUEUES) {
        ret = SDK_RET_INVALID_ARG;
        return ret;
    };

    mp = mpartition::get_instance();
    SDK_ASSERT(mp);

    data.action_u.eth_rx_rss_indir_eth_rx_rss_indir.enable = enable;
    data.action_u.eth_rx_rss_indir_eth_rx_rss_indir.qid = qid;

    tbl_index = (hw_lif_id * ETH_RSS_LIF_INDIR_TBL_SZ) +
                (index * ETH_RSS_LIF_INDIR_TBL_ENTRY_SZ);
    tbl_base = mp->start_addr(MEM_REGION_RSS_INDIR_TABLE_NAME);
    tbl_base = (tbl_base + ETH_RSS_INDIR_TBL_SZ) & ~(ETH_RSS_INDIR_TBL_SZ - 1);
    addr = tbl_base + tbl_index;

    ret = sdk::asic::asic_mem_write(addr, (uint8_t *)&data.action_u,
                              sizeof(data.action_u));
    if (ret != SDK_RET_OK) {
        return ret;
    }

    p4plus_invalidate_cache(addr, sizeof(data.action_u),
                            P4PLUS_CACHE_INVALIDATE_RXDMA);

    return ret;
}

sdk_ret_t
p4plus_rxdma_rss_indir_get (uint32_t hw_lif_id,
                            uint8_t index,
                            eth_rx_rss_indir_actiondata_t *data)
{
    uint64_t tbl_base;
    uint64_t tbl_index;
    uint64_t addr;
    mpartition *mp;
    sdk_ret_t ret = SDK_RET_OK;

    if (hw_lif_id >= MAX_LIFS ||
        index >= ETH_RSS_LIF_INDIR_TBL_SZ) {
        ret = SDK_RET_INVALID_ARG;
        return ret;
    };

    mp = mpartition::get_instance();
    SDK_ASSERT(mp);

    tbl_index = (hw_lif_id * ETH_RSS_LIF_INDIR_TBL_SZ) +
                (index * ETH_RSS_LIF_INDIR_TBL_ENTRY_SZ);
    tbl_base = mp->start_addr(MEM_REGION_RSS_INDIR_TABLE_NAME);
    tbl_base = (tbl_base + ETH_RSS_INDIR_TBL_SZ) & ~(ETH_RSS_INDIR_TBL_SZ - 1);
    addr = tbl_base + tbl_index;

    ret = sdk::asic::asic_mem_read(addr, (uint8_t *)&data->action_u,
                             sizeof(data->action_u));
    if(ret != SDK_RET_OK) {
        return ret;
    }

    return SDK_RET_OK;
}
