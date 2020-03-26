//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
#ifndef __HAL_UPG_IPC_HPP__
#define __HAL_UPG_IPC_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/types.hpp"
#include "nic/hal/core/event_ipc.hpp"
#include "nic/hal/iris/upgrade/upgrade.hpp"
#include "nic/hal/iris/upgrade/nicmgr_upgrade.hpp"

namespace hal {
namespace upgrade {

#define UPG_RSP_ERR_STR_MAXLEN 256

// event identifiers
typedef enum upg_msg_id_e {
    MSG_ID_UPG_COMPAT_CHECK   = 1,
    MSG_ID_UPG_SAVE_STATE     = 2,
    MSG_ID_UPG_QUIESCE        = 3,
    MSG_ID_UPG_LINK_UP        = 4,
    MSG_ID_UPG_LINK_DOWN      = 5,
    MSG_ID_UPG_HOSTUP         = 6,
    MSG_ID_UPG_HOSTDOWN       = 7,
    MSG_ID_UPG_POST_LINK_UP   = 8,
    MSG_ID_UPG_POST_HOSTDOWN  = 9,
    MSG_ID_UPG_POST_RESTART   = 10,
    MSG_ID_UPG_SUCCESS        = 11,
    MSG_ID_UPG_FAIL           = 12,
    MSG_ID_UPG_ABORT          = 13,
} upg_msg_id_t;

// response code
typedef enum upg_rsp_code_s {
    UPG_RSP_PENDING = 0,
    UPG_RSP_SUCCESS = 1,
    UPG_RSP_FAIL,
    UPG_RSP_INPROGRESS
} upg_rsp_code_t;

// upgrade message
typedef struct upg_msg_s {
    upg_msg_id_t   msg_id;
    upg_rsp_code_t rsp_code;
    uint32_t       prev_exec_state;
    char rsp_err_string[UPG_RSP_ERR_STR_MAXLEN];
} upg_msg_t;

void upg_event_init(void);
HdlrResp upg_event_notify(upg_msg_id_t id, UpgCtx& upgCtx,
                          uint32_t prevstate);

}   // namespace upgrade
}   // namespace hal

using hal::upgrade::upg_msg_id_t;
using hal::upgrade::upg_msg_t;
using hal::upgrade::upg_rsp_code_t;
using upg_msg_id_t::MSG_ID_UPG_COMPAT_CHECK;
using upg_msg_id_t::MSG_ID_UPG_SAVE_STATE;
using upg_msg_id_t::MSG_ID_UPG_QUIESCE;
using upg_msg_id_t::MSG_ID_UPG_LINK_UP;
using upg_msg_id_t::MSG_ID_UPG_LINK_DOWN;
using upg_msg_id_t::MSG_ID_UPG_HOSTUP;
using upg_msg_id_t::MSG_ID_UPG_HOSTDOWN;
using upg_msg_id_t::MSG_ID_UPG_HOSTDOWN;
using upg_msg_id_t::MSG_ID_UPG_POST_LINK_UP;
using upg_msg_id_t::MSG_ID_UPG_POST_HOSTDOWN;
using upg_msg_id_t::MSG_ID_UPG_POST_RESTART;
using upg_msg_id_t::MSG_ID_UPG_SUCCESS;
using upg_msg_id_t::MSG_ID_UPG_FAIL;
using upg_msg_id_t::MSG_ID_UPG_ABORT;

using upg_rsp_code_t::UPG_RSP_PENDING;
using upg_rsp_code_t::UPG_RSP_SUCCESS;
using upg_rsp_code_t::UPG_RSP_FAIL;
using upg_rsp_code_t::UPG_RSP_INPROGRESS;

#endif   // __HAL_UPG_IPC_HPP__

