// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#include "nic/sdk/include/sdk/base.hpp"
#include "lib/thread/thread.hpp"
#include "nic/hal/pd/asic_pd.hpp"
#include "nic/sdk/lib/pal/pal.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/sdk/asic/pd/pd.hpp"
#include "gen/p4gen/common_rxdma_actions/include/common_rxdma_actions_p4pd.h"
#include "gen/p4gen/common_txdma_actions/include/common_txdma_actions_p4pd.h"
#include "gen/p4gen/common_rxdma_actions/include/common_rxdma_actions_p4pd_table.h"
#include "gen/p4gen/common_txdma_actions/include/common_txdma_actions_p4pd_table.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/hal/pd/asicpd/asic_pd_common.hpp"
#include "platform/capri/capri_txs_scheduler.hpp"
#include "platform/capri/capri_tbl_rw.hpp"
#include "nic/hal/pd/capri/capri_sw_phv.hpp"
#include "nic/sdk/platform/drivers/xcvr.hpp"

using namespace sdk::platform::capri;

namespace hal {
namespace pd {

hal_ret_t
asicpd_toeplitz_init (void)
{
    p4pd_table_properties_t tbl_ctx;
    p4pd_global_table_properties_get(P4_COMMON_RXDMA_ACTIONS_TBL_ID_ETH_RX_RSS_INDIR,
                                     &tbl_ctx);
    capri_toeplitz_init(tbl_ctx.stage, tbl_ctx.stage_tableid);
    return HAL_RET_OK;
}

hal_ret_t
asicpd_p4plus_table_init (hal::hal_cfg_t *hal_cfg)
{
    p4pd_table_properties_t tbl_ctx_apphdr;
    p4pd_table_properties_t tbl_ctx_apphdr_ext;
    p4pd_table_properties_t tbl_ctx_apphdr_off;
    p4pd_table_properties_t tbl_ctx_apphdr_ext_off;
    p4pd_table_properties_t tbl_ctx_txdma_act;
    p4pd_table_properties_t tbl_ctx_txdma_act_ext;

    // P4 plus table inits
    p4pd_global_table_properties_get(P4_COMMON_RXDMA_ACTIONS_TBL_ID_COMMON_P4PLUS_STAGE0_APP_HEADER_TABLE,
                                     &tbl_ctx_apphdr);
    p4pd_global_table_properties_get(P4_COMMON_RXDMA_ACTIONS_TBL_ID_COMMON_P4PLUS_STAGE0_APP_HEADER_EXT_TABLE,
                                     &tbl_ctx_apphdr_ext);
    p4pd_global_table_properties_get(P4_COMMON_RXDMA_ACTIONS_TBL_ID_COMMON_P4PLUS_STAGE0_APP_HEADER_TABLE_OFFSET_64,
                                     &tbl_ctx_apphdr_off);
    p4pd_global_table_properties_get(P4_COMMON_RXDMA_ACTIONS_TBL_ID_COMMON_P4PLUS_STAGE0_APP_HEADER_TABLE_EXT_OFFSET_64,
                                     &tbl_ctx_apphdr_ext_off);
    p4pd_global_table_properties_get(P4_COMMON_TXDMA_ACTIONS_TBL_ID_TX_TABLE_S0_T0,
                                     &tbl_ctx_txdma_act);
    p4pd_global_table_properties_get(P4_COMMON_TXDMA_ACTIONS_TBL_ID_TX_TABLE_S0_T1,
                                     &tbl_ctx_txdma_act_ext);
    capri_p4plus_table_init(hal_cfg->platform,
                            tbl_ctx_apphdr.stage,
                            tbl_ctx_apphdr.stage_tableid,
                            tbl_ctx_apphdr_ext.stage,
                            tbl_ctx_apphdr_ext.stage_tableid,
                            tbl_ctx_apphdr_off.stage,
                            tbl_ctx_apphdr_off.stage_tableid,
                            tbl_ctx_apphdr_ext_off.stage,
                            tbl_ctx_apphdr_ext_off.stage_tableid,
                            tbl_ctx_txdma_act.stage,
                            tbl_ctx_txdma_act.stage_tableid,
                            tbl_ctx_txdma_act_ext.stage,
                            tbl_ctx_txdma_act_ext.stage_tableid);
    return HAL_RET_OK;
}

hal_ret_t
asicpd_p4plus_recirc_init (void)
{
    capri_p4plus_recirc_init();
    return HAL_RET_OK;
}

hal_ret_t
asicpd_stats_addr_get (int tblid, uint32_t index,
                       asicpd_stats_region_info_t *region_arr, int arrlen,
                       hbm_addr_t *stats_addr_p)
{
    p4pd_table_properties_t       tbl_ctx;
    hbm_addr_t                    stats_base_addr;

    stats_base_addr = get_mem_addr(CAPRI_HBM_REG_P4_ATOMIC_STATS);
    HAL_ASSERT(stats_base_addr != INVALID_MEM_ADDRESS);

    for (int i = 0; i < arrlen; i++) {
        p4pd_table_properties_get(region_arr[i].tblid, &tbl_ctx);
        if (tblid == region_arr[i].tblid) {
            if (index < tbl_ctx.tabledepth) {
                *stats_addr_p = stats_base_addr +
                                (index << region_arr[i].tbldepthshift);
                return HAL_RET_OK;
            } else {
                return HAL_RET_ERR;
            }
        }
        stats_base_addr += (tbl_ctx.tabledepth << region_arr[i].tbldepthshift);
    }
    return HAL_RET_OK;
}

hal_ret_t
asicpd_stats_region_init (asicpd_stats_region_info_t *region_arr, int arrlen)
{
    p4pd_table_properties_t       tbl_ctx;
    hbm_addr_t                    stats_base_addr;
    uint64_t                      stats_region_start;
    uint64_t                      stats_region_size;
    uint64_t                      bit31_base = 0;

    stats_region_start = stats_base_addr =
        get_mem_addr(CAPRI_HBM_REG_P4_ATOMIC_STATS);
    HAL_ASSERT(stats_base_addr != INVALID_MEM_ADDRESS);
    stats_region_size = (get_mem_size_kb(CAPRI_HBM_REG_P4_ATOMIC_STATS) << 10);

    // reset bit 31 (saves one ASM instruction)
    bit31_base = stats_region_start & ((uint64_t)1<<31);
    stats_region_start &= ~((uint64_t)1 << 31);
    stats_base_addr &= ~((uint64_t)1 << 31);

    for (int i = 0; i < arrlen; i++) {
        p4pd_table_properties_get(region_arr[i].tblid, &tbl_ctx);
        HAL_TRACE_DEBUG("Table {} stats base addr {:#x}",
                        tbl_ctx.tablename, stats_base_addr | bit31_base);
        capri_table_constant_write(stats_base_addr,
                                   tbl_ctx.stage, tbl_ctx.stage_tableid,
                                   (tbl_ctx.gress == P4_GRESS_INGRESS));
        stats_base_addr += (tbl_ctx.tabledepth << region_arr[i].tbldepthshift);
    }
    assert(stats_base_addr <  (stats_region_start +  stats_region_size));
    return HAL_RET_OK;
}

// asicpd_sw_phv_inject
// Inject a software phv(maintenance PHV) into asic
hal_ret_t
asicpd_sw_phv_inject (asicpd_swphv_type_t type, uint8_t prof_num,
		uint8_t start_idx, uint8_t num_flits, void *data) {
    return capri_sw_phv_inject(type, prof_num, start_idx, num_flits, data);
}

// asicpd_sw_phv_get
// get software PHV state from asic
hal_ret_t
asicpd_sw_phv_get (asicpd_swphv_type_t type, uint8_t prof_num,
	asicpd_sw_phv_state_t *state) {
    return capri_sw_phv_get(type, prof_num, state);
}

hal_ret_t
asic_pd_hbm_bw_get (pd_hbm_bw_get_args_t *hbm_bw_args)
{
    sdk_ret_t sdk_ret;
    sdk_ret = capri_hbm_bw(hbm_bw_args->num_samples,
                        hbm_bw_args->sleep_interval,
                        true,
                        hbm_bw_args->hbm_bw);
    return hal_sdk_ret_to_hal_ret(sdk_ret);
}

hal_ret_t
asic_pd_llc_setup (pd_llc_get_args_t *llc_args)
{
    sdk_ret_t sdk_ret;
    sdk_ret = capri_nx_setup_llc_counters(llc_args->mask);
    return hal_sdk_ret_to_hal_ret(sdk_ret);
}

hal_ret_t
asic_pd_llc_get (pd_llc_get_args_t *llc_args)
{
    sdk_ret_t sdk_ret;
    sdk_ret = capri_nx_get_llc_counters(llc_args->data);
    return hal_sdk_ret_to_hal_ret(sdk_ret);
}

#if 0
hal_ret_t
asic_pd_scheduler_stats_get (pd_scheduler_stats_get_args_t *scheduler_stats_args)
{
    hal_ret_t ret;
    capri_txs_scheduler_stats_t asic_stats = {};
    debug::SchedulerStatsResponse *response = scheduler_stats_args->response;
    sdk_ret_t     sdk_ret;


    sdk_ret = capri_txs_scheduler_stats_get(&asic_stats);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    response->set_doorbell_set_count(asic_stats.doorbell_set_count);
    response->set_doorbell_clear_count(asic_stats.doorbell_clear_count);
    response->set_ratelimit_start_count(asic_stats.ratelimit_start_count);
    response->set_ratelimit_stop_count(asic_stats.ratelimit_stop_count);
    for (unsigned i = 0; i < HAL_ARRAY_SIZE(asic_stats.cos_stats); i++) {
        auto cos_entry = response->add_cos_entry();
        cos_entry->set_cos(asic_stats.cos_stats[i].cos);
        cos_entry->set_doorbell_count(asic_stats.cos_stats[i].doorbell_count);
        cos_entry->set_xon_status(asic_stats.cos_stats[i].xon_status);
    }
    return HAL_RET_OK;
}
#endif

hal_ret_t
xcvr_valid_check_enable (bool enable)
{
    sdk::platform::xcvr_set_valid_check(enable);
    return HAL_RET_OK;
}

}    // namespace pd
}    // namespace hal
