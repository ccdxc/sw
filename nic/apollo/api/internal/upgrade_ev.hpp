//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines event handlers during process upgrade
///
//----------------------------------------------------------------------------

#ifndef __API_UPGRADE_EV_HPP__
#define __API_UPGRADE_EV_HPP__

#include "include/sdk/types.hpp"
#include "include/sdk/platform.hpp"

namespace api {

/// \brief upgrade event message ids to threads
#define UPG_EV_MSG_ID_ENTRIES(E)                                              \
    E(UPG_MSG_ID_NONE,            SDK_IPC_MSG_ID_MAX + 1,  "invalid")         \
    E(UPG_MSG_ID_COMPAT_CHECK,    SDK_IPC_MSG_ID_MAX + 2,  "compatcheck")     \
    E(UPG_MSG_ID_START,           SDK_IPC_MSG_ID_MAX + 3,  "start")           \
    E(UPG_MSG_ID_BACKUP,          SDK_IPC_MSG_ID_MAX + 4,  "backup")          \
    E(UPG_MSG_ID_LINK_DOWN,       SDK_IPC_MSG_ID_MAX + 5,  "linkdown")        \
    E(UPG_MSG_ID_HOSTDEV_RESET,   SDK_IPC_MSG_ID_MAX + 6,  "hostdevreset")    \
    E(UPG_MSG_ID_QUIESCE,         SDK_IPC_MSG_ID_MAX + 7,  "quiesce")         \
    E(UPG_MSG_ID_PREP_SWITCHOVER, SDK_IPC_MSG_ID_MAX + 8,  "prep_switchover") \
    E(UPG_MSG_ID_SWITCHOVER,      SDK_IPC_MSG_ID_MAX + 9,  "switchover")      \
    E(UPG_MSG_ID_READY,           SDK_IPC_MSG_ID_MAX + 10, "ready")           \
    E(UPG_MSG_ID_REPEAL,          SDK_IPC_MSG_ID_MAX + 11, "repeal")          \
    E(UPG_MSG_ID_FINISH,          SDK_IPC_MSG_ID_MAX + 12, "finish")          \
    E(UPG_MSG_ID_MAX,             SDK_IPC_MSG_ID_MAX + 13, "max-invalid")

SDK_DEFINE_ENUM(upg_ev_msg_id_t, UPG_EV_MSG_ID_ENTRIES)
SDK_DEFINE_ENUM_TO_STR(upg_ev_msg_id_t, UPG_EV_MSG_ID_ENTRIES)
#undef UPG_EV_MSG_ID_ENTRIES

static inline const char *
upg_msgid2str (upg_ev_msg_id_t id)
{
    return UPG_EV_MSG_ID_ENTRIES_str(id);
}

/// \brief upgrade event parameters passed to the threads
typedef struct upg_ev_params_s {
    upg_ev_msg_id_t id;                ///< event message id
    sdk::platform::upg_mode_t mode;    ///< upgrade mode
    sdk_ret_t rsp_code;                ///< response code
} upg_ev_params_t;

/// \brief upgrade event handler
typedef sdk_ret_t (*upg_ev_hdlr_t)(upg_ev_params_t *params);

sdk_ret_t upg_graceful_init(pds_init_params_t *params);
sdk_ret_t upg_hitless_init(pds_init_params_t *params);
sdk_ret_t nicmgr_upg_graceful_init(void);
sdk_ret_t nicmgr_upg_hitless_init(void);
void upg_ev_process_response(sdk_ret_t ret, upg_ev_msg_id_t id);
sdk_ret_t upg_event_send(sdk::upg::upg_ev_params_t *params);
sdk_ret_t upg_obj_restore(upg_mode_t mode);

}   // namespace api

#include "nic/apollo/api/internal/upgrade_ev_graceful.hpp"
#include "nic/apollo/api/internal/upgrade_ev_hitless.hpp"

using api::upg_ev_msg_id_t::UPG_MSG_ID_COMPAT_CHECK;
using api::upg_ev_msg_id_t::UPG_MSG_ID_START;
using api::upg_ev_msg_id_t::UPG_MSG_ID_BACKUP;
using api::upg_ev_msg_id_t::UPG_MSG_ID_LINK_DOWN;
using api::upg_ev_msg_id_t::UPG_MSG_ID_HOSTDEV_RESET;
using api::upg_ev_msg_id_t::UPG_MSG_ID_QUIESCE;
using api::upg_ev_msg_id_t::UPG_MSG_ID_PREP_SWITCHOVER;
using api::upg_ev_msg_id_t::UPG_MSG_ID_SWITCHOVER;
using api::upg_ev_msg_id_t::UPG_MSG_ID_READY;
using api::upg_ev_msg_id_t::UPG_MSG_ID_REPEAL;
using api::upg_ev_msg_id_t::UPG_MSG_ID_FINISH;
using api::upg_ev_msg_id_t::UPG_MSG_ID_MAX;

#endif   // __API_UPGRADE_EV_HPP__
