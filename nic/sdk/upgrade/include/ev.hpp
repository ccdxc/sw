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

#include "include/sdk/globals.hpp"
#include "upgrade/include/upgrade.hpp"

namespace sdk {
namespace upg {

/// \brief upgrade event ids
/// over the releases, should not change the order of it
/// as it breaks the backward compatibility
typedef enum upg_ev_id_s {
    UPG_EV_NONE         = 0,
    UPG_EV_COMPAT_CHECK = SDK_IPC_EVENT_ID_UPGMGR_MIN,
    UPG_EV_START,
    UPG_EV_BACKUP,
    UPG_EV_PREPARE,
    UPG_EV_PREP_SWITCHOVER,
    UPG_EV_SWITCHOVER,
    UPG_EV_ROLLBACK,
    UPG_EV_READY,
    UPG_EV_REPEAL,
    UPG_EV_FINISH,
    UPG_EV_EXIT,
    UPG_EV_MAX = SDK_IPC_EVENT_ID_UPGMGR_MAX,
} upg_ev_id_t;

/// \brief asynchronous response callback by the event handler
/// cookie is passed to the event handler and it should not be modified
typedef void (*upg_async_ev_response_cb)(sdk_ret_t status, const void *cookie);

static inline const char *
upg_event2str (upg_ev_id_t id)
{
    return ("todo");
}

static inline upg_ev_id_t
upg_stage2event (upg_stage_t stage)
{
    switch(stage) {
    case UPG_STAGE_COMPAT_CHECK:
        return UPG_EV_COMPAT_CHECK;
    case UPG_STAGE_START:
        return UPG_EV_START;
    case UPG_STAGE_BACKUP:
        return UPG_EV_BACKUP;
    case UPG_STAGE_PREPARE:
        return UPG_EV_PREPARE;
    case UPG_STAGE_PREP_SWITCHOVER:
        return UPG_EV_PREP_SWITCHOVER;
    default:
        // should handle all the stages
        SDK_ASSERT(0);
    }
}

/// \brief upgrade event msg
typedef struct upg_ev_params_s {
    upg_ev_id_t id;                       ///< upgrade event id
    sdk::platform::upg_mode_t mode;       ///< upgrade mode
    upg_async_ev_response_cb response_cb; ///< response callback
    void *response_cookie;                ///< response cookie
    // TODO other infos
} __PACK__ upg_ev_params_t;

/// \brief upgrade event handler
typedef sdk_ret_t (*upg_ev_hdlr_t)(upg_ev_params_t *params);

/// \brief upgrade event handlers for upgrade
typedef struct upg_ev_s {

    // service name
    char svc_name[64];

    // service id
    uint32_t svc_id;

    /// compat checks should be done here (on A)
    upg_ev_hdlr_t compat_check;

    /// start of a new upgrade, mount check the existance of B should be
    /// done here (on A).
    upg_ev_hdlr_t start;

    /// software states should be saved here (on A).
    upg_ev_hdlr_t backup;

    /// new process spawning should be done here
    /// processs should be paused here to a safe point for switchover (on A)
    upg_ev_hdlr_t prepare;

    /// hardware quiescing should be done here (on A)
    upg_ev_hdlr_t prepare_switchover;

    /// switching to B (on B)
    upg_ev_hdlr_t switchover;

    /// rollback if there is failure (on B)
    upg_ev_hdlr_t rollback;

    /// making sure B bringup is successful (on B)
    upg_ev_hdlr_t ready;

    /// repeal an upgrade (on A / B)
    /// an repeal (on A)
    ///   after backup, requires the process to cleanup its saved states
    ///   after quiesce, and ready for re-spawn
    /// an repeal (on B)
    ///   needs to restore old state (rollback), and ready to re-spawn previous
    ///   version. this won't come if there a rollback request already
    upg_ev_hdlr_t repeal;

    /// finish the upgrade (on A / B)
    /// processs can do final cleanup
    upg_ev_hdlr_t finish;

    /// completed the upgrade (on A / B)
    /// processs should shutdown by receiving this event
    upg_ev_hdlr_t exit;

} upg_ev_t;

void upg_ev_hdlr_register(upg_ev_t &ev);

}   // namespace upg
}   // namespace sdk

using sdk::upg::upg_ev_id_t::UPG_EV_NONE;
using sdk::upg::upg_ev_id_t::UPG_EV_COMPAT_CHECK;
using sdk::upg::upg_ev_id_t::UPG_EV_START;
using sdk::upg::upg_ev_id_t::UPG_EV_BACKUP;
using sdk::upg::upg_ev_id_t::UPG_EV_PREPARE;
using sdk::upg::upg_ev_id_t::UPG_EV_PREP_SWITCHOVER;
using sdk::upg::upg_ev_id_t::UPG_EV_SWITCHOVER;

/// @}

#endif   // __UPGRADE_EV_HPP__
