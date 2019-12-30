// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
#include <string.h>
#include "nic/sdk/include/sdk/table.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "gen/p4gen/p4/include/ftl.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "gen/p4gen/p4/include/p4pd.h"
#include "flow_table_pd.hpp"

using table::TableFlowEntry;
using sdk::table::sdk_table_factory_params_t;
using sdk::table::sdk_table_api_params_t;
using hal::pd::flow_table_pd;

typedef char* (*key2str_t)(void *key);
typedef char* (*appdata2str_t)(void *data);

static char*
key2str(void *key) {
    thread_local static char str[256] = { 0 };
    char srcstr[INET6_ADDRSTRLEN] = { 0 };
    char dststr[INET6_ADDRSTRLEN] = { 0 };
    flow_hash_info_entry_t     *swkey = static_cast<flow_hash_info_entry_t*>(key);
    SDK_ASSERT(swkey);

    if (swkey->flow_lkp_metadata_lkp_type == FLOW_KEY_LOOKUP_TYPE_IPV6) {
        inet_ntop(AF_INET6, swkey->flow_lkp_metadata_lkp_src,
                  srcstr, INET6_ADDRSTRLEN);
        inet_ntop(AF_INET6, swkey->flow_lkp_metadata_lkp_dst,
                  dststr, INET6_ADDRSTRLEN);
    } else if (swkey->flow_lkp_metadata_lkp_type == FLOW_KEY_LOOKUP_TYPE_MAC) {
        snprintf(srcstr, INET6_ADDRSTRLEN, "%02x:%02x:%02x:%02x:%02x:%02x",
                 swkey->flow_lkp_metadata_lkp_src[0],
                 swkey->flow_lkp_metadata_lkp_src[1],
                 swkey->flow_lkp_metadata_lkp_src[2],
                 swkey->flow_lkp_metadata_lkp_src[3],
                 swkey->flow_lkp_metadata_lkp_src[4],
                 swkey->flow_lkp_metadata_lkp_src[5]);
        snprintf(dststr, INET6_ADDRSTRLEN, "%02x:%02x:%02x:%02x:%02x:%02x",
                 swkey->flow_lkp_metadata_lkp_dst[0],
                 swkey->flow_lkp_metadata_lkp_dst[1],
                 swkey->flow_lkp_metadata_lkp_dst[2],
                 swkey->flow_lkp_metadata_lkp_dst[3],
                 swkey->flow_lkp_metadata_lkp_dst[4],
                 swkey->flow_lkp_metadata_lkp_dst[5]);
    } else if (swkey->flow_lkp_metadata_lkp_type == FLOW_KEY_LOOKUP_TYPE_IPV4) {
        inet_ntop(AF_INET, swkey->flow_lkp_metadata_lkp_src, srcstr, INET_ADDRSTRLEN);
        inet_ntop(AF_INET, swkey->flow_lkp_metadata_lkp_dst, dststr, INET_ADDRSTRLEN);
    }

    snprintf(str, sizeof(str), "type=%lu,inst=%lu,dst=%s,"
             "src=%s,dport=%lu,sport=%lu,proto=%lu,vrf=%lu",
             swkey->flow_lkp_metadata_lkp_type,
             swkey->flow_lkp_metadata_lkp_inst,
             dststr, srcstr,
             swkey->flow_lkp_metadata_lkp_dport,
             swkey->flow_lkp_metadata_lkp_sport,
             swkey->flow_lkp_metadata_lkp_proto,
             swkey->flow_lkp_metadata_lkp_vrf);
    return str;
}

static char*
appdata2str(void *entry) {
    thread_local static char str[256] = { 0 };
    flow_hash_info_entry_t     *swentry = static_cast<flow_hash_info_entry_t*>(entry);
    SDK_ASSERT(swentry);

    snprintf(str, sizeof(str), "export_en=%lu,flow_index=%lu",
             swentry->export_en, swentry->flow_index);
    return str;
}

flow_table_pd *
flow_table_pd::factory() {
    hal_ret_t ret = HAL_RET_OK;
    flow_table_pd *ftpd = NULL;

    ftpd = new flow_table_pd();
    SDK_ASSERT_RETURN(ftpd, NULL);

    ret = ftpd->init();
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to create flow table pd. ret={}", ret);
        delete ftpd;
        return NULL;
    }

    return ftpd;
}

void
flow_table_pd::destroy(flow_table_pd *ftpd) {
    ftl_base::destroy(ftpd->table_);
    return;
}

hal_ret_t
flow_table_pd::init() {
    p4pd_table_properties_t tinfo, ctinfo;
    sdk_table_factory_params_t params = { 0 };
    
    p4pd_table_properties_get(P4TBL_ID_FLOW_HASH, &tinfo);
    table_name_ = tinfo.tablename;
    table_size_ = tinfo.tabledepth;

    p4pd_table_properties_get(tinfo.oflow_table_id, &ctinfo);
    oflow_table_size_ = ctinfo.tabledepth;

    params.table_id = P4TBL_ID_FLOW_HASH;
    params.num_hints = P4_FLOW_NUM_HINTS_PER_ENTRY;
    params.max_recircs = 8;
    params.key2str = key2str;
    params.appdata2str = appdata2str;
    params.entry_alloc_cb = flow_hash_info_entry_t::alloc;
    // params.entry_trace_en = true;

    table_ = ftl_base::factory(&params);
    SDK_ASSERT_RETURN(table_, HAL_RET_OOM);
    return HAL_RET_OK;
}

hal_ret_t
flow_table_pd::stats_get(sys::TableStatsEntry *stats_entry) {
    sdk_ret_t sret = SDK_RET_OK;

    sdk::table::sdk_table_stats_t table_stats = { 0 };
    sdk::table::sdk_table_api_stats_t api_stats = { 0 };
    
    stats_entry->set_table_name(table_name_);
    stats_entry->set_table_size(table_size_);
    stats_entry->set_overflow_table_size(oflow_table_size_);

    sret = table_->stats_get(&api_stats, &table_stats);
    if (sret == SDK_RET_OK) {
        stats_entry->set_entries_in_use(table_stats.entries);
        stats_entry->set_overflow_entries_in_use(table_stats.collisions);

        stats_entry->set_num_inserts(api_stats.insert);
        stats_entry->set_num_insert_errors(api_stats.insert_fail + api_stats.insert_duplicate);
        stats_entry->set_num_deletes(api_stats.remove);
        stats_entry->set_num_delete_errors(api_stats.remove_fail + api_stats.remove_not_found);
    }
    
    return hal_sdk_ret_to_hal_ret(sret);
}

// -----------------------------------------------------------------------
// TableResponseEntryFill: fills proto
// -----------------------------------------------------------------------
static void
table_entry_fill(sdk_table_api_params_t *params) {
    char *str = NULL;
    flow_hash_info_entry_t *hwentry = (flow_hash_info_entry_t *) params->entry;
    TableResponse *rsp = static_cast<TableResponse*>(params->cbdata);
    TableFlowEntry *entry = rsp->mutable_flow_table()->add_flow_entry();

    if (hwentry->entry_valid) {
        str = key2str(hwentry);
        entry->set_key(str);
        HAL_TRACE_VERBOSE("key {}", str);

        str = appdata2str(hwentry);
        entry->set_data(str);
        HAL_TRACE_VERBOSE("app data {}", str);

        entry->set_primary_index_valid(params->handle.pvalid());
        entry->set_primary_index(params->handle.pindex());
        entry->set_secondary_index_valid(params->handle.svalid());
        entry->set_secondary_index(params->handle.sindex());

        HAL_TRACE_VERBOSE("pindex valid {}, pindex {}, sindex valid {}, sindex {}",
                          params->handle.pvalid(), params->handle.pindex(),
                          params->handle.svalid(), params->handle.sindex());
    }

    return;
}

// -----------------------------------------------------------------------
// dump(): Table dump handler
// -----------------------------------------------------------------------
hal_ret_t
flow_table_pd::dump(TableResponse *rsp) {
    sdk_table_api_params_t params = { 0 };

    params.cbdata = rsp;
    params.itercb = table_entry_fill;
    table_->iterate(&params);
    return HAL_RET_OK;
}


// -----------------------------------------------------------------------
// insert(): Insert flow into flow table
// -----------------------------------------------------------------------
hal_ret_t
flow_table_pd::insert(void *entry,
                      uint32_t *hash_value, bool hash_valid) {
    sdk_table_api_params_t params;
    sdk_ret_t sret = SDK_RET_OK;
    hal_ret_t ret = HAL_RET_OK;

    bzero((void *)&params, sizeof(sdk_table_api_params_t));
    params.entry = (base_table_entry_t *)entry;
    params.entry_size = flow_hash_info_entry_t::entry_size();
    if (hash_valid) {
        params.hash_32b = *hash_value;
        params.hash_valid = true;
    }

    sret = table_->insert(&params);
    ret = hal_sdk_ret_to_hal_ret(sret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error in flowtable insert, sret {}, key {}", sret, key2str(params.entry));
        HAL_TRACE_ERR("Error in flowtable insert, app data {}, "
                      "pindex valid {}, pindex {}, sindex valid {}, sindex {}",
                      appdata2str(params.entry), params.handle.pvalid(),
                      params.handle.pindex(), params.handle.svalid(),
                      params.handle.sindex());

        return ret;
    }
    if (hash_valid == false) {
        *hash_value = params.hash_32b;
    }

    HAL_TRACE_VERBOSE("flowtable insert, hash {}, key {}", params.hash_32b, key2str(params.entry));
    HAL_TRACE_VERBOSE("flowtable insert: app data {}, pindex valid {}, "
                      "pindex {}, sindex valid {}, sindex {}",
                      appdata2str(params.entry), params.handle.pvalid(),
                      params.handle.pindex(), params.handle.svalid(),
                      params.handle.sindex());

    return HAL_RET_OK;
}

// -----------------------------------------------------------------------
// update(): Update flow into flow table
// -----------------------------------------------------------------------
hal_ret_t
flow_table_pd::update(void *entry,
                      uint32_t *hash_value, bool hash_valid) {
    sdk_table_api_params_t params = { 0 };
    sdk_ret_t sret = SDK_RET_OK;
    hal_ret_t ret = HAL_RET_OK;
    
    bzero((void *)&params, sizeof(sdk_table_api_params_t));
    params.entry = (base_table_entry_t *)entry;
    params.entry_size = flow_hash_info_entry_t::entry_size();
    if (hash_valid) {
        params.hash_32b = *hash_value;
        params.hash_valid = 1;
    }

    sret = table_->update(&params);
    ret = hal_sdk_ret_to_hal_ret(sret);
    if (ret != HAL_RET_OK) {
        return ret;
    }
    if (hash_valid == false) {
        *hash_value = params.hash_32b;
    }

    return HAL_RET_OK;
}

// -----------------------------------------------------------------------
// remove(): Remove flow from flow table
// -----------------------------------------------------------------------
hal_ret_t
flow_table_pd::remove(void *entry) {
    sdk_table_api_params_t params;
    sdk_ret_t sret = SDK_RET_OK;
    hal_ret_t ret = HAL_RET_OK;

    bzero((void *)&params, sizeof(sdk_table_api_params_t));
    params.entry = (base_table_entry_t *)entry;
    params.entry_size = flow_hash_info_entry_t::entry_size();
    sret = table_->remove(&params);

    ret = hal_sdk_ret_to_hal_ret(sret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error in flowtable remove, sret {}, key {}", sret, key2str(params.entry));
        HAL_TRACE_ERR("Error in flowtable remove, app data {}, pindex valid {}, "
                      " pindex {}, sindex valid {}, sindex {}",
                      appdata2str(params.entry), params.handle.pvalid(),
                      params.handle.pindex(), params.handle.svalid(),
                      params.handle.sindex());
    } else {
        HAL_TRACE_VERBOSE("flowtable remove, key {}", key2str(params.entry));
        HAL_TRACE_VERBOSE("flowtable remove, app data {}, pindex valid {}, "
                          "pindex {}, sindex valid {}, sindex {}",
                          appdata2str(params.entry), params.handle.pvalid(),
                          params.handle.pindex(), params.handle.svalid(),
                          params.handle.sindex());
    }
    return hal_sdk_ret_to_hal_ret(sret);
}

hal_ret_t
flow_table_pd::meta_get(table::TableMetadataResponseMsg *rsp_msg) {
    sdk::table::sdk_table_stats_t table_stats = { 0 };
    sdk::table::sdk_table_api_stats_t api_stats = { 0 };
    sdk_ret_t sret = SDK_RET_OK;

    auto table_meta_flow = rsp_msg->add_table_meta();
    table_meta_flow->set_table_id(P4TBL_ID_FLOW_HASH);
    table_meta_flow->set_kind(table::TABLE_FLOW);
    table_meta_flow->set_table_name(table_name_);

    auto flow_meta = table_meta_flow->mutable_flow_meta();
    flow_meta->set_capacity(table_size_);
    flow_meta->set_coll_capacity(oflow_table_size_);

    sret = table_->stats_get(&api_stats, &table_stats);
    if (sret == SDK_RET_OK) {
        flow_meta->set_hash_usage(table_stats.entries);
        flow_meta->set_coll_usage(table_stats.collisions);

        flow_meta->set_num_inserts(api_stats.insert);
        flow_meta->set_num_insert_failures(api_stats.insert_fail + api_stats.insert_duplicate);
        flow_meta->set_num_updates(api_stats.update);
        flow_meta->set_num_update_failures(api_stats.update_fail);
        flow_meta->set_num_deletes(api_stats.remove);
        flow_meta->set_num_delete_failures(api_stats.remove_fail + api_stats.remove_not_found);
    }

    return hal_sdk_ret_to_hal_ret(sret);
}

hal_ret_t
flow_table_pd::get(void *entry, FlowHashGetResponse *rsp) {
    flow_hash_appdata_t swappdata = { 0 };
    sdk_table_api_params_t params = { 0 };
    sdk_ret_t sret = SDK_RET_OK;
    char *str = NULL;

    params.entry = (base_table_entry_t *)entry;
    params.appdata = &swappdata;
    params.entry_size = flow_hash_info_entry_t::entry_size();
    sret = table_->get(&params);
    if (sret == SDK_RET_OK) {
        str = key2str(params.entry);
        rsp->set_key(str);

        str = appdata2str(params.entry);
        rsp->set_data(str);
        rsp->set_primary_index_valid(params.handle.pvalid());
        rsp->set_primary_index(params.handle.pindex());
        rsp->set_secondary_index_valid(params.handle.svalid());
        rsp->set_secondary_index(params.handle.sindex());
    }

    return hal_sdk_ret_to_hal_ret(sret);
}
