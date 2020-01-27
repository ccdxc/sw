//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include "asic/pd/pd_upgrade.hpp"
#include "platform/capri/capri_upgrade.hpp"
#include "platform/capri/capri_hbm_rw.hpp"
#include "platform/capri/capri_tbl_rw.hpp"
#include "platform/capri/capri_toeplitz.hpp"

namespace sdk {
namespace asic {
namespace pd {
// called by upgrade in pre-init stage.
// prepares the property values based on the configuration.
// this configuration is pushed to the hw during P4 quisce state during
// the upgrade switch over
//
// cfgs_max is the maximum depth of the property config array.
// function returns the number of entries.
uint32_t
asicpd_upg_table_property_get (p4pd_pipeline_t pipeline,
                               p4_upg_table_property_t *cfg,
                               uint32_t cfgs_max)
{
    p4pd_table_properties_t tbl_ctx;
    p4pd_pipeline_t pipe;
    uint32_t ntbls = 0;
    uint64_t pc;
    uint32_t toff;
    uint64_t asm_base, asm_err_offset;

    if (pipeline == P4_PIPELINE_INGRESS || pipeline == P4_PIPELINE_EGRESS) {
        for (uint32_t i = p4pd_tableid_min_get(); i < p4pd_tableid_max_get(); i++) {
            p4pd_table_properties_get(i, &tbl_ctx);
            if (tbl_ctx.is_oflow_table && tbl_ctx.table_type == P4_TBL_TYPE_TCAM) {
                // OTCAM and hash table share the same table id
                // so mpu_pc shouldn't be overwritten
                continue;
            }
            if (!tbl_ctx.tablename) {
                continue;
            }
            if (tbl_ctx.gress == P4_GRESS_INGRESS) {
                pipe = P4_PIPELINE_INGRESS;
            } else {
                pipe = P4_PIPELINE_EGRESS;
            }
            if (pipe != pipeline) {
                continue;
            }
            asm_base = asicpd_table_asm_base_addr_get(i);
            asm_err_offset = asicpd_table_asm_err_offset_get(i);
            if (asm_base == 0) {
                continue;
            }

            SDK_ASSERT(ntbls < cfgs_max);
            if (tbl_ctx.table_location == P4_TBL_LOCATION_HBM) {
                cfg[ntbls].mem_offset = get_mem_addr(tbl_ctx.tablename);
            } else {
                cfg[ntbls].mem_offset = INVALID_MEM_ADDRESS;
            }
            SDK_TRACE_INFO("pipeline %s, tblname %s, tableid %u, stage %u, "
                           "stage-tableid %u, mem %lx, ntbl %d",
                           pipeline == P4_PIPELINE_INGRESS ? "ingress" : "egress",
                           tbl_ctx.tablename,
                           tbl_ctx.tableid, tbl_ctx.stage, tbl_ctx.stage_tableid,
                           (long)cfg[ntbls].mem_offset, ntbls);
            cfg[ntbls].tableid = tbl_ctx.tableid;
            cfg[ntbls].stage = tbl_ctx.stage;
            cfg[ntbls].stage_tableid = tbl_ctx.stage_tableid;
            cfg[ntbls].asm_err_offset = asm_err_offset;
            cfg[ntbls].asm_base = asm_base;
            ntbls++;
            if (tbl_ctx.table_thread_count > 1) {
                toff = ntbls - 1;
                for (int j = 1; j < tbl_ctx.table_thread_count; j++) {
                    SDK_ASSERT(ntbls < cfgs_max);
                    SDK_TRACE_INFO("pipeline %s, tblname %s, tableid %u, stage %u, "
                                   "stage-tableid %u, thread %u, mem %lx, ntbl %d",
                                   pipeline == P4_PIPELINE_INGRESS ? "ingress" : "egress",
                                   j, tbl_ctx.tablename,
                                   tbl_ctx.tableid, tbl_ctx.stage, tbl_ctx.thread_table_id[j],
                                   (long)cfg[ntbls].mem_offset, ntbls);
                    cfg[ntbls].tableid = tbl_ctx.tableid;
                    cfg[ntbls].stage = tbl_ctx.stage;
                    cfg[ntbls].stage_tableid = tbl_ctx.thread_table_id[j];
                    cfg[ntbls].asm_err_offset = asm_err_offset;
                    cfg[ntbls].asm_base = asm_base;
                    cfg[ntbls].mem_offset = cfg[toff].mem_offset;
                    ntbls++;
                }
            }
        }
    } else if (pipeline == P4_PIPELINE_RXDMA) {
        for (uint32_t i = p4pd_rxdma_tableid_min_get();
             i < p4pd_rxdma_tableid_max_get(); i++) {
            p4pluspd_rxdma_table_properties_get(i, &tbl_ctx);
            pc = sdk::platform::capri::capri_get_p4plus_table_mpu_pc(i);
            if (pc == 0) {
                continue;
            }
            SDK_ASSERT(ntbls < cfgs_max);
            if (tbl_ctx.table_location == P4_TBL_LOCATION_HBM) {
                cfg[ntbls].mem_offset = get_mem_addr(tbl_ctx.tablename);
            } else {
                cfg[ntbls].mem_offset = INVALID_MEM_ADDRESS;
            }
            SDK_TRACE_INFO("pipeline rxdma, tblname %s, tableid %u, stage %u, "
                           "stage-tableid %u, mem %lx, ntbl %d",
                           tbl_ctx.tablename, i, tbl_ctx.stage,
                           tbl_ctx.stage_tableid, (long)cfg[ntbls].mem_offset, ntbls);
            cfg[ntbls].tableid = i;
            cfg[ntbls].stage = tbl_ctx.stage;
            cfg[ntbls].stage_tableid = tbl_ctx.stage_tableid;
            cfg[ntbls].asm_base = pc;
            ntbls++;
        }
    } else if (pipeline == P4_PIPELINE_TXDMA) {
        for (uint32_t i = p4pd_txdma_tableid_min_get();
             i < p4pd_txdma_tableid_max_get(); i++) {
            p4pluspd_txdma_table_properties_get(i, &tbl_ctx);
            pc = sdk::platform::capri::capri_get_p4plus_table_mpu_pc(i);
            if (pc == 0) {
                continue;
            }
            SDK_ASSERT(ntbls < cfgs_max);
            if (tbl_ctx.table_location == P4_TBL_LOCATION_HBM) {
                cfg[ntbls].mem_offset = get_mem_addr(tbl_ctx.tablename);
            } else {
                cfg[ntbls].mem_offset = INVALID_MEM_ADDRESS;
            }
            SDK_TRACE_INFO("pipeline txdma, tblname %s, tableid %u, stage %u, "
                           "stage-tableid %u, mem %lx, ntbl %d",
                           tbl_ctx.tablename, i, tbl_ctx.stage, tbl_ctx.stage_tableid,
                           (long)cfg[ntbls].mem_offset, ntbls);
            cfg[ntbls].tableid = i;
            cfg[ntbls].stage = tbl_ctx.stage;
            cfg[ntbls].stage_tableid = tbl_ctx.stage_tableid;
            cfg[ntbls].asm_base = pc;
            ntbls++;
        }
    }
    return ntbls;
}

// called during upgrade in quiesced state
sdk_ret_t
asicpd_upg_table_property_set (p4pd_pipeline_t pipeline,
                               p4_upg_table_property_t *cfg,
                               uint32_t ncfgs)
{
    return sdk::platform::capri::capri_upg_table_property_set(
        pipeline, cfg, ncfgs);
}

sdk_ret_t
asicpd_upg_rss_table_property_get (const char *handle, uint32_t tableid,
                                   p4_upg_table_property_t *rss)
{
    p4pd_table_properties_t tbl_ctx;

    p4pd_global_table_properties_get(tableid, &tbl_ctx);
    rss->tableid = tableid;
    rss->stage = tbl_ctx.stage;
    rss->stage_tableid = tbl_ctx.stage_tableid;
    return sdk::platform::capri::capri_rss_table_base_pc_get(handle, &rss->mem_offset,
                                                             &rss->asm_base);
}

void
asicpd_upg_rss_table_property_set (p4_upg_table_property_t *rss)
{
    sdk::platform::capri::capri_rss_table_config(rss->stage, rss->stage_tableid,
                                                 rss->mem_offset, rss->asm_base);
}

}    // namespace pd
}    // namespace asic
}    // namespace sdk
