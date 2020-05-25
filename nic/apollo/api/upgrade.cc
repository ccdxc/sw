//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_upgrade.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/upgrade_state.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/core.hpp"
#include "nic/apollo/api/port.hpp"
#include "nic/apollo/nicmgr/nicmgr.hpp"

namespace api {

sdk_ret_t
upg_init (pds_init_params_t *params)
{
    sdk_ret_t ret;
    upg_shm *shm;
    upg_mode_t mode;
    sdk::upg::upg_dom_t dom;

    mode = sdk::upg::upg_init_mode();
    dom = sdk::upg::upg_init_domain();

    PDS_TRACE_DEBUG("Setting bootup upgrade mode to %s",
                    mode == upg_mode_t::UPGRADE_MODE_NONE ? "NONE" :
                    mode == upg_mode_t::UPGRADE_MODE_GRACEFUL ? "Graceful" :
                    "Hitless");

    // initialize upgrade state and call the upgade compatibitly checks
    if ((g_upg_state = upg_state::factory(params)) == NULL) {
        PDS_TRACE_ERR("Upgrade state creation failed");
        return SDK_RET_ERR;
    }

    // create a read-only shared memory instance for upgrade boot
    if (!sdk::platform::upgrade_mode_none(mode)) {
        if ((shm = upg_shm::factory(false)) == NULL) {
            PDS_TRACE_ERR("Upgrade shared memory instance creation failed");
            return SDK_RET_ERR;
        }
        g_upg_state->set_restore_shm(shm);
    }

    // save upgrade init mode and domain
    g_upg_state->set_upg_init_mode(mode);
    g_upg_state->set_upg_init_domain(dom);

    // offset the memory regions based on the regions in use
    if (sdk::platform::upgrade_mode_hitless(mode)) {
        ret = g_pds_state.mempartition()->upgrade_hitless_offset_regions(
            api::g_pds_state.cfg_path().c_str(), false);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Upgrade hitless memory offset failed");
            return ret;
        }
    }

    // pds hal registers for upgrade events
    ret = upg_graceful_init(params);
    if (ret == SDK_RET_OK) {
        ret = upg_hitless_init(params);
    }
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Upgrade graceful/hitless init failed");
        return ret;
    }

    // nicmgr registers for upgrade events
    ret = nicmgr_upg_graceful_init();
    if (ret == SDK_RET_OK) {
        ret = nicmgr_upg_hitless_init();
    }
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Upgrade nicmgr graceful/hitless init failed");
        return ret;
    }

    return SDK_RET_OK;
}

}   // namespace api
