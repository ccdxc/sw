//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// LPM library's common functions across pipelines
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/impl/lpm/lpm.hpp"
#include "nic/apollo/api/impl/lpm/lpm_priv.hpp"
#include "nic/apollo/api/impl/lpm/lpm_impl.hpp"

sdk_ret_t
lpm_build_tree (lpm_itable_t *itable, uint32_t default_nh, uint32_t max_routes,
                mem_addr_t lpm_tree_root_addr, uint32_t lpm_mem_size)
{
    sdk_ret_t           ret;
    mem_addr_t          addr;
    lpm_stage_info_t    *last_stage_info;
    lpm_stage_meta_t    smeta;
    uint32_t            nstages, nkeys_per_table;

    PDS_TRACE_DEBUG("Building LPM tree type %u, interval count %u, "
                    "default nh %u, max routes %u, root addr 0x%llx, "
                    "LPM block size %u", itable->tree_type,
                    itable->num_intervals, default_nh, max_routes,
                    lpm_tree_root_addr, lpm_mem_size);

    /**< compute the # of stages, required including the def route */
    nstages = lpm_stages(itable->tree_type, ((max_routes + 1) << 1));
    SDK_ASSERT(nstages <= LPM_MAX_STAGES);

    /**< initialize all the stage meta */
    memset(&smeta, 0, sizeof(smeta));
    smeta.tree_type = itable->tree_type;
    smeta.key_sz = lpm_entry_key_size(itable->tree_type);
    smeta.keys_per_table = lpm_keys_per_table(itable->tree_type);
    addr = lpm_tree_root_addr;
    for (uint32_t i = 0, ntables = 1; i < nstages; i++) {
        smeta.stage_info[i].curr_table_addr = addr;
        addr += ntables * LPM_TABLE_SIZE;
        smeta.stage_info[i].end_marker_addr = addr;
        ntables *= (smeta.keys_per_table + 1);
        memset(smeta.stage_info[i].curr_table, 0xFF, LPM_TABLE_SIZE);
    }
    SDK_ASSERT(addr <= lpm_tree_root_addr + lpm_mem_size);

    /**
     * walk all the interval tree nodes & from the interval tree bottom up,
     * last stage has half as many keys as other stages, so divide
     * lpm_keys_per_table() by 2
     */
    nkeys_per_table = smeta.keys_per_table >> 1;
    last_stage_info = &smeta.stage_info[nstages-1];
    /**< set the default data for the first table */
    lpm_set_default_data(smeta.tree_type, last_stage_info, default_nh);
    for (uint32_t i = 0; i < itable->num_intervals; i++) {
        lpm_add_key_to_last_stage(smeta.tree_type, last_stage_info,
                                  &itable->nodes[i]);
        if (last_stage_info->curr_index == nkeys_per_table) {
            /**< current table is full and ready to write */
            /**< write the table here only if there are more nodes */
            /**< otherwise, lpm_flush_tables() will write it */
            if (++i < itable->num_intervals) {
                /**< promote the next node, if any, to the previous stage(s) */
                lpm_promote_route(&itable->nodes[i], nstages - 2, &smeta);
                /**< Write the current table to HW memory */
                lpm_write_last_stage_table(smeta.tree_type, last_stage_info);
                /**< set the default data for the next table */
                /**< it comes from the most recently promoted entry */
                lpm_set_default_data(smeta.tree_type, last_stage_info,
                                     itable->nodes[i].data);
            }
        }
    }
    ret = lpm_flush_tables(&smeta, nstages, itable);
    SDK_ASSERT(ret == SDK_RET_OK);
    ret = lpm_finalize_stages(&smeta, nstages, itable);
    SDK_ASSERT(ret == SDK_RET_OK);
    return SDK_RET_OK;
}

sdk_ret_t
lpm_tree_create (route_table_t *route_table, itree_type_t type,
                 mem_addr_t lpm_tree_root_addr, uint32_t lpm_mem_size)
{
    sdk_ret_t       ret;
    lpm_itable_t    itable;

    if (unlikely(((route_table->af != IP_AF_IPV4) &&
                  (route_table->af != IP_AF_IPV6)))) {
        return sdk::SDK_RET_INVALID_ARG;
    }
    SDK_ASSERT(route_table->num_routes <= route_table->max_routes);

    PDS_TRACE_DEBUG("Creating LPM tree type %u, route count %u, "
                    "default nh %u, max routes %u, root addr 0x%llx, "
                    "LPM block size %u", type, route_table->num_routes,
                    route_table->default_nhid, route_table->max_routes,
                    lpm_tree_root_addr, lpm_mem_size);

    /**< sort the given route table */
    qsort_r(route_table->routes, route_table->num_routes,
            sizeof(route_t), route_compare_cb, route_table);

    /**< allocate memory for creating all interval tree nodes */
    memset(&itable, 0, sizeof(itable));
    itable.nodes =
            (lpm_inode_t *)malloc(sizeof(lpm_inode_t) *
                                  (route_table->num_routes << 1));
    if (itable.nodes == NULL) {
        return sdk::SDK_RET_OOM;
    }
    itable.tree_type = type;
    ret = lpm_build_interval_table(route_table, &itable);
    if (unlikely(ret != SDK_RET_OK)) {
        goto cleanup;
    }
    ret = lpm_build_tree(&itable, route_table->default_nhid,
                         route_table->max_routes, lpm_tree_root_addr,
                         lpm_mem_size);

cleanup:

    free(itable.nodes);
    return ret;
}
