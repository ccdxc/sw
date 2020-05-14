//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_INFRA_IPC_PDSA_HDLR_HPP__
#define __VPP_INFRA_IPC_PDSA_HDLR_HPP__

#include "nic/apollo/api/core/msg.h"

typedef sdk::sdk_ret_t (*pds_cmd_cb_t)(const pds_cmd_msg_t *msg,
                                     pds_cmd_rsp_t *response);

int pds_ipc_register_cmd_callbacks(pds_cmd_msg_id_t msg_id,
								   pds_cmd_cb_t cmd_hdl_cb);

#endif // __VPP_INFRA_IPC_PDSA_HDLR_HPP__
