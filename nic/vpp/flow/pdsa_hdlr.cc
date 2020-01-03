//
//  {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
//
// Handlers for all messages from PDS Agent

#include <arpa/inet.h>
#include "nic/vpp/infra/ipc/pdsa_hdlr.hpp"
#include "nic/vpp/infra/ipc/pdsa_vpp_hdlr.h"
#include "pdsa_hdlr.h"

// local cache to enable rollbacks
static uint32_t cached_flow_idle_timeouts[IPPROTO_MAX];

static inline void
pdsa_flow_cfg_idle_timeout_parse (const pds_security_profile_cfg_msg_t *sp_msg,
                                  uint32_t *timeouts)
{
    int idx;

    for (idx = 0; idx < IPPROTO_MAX; idx++) {
        timeouts[idx] = sp_msg->spec.other_idle_timeout;
    }
    timeouts[IPPROTO_TCP]  = sp_msg->spec.tcp_idle_timeout;
    timeouts[IPPROTO_UDP]  = sp_msg->spec.udp_idle_timeout;
    timeouts[IPPROTO_ICMP] = sp_msg->spec.icmp_idle_timeout;
    return;
}

static inline void
pdsa_flow_cfg_idle_timeout_build (uint32_t *timeouts,
                                  pds_security_profile_cfg_msg_t *sp_msg)
{
    sp_msg->spec.tcp_idle_timeout  = timeouts[IPPROTO_TCP];
    sp_msg->spec.udp_idle_timeout  = timeouts[IPPROTO_UDP];
    sp_msg->spec.icmp_idle_timeout = timeouts[IPPROTO_ICMP];

    // all other idle timeout values are same, fill from v6 sample
    sp_msg->spec.other_idle_timeout = timeouts[IPPROTO_IPV6];
    return;
}

static sdk::sdk_ret_t
pdsa_flow_cfg_process (const pds_msg_t *msg, pds_msg_t *unused)
{
    uint32_t new_flow_idle_timeouts[IPPROTO_MAX];

    switch (msg->cfg_msg.op) {
    case API_OP_CREATE:
    case API_OP_UPDATE:
        pdsa_flow_cfg_idle_timeout_parse(&msg->cfg_msg.security_profile,
                                         new_flow_idle_timeouts);
        break;
    case API_OP_DELETE:
        // reset to defaults, zero in this case.
        memset(new_flow_idle_timeouts, 0, sizeof(new_flow_idle_timeouts));
        break;
    default:
        return sdk::SDK_RET_OK;
    }

    // store the current values of timeouts locally, will be needed
    // for rollbacks
    pds_flow_idle_timeout_get(cached_flow_idle_timeouts,
                              sizeof(cached_flow_idle_timeouts));

    pds_flow_idle_timeout_set(new_flow_idle_timeouts,
                              sizeof(new_flow_idle_timeouts));

    return sdk::SDK_RET_OK;
}

static sdk::sdk_ret_t
pdsa_flow_cfg_rollback (const pds_msg_t *msg, pds_msg_t *unused)
{
    ipc_log_notice("Restoring cached idle timeouts");

    // restore the values of timeouts from cache
    pds_flow_idle_timeout_set(cached_flow_idle_timeouts,
                              sizeof(cached_flow_idle_timeouts));

    return sdk::SDK_RET_OK;
}

static sdk::sdk_ret_t
pdsa_flow_cfg_read (const pds_msg_t *msg, pds_msg_t *reply)
{
    uint32_t cur_flow_idle_timeouts[IPPROTO_MAX];
    pds_security_profile_cfg_msg_t *sp_msg;

    ipc_log_notice("Read security profile command received");

    sp_msg = &reply->cfg_msg.security_profile;
    memset(sp_msg, 0, sizeof(pds_security_profile_cfg_msg_t));

    // read from vpp internal storage
    pds_flow_idle_timeout_get(cur_flow_idle_timeouts,
                              sizeof(cur_flow_idle_timeouts));

    sp_msg->spec.key = msg->cfg_msg.security_profile.key;
    pdsa_flow_cfg_idle_timeout_build(cur_flow_idle_timeouts, sp_msg);

    return sdk::SDK_RET_OK;
}

void
pdsa_flow_hdlr_init (void)
{
    // initialize callbacks for cfg/oper messages received from pds-agent

    pds_ipc_register_callback(PDS_CFG_MSG_ID_SECURITY_PROFILE,
                              PDS_IPC_MSG_OP_PROCESS, pdsa_flow_cfg_process);

    pds_ipc_register_callback(PDS_CFG_MSG_ID_SECURITY_PROFILE,
                              PDS_IPC_MSG_OP_ROLLBACK, pdsa_flow_cfg_rollback);

    pds_ipc_register_callback(PDS_CFG_MSG_ID_SECURITY_PROFILE,
                              PDS_IPC_MSG_OP_COMMAND, pdsa_flow_cfg_read);
}
