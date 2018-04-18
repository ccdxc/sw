// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/gen/proto/hal/table.pb.h"
#include "nic/hal/pd/iris/debug/table_pd.hpp"

using table::TableMetadataResponseMsg;

namespace hal {
namespace pd {

hal_ret_t
pd_table_metadata_get (pd_table_metadata_get_args_t *args)
{
    hal_ret_t ret = HAL_RET_OK;
    directmap *dm = NULL;
    tcam *tm = NULL;
    sdk::table::hash *h = NULL;
    Flow *flow = NULL;
    Met *met = NULL;
    TableMetadataResponseMsg *rsp_msg = args->rsp;

    // DirectMap
    for (int i = P4TBL_ID_INDEX_MIN; i < P4TBL_ID_INDEX_MAX; i++) {
        dm = g_hal_state_pd->dm_table(i);
        if (dm == NULL) {
            continue;
        }
        auto table_meta = rsp_msg->add_table_meta();
        table_meta->set_table_id(i);
        table_meta->set_kind(table::TABLE_INDEX);
        table_meta->set_table_name(dm->name());
        auto dm_meta = table_meta->mutable_index_meta();
        dm_meta->set_capacity(dm->capacity());
        dm_meta->set_usage(dm->num_entries_in_use());
        dm_meta->set_num_inserts(dm->num_inserts());
        dm_meta->set_num_insert_failures(dm->num_insert_errors());
        dm_meta->set_num_updates(dm->num_updates());
        dm_meta->set_num_update_failures(dm->num_update_errors());
        dm_meta->set_num_deletes(dm->num_deletes());
        dm_meta->set_num_delete_failures(dm->num_delete_errors());
    }

    // Tcam
    for (int i = P4TBL_ID_TCAM_MIN; i < P4TBL_ID_TCAM_MAX; i++) {
        tm = g_hal_state_pd->tcam_table(i);
        if (tm == NULL) {
            continue;
        }
        auto table_meta = rsp_msg->add_table_meta();
        table_meta->set_table_id(i);
        table_meta->set_kind(table::TABLE_TCAM);
        table_meta->set_table_name(tm->name());
        auto tcam_meta = table_meta->mutable_tcam_meta();
        tcam_meta->set_capacity(tm->capacity());
        tcam_meta->set_usage(tm->num_entries_in_use());
        tcam_meta->set_num_inserts(tm->num_inserts());
        tcam_meta->set_num_insert_failures(tm->num_insert_errors());
        tcam_meta->set_num_updates(tm->num_updates());
        tcam_meta->set_num_update_failures(tm->num_update_errors());
        tcam_meta->set_num_deletes(tm->num_deletes());
        tcam_meta->set_num_delete_failures(tm->num_delete_errors());
    }

    // Hash
    for (int i = P4TBL_ID_HASH_OTCAM_MIN; i < P4TBL_ID_HASH_OTCAM_MAX; i++) {
        h = g_hal_state_pd->hash_tcam_table(i);
        if (h == NULL) {
            continue;
        }
        auto table_meta = rsp_msg->add_table_meta();
        table_meta->set_table_id(i);
        table_meta->set_kind(table::TABLE_HASH);
        table_meta->set_table_name(h->name());
        auto hash_meta = table_meta->mutable_hash_meta();
        hash_meta->set_has_otcam(h->oflow_capacity() ? true : false);
        hash_meta->set_capacity(h->capacity());
        hash_meta->set_otcam_capacity(h->oflow_capacity());
        hash_meta->set_hash_usage(h->num_entries_in_use());
        hash_meta->set_otcam_usage(h->oflow_num_entries_in_use());
        hash_meta->set_num_inserts(h->num_inserts());
        hash_meta->set_num_insert_failures(h->num_insert_errors());
        hash_meta->set_num_updates(h->num_updates());
        hash_meta->set_num_update_failures(h->num_update_errors());
        hash_meta->set_num_deletes(h->num_deletes());
        hash_meta->set_num_delete_failures(h->num_delete_errors());
    }

    // Flow
    flow = g_hal_state_pd->flow_table();
    auto table_meta_flow = rsp_msg->add_table_meta();
    table_meta_flow->set_table_id(flow->table_id());
    table_meta_flow->set_kind(table::TABLE_FLOW);
    table_meta_flow->set_table_name(flow->table_name());
    auto flow_meta = table_meta_flow->mutable_flow_meta();
    flow_meta->set_capacity(flow->table_capacity());
    flow_meta->set_coll_capacity(flow->oflow_table_capacity());
    flow_meta->set_hash_usage(flow->table_num_entries_in_use());
    flow_meta->set_coll_usage(flow->oflow_table_num_entries_in_use());
    flow_meta->set_num_inserts(flow->table_num_inserts());
    flow_meta->set_num_insert_failures(flow->table_num_insert_errors());
    flow_meta->set_num_updates(flow->table_num_updates());
    flow_meta->set_num_update_failures(flow->table_num_update_errors());
    flow_meta->set_num_deletes(flow->table_num_deletes());
    flow_meta->set_num_delete_failures(flow->table_num_delete_errors());

    // Met
    met = g_hal_state_pd->met_table();
    auto table_meta_met = rsp_msg->add_table_meta();
    table_meta_met->set_table_id(met->table_id());
    table_meta_met->set_kind(table::TABLE_MET);
    table_meta_met->set_table_name(met->table_name());
    auto met_meta = table_meta_met->mutable_met_meta();
    met_meta->set_capacity(met->table_capacity());
    met_meta->set_usage(met->table_num_entries_in_use());
    met_meta->set_num_inserts(met->table_num_inserts());
    met_meta->set_num_insert_failures(met->table_num_insert_errors());
    met_meta->set_num_updates(met->table_num_updates());
    met_meta->set_num_update_failures(met->table_num_update_errors());
    met_meta->set_num_deletes(met->table_num_deletes());
    met_meta->set_num_delete_failures(met->table_num_delete_errors());

    return ret;
}

hal_ret_t
pd_table_kind_get_from_id (uint32_t table_id, table::TableKind *kind)
{
    hal_ret_t   ret = HAL_RET_OK;

    if (table_id >= P4TBL_ID_INDEX_MIN && table_id < P4TBL_ID_INDEX_MAX) {
        *kind = table::TABLE_INDEX;
        return ret;
    }

    if (table_id >= P4TBL_ID_TCAM_MIN && table_id < P4TBL_ID_TCAM_MAX) {
        *kind = table::TABLE_TCAM;
        return ret;
    }

    if (table_id >= P4TBL_ID_HASH_OTCAM_MIN && table_id < P4TBL_ID_HASH_OTCAM_MAX) {
        *kind = table::TABLE_HASH;
        return ret;
    }

    if (table_id == P4TBL_ID_FLOW_HASH) {
        *kind = table::TABLE_FLOW;
        return ret;
    }

    if (table_id == P4_REPL_TABLE_ID) {
        *kind = table::TABLE_MET;
        return ret;
    }

    return HAL_RET_INVALID_ARG;
}


bool pd_table_directmap_entry(uint32_t index, void *data, const void *cb_data)
{
    char buff[4096] = {0};
    directmap_entry_cb_t *cb = (directmap_entry_cb_t *)cb_data;
    directmap *dm = cb->dm;
    TableIndexMsg *msg = cb->msg;
    TableIndexEntry *entry = msg->add_index_entry();

    dm->entry_to_str(data, index, buff, sizeof(buff));

    HAL_TRACE_DEBUG("Entry: {}", buff);

    entry->set_index(index);
    entry->set_entry(buff);

    return TRUE;
}

hal_ret_t
pd_table_index_get_entries (uint32_t table_id, TableResponse *rsp)
{
    hal_ret_t   ret = HAL_RET_OK;
    directmap_entry_cb_t cb = {0};
    TableIndexMsg *msg = rsp->mutable_index_table();

    cb.dm = g_hal_state_pd->dm_table(table_id);
    HAL_ASSERT_RETURN((cb.dm != NULL), HAL_RET_ERR);
    cb.msg = msg;

    cb.dm->iterate(pd_table_directmap_entry, &cb);

    return ret;
}

bool pd_table_tcam_entry(void *key, void *key_mask,
                         void *data, uint32_t tcam_idx, const void *cb_data)
{
    char buff[4096] = {0};
    tcam_entry_cb_t *cb = (tcam_entry_cb_t*)cb_data;
    tcam *tcam = cb->tcam_table;
    TableTcamMsg *msg = cb->msg;
    TableTcamEntry *entry = msg->add_tcam_entry();

    tcam->entry_to_str(key, key_mask, data, tcam_idx, buff, sizeof(buff));

    HAL_TRACE_DEBUG("Entry: {}", buff);

    entry->set_index(tcam_idx);
    entry->set_entry(buff);

    return TRUE;
}

hal_ret_t
pd_table_tcam_get_entries (uint32_t table_id, TableResponse *rsp)
{
    hal_ret_t   ret = HAL_RET_OK;
    tcam_entry_cb_t cb = {0};
    TableTcamMsg *msg = rsp->mutable_tcam_table();

    cb.tcam_table = g_hal_state_pd->tcam_table(table_id);
    HAL_ASSERT_RETURN((cb.tcam_table != NULL), HAL_RET_ERR);
    cb.msg = msg;

    cb.tcam_table->iterate(pd_table_tcam_entry, &cb);

    return ret;
}

bool pd_table_hash_entry(void *key, void *key_mask,
                         void *data, uint32_t index, const void *cb_data)
{
    char buff[4096] = {0};
    pd_hash_entry_cb_t *cb = (pd_hash_entry_cb_t*)cb_data;
    sdk_hash *hash = cb->hash_table;
    TableHashMsg *msg = cb->msg;
    TableHashEntry *entry = msg->add_hash_entry();

    hash->entry_to_str(key, key_mask, data, index, buff, sizeof(buff));

    HAL_TRACE_DEBUG("Entry: {}", buff);

    entry->set_in_otcam(sdk_hash::is_dleft(index) ? false : true);
    entry->set_index(index);
    entry->set_entry(buff);

    return TRUE;
}

hal_ret_t
pd_table_hash_get_entries (uint32_t table_id, TableResponse *rsp)
{
    hal_ret_t   ret = HAL_RET_OK;
    pd_hash_entry_cb_t cb = {0};
    TableHashMsg *msg = rsp->mutable_hash_table();

    cb.hash_table = g_hal_state_pd->hash_tcam_table(table_id);
    HAL_ASSERT_RETURN((cb.hash_table != NULL), HAL_RET_ERR);
    cb.msg = msg;

    cb.hash_table->iterate(pd_table_hash_entry, &cb, sdk_hash::BOTH);

    return ret;
}

// Flow entry
bool pd_table_flow_entry(uint32_t index, const void *cb_data)
{
    char buff[8192] = {0};
    pd_flow_entry_cb_t *cb = (pd_flow_entry_cb_t*)cb_data;
    Flow *flow = cb->flow;
    TableFlowMsg *msg = cb->msg;
    TableFlowEntry *entry = msg->add_flow_entry();

    flow->entry_to_str(index, buff, sizeof(buff));

    HAL_TRACE_DEBUG("Entry: {}", buff);

    entry->set_index(index);
    entry->set_entry(buff);

    return TRUE;
}

hal_ret_t
pd_table_flow_get_entries (uint32_t table_id, TableResponse *rsp)
{
    hal_ret_t   ret = HAL_RET_OK;
    pd_flow_entry_cb_t cb = {0};
    TableFlowMsg *msg = rsp->mutable_flow_table();

    cb.flow = g_hal_state_pd->flow_table();
    HAL_ASSERT_RETURN((cb.flow != NULL), HAL_RET_ERR);
    cb.msg = msg;

    cb.flow->iterate(pd_table_flow_entry, &cb);

    return ret;
}

hal_ret_t
pd_table_get (pd_table_get_args_t *args)
{
    hal_ret_t ret = HAL_RET_OK;
    TableSpec *spec = args->spec;
    TableResponse *rsp = args->rsp;
    TableKind kind = table::TABLE_NONE;

    auto key = spec->key();
    if (key.id_or_name_case() == table::TableIdName::kTableId) {
        ret = pd_table_kind_get_from_id(key.table_id(), &kind);
        if (ret == HAL_RET_INVALID_ARG) {
            HAL_TRACE_ERR("Failed to find table with id: {}", key.table_id());
            return HAL_RET_INVALID_ARG;
        }
        switch (kind) {
        case table::TABLE_INDEX:
            ret = pd_table_index_get_entries(key.table_id(), rsp);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Failed to get entries of table id: {}",
                              key.table_id());
            }
            break;
        case table::TABLE_TCAM:
            ret = pd_table_tcam_get_entries(key.table_id(), rsp);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Failed to get entries of table id: {}",
                              key.table_id());
            }
            break;
        case table::TABLE_HASH:
            ret = pd_table_hash_get_entries(key.table_id(), rsp);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Failed to get entries of table id: {}",
                              key.table_id());
            }
            break;
        case table::TABLE_FLOW:
            ret = pd_table_flow_get_entries(key.table_id(), rsp);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Failed to get entries of table id: {}",
                              key.table_id());
            }
            break;
        case table::TABLE_MET:
            break;
        default:
            // do nothing
            HAL_TRACE_ERR("Invalid table type: {}", kind);
        }
    } else {
        // TODO: Get table kind from name
    }

    return ret;
}

}    // namespace pd
}    // namespace hal
