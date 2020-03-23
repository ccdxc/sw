// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "asic/pd/pd.hpp"
#include "asic/pd/pd_internal.hpp"
#include "lib/utils/time_profile.hpp"
#include "platform/utils/mpartition.hpp"
#if defined(APOLLO) || defined(ARTEMIS) || defined(APULU) || defined(ATHENA)
#include "gen/p4gen/p4plus_rxdma/include/p4plus_rxdma_p4pd.h"
#include "gen/p4gen/p4plus_txdma/include/p4plus_txdma_p4pd.h"
#endif

namespace sdk {
namespace asic {
namespace pd {

static uint64_t table_asm_base[P4TBL_ID_MAX];
static uint64_t table_asm_err_offset[P4TBL_ID_MAX];

static uint64_t
get_table_asm_base_addr (uint32_t tableid)
{
    return table_asm_base[tableid];
}

static uint64_t
get_table_asm_err_offset (uint32_t tableid)
{
    return table_asm_err_offset[tableid];
}

void
asicpd_copy_table_info (p4_table_mem_layout_t *out,
                        p4pd_table_mem_layout_t *in,
                        p4pd_table_properties_t *tbl_ctx)
{
    out->entry_width = in->entry_width;
    out->entry_width_bits = in->entry_width_bits;
    out->start_index = in->start_index;
    out->end_index = in->end_index;
    out->top_left_x = in->top_left_x;
    out->top_left_y = in->top_left_y;
    out->top_left_block = in->top_left_block;
    out->btm_right_x = in->btm_right_x;
    out->btm_right_y = in->btm_right_y;
    out->btm_right_block = in->btm_right_block;
    out->num_buckets = in->num_buckets;
    out->tabledepth = tbl_ctx->tabledepth;
    out->base_mem_pa = tbl_ctx->base_mem_pa;
    out->base_mem_va = tbl_ctx->base_mem_va;
    out->tablename = tbl_ctx->tablename;
    return;
}

sdk_ret_t
asicpd_p4plus_table_mpu_base_init (p4pd_cfg_t *p4pd_cfg)
{
    sdk_ret_t ret = SDK_RET_OK;
    char action_name[P4ACTION_NAME_MAX_LEN] = {0};
    char progname[P4ACTION_NAME_MAX_LEN] = {0};
    uint64_t action_rxdma_asm_base;
    uint64_t action_txdma_asm_base;
    uint64_t table_rxdma_asm_base;
    uint64_t table_txdma_asm_base;
    p4pd_table_properties_t tbl_info;

    for (uint32_t i = p4pd_rxdma_tableid_min_get();
         i < p4pd_rxdma_tableid_max_get(); i++) {
        snprintf(progname, P4ACTION_NAME_MAX_LEN, "%s%s",
                 p4pd_rxdma_tbl_names[i], ".bin");

        ret = sdk::p4::p4_program_to_base_addr(p4pd_cfg->p4pd_rxdma_pgm_name,
                                               progname, &table_rxdma_asm_base);
        if (ret != SDK_RET_OK) {
            continue;
        }
        asicpd_set_table_rxdma_asm_base(i, table_rxdma_asm_base);
        for (int j = 0; j < p4pd_rxdma_get_max_action_id(i); j++) {
            p4pd_rxdma_get_action_name(i, j, action_name);
            action_rxdma_asm_base = 0;
            sdk::p4::p4_program_label_to_offset(p4pd_cfg->p4pd_rxdma_pgm_name,
                                                progname, action_name,
                                                &action_rxdma_asm_base);

            // action base is in byte and 64B aligned
            action_rxdma_asm_base >>= 6;
            SDK_TRACE_DEBUG("program name %s, action name %s, action pc 0x%x",
                            progname, action_name, action_rxdma_asm_base);
            asicpd_set_action_rxdma_asm_base(i, j, action_rxdma_asm_base);
        }
    }

    for (uint32_t i = p4pd_txdma_tableid_min_get();
         i < p4pd_txdma_tableid_max_get(); i++) {
        snprintf(progname, P4ACTION_NAME_MAX_LEN, "%s%s",
                 p4pd_txdma_tbl_names[i], ".bin");
        ret = sdk::p4::p4_program_to_base_addr(p4pd_cfg->p4pd_txdma_pgm_name,
                                               progname, &table_txdma_asm_base);
        if (ret != SDK_RET_OK) {
            continue;
        }
        asicpd_set_table_txdma_asm_base(i, table_txdma_asm_base);
        for (int j = 0; j < p4pd_txdma_get_max_action_id(i); j++) {
            p4pd_txdma_get_action_name(i, j, action_name);
            action_txdma_asm_base = 0;
            sdk::p4::p4_program_label_to_offset(p4pd_cfg->p4pd_txdma_pgm_name,
                                                progname, action_name,
                                                &action_txdma_asm_base);

            /* Action base is in byte and 64B aligned... */
            action_txdma_asm_base>>= 6;
            SDK_TRACE_DEBUG("program name %s, action name %s, action pc 0x%x",
                            progname, action_name, action_txdma_asm_base);
            asicpd_set_action_txdma_asm_base(i, j, action_txdma_asm_base);
        }
    }

    // config only if it is  hard init
    if (sdk::asic::is_hard_init()) {
        // P4+ MPU PC initialize
        for (uint32_t i = p4pd_rxdma_tableid_min_get();
             i < p4pd_rxdma_tableid_max_get(); i++) {
            p4pd_global_table_properties_get(i, &tbl_info);

            asicpd_program_p4plus_table_mpu_pc(i, tbl_info.stage_tableid,
                                               tbl_info.stage);
        }

        for (uint32_t i = p4pd_txdma_tableid_min_get();
             i < p4pd_txdma_tableid_max_get(); i++) {
            p4pd_global_table_properties_get(i, &tbl_info);

            asicpd_program_p4plus_table_mpu_pc(i, tbl_info.stage_tableid,
                                               tbl_info.stage);
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
asicpd_table_mpu_base_init (p4pd_cfg_t *p4pd_cfg)
{
    char        action_name[P4ACTION_NAME_MAX_LEN] = {0};
    char        progname[P4ACTION_NAME_MAX_LEN] = {0};
    uint64_t    action_asm_base;

    for (uint32_t i = p4pd_tableid_min_get(); i < p4pd_tableid_max_get(); i++) {
        snprintf(progname, P4ACTION_NAME_MAX_LEN, "%s%s",
                 p4pd_tbl_names[i], ".bin");

        sdk::p4::p4_program_to_base_addr(p4pd_cfg->p4pd_pgm_name, progname,
                                         &table_asm_base[i]);

        for (int j = 0; j < p4pd_get_max_action_id(i); j++) {
            p4pd_get_action_name(i, j, action_name);
            action_asm_base = 0;
            sdk::p4::p4_program_label_to_offset(p4pd_cfg->p4pd_pgm_name,
                                                progname, action_name,
                                                &action_asm_base);

            // action base is in byte and 64B aligned
            SDK_ASSERT((action_asm_base & 0x3f) == 0);
            action_asm_base >>= 6;
            SDK_TRACE_DEBUG("program name %s, action name %s, action pc 0x%x",
                            progname, action_name, action_asm_base);
            asicpd_set_action_asm_base(i, j, action_asm_base);
        }

        // compute error program offset for each table
        snprintf(action_name, P4ACTION_NAME_MAX_LEN, "%s_error",
                 p4pd_tbl_names[i]);
        sdk::p4::p4_program_label_to_offset(p4pd_cfg->p4pd_pgm_name, progname,
                                            action_name,
                                            &table_asm_err_offset[i]);
        SDK_ASSERT((table_asm_err_offset[i] & 0x3f) == 0);

        table_asm_err_offset[i] >>= 6;
        SDK_TRACE_DEBUG("program name %s, action name %s, action pc 0x%x",
                        progname, action_name, table_asm_err_offset[i]);
    }

    return SDK_RET_OK;
}

sdk_ret_t
asicpd_program_table_mpu_pc (void)
{
    p4pd_table_properties_t       tbl_ctx;

    for (uint32_t i = p4pd_tableid_min_get(); i < p4pd_tableid_max_get(); i++) {
        p4pd_table_properties_get(i, &tbl_ctx);
        if (tbl_ctx.is_oflow_table && tbl_ctx.table_type == P4_TBL_TYPE_TCAM) {
            // OTCAM and hash table share the same table id
            // so mpu_pc shouldn't be overwritten
            continue;
        }
        asicpd_program_tbl_mpu_pc(tbl_ctx.tableid,
                                  (tbl_ctx.gress == P4_GRESS_INGRESS),
                                  tbl_ctx.stage, tbl_ctx.stage_tableid,
                                   table_asm_err_offset[i], table_asm_base[i]);

        if (tbl_ctx.table_thread_count > 1) {
            for (int j = 1; j < tbl_ctx.table_thread_count; j++) {
                asicpd_program_tbl_mpu_pc(tbl_ctx.tableid,
                                          (tbl_ctx.gress == P4_GRESS_INGRESS),
                                          tbl_ctx.stage,
                                          tbl_ctx.thread_table_id[j],
                                          table_asm_err_offset[i],
                                          table_asm_base[i]);
            }
        }
    }

    return SDK_RET_OK;
}

sdk_ret_t
asicpd_program_hbm_table_base_addr (bool hw_init)
{
    p4pd_table_properties_t       tbl_ctx;
    p4pd_pipeline_t               pipe;

    for (uint32_t i = p4pd_tableid_min_get(); i < p4pd_tableid_max_get(); i++) {
        p4pd_global_table_properties_get(i, &tbl_ctx);
        if (tbl_ctx.table_location != P4_TBL_LOCATION_HBM) {
            continue;
        }
        if (tbl_ctx.gress == P4_GRESS_INGRESS) {
            pipe = P4_PIPELINE_INGRESS;
        } else {
            pipe = P4_PIPELINE_EGRESS;
        }

        asic_program_hbm_table_base_addr(i, tbl_ctx.stage_tableid,
                                         tbl_ctx.tablename, tbl_ctx.stage,
                                         pipe, hw_init);

        if (tbl_ctx.table_thread_count > 1) {
            for (int j = 1; j < tbl_ctx.table_thread_count; j++) {
                asic_program_hbm_table_base_addr(-1, tbl_ctx.thread_table_id[j],
                                                 tbl_ctx.tablename,
                                                 tbl_ctx.stage, pipe, hw_init);
            }
        }
    }

    for (uint32_t i = p4pd_rxdma_tableid_min_get();
         i < p4pd_rxdma_tableid_max_get(); i++) {
        p4pd_global_table_properties_get(i, &tbl_ctx);
        if (tbl_ctx.table_location != P4_TBL_LOCATION_HBM) {
            continue;
        }
        asic_program_hbm_table_base_addr(i, tbl_ctx.stage_tableid,
                                         tbl_ctx.tablename, tbl_ctx.stage,
                                         P4_PIPELINE_RXDMA, hw_init);
    }

    for (uint32_t i = p4pd_txdma_tableid_min_get();
         i < p4pd_txdma_tableid_max_get(); i++) {
        p4pd_global_table_properties_get(i, &tbl_ctx);
        if (tbl_ctx.table_location != P4_TBL_LOCATION_HBM) {
            continue;
        }
        asic_program_hbm_table_base_addr(i, tbl_ctx.stage_tableid,
                                         tbl_ctx.tablename, tbl_ctx.stage,
                                         P4_PIPELINE_TXDMA, hw_init);
    }

    return SDK_RET_OK;
}

// called by upgrade in pre-init stage.
// prepares the property values based on the configuration.
// this configuration is pushed to the hw during P4 quisce state during
// the upgrade switch over
//
// cfgs_max is the maximum depth of the property config array.
// function returns the number of entries.
uint32_t
asicpd_tbl_eng_cfg_get (p4pd_pipeline_t pipeline, p4_tbl_eng_cfg_t *cfg,
                        uint32_t cfgs_max)
{
    p4pd_table_properties_t tbl_ctx;
    p4pd_pipeline_t pipe;
    uint32_t ntbls = 0;
    uint64_t pc;
    uint32_t toff;
    uint64_t asm_base, asm_err_offset;

    if (pipeline == P4_PIPELINE_INGRESS || pipeline == P4_PIPELINE_EGRESS) {
        for (uint32_t i = p4pd_tableid_min_get();
             i < p4pd_tableid_max_get(); i++) {

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

            asm_base = get_table_asm_base_addr(i);
            asm_err_offset = get_table_asm_err_offset(i);
            if (asm_base == 0) {
                continue;
            }

            SDK_ASSERT(ntbls < cfgs_max);
            if (tbl_ctx.table_location == P4_TBL_LOCATION_HBM) {
                cfg[ntbls].mem_offset = asicpd_get_mem_addr(tbl_ctx.tablename);
            } else {
                cfg[ntbls].mem_offset = INVALID_MEM_ADDRESS;
            }
            SDK_TRACE_INFO("pipeline %s, tblname %s, tableid %u, stage %u, "
                           "stage-tableid %u, mem %lx, ntbl %d",
                           pipeline == P4_PIPELINE_INGRESS ? "ingress" : "egress",
                           tbl_ctx.tablename, tbl_ctx.tableid, tbl_ctx.stage,
                           tbl_ctx.stage_tableid, (long) cfg[ntbls].mem_offset,
                           ntbls);

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
                                   tbl_ctx.tableid, tbl_ctx.stage,
                                   tbl_ctx.thread_table_id[j],
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
            pc = asicpd_get_p4plus_table_mpu_pc(i);
            if (pc == 0) {
                continue;
            }
            SDK_ASSERT(ntbls < cfgs_max);
            if (tbl_ctx.table_location == P4_TBL_LOCATION_HBM) {
                cfg[ntbls].mem_offset = asicpd_get_mem_addr(tbl_ctx.tablename);
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
            pc = asicpd_get_p4plus_table_mpu_pc(i);
            if (pc == 0) {
                continue;
            }
            SDK_ASSERT(ntbls < cfgs_max);
            if (tbl_ctx.table_location == P4_TBL_LOCATION_HBM) {
                cfg[ntbls].mem_offset = asicpd_get_mem_addr(tbl_ctx.tablename);
            } else {
                cfg[ntbls].mem_offset = INVALID_MEM_ADDRESS;
            }
            SDK_TRACE_INFO("pipeline txdma, tblname %s, tableid %u, stage %u, "
                           "stage-tableid %u, mem %lx, ntbl %d",
                           tbl_ctx.tablename, i, tbl_ctx.stage,
                           tbl_ctx.stage_tableid,
                           (long) cfg[ntbls].mem_offset, ntbls);

            cfg[ntbls].tableid = i;
            cfg[ntbls].stage = tbl_ctx.stage;
            cfg[ntbls].stage_tableid = tbl_ctx.stage_tableid;
            cfg[ntbls].asm_base = pc;
            ntbls++;
        }
    }
    return ntbls;
}

}    // namespace pd
}    // namespace asic
}    // namespace sdk
