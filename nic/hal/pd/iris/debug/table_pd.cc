// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "gen/proto/table.pb.h"
#include "nic/hal/pd/iris/debug/table_pd.hpp"
#include "nic/hal/svc/table_svc.hpp"
#include "nic/sdk/lib/table/sldirectmap/sldirectmap.hpp"

using table::TableMetadataResponseMsg;

namespace hal {
namespace pd {

hal_ret_t
pd_table_metadata_get (pd_func_args_t *pd_func_args)
{
    hal_ret_t ret = HAL_RET_OK;
    pd_table_metadata_get_args_t *args = pd_func_args->pd_table_metadata_get;
    directmap *dm = NULL;
    sldirectmap *sldm = NULL;
    tcam *tm = NULL;
    sdk::table::hash *h = NULL;
    sdk::table::sdk_table_api_stats_t api_stats = { 0 };
    Met *met = NULL;
    TableMetadataResponseMsg *rsp_msg = args->rsp;

    for (int i = P4TBL_ID_INDEX_MIN; i <= P4TBL_ID_INDEX_MAX; i++) {
        // SLDirectMap
        if (i == P4TBL_ID_SESSION_STATE || i == P4TBL_ID_FLOW_INFO ||
            i == P4TBL_ID_FLOW_STATS) {

            sldm = (sldirectmap *) g_hal_state_pd->dm_table(i);
            if (sldm == NULL) {
                continue;
            }
            auto table_meta = rsp_msg->add_table_meta();
            table_meta->set_table_id(i);
            table_meta->set_kind(table::TABLE_INDEX);
            table_meta->set_table_name(sldm->name());

            auto dm_meta = table_meta->mutable_index_meta();

            sldm->stats_get(&api_stats);

            dm_meta->set_capacity(sldm->capacity());
            dm_meta->set_usage(sldm->inuse());
            dm_meta->set_num_inserts(api_stats.insert);
            dm_meta->set_num_insert_failures(api_stats.insert_fail);
            dm_meta->set_num_updates(api_stats.update);
            dm_meta->set_num_update_failures(api_stats.update_fail);
            dm_meta->set_num_deletes(api_stats.remove);
            dm_meta->set_num_delete_failures(api_stats.remove_fail);
        } else {
            //DirectMap
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
    }

    // DirectMap - RXDMA
    for (int i = P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN; i <= P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MAX; i++) {
        dm = g_hal_state_pd->p4plus_rxdma_dm_table(i);
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

    // DirectMap - TXDMA
    for (int i = P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN; i <= P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MAX; i++) {
        dm = g_hal_state_pd->p4plus_txdma_dm_table(i);
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
    for (int i = P4TBL_ID_TCAM_MIN; i <= P4TBL_ID_TCAM_MAX; i++) {
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
    for (int i = P4TBL_ID_HASH_OTCAM_MIN; i <= P4TBL_ID_HASH_OTCAM_MAX; i++) {
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
    g_hal_state_pd->flow_table_pd_get()->meta_get(rsp_msg);

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

    if (table_id >= P4TBL_ID_INDEX_MIN && table_id <= P4TBL_ID_INDEX_MAX) {
        *kind = table::TABLE_INDEX;
        return ret;
    }

    if (table_id >= P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN && table_id <= P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MAX) {
        *kind = table::TABLE_INDEX;
        return ret;
    }

    if (table_id >= P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN && table_id <= P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MAX) {
        *kind = table::TABLE_INDEX;
        return ret;
    }

    if (table_id >= P4TBL_ID_TCAM_MIN && table_id <= P4TBL_ID_TCAM_MAX) {
        *kind = table::TABLE_TCAM;
        return ret;
    }

    if (table_id >= P4TBL_ID_HASH_OTCAM_MIN && table_id <= P4TBL_ID_HASH_OTCAM_MAX) {
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

void pd_table_sldirectmap_entry(sdk::table::sdk_table_api_params_t *params)
{
    char buff[4096] = {0};
    sldirectmap_entry_cb_t *cb = (sldirectmap_entry_cb_t *)params->cbdata;
    sldirectmap *sldm = cb->sldm;
    TableIndexMsg *msg = cb->msg;
    TableIndexEntry *entry = msg->add_index_entry();

    sldm->entry_to_str(params->actiondata, params->handle.pindex(), buff, sizeof(buff));

    HAL_TRACE_DEBUG("Entry: {}", buff);

    entry->set_index(params->handle.pindex());
    entry->set_entry(buff);
}

hal_ret_t
pd_table_index_get_entries (uint32_t table_id, TableResponse *rsp, bool get_all)
{
    hal_ret_t   ret = HAL_RET_OK;
    TableIndexMsg *msg = rsp->mutable_index_table();

    HAL_TRACE_DEBUG("Entry: {}", table_id);

    //SLDirectmap tables
    if (table_id == P4TBL_ID_SESSION_STATE || table_id == P4TBL_ID_FLOW_INFO ||
        table_id == P4TBL_ID_FLOW_STATS) {
        sldirectmap_entry_cb_t sldm_cb = {0};
        sdk::table::sdk_table_api_params_t params = {0};

        sldm_cb.sldm  = (sldirectmap *) g_hal_state_pd->dm_table(table_id);

        if ((sldm_cb.sldm == NULL) && (get_all)) {
            //for get all table case, return success if a table is not initialized.
            return ret;
        }
        TABLE_TEST_TRACE_RSP_RETURN((sldm_cb.sldm != NULL), rsp,
                                    HAL_RET_ENTRY_NOT_FOUND,
                                    "Failed to find the table id: {}",
                                    table_id);

        sldm_cb.msg   = msg;

        params.cbdata = &sldm_cb;
        params.itercb = pd_table_sldirectmap_entry;

        sldm_cb.sldm->iterate(&params);

        HAL_TRACE_DEBUG("Exit: {}", table_id);

        return ret;
    }

    directmap_entry_cb_t cb = {0};

    //Directmap tables
    if (table_id >= P4TBL_ID_INDEX_MIN && table_id <= P4TBL_ID_INDEX_MAX) {
        cb.dm = g_hal_state_pd->dm_table(table_id);
    } else if (table_id >= P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN && table_id <= P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MAX) {
        cb.dm = g_hal_state_pd->p4plus_rxdma_dm_table(table_id);
        HAL_TRACE_DEBUG("Rxdma: {}", table_id);
    } else if (table_id >= P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN && table_id <= P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MAX) {
        cb.dm = g_hal_state_pd->p4plus_txdma_dm_table(table_id);
    }

    if ((cb.dm == NULL) && (get_all)) {
        //for get all table case, return success if a table is not initialized.
        return ret;
    }
    TABLE_TEST_TRACE_RSP_RETURN((cb.dm != NULL), rsp,
                                HAL_RET_ENTRY_NOT_FOUND,
                                "Failed to find the table id: {}",
                                table_id);
    cb.msg = msg;

    cb.dm->iterate(pd_table_directmap_entry, &cb);

    HAL_TRACE_DEBUG("Exit: {}", table_id);

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
pd_table_tcam_get_entries (uint32_t table_id, TableResponse *rsp, bool get_all)
{
    hal_ret_t   ret = HAL_RET_OK;
    tcam_entry_cb_t cb = {0};
    TableTcamMsg *msg = rsp->mutable_tcam_table();

    cb.tcam_table = g_hal_state_pd->tcam_table(table_id);

    if ((cb.tcam_table == NULL) && (get_all)) {
        //for get all table case, return success if a table is not initialized.
        return ret;
    }
    TABLE_TEST_TRACE_RSP_RETURN((cb.tcam_table != NULL), rsp,
                                HAL_RET_ENTRY_NOT_FOUND,
                                "Failed to find the tcam table id: {}",
                                table_id);
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
pd_table_hash_get_entries (uint32_t table_id, TableResponse *rsp, bool get_all)
{
    hal_ret_t   ret = HAL_RET_OK;
    pd_hash_entry_cb_t cb = {0};
    TableHashMsg *msg = rsp->mutable_hash_table();

    cb.hash_table = g_hal_state_pd->hash_tcam_table(table_id);

    if ((cb.hash_table == NULL) && (get_all)) {
        //for get all table case, return success if a table is not initialized.
        return ret;
    }
    TABLE_TEST_TRACE_RSP_RETURN((cb.hash_table != NULL), rsp,
                              HAL_RET_ENTRY_NOT_FOUND,
                              "Failed to find the hash table id: {}",
                              table_id);
    cb.msg = msg;

    cb.hash_table->iterate(pd_table_hash_entry, &cb, sdk_hash::BOTH);

    return ret;
}

// Met
bool
pd_table_met_entry(uint32_t repl_list_idx,
                   Met *met, const void *cb_data)
{
    char buff[8192];
    pd_met_entry_cb_t *cb = (pd_met_entry_cb_t*)cb_data;
    TableMetMsg *msg = cb->msg;
    TableMetEntry *entry = msg->add_met_entry();


    met->repl_list_to_str(repl_list_idx, buff,
                          sizeof(buff));

    HAL_TRACE_DEBUG("Entry: {}", buff);

    entry->set_index(repl_list_idx);
    entry->set_entry(buff);

    return true;
}


hal_ret_t
pd_table_met_get_entries (uint32_t table_id, TableResponse *rsp, bool get_all)
{
    hal_ret_t   ret = HAL_RET_OK;
    pd_met_entry_cb_t cb = {0};
    TableMetMsg *msg = rsp->mutable_met_table();
    Met *met = NULL;

    met = g_hal_state_pd->met_table();

    //for get all, skip the table if it is not initialized
    if ((met == NULL) && (get_all))
        return ret;

    SDK_ASSERT_RETURN((met != NULL), HAL_RET_ERR);
    cb.msg = msg;

    met->iterate(pd_table_met_entry, &cb);

    return ret;
}

hal_ret_t
pd_table_get_entries(uint32_t table_id, TableResponse *rsp, bool get_all)
{
    hal_ret_t ret = HAL_RET_OK;
    TableKind kind = table::TABLE_NONE;

    ret = pd_table_kind_get_from_id(table_id, &kind);
    TABLE_TEST_TRACE_RSP_RETURN((ret == HAL_RET_OK), rsp,
            ret, "Failed to find table with id: {}",
            table_id);

    HAL_TRACE_VERBOSE("Retrieving Table entries for table id {}", table_id);

    switch (kind) {
        case table::TABLE_INDEX:
            ret = pd_table_index_get_entries(table_id, rsp, get_all);
            break;
        case table::TABLE_TCAM:
            ret = pd_table_tcam_get_entries(table_id, rsp, get_all);
            break;
        case table::TABLE_HASH:
            ret = pd_table_hash_get_entries(table_id, rsp, get_all);
            break;
        case table::TABLE_FLOW:
            // TODO multi-thread support for FTL iterate
            // ret = g_hal_state_pd->flow_table_pd_get()->dump(rsp);
            break;
        case table::TABLE_MET:
            ret = pd_table_met_get_entries(table_id, rsp, get_all);
            break;
        default:
            // do nothing
            HAL_TRACE_ERR("Invalid table type: {}", kind);
    }
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get entries of table id: {}", table_id);
    } else {
        // Set the api status to OK. If failure is seen in the guts,
        // then respective function would have set the correct api status.
        rsp->set_api_status(types::API_STATUS_OK);
    }
    return ret;
}

hal_ret_t
pd_table_get (pd_func_args_t *pd_func_args)
{
    uint32_t  table_id;
    hal_ret_t ret = HAL_RET_OK;
    pd_table_get_args_t *args = pd_func_args->pd_table_get;
    TableSpec *spec = args->spec;
    TableResponseMsg *rsp = args->rsp;
    TableResponse    *response;

    auto key = spec->key();

    if (!spec->has_key()) {
        HAL_TRACE_VERBOSE("Retrieving Table entries for ALL tables");

        for (table_id = P4TBL_ID_INDEX_MIN;
             table_id <= P4TBL_ID_INDEX_MAX; table_id++) {
            response = rsp->add_response();
            ret = pd_table_get_entries(table_id, response, true);

            if (ret != HAL_RET_OK) goto end;
        }

        for (table_id = P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN;
             table_id <= P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MAX; table_id++) {
            response = rsp->add_response();
            ret = pd_table_get_entries(table_id, response, true);

            if (ret != HAL_RET_OK) goto end;
        }

        for (table_id = P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN;
             table_id <= P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MAX; table_id++) {
            response = rsp->add_response();
            ret = pd_table_get_entries(table_id, response, true);

            if (ret != HAL_RET_OK) goto end;
        }

        for (table_id = P4TBL_ID_TCAM_MIN;
             table_id <= P4TBL_ID_TCAM_MAX; table_id++) {
            response = rsp->add_response();
            ret = pd_table_get_entries(table_id, response, true);

            if (ret != HAL_RET_OK) goto end;
        }

        for (table_id = P4TBL_ID_HASH_OTCAM_MIN;
             table_id <= P4TBL_ID_HASH_OTCAM_MAX; table_id++) {
            response = rsp->add_response();
            ret = pd_table_get_entries(table_id, response, true);

            if (ret != HAL_RET_OK) goto end;
        }
        response = rsp->add_response();
        ret = pd_table_get_entries(P4TBL_ID_FLOW_HASH, response, true);
        if (ret != HAL_RET_OK) goto end;

        response = rsp->add_response();
        ret = pd_table_get_entries(P4_REPL_TABLE_ID, response, true);
        if (ret != HAL_RET_OK) goto end;

    } else if (key.id_or_name_case() == table::TableIdName::kTableId) {

        HAL_TRACE_VERBOSE("Retrieving Table entries for specific table");
        response = rsp->add_response();

        ret = pd_table_get_entries(key.table_id(), response, false);
    }

end:
    return ret;
}

}    // namespace pd
}    // namespace hal
