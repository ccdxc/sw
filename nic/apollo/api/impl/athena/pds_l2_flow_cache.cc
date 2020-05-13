//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// athena L2 flow cache implementation
///
//----------------------------------------------------------------------------

#ifndef P4_14
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/platform.hpp"
#include "nic/sdk/include/sdk/table.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/sdk/lib/p4/p4_utils.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/include/athena/pds_l2_flow_cache.h"
#include "nic/apollo/api/include/athena/pds_vnic.h"
#include "nic/apollo/api/include/athena/pds_flow_session_info.h"
#include "nic/apollo/p4/include/athena_defines.h"
#include "nic/apollo/p4/include/athena_table_sizes.h"
#include "ftl_wrapper.h"
#include "gen/p4gen/athena/include/p4pd.h"
//#include "pds_flow_session_ctx.hpp"

using namespace sdk;
using namespace sdk::table;

extern "C" {

static ftl_base *l2_flow_table;

uint32_t l2_flow_entry_count;

typedef struct pds_l2_flow_iterate_cbdata_s {
    pds_l2_flow_iter_cb_t        iter_cb;
    pds_l2_flow_iter_cb_arg_t    *iter_cb_arg;
} pds_l2_flow_iter_cbdata_t;


static sdk_ret_t
l2_flow_cache_entry_setup_key (l2_flow_hash_entry_t *entry,
                            pds_l2_flow_key_t *key)
{
    if (!entry) {
        PDS_TRACE_ERR("entry is null");
        return SDK_RET_INVALID_ARG;
    }

    uint64_t     mac = 0;
    l2flow_set_key_vnic_id(entry, key->vnic_id);
    memcpy((uint8_t*)&mac,
	   key->dmac,
	   ETH_ADDR_LEN);
    l2flow_set_key_dmac(entry, mac);
    return SDK_RET_OK;
}

sdk_ret_t
pds_l2_flow_cache_create ()
{
    sdk_table_factory_params_t factory_params = { 0 };

#if 0
    sdk_ret_t ret = (sdk_ret_t)
                    pds_flow_session_ctx_init(SESSION_CTX_LOCK_INTERNAL);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Session context init failed with ret %u\n", ret);
        return ret;
    }
#endif

    factory_params.table_id = P4TBL_ID_L2_FLOW;
    factory_params.num_hints = 5;
    factory_params.max_recircs = 8;
    factory_params.key2str = NULL;
    factory_params.appdata2str = NULL;
    factory_params.entry_trace_en = true;

    if ((l2_flow_table = l2_flow_hash::factory(&factory_params)) == NULL) {
        PDS_TRACE_ERR("L2 Table creation failed.");
        return SDK_RET_OOM;
    }
    return SDK_RET_OK;
}

void
pds_l2_flow_cache_set_core_id (unsigned int core_id)
{
    l2_flow_table->set_thread_id(core_id);
}

pds_ret_t
pds_l2_flow_cache_entry_create (pds_l2_flow_spec_t *spec)
{
    sdk_ret_t ret;
    sdk_table_api_params_t params = { 0 };
    l2_flow_hash_entry_t entry;
    uint32_t index;
    uint16_t vnic_id;

    if (!spec) {
        PDS_TRACE_ERR("spec is null");
        return PDS_RET_INVALID_ARG;
    }
    vnic_id = spec->key.vnic_id;
    if (spec->key.vnic_id == 0 ||
        spec->key.vnic_id > PDS_VNIC_ID_MAX) {
        //PDS_TRACE_ERR("Vnic id %u invalid", vnic_id);
        return PDS_RET_INVALID_ARG;
    }

    index = spec->data.index;
    if (index > PDS_FLOW_SESSION_INFO_ID_MAX) {
        PDS_TRACE_ERR("session id %u is invalid", index);
        return PDS_RET_INVALID_ARG;
    }

    entry.clear();
    if ((ret = l2_flow_cache_entry_setup_key(&entry, &spec->key))
             != SDK_RET_OK)
         return (pds_ret_t) ret;
    l2flow_set_index(&entry, index);
    params.entry = &entry;
    ret = l2_flow_table->insert(&params);
#if 0
    if (ret == SDK_RET_OK) {
        //PDS_TRACE_VERBOSE("session_ctx create session_id %u pindex %u "
        //                  "sindex %u", index, params.handle.pindex(),
        //                  params.handle.sindex());
        ret = (sdk_ret_t)
              pds_flow_session_ctx_set(index, params.handle.pindex(),
                                       params.handle.sindex(),
                                       params.handle.pvalid());
    }
#endif
    return (pds_ret_t) ret;
}

pds_ret_t
pds_l2_flow_cache_entry_read (pds_l2_flow_key_t *key,
                           pds_l2_flow_info_t *info)
{
    sdk_ret_t ret;
    sdk_table_api_params_t params = { 0 };
    l2_flow_hash_entry_t entry;

    if (!key || !info) {
        PDS_TRACE_ERR("key/info is null");
        return PDS_RET_INVALID_ARG;
    }

    entry.clear();
    if ((ret = l2_flow_cache_entry_setup_key(&entry, key))
             != SDK_RET_OK)
         return (pds_ret_t)ret;
    params.entry = &entry;
    ret = l2_flow_table->get(&params);
    if (ret == SDK_RET_OK) {
        info->spec.data.index = entry.idx;
        return (pds_ret_t)ret;
    }
    else {
        return PDS_RET_ENTRY_NOT_FOUND;
    }
}

pds_ret_t
pds_l2_flow_cache_entry_update (pds_l2_flow_spec_t *spec)
{
    sdk_ret_t ret;
    sdk_table_api_params_t params = { 0 };
    l2_flow_hash_entry_t entry;
    uint32_t index;

    if (!spec) {
        PDS_TRACE_ERR("spec is null");
        return PDS_RET_INVALID_ARG;
    }

    index = spec->data.index;
    if (index > PDS_FLOW_SESSION_INFO_ID_MAX) {
        PDS_TRACE_ERR("session id %u is invalid", index);
        return PDS_RET_INVALID_ARG;
    }

    entry.clear();
    if ((ret = l2_flow_cache_entry_setup_key(&entry, &spec->key))
             != SDK_RET_OK)
         return (pds_ret_t)ret;
    l2flow_set_index(&entry, index);
    params.entry = &entry;
    ret = l2_flow_table->update(&params);
#if 0
    if (ret == SDK_RET_OK) {
        //PDS_TRACE_VERBOSE("session_ctx update session_id %u pindex %u "
        //                  "sindex %u", index, params.handle.pindex(),
        //                  params.handle.sindex());
        ret = (sdk_ret_t)
              pds_flow_session_ctx_set(index, params.handle.pindex(),
                                       params.handle.sindex(),
                                       params.handle.pvalid());
    }
#endif
    return (pds_ret_t) ret;
}

#if 0
static void
l2_flow_table_entry_move (base_table_entry_t *base_entry,
                      handle_t old_handle, 
                      handle_t new_handle,
                      bool move_complete)
{
    l2_flow_hash_entry_t *entry = (l2_flow_hash_entry_t *)base_entry;
    uint32_t session_id = entry->get_idx();
    uint32_t cache_id;

    cache_id = new_handle.pvalid() ? 
               new_handle.pindex() : new_handle.sindex();
    pds_flow_session_ctx_move(session_id, cache_id, 
                              new_handle.pvalid(), move_complete);
}
#endif

pds_ret_t
pds_l2_flow_cache_entry_delete (pds_l2_flow_key_t *key)
{
    sdk_ret_t ret;
    sdk_table_api_params_t params = { 0 };
    l2_flow_hash_entry_t entry;

    if (!key) {
        PDS_TRACE_ERR("key is null");
        return PDS_RET_INVALID_ARG;
    }

    entry.clear();
    if ((ret = l2_flow_cache_entry_setup_key(&entry, key))
             != SDK_RET_OK)
         return (pds_ret_t) ret;
    params.entry = &entry;
    //params.movecb = l2_flow_table_entry_move;
    ret = l2_flow_table->remove(&params);
#if 0
    if (ret == SDK_RET_OK) {
        pds_flow_session_ctx_clr(entry.get_idx());
    }
#endif
    return (pds_ret_t) ret;
}

#if 0
pds_ret_t
pds_l2_flow_cache_entry_delete_by_flow_info (pds_l2_flow_info_t *info)
{
    pds_ret_t ret;
    sdk_table_api_params_t params = { 0 };
    l2_flow_hash_entry_t entry;
    uint32_t cache_id;
    bool primary;

    if (!info) {
        PDS_TRACE_ERR("flow info is null");
        return PDS_RET_INVALID_ARG;
    }
    ret = pds_flow_session_ctx_get_clr(info->spec.data.index,
                                       &cache_id, &primary);
    if (ret == PDS_RET_ENTRY_NOT_FOUND) {

        // This is a soft error so no need to log;
        // either session was never mapped to cache, or was already deleted.
        return PDS_RET_OK;
    }
    if (ret != PDS_RET_OK) {
        return ret;
    }
    //PDS_TRACE_VERBOSE("delete_by_flow_info cache_id %u primary %u",
    //                  cache_id, primary);
    if (primary) {
        params.handle.pindex(cache_id);
    } else {
        params.handle.sindex(cache_id);
    }
    params.entry = &entry;
    ret = (pds_ret_t) l2_flow_table->get_with_handle(&params);
    if (ret != PDS_RET_OK) {
        PDS_TRACE_ERR("Failed to get cache handle for cache_id %u",
                      cache_id);
        return ret;
    }
    params.hash_valid = true;
    params.hash_32b = cache_id;
    //params.movecb = l2_flow_table_entry_move;
    return (pds_ret_t) l2_flow_table->remove(&params);
}
#endif

void
pds_l2_flow_cache_delete ()
{
    l2_flow_table->destroy(l2_flow_table);
    //pds_flow_session_ctx_fini();
    return;
}

static void
l2_flow_cache_entry_iterate_cb (sdk_table_api_params_t *params)
{
    l2_flow_hash_entry_t *hwentry = (l2_flow_hash_entry_t *)params->entry;
    pds_l2_flow_iter_cbdata_t *cbdata = (pds_l2_flow_iter_cbdata_t *)params->cbdata;
    pds_l2_flow_key_t *key;
    pds_l2_flow_data_t *data;
    uint64_t     dmac;
    //uint8_t     mac[ETH_ADDR_LEN] = {0};

    if (hwentry->entry_valid) {
        l2_flow_entry_count++;
        key = &cbdata->iter_cb_arg->l2_flow_key;
        data = &cbdata->iter_cb_arg->l2_flow_appdata;
        key->vnic_id = l2flow_get_key_vnic_id(hwentry);
	dmac = l2flow_get_key_dmac(hwentry);
	//MAC_UINT64_TO_ADDR(mac, dmac);
	memcpy(key->dmac, (uint8_t*)&dmac, ETH_ADDR_LEN);
        data->index = l2flow_get_index(hwentry);
        cbdata->iter_cb(cbdata->iter_cb_arg);
    }
    return;
}

pds_ret_t
pds_l2_flow_cache_entry_iterate (pds_l2_flow_iter_cb_t iter_cb,
                              pds_l2_flow_iter_cb_arg_t *iter_cb_arg)
{
    sdk_table_api_params_t params = { 0 };
    pds_l2_flow_iter_cbdata_t cbdata = { 0 };

    if (iter_cb == NULL || iter_cb_arg == NULL) {
        PDS_TRACE_ERR("itercb or itercb_arg is null");
        return PDS_RET_INVALID_ARG;
     }

    cbdata.iter_cb = iter_cb;
    cbdata.iter_cb_arg = iter_cb_arg;
    params.itercb = l2_flow_cache_entry_iterate_cb;
    params.cbdata = &cbdata;
    params.force_hwread = false;
    l2_flow_entry_count = 0;
    return (pds_ret_t)l2_flow_table->iterate(&params);
}

pds_ret_t
pds_l2_flow_cache_stats_get (int32_t core_id, pds_l2_flow_stats_t *stats)
{
    sdk_ret_t ret;
    sdk_table_api_stats_t api_stats = { 0 };
    sdk_table_stats_t table_stats = { 0 };
    int i;

    if (!stats) {
        PDS_TRACE_ERR("Stats is null");
        return PDS_RET_INVALID_ARG;
    }

    if (core_id != -1)
        ret = l2_flow_table->stats_get(&api_stats, &table_stats, false, core_id);
    else
        ret = l2_flow_table->stats_get(&api_stats, &table_stats);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Stats get failed");
        return (pds_ret_t)ret;
    }
    // Populate api statistics
    stats->api_insert = api_stats.insert;
    stats->api_insert_duplicate = api_stats.insert_duplicate;
    stats->api_insert_fail = api_stats.insert_fail;
    stats->api_insert_recirc_fail = api_stats.insert_recirc_fail;
    stats->api_remove = api_stats.remove;
    stats->api_remove_not_found = api_stats.remove_not_found;
    stats->api_remove_fail = api_stats.remove_fail;
    stats->api_update = api_stats.update;
    stats->api_update_fail = api_stats.update_fail;
    stats->api_get = api_stats.get;
    stats->api_get_fail = api_stats.get_fail;
    stats->api_reserve = api_stats.reserve;
    stats->api_reserve_fail = api_stats.reserve_fail;
    stats->api_release = api_stats.release;
    stats->api_release_fail = api_stats.release_fail;

    // Populate table statistics
    stats->table_entries = table_stats.entries;
    stats->table_collisions = table_stats.collisions;
    stats->table_insert = table_stats.insert;
    stats->table_remove = table_stats.remove;
    stats->table_read = table_stats.read;
    stats->table_write = table_stats.write;
    for(i = 0; i < PDS_L2_FLOW_TABLE_MAX_RECIRC; i++) {
        stats->table_insert_lvl[i] = table_stats.insert_lvl[i];
        stats->table_remove_lvl[i] = table_stats.remove_lvl[i];
    }
    return PDS_RET_OK;
}

pds_ret_t
pds_l2_flow_cache_table_clear(void)
{
    sdk_table_api_params_t      params = {0};
    return (pds_ret_t)l2_flow_table->clear(TRUE, FALSE, &params);
}

}
#endif
