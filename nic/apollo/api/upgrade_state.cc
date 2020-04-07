//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file implements upgrade states handling
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/mem.hpp"
#include "nic/sdk/lib/kvstore/kvstore.hpp"
#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/framework/api.h"
#include "nic/apollo/api/upgrade_state.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/nexthop_group.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/api/mapping.hpp"

namespace api {

upg_state *g_upg_state;

#define PDS_UPG_SHM_NAME        "pds_upgrade"
#define PDS_UPG_SHM_PSTATE_NAME "pds_upgrade_pstate"
// TODO: below size depends on the size of the config hw states to be saved
// and other nicmgr/linkmgr states etc. need to calculate for the maximum
// and adjust its size.
#define PDS_UPG_SHM_SIZE (100 << 10)  // 100KB

sdk_ret_t
upg_state::init_(bool shm_create) {
    sdk::lib::shm_mode_e mode = shm_create ? sdk::lib::SHM_CREATE_ONLY : sdk::lib::SHM_OPEN_ONLY;
    const char *op = shm_create ? "create" : "open";

    try {
        // if create, delete and re-create as previous size and current size may be different
        if (shm_create) {
            shmmgr::remove(PDS_UPG_SHM_NAME);
        }
        shm_mmgr_ = shmmgr::factory(PDS_UPG_SHM_NAME, PDS_UPG_SHM_SIZE, mode, NULL);
        if (shm_mmgr_ == NULL) {
            PDS_TRACE_ERR("Upgrade shared mem %s failed", op);
            return SDK_RET_ERR;
        }
    } catch (...) {
        PDS_TRACE_ERR("Upgrade shared mem %s failed", op);
        return SDK_RET_ERR;
    }

    pstate_ = (upg_pstate_t *)shm_mmgr_->segment_alloc(PDS_UPG_SHM_PSTATE_NAME,
                                                      sizeof(upg_pstate_t), shm_create);
    if (!pstate_) {
        PDS_TRACE_ERR("Upgrade pstate %s failed", op);
        return SDK_RET_ERR;
    }

    PDS_TRACE_DEBUG("Upgrade shared mem %s done", op);
    return SDK_RET_OK;
}

void
upg_state::destroy(upg_state *state) {
    upg_ctxt::destroy(state->upg_ctx());
    SDK_FREE(api::PDS_MEM_ALLOC_UPG, state);
    shmmgr::remove(PDS_UPG_SHM_NAME);
}

upg_state *
upg_state::factory(bool shm_create) {
    sdk_ret_t ret;
    void *mem;
    upg_state *ustate;

    mem = SDK_CALLOC(api::PDS_MEM_ALLOC_UPG, sizeof(upg_state));
    if (!mem) {
        PDS_TRACE_ERR("Upgrade state alloc failed");
        return NULL;
    }
    ustate = new (mem) upg_state();
    ret = ustate->init_(shm_create);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Upgrade state init failed");
        goto err_exit;
    }

    // initialize the ugprade object context
    ustate->set_upg_ctx(upg_ctxt::factory());
    return ustate;

err_exit:

    SDK_FREE(api::PDS_MEM_ALLOC_UPG, ustate);
    ustate = NULL;
    return NULL;
}

uint32_t
upg_state::tbl_eng_cfg(p4pd_pipeline_t pipe, p4_tbl_eng_cfg_t **cfg, uint32_t *max_cfgs) {
    *cfg = &tbl_eng_cfgs_[pipe][0];
    *max_cfgs = P4TBL_ID_MAX;
    return tbl_eng_cfgs_count_[pipe];
}

void
upg_state::incr_tbl_eng_cfg_count(p4pd_pipeline_t pipe, uint32_t ncfgs) {
    tbl_eng_cfgs_count_[pipe] += ncfgs;
}

void
upg_state::set_qstate_cfg(uint64_t addr, uint32_t size, uint32_t pgm_off) {
    qstate_cfg_t q;
    q.addr = addr;
    q.size = size;
    q.pgm_off = pgm_off;
    qstate_cfgs_.push_back(q);
}

bool
upg_state::backup_stateful_obj_cb(void *obj, void *obj_ctxt) {
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

void
upg_state::backup_statless_obj_cb(void *key, void *val, void *obj_ctxt) {
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

    nh_group_ht = nexthop_group_db()->nh_group_ht();
    return (nh_group_ht->walk(upg_state::backup_stateful_obj_cb,
                                              (void *)&obj_id));
}

static inline
sdk_ret_t backup_mapping(void) {
    sdk::lib::kvstore *kvs;
    obj_id_t obj_id = OBJ_ID_MAPPING;

    kvs = api::g_pds_state.kvstore();
    return (kvs->iterate(upg_state::backup_statless_obj_cb,
                                         (void *)&obj_id));
}

sdk_ret_t
upg_state::backup(void) {
    sdk_ret_t ret;

    // get and initialize a segment from shread memory for write
    ret = upg_ctx()->init(true);
    if (ret != SDK_RET_OK) {
        return ret;
    }
    // set the backup status to true. will set to false if there is a failure
    set_status(true);

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

    if (status() == false) {
        PDS_TRACE_ERR("Backup failed");
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

}    // namespace api
