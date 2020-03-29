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

#include "nic/apollo/upgrade/include/upgrade.hpp"

namespace upg {

/// \brief upgrade event ids
typedef enum upg_ev_id_s {
    UPG_EV_NONE                 = 0,
    UPG_EV_COMPAT_CHECK         = 1,
    UPG_EV_START                = 2,
    UPG_EV_BACKUP               = 3,
    UPG_EV_PREPARE              = 4,
    UPG_EV_PREPARE_SWITCHOVER   = 5,
    UPG_EV_SWITCHOVER           = 6,
    UPG_EV_ROLLBACK             = 7,
    UPG_EV_READY                = 8,
    UPG_EV_REPEAL               = 9,
    UPG_EV_FINISH               = 10,
    UPG_EV_EXIT                 = 11,
    UPG_EV_MAX                  = 12,
} upg_ev_id_t;

#define UPG_EV_PDS_AGENT_NAME "pds-agent"

/// \brief asynchronous response callback by the event handler
/// cookie is passed to the event handler and it should not be modified
typedef void (*upg_async_ev_response_cb)(sdk_ret_t status, const void *cookie);

static inline const char *
upg_event2str(upg_ev_id_t id)
{
    return ("todo");
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

using upg::upg_ev_id_t::UPG_EV_NONE;
using upg::upg_ev_id_t::UPG_EV_COMPAT_CHECK;
using upg::upg_ev_id_t::UPG_EV_START;
using upg::upg_ev_id_t::UPG_EV_BACKUP;
using upg::upg_ev_id_t::UPG_EV_PREPARE;
using upg::upg_ev_id_t::UPG_EV_PREPARE_SWITCHOVER;
using upg::upg_ev_id_t::UPG_EV_SWITCHOVER;

/// @}

#endif   // __UPGRADE_EV_HPP__
