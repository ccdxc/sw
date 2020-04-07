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

#define PDS_UPGRADE_STORE_NAME "pds_upg_objs_info"
#define PDS_UPGRADE_STORE_META_SIZE (50 * 1024)
#define PDS_UPGRADE_STORE_OBJ_OFFSET (1024)

sdk_ret_t
upg_ctxt::init(bool create) {
    shmmgr *shm_mmgr_;

    SDK_ASSERT(api::g_upg_state->shm_mgr() != NULL);
    shm_mmgr_ = api::g_upg_state->shm_mgr();

    try {
        mem_ = (char *)shm_mmgr_->segment_alloc(PDS_UPGRADE_STORE_NAME,
                                                PDS_UPGRADE_STORE_META_SIZE,
                                                create);
        if (!mem_) {
            PDS_TRACE_ERR("Failed to init shared memory segment for:%s",
                                 create == true ? "backup" : "restore");
            return SDK_RET_OOM;
        }
    } catch (...) {
        PDS_TRACE_ERR("Failed to init shared memory segment for:%s",
                             create == true ? "backup" : "restore");
        return SDK_RET_OOM;
    }
    obj_size_   = PDS_UPGRADE_STORE_META_SIZE - PDS_UPGRADE_STORE_OBJ_OFFSET;
    obj_offset_ = PDS_UPGRADE_STORE_OBJ_OFFSET;
    if (create) {
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
    SDK_FREE(api::PDS_MEM_ALLOC_ID_UPG_CTXT, uctxt);
}

} // namespace api
