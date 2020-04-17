//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_INFRA_IPC_PDSA_HDLR_HPP__
#define __VPP_INFRA_IPC_PDSA_HDLR_HPP__

#include "nic/apollo/api/core/msg.h"

typedef struct pds_nat_cfg_cmd_ctxt_s {
    uint16_t num_entries;
    pds_nat_port_block_cfg_msg_t cfg[0];
} pds_nat_port_block_cmd_ctxt_t;

typedef struct pds_cmd_ctxt_s {
    uint16_t curr_count;
    union {
        pds_nat_port_block_cmd_ctxt_t *nat_ctxt;
        pds_vnic_stats_t              *vnic_stats;
    };
} pds_cmd_ctxt_t;

typedef sdk::sdk_ret_t (*pds_cmd_cb)(const pds_cmd_msg_t *msg, pds_cmd_ctxt_t *ctxt);

int pds_ipc_register_cmd_callbacks(pds_msg_id_t msg_id, pds_cmd_cb cmd_hdl_cb,
                                   pds_cmd_cb ctxt_init_cb = NULL,
                                   pds_cmd_cb ctxt_destroy_cb = NULL);

#endif // __VPP_INFRA_IPC_PDSA_HDLR_HPP__
