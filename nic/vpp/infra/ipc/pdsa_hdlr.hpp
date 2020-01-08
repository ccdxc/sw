//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_INFRA_IPC_HPP__
#define __VPP_INFRA_IPC_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/core/msg.h"
#include "nic/apollo/api/core/msg.h"

// callback function prototype
typedef sdk::sdk_ret_t (*pds_cfg_set_cb)(const pds_cfg_msg_t *msg);
typedef sdk::sdk_ret_t (*pds_cfg_del_cb)(const pds_cfg_msg_t *msg);
typedef sdk::sdk_ret_t (*pds_cfg_act_cb)(const pds_cfg_msg_t *msg);

// function prototypes
int pds_ipc_register_callbacks(obj_id_t id,
                               pds_cfg_set_cb set_cb_fn,
                               pds_cfg_del_cb del_cb_fn,
                               pds_cfg_act_cb act_cb_fn);


#endif    // __VPP_INFRA_IPC_HPP__
