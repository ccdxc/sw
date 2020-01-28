//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_INFRA_IPC_PDSA_HDLR_HPP__
#define __VPP_INFRA_IPC_PDSA_HDLR_HPP__

typedef sdk::sdk_ret_t (*pds_cmd_cb)(const pds_cmd_msg_t *msg);

int pds_ipc_register_cmd_callbacks(pds_msg_id_t msg_id, pds_cmd_cb cb_fn);

#endif // __VPP_INFRA_IPC_PDSA_HDLR_HPP__
