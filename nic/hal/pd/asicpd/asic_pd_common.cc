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
#include "nic/sdk/asic/cmn/asic_hbm.hpp"
#include "nic/hal/pd/asicpd/asic_pd_common.hpp"
#include "nic/sdk/platform/drivers/xcvr.hpp"

using namespace sdk::asic::pd;

namespace hal {
namespace pd {

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

    sdk::asic::pd::asicpd_p4plus_table_init(hal_cfg->platform,
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
                                            tbl_ctx_txdma_act_ext.stage_tableid,
                                            0, 0); /* WIP: args for elba */
    return HAL_RET_OK;
}

hal_ret_t
asicpd_stats_addr_get (int tblid, uint32_t index,
                       asicpd_stats_region_info_t *region_arr, int arrlen,
                       mem_addr_t *stats_addr_p)
{
    p4pd_table_properties_t       tbl_ctx;
    mem_addr_t                    stats_base_addr;

    stats_base_addr = asicpd_get_mem_addr(ASIC_HBM_REG_P4_ATOMIC_STATS);
    SDK_ASSERT(stats_base_addr != INVALID_MEM_ADDRESS);

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
    mem_addr_t                    stats_base_addr;
    uint64_t                      stats_region_start;
    uint64_t                      stats_region_size;

    stats_region_start = stats_base_addr =
        asicpd_get_mem_addr(ASIC_HBM_REG_P4_ATOMIC_STATS);
    SDK_ASSERT(stats_base_addr != INVALID_MEM_ADDRESS);
    stats_region_size =
        (asicpd_get_mem_size_kb(ASIC_HBM_REG_P4_ATOMIC_STATS) << 10);

    // subtract 2G (saves ASM instructions)
    stats_region_start -= ((uint64_t)1 << 31);
    stats_base_addr -= ((uint64_t)1 << 31);

    for (int i = 0; i < arrlen; i++) {
        p4pd_table_properties_get(region_arr[i].tblid, &tbl_ctx);
        asicpd_program_table_constant (region_arr[i].tblid, stats_base_addr);
        stats_base_addr += (tbl_ctx.tabledepth << region_arr[i].tbldepthshift);
    }
    assert(stats_base_addr <  (stats_region_start +  stats_region_size));
    return HAL_RET_OK;
}

hal_ret_t
xcvr_valid_check_enable (bool enable)
{
    sdk::platform::xcvr_set_valid_check(enable);
    return HAL_RET_OK;
}

}    // namespace pd
}    // namespace hal
