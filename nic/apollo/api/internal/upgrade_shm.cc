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
#include "nic/sdk/include/sdk/platform.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/api/internal/upgrade_shm.hpp"
#include "nic/apollo/api/upgrade_state.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {

#define PDS_API_UPG_SHM_DIR_NAME    "/update"
#define PDS_API_UPG_SHM_NAME        "pds_api_upgdata"
// TODO: below size depends on the size of the config hw states to be saved
// and other nicmgr/linkmgr states etc. need to calculate for the maximum
// and adjust its size.
#define PDS_API_UPG_SHM_SIZE        (2 * 1024 * 1024)  // 2MB

sdk_ret_t
upg_shm::init_(bool shm_create) {
    sdk::lib::shm_mode_e mode = shm_create ? sdk::lib::SHM_CREATE_ONLY :
                                             sdk::lib::SHM_OPEN_READ_ONLY;
    const char *op = shm_create ? "create" : "open";
    upg_mode_t upg_init_mode = api::g_upg_state->upg_init_mode();
    std::string fname = PDS_API_UPG_SHM_NAME;
    struct stat st = { 0 };

    // for hitless, open if shm_create is false (shared from A to B). create if
    // shm_create is true (new states on B). cannot use the same name as A, as A
    // need to reuse this if there is an upgrade failure
    if (sdk::platform::upgrade_mode_hitless(upg_init_mode)) {
        sdk::upg::upg_dom_t dom =  api::g_upg_state->upg_init_domain();

        if ((shm_create && sdk::upg::upg_domain_b(dom)) ||
            (!shm_create && sdk::upg::upg_domain_a(dom))) {
                fname = fname + "_dom_b";
        }
    }

    // on naples, create the direcotry if it not exist
    // sim/mock uses /dev/shm as there is no permission to create /update/
    if (api::g_pds_state.platform_type() == platform_type_t::PLATFORM_TYPE_HW) {
        std::string fpath = PDS_API_UPG_SHM_DIR_NAME;

        if (stat(fpath.c_str(), &st) == -1) {
            if (mkdir(fpath.c_str(), 0755) < 0) {
                SDK_TRACE_ERR("Directory %s/ doesn't exist, failed to create one\n",
                              fpath.c_str());
                return SDK_RET_ERR;
            }
        }
        fname = fpath + "/" + fname;
    }

    try {
        // if create, delete and re-create as previous size and current size may be different
        if (shm_create) {
            shmmgr::remove(fname.c_str());
        }
        shm_mmgr_ = shmmgr::factory(fname.c_str(), PDS_API_UPG_SHM_SIZE, mode, NULL);
        if (shm_mmgr_ == NULL) {
            PDS_TRACE_ERR("Upgrade shared mem %s failed for %s", op, fname.c_str());
            return SDK_RET_ERR;
        }
    } catch (...) {
        PDS_TRACE_ERR("Upgrade shared mem %s failed for %s", op, fname.c_str());
        return SDK_RET_ERR;
    }

    PDS_TRACE_DEBUG("Upgrade shared mem %s done for %s", op, fname.c_str());
    return SDK_RET_OK;
}

void
upg_shm::destroy(upg_shm *shm) {
    shm->shm_mgr()->destroy(shm->shm_mgr());
    SDK_FREE(api::PDS_MEM_ALLOC_UPG, shm);
}

// called during backup or bringup(for restoring)
// caller should invoke this only if it is not created already
upg_shm *
upg_shm::factory(bool create) {
    sdk_ret_t ret;
    void *mem;
    upg_shm *shm;

    mem = SDK_CALLOC(api::PDS_MEM_ALLOC_UPG, sizeof(upg_shm));
    if (!mem) {
        PDS_TRACE_ERR("Upgrade shm alloc failed");
        return NULL;
    }
    shm = new (mem) upg_shm();

    ret = shm->init_(create);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Upgrade state init failed, ret %u", ret);
        goto err_exit;
    }

    // initialize the ugprade object context
    shm->api_upg_ctx_ = upg_ctxt::factory();
    shm->nicmgr_upg_ctx_ = upg_ctxt::factory();

    if (!shm->api_upg_ctx_ || !shm->nicmgr_upg_ctx_) {
        PDS_TRACE_ERR("Upgrade shared mem %s, context allocation failed",
                      create ? "create" : "open");
        goto err_exit;
    }
    return shm;

err_exit:

    SDK_FREE(api::PDS_MEM_ALLOC_UPG, shm);
    return NULL;
}

}    // namespace api
