//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_INFRA_IPC_HPP__
#define __VPP_INFRA_IPC_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/core/msg.h"
#include "nic/apollo/api/core/msg.h"

// types for msg processing
typedef enum pds_ipc_msg_op_e {
    PDS_IPC_MSG_OP_RESERVE,
    PDS_IPC_MSG_OP_RELEASE,
    PDS_IPC_MSG_OP_PROCESS,
    PDS_IPC_MSG_OP_ROLLBACK,
    PDS_IPC_MSG_OP_COMMAND,

    PDS_IPC_MSG_OP_MAX
} pds_ipc_msg_op_t;

// callback function prototype
typedef sdk::sdk_ret_t (*pds_ipc_msg_cb)(const pds_msg_t *msg, pds_msg_t *ret);

// function prototypes
//int pds_vpp_ipc_init(void);
int pds_ipc_register_callback(pds_msg_id_t msgid,
                              pds_ipc_msg_op_t operation,
                              pds_ipc_msg_cb cb_fn);

#endif    // __VPP_INFRA_IPC_HPP__
