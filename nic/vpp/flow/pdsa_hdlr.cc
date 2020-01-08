//
//  {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
//
// Handlers for all messages from PDS Agent

#include <arpa/inet.h>
#include "nic/vpp/infra/ipc/pdsa_hdlr.hpp"
#include "nic/vpp/infra/ipc/pdsa_vpp_hdlr.h"
#include "pdsa_hdlr.h"

static sdk::sdk_ret_t
pdsa_flow_cfg_set (const pds_cfg_msg_t *msg)
{
    uint32_t new_flow_idle_timeouts[IPPROTO_MAX];
    auto sp_msg = &msg->security_profile;

    memset(new_flow_idle_timeouts, 0, sizeof(new_flow_idle_timeouts));

    for (int idx = 0; idx < IPPROTO_MAX; idx++) {
        new_flow_idle_timeouts[idx] = sp_msg->spec.other_idle_timeout;
    }
    new_flow_idle_timeouts[IPPROTO_TCP]  = sp_msg->spec.tcp_idle_timeout;
    new_flow_idle_timeouts[IPPROTO_UDP]  = sp_msg->spec.udp_idle_timeout;
    new_flow_idle_timeouts[IPPROTO_ICMP] = sp_msg->spec.icmp_idle_timeout;

    pds_flow_idle_timeout_set(new_flow_idle_timeouts,
                              sizeof(new_flow_idle_timeouts));

    return sdk::SDK_RET_OK;
}

static sdk::sdk_ret_t
pdsa_flow_cfg_clear (const pds_cfg_msg_t *msg)
{
    uint32_t new_flow_idle_timeouts[IPPROTO_MAX];

    memset(new_flow_idle_timeouts, 0, sizeof(new_flow_idle_timeouts));
    pds_flow_idle_timeout_set(new_flow_idle_timeouts,
                              sizeof(new_flow_idle_timeouts));

    return sdk::SDK_RET_OK;
}

void
pdsa_flow_hdlr_init (void)
{
    // initialize callbacks for cfg/oper messages received from pds-agent

    pds_ipc_register_callbacks(OBJ_ID_SECURITY_PROFILE,
                               pdsa_flow_cfg_set, 
                               pdsa_flow_cfg_clear,
                               NULL);
}
