//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/asic/pd/scheduler.hpp"
#include "nic/apollo/api/include/pds_upgrade.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/upgrade_state.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/core.hpp"
#include "nic/apollo/api/port.hpp"
#include "nic/apollo/api/impl/lif_impl.hpp"

namespace api {

static bool
backup_stateful_obj_cb(void *obj, void *obj_ctxt) {
    sdk_ret_t ret;
    string keystr;
    uint32_t obj_id = *(uint32_t *)obj_ctxt;

    if (!obj) {
        return false;
    }

    switch (obj_id) {
    case OBJ_ID_NEXTHOP_GROUP:
        keystr = ((nexthop_group *)obj)->key2str();
        ret = ((nexthop_group *)obj)->backup();
        break;

    default:
        SDK_ASSERT(0);
    }

    if (ret != SDK_RET_OK) {
        api::g_upg_state->set_status(false);
        PDS_TRACE_ERR("Backup stateful obj id:%u failed for key:%s, rcode:%u",
                                                         obj_id, keystr, ret);
        return true; // stop the walk
    }
    return false;
}

static void
backup_statless_obj_cb(void *key, void *val, void *obj_ctxt) {
    sdk_ret_t ret;
    uint32_t obj_id = *(uint32_t *)obj_ctxt;
    pds_obj_key_t *pkey = (pds_obj_key_t *)key;

    if (!key || !val) {
        return;
    }

    switch (obj_id) {
    case OBJ_ID_MAPPING:
        mapping_entry *entry;
        entry = mapping_entry::build(pkey);
        ret = entry->backup();
        mapping_entry::soft_delete(entry);
        break;

    default:
        SDK_ASSERT(0);
    }

    if (ret != SDK_RET_OK) {
        api::g_upg_state->set_status(false);
        PDS_TRACE_ERR("Backup stateless obj id:%u failed for key:%s, rcode:%u",
                                                     obj_id, pkey->str(), ret);
    }
    return;
}

static inline
sdk_ret_t backup_nexthop_group(void) {
    ht *nh_group_ht;
    obj_id_t obj_id = OBJ_ID_NEXTHOP_GROUP;

    SDK_ASSERT(g_upg_state->api_upg_ctx() != NULL);
    //fill up meta for nh group
    upg_obj_stash_meta_t *hdr =
            (upg_obj_stash_meta_t *)g_upg_state->api_upg_ctx()->mem();
    hdr[obj_id].obj_id = obj_id;
    hdr[obj_id].offset = g_upg_state->api_upg_ctx()->obj_offset();
    nh_group_ht = nexthop_group_db()->nh_group_ht();
    hdr[obj_id].obj_count = nh_group_ht->num_entries();
    return (nh_group_ht->walk(backup_stateful_obj_cb, (void *)&obj_id));
}

static inline
sdk_ret_t backup_mapping(void) {
    sdk::lib::kvstore *kvs;
    obj_id_t obj_id = OBJ_ID_MAPPING;

    // todo fix kvstore iterate, check PR 21927
    kvs = api::g_pds_state.kvstore();
    return (kvs->iterate(backup_statless_obj_cb, (void *)&obj_id, "mapping"));
}

static sdk_ret_t
upg_ev_compat_check (upg_ev_params_t *params)
{
    return SDK_RET_OK;
}

static sdk_ret_t
upg_ev_start (upg_ev_params_t *params)
{
    return SDK_RET_OK;
}

static sdk_ret_t
upg_ev_backup (upg_ev_params_t *params)
{
    sdk_ret_t ret;

    // get and initialize a segment from shread memory for write
    ret = g_upg_state->api_upg_ctx()->init(PDS_UPGRADE_API_OBJ_STORE_NAME,
                                           PDS_UPGRADE_API_OBJ_STORE_SIZE, true);
    if (ret != SDK_RET_OK) {
        return ret;
    }
    // set the backup status to true. will set to false if there is a failure
    g_upg_state->set_status(true);

    for (uint32_t id = (uint32_t )OBJ_ID_NONE + 1; id < OBJ_ID_MAX - 1; id++) {
        switch (id) {
        case OBJ_ID_NEXTHOP_GROUP:
            ret = backup_nexthop_group();
            break;

        case OBJ_ID_MAPPING:
            ret = backup_mapping();
            break;

        default:
            break;
        }
    }   //end for

    if (g_upg_state->status() == false) {
        PDS_TRACE_ERR("Backup failed");
        return SDK_RET_ERR;
    }

    ret = impl_base::pipeline_impl()->upgrade_backup();
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Upgrade pipeline backup failed, err %u", ret);
    }
    return ret;
}

static sdk_ret_t
upg_ev_ready (upg_ev_params_t *params)
{
    return SDK_RET_OK;
}

static sdk_ret_t
upg_ev_quiesce (upg_ev_params_t *params)
{
    return SDK_RET_OK;
}

static sdk_ret_t
upg_ev_switchover (upg_ev_params_t *params)
{
    sdk_ret_t ret = impl_base::pipeline_impl()->upgrade_switchover();
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Upgrade pipeline switchover failed, err %u", ret);
    }
    return ret;
}

static sdk_ret_t
upg_ev_repeal (upg_ev_params_t *params)
{
    return SDK_RET_OK;
}

sdk_ret_t
upg_hitless_init (pds_init_params_t *params)
{
    upg_ev_hitless_t ev_hdlr;

    // fill upgrade events for graceful
    memset(&ev_hdlr, 0, sizeof(ev_hdlr));
    // thread name is used for just identification here
    strncpy(ev_hdlr.thread_name, "hal", sizeof(ev_hdlr.thread_name));
    ev_hdlr.compat_check_hdlr = upg_ev_compat_check;
    ev_hdlr.start_hdlr = upg_ev_start;
    ev_hdlr.backup_hdlr = upg_ev_backup;
    ev_hdlr.ready_hdlr = upg_ev_ready;
    ev_hdlr.quiesce_hdlr = upg_ev_quiesce;
    ev_hdlr.switchover_hdlr = upg_ev_switchover;
    ev_hdlr.repeal_hdlr = upg_ev_repeal;

    // register for upgrade events
    upg_ev_thread_hdlr_register(ev_hdlr);

    return SDK_RET_OK;
}

}    // namespace api
