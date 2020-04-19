//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file deals with upgrade context methods used for backup and restore
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/core.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/upgrade_state.hpp"
#include "nic/apollo/api/internal/upg_ctxt.hpp"

namespace api {

// this is for the header information pointing to each object types
#define PDS_UPGRADE_STORE_OBJ_OFFSET (16 * sizeof(upg_obj_stash_meta_t))

sdk_ret_t
upg_ctxt::init(const char *obj_store_name, size_t obj_store_size,
               bool obj_store_create) {
    shmmgr *shm_mmgr_;

    SDK_ASSERT(api::g_upg_state->shm_mgr() != NULL);
    shm_mmgr_ = api::g_upg_state->shm_mgr();

    try {
        mem_ = (char *)shm_mmgr_->segment_find(obj_store_name, obj_store_create,
                                               obj_store_create ? obj_store_size: 0);
        if (!mem_) {
            PDS_TRACE_ERR("Failed to init shared memory segment for:%s",
                          obj_store_create == true ? "backup" : "restore");
            return SDK_RET_OOM;
        }
    } catch (...) {
        PDS_TRACE_ERR("Failed to init shared memory segment for:%s",
                      obj_store_create == true ? "backup" : "restore");
        return SDK_RET_OOM;
    }
    obj_size_   = obj_store_size - PDS_UPGRADE_STORE_OBJ_OFFSET;
    obj_offset_ = PDS_UPGRADE_STORE_OBJ_OFFSET;
    if (obj_store_create) {
        memset(mem_, 0, PDS_UPGRADE_STORE_OBJ_OFFSET);
    }
    return SDK_RET_OK;
}

upg_ctxt *
upg_ctxt::factory(void) {
    upg_ctxt *instance;
    void *mem;

    mem = SDK_CALLOC(PDS_MEM_ALLOC_ID_UPG_CTXT, sizeof(upg_ctxt));
    if (!mem) {
        PDS_TRACE_ERR("Failed to create upg_ctxt instance");
        return NULL;
    }
    instance = new (mem) upg_ctxt();
    if (!instance) {
        PDS_TRACE_ERR("Failed to create upg_ctxt instance");
        return NULL;
    }
    return instance;
}

void
upg_ctxt::destroy(upg_ctxt *uctxt) {
    if (uctxt) {
        SDK_FREE(api::PDS_MEM_ALLOC_ID_UPG_CTXT, uctxt);
    }
}

} // namespace api
