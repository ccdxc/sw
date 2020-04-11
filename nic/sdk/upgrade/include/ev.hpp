//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines event handlers during process upgrade
///
//----------------------------------------------------------------------------

/// \defgroup UPGRADE Upgrade event handlers
/// \ingroup  UPG_EVENT_HDLR
/// @{
///

#ifndef __UPGRADE_EV_HPP__
#define __UPGRADE_EV_HPP__

#include "string.h"
#include "include/sdk/globals.hpp"
#include "upgrade/include/upgrade.hpp"

namespace sdk {
namespace upg {

// event-ids offset from sdk upgmgr reserved ids
#define EV_ID_UPGMGR(off) (SDK_IPC_EVENT_ID_UPGMGR_MIN + off)
/// \brief upgrade event ids
/// ...WARNING...over the releases, should not change the order of it
/// as it breaks the backward compatibility. it should be extended
/// in the end
#define UPG_EV_ENTRIES(E)                                                   \
    E(UPG_EV_NONE,            EV_ID_UPGMGR(UPG_STAGE_NONE),            "")  \
    E(UPG_EV_COMPAT_CHECK,    EV_ID_UPGMGR(UPG_STAGE_COMPAT_CHECK),    "")  \
    E(UPG_EV_START,           EV_ID_UPGMGR(UPG_STAGE_START),           "")  \
    E(UPG_EV_BACKUP,          EV_ID_UPGMGR(UPG_STAGE_BACKUP),          "")  \
    E(UPG_EV_PREPARE,         EV_ID_UPGMGR(UPG_STAGE_PREPARE),         "")  \
    E(UPG_EV_SYNC,            EV_ID_UPGMGR(UPG_STAGE_SYNC),            "")  \
    E(UPG_EV_PREP_SWITCHOVER, EV_ID_UPGMGR(UPG_STAGE_PREP_SWITCHOVER), "")  \
    E(UPG_EV_SWITCHOVER,      EV_ID_UPGMGR(UPG_STAGE_SWITCHOVER),      "")  \
    E(UPG_EV_READY,           EV_ID_UPGMGR(UPG_STAGE_READY),           "")  \
    E(UPG_EV_RESPAWN,         EV_ID_UPGMGR(UPG_STAGE_RESPAWN),         "")  \
    E(UPG_EV_ROLLBACK,        EV_ID_UPGMGR(UPG_STAGE_ROLLBACK),        "")  \
    E(UPG_EV_REPEAL,          EV_ID_UPGMGR(UPG_STAGE_REPEAL),          "")  \
    E(UPG_EV_FINISH,          EV_ID_UPGMGR(UPG_STAGE_FINISH),          "")  \
    E(UPG_EV_MAX,             (SDK_IPC_EVENT_ID_UPGMGR_MAX),           "")

SDK_DEFINE_ENUM(upg_ev_id_t, UPG_EV_ENTRIES)
SDK_DEFINE_ENUM_TO_STR(upg_ev_id_t, UPG_EV_ENTRIES)
#undef UPG_EV_ENTRIES

/// \brief asynchronous response callback by the event handler
/// cookie is passed to the event handler and it should not be modified
typedef void (*upg_async_ev_response_cb_t)(sdk_ret_t status, const void *cookie);

static inline const char *
upg_event2str (upg_ev_id_t id)
{
    return UPG_EV_ENTRIES_str(id);
}

static inline upg_ev_id_t
upg_stage2event (upg_stage_t stage)
{
    if (stage >= UPG_STAGE_MAX) {
        SDK_ASSERT(0);
    }
    return (upg_ev_id_t)EV_ID_UPGMGR(stage);
}

// environment variable setup by the process loader during bringup
// if it not set, regular boot is assumed
static inline upg_mode_t
upg_init_mode(void)
{
    const char *m = getenv("UPG_INIT_MODE");
    upg_mode_t mode;

    if (!m) {
       return upg_mode_t::UPGRADE_MODE_NONE;
    }
    if (strncmp(m, "graceful", strlen("graceful")) == 0) {
        mode = upg_mode_t::UPGRADE_MODE_GRACEFUL;
    } else if (strncmp(m, "hitless", strlen("hitless")) == 0) {
        mode = upg_mode_t::UPGRADE_MODE_HITLESS;
    } else if (strncmp(m, "none", strlen("none")) == 0) {
        mode = upg_mode_t::UPGRADE_MODE_NONE;
    } else {
        SDK_ASSERT(0);
    }
    return mode;
}

/// \brief upgrade event msg
typedef struct upg_ev_params_s {
    upg_ev_id_t id;                         ///< upgrade event id
    sdk::platform::upg_mode_t mode;         ///< upgrade mode
    upg_async_ev_response_cb_t response_cb; ///< response callback
    void *response_cookie;                ///< response cookie
    // TODO other infos
} __PACK__ upg_ev_params_t;

/// \brief upgrade event handler
typedef sdk_ret_t (*upg_ev_hdlr_t)(upg_ev_params_t *params);

/// \brief upgrade event handlers for upgrade
typedef struct upg_ev_s {

    // service name. used for debug trace only
    char svc_name[SDK_MAX_NAME_LEN];

    // service ipc id
    uint32_t svc_ipc_id;

    /// compat checks should be done here (on A)
    upg_ev_hdlr_t compat_check_hdlr;

    /// start of a new upgrade, mount check the existance of B should be
    /// done here (on A).
    upg_ev_hdlr_t start_hdlr;

    /// software states should be saved here (on A).
    upg_ev_hdlr_t backup_hdlr;

    /// new process spawning should be done here
    /// processs should be paused here to a safe point for switchover (on A)
    upg_ev_hdlr_t prepare_hdlr;

    /// hardware quiescing should be done here (on A)
    upg_ev_hdlr_t prepare_switchover_hdlr;

    /// switching to B (on B)
    upg_ev_hdlr_t switchover_hdlr;

    /// rollback if there is failure (on B)
    upg_ev_hdlr_t rollback_hdlr;

    /// config replay, operational table syncing (on B)
    upg_ev_hdlr_t sync_hdlr;

    /// respawn processes (on B)
    upg_ev_hdlr_t respawn_hdlr;

    /// making sure B bringup is successful (on B)
    upg_ev_hdlr_t ready_hdlr;

    /// repeal an upgrade (on A / B)
    /// an repeal (on A)
    ///   after backup, requires the process to cleanup its saved states
    ///   after quiesce, and ready for re-spawn
    /// an repeal (on B)
    ///   needs to restore old state (rollback), and ready to re-spawn previous
    ///   version. this won't come if there a rollback request already
    upg_ev_hdlr_t repeal_hdlr;

    /// finish the upgrade (on A / B)
    /// processs can do final cleanup
    upg_ev_hdlr_t finish_hdlr;

} upg_ev_t;

void upg_ev_hdlr_register(upg_ev_t &ev);

#undef EV_ID_UPGMGR

}   // namespace upg
}   // namespace sdk

using sdk::upg::upg_ev_id_t::UPG_EV_NONE;
using sdk::upg::upg_ev_id_t::UPG_EV_COMPAT_CHECK;
using sdk::upg::upg_ev_id_t::UPG_EV_START;
using sdk::upg::upg_ev_id_t::UPG_EV_BACKUP;
using sdk::upg::upg_ev_id_t::UPG_EV_PREPARE;
using sdk::upg::upg_ev_id_t::UPG_EV_PREP_SWITCHOVER;
using sdk::upg::upg_ev_id_t::UPG_EV_SWITCHOVER;
using sdk::upg::upg_ev_id_t::UPG_EV_READY;
using sdk::upg::upg_ev_id_t::UPG_EV_ROLLBACK;
using sdk::upg::upg_ev_id_t::UPG_EV_REPEAL;
using sdk::upg::upg_ev_id_t::UPG_EV_FINISH;
using sdk::upg::upg_ev_id_t::UPG_EV_MAX;

/// @}

#endif   // __UPGRADE_EV_HPP__
