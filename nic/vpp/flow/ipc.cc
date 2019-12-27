//
//  {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
//

#include "nic/vpp/infra/ipc/ipc.hpp"
#include "nic/vpp/infra/ipc/pds_ipc.h"

// extern declarations for "C" code
extern "C" void pds_flow_idle_timeout_get(uint32_t *flow_idle_timeout,
                                          size_t sz);
extern "C" void pds_flow_idle_timeout_set(const uint32_t *flow_idle_timeout,
                                          size_t sz);
extern "C" void pds_flow_cfg_init(void);

// local cache to enable rollbacks
static uint32_t cached_flow_idle_timeouts[IPPROTO_MAX];

// callback function to process configuration message. Create and Update
// change the config, delete restores defaults
static sdk::sdk_ret_t
pds_flow_cfg_process (const pds_msg_t *msg, pds_msg_t *unused) {
    const pds_security_profile_cfg_msg_t *spmsg;
    uint32_t new_flow_idle_timeouts[IPPROTO_MAX];
    uint32_t tcpto, udpto, icmpto, otherto;
    int i;

    spmsg = &msg->cfg_msg.security_profile;

    switch (msg->cfg_msg.op) {
    case API_OP_CREATE:
    case API_OP_UPDATE:
        tcpto = spmsg->spec.tcp_idle_timeout;
        udpto = spmsg->spec.udp_idle_timeout;
        icmpto = spmsg->spec.icmp_idle_timeout;
        otherto = spmsg->spec.other_idle_timeout;
        break;

    case API_OP_DELETE:
        // default timeouts (from nic/apollo/agent/protos/policy.proto)
        tcpto = PDS_DEFAULT_TCP_IDLE_TIMEOUT;
        udpto = PDS_DEFAULT_UDP_IDLE_TIMEOUT;
        icmpto = PDS_DEFAULT_ICMP_IDLE_TIMEOUT;
        otherto = PDS_DEFAULT_OTHER_IDLE_TIMEOUT;
        break;

    default:
        // don't do anything
        return sdk::SDK_RET_OK;
    }

    // store the current values of timeouts locally
    pds_flow_idle_timeout_get(cached_flow_idle_timeouts,
                              sizeof(cached_flow_idle_timeouts));
    ipc_log_notice("Caching current idle timeouts");

    ipc_log_notice("idle timeouts from secprof tcp:%d udp:%d icmp:%d other:%d",
                   tcpto, udpto, icmpto, otherto);

    for (i = 0; i < IPPROTO_MAX; i++) {
        new_flow_idle_timeouts[i] = otherto;
    }
    new_flow_idle_timeouts[IPPROTO_TCP] = tcpto;
    new_flow_idle_timeouts[IPPROTO_UDP] = udpto;
    new_flow_idle_timeouts[IPPROTO_ICMP] = icmpto;

    pds_flow_idle_timeout_set(new_flow_idle_timeouts,
                              sizeof(new_flow_idle_timeouts));

    return sdk::SDK_RET_OK;
}

// callback function for restoring changed config on batch failure
static sdk::sdk_ret_t
pds_flow_cfg_rollback (const pds_msg_t *msg, pds_msg_t *unused) {
    ipc_log_notice("Restoring cached idle timeouts");

    // restore the values of timeouts from cache
    pds_flow_idle_timeout_set(cached_flow_idle_timeouts,
                              sizeof(cached_flow_idle_timeouts));

    return sdk::SDK_RET_OK;
}

// callback for command, more specifically, for reading config
static sdk::sdk_ret_t
pds_flow_cfg_read (const pds_msg_t *msg, pds_msg_t *reply) {
    uint32_t cur_flow_idle_timeouts[IPPROTO_MAX];
    pds_security_profile_cfg_msg_t *spmsg;

    ipc_log_notice("Read security profile command received");

    spmsg = &reply->cfg_msg.security_profile;
    memset(spmsg, 0, sizeof(pds_security_profile_cfg_msg_t));
    // copy the key from request
    spmsg->spec.key = msg->cfg_msg.security_profile.key;

    pds_flow_idle_timeout_get(cur_flow_idle_timeouts,
                              sizeof(cur_flow_idle_timeouts));

    spmsg->spec.tcp_idle_timeout = cur_flow_idle_timeouts[IPPROTO_TCP];
    spmsg->spec.udp_idle_timeout = cur_flow_idle_timeouts[IPPROTO_UDP];;
    spmsg->spec.icmp_idle_timeout = cur_flow_idle_timeouts[IPPROTO_ICMP];
    spmsg->spec.other_idle_timeout = cur_flow_idle_timeouts[IPPROTO_IPV6];

    return sdk::SDK_RET_OK;
}

// initialize callbacks for flow configuration
//
// Note: This is called from C code, and must have C linkage
//
void
pds_flow_cfg_init (void) {
    int i;
    uint32_t cur_flow_idle_timeouts[IPPROTO_MAX];

    // the get isn't required, strictly speaking
    pds_flow_idle_timeout_get(cur_flow_idle_timeouts,
                              sizeof(cur_flow_idle_timeouts));

    // initialize default values for idle timeouts
    for (i = 0; i < IPPROTO_MAX; i++) {
        cur_flow_idle_timeouts[i] = PDS_DEFAULT_OTHER_IDLE_TIMEOUT;
    }
    cur_flow_idle_timeouts[IPPROTO_TCP] = PDS_DEFAULT_TCP_IDLE_TIMEOUT;
    cur_flow_idle_timeouts[IPPROTO_UDP] = PDS_DEFAULT_UDP_IDLE_TIMEOUT;
    cur_flow_idle_timeouts[IPPROTO_ICMP] = PDS_DEFAULT_ICMP_IDLE_TIMEOUT;

    // set up defaults
    pds_flow_idle_timeout_set(cur_flow_idle_timeouts,
                              sizeof(cur_flow_idle_timeouts));

    // no reserve required
    // no release required
    pds_ipc_register_callback(PDS_CFG_MSG_ID_SECURITY_PROFILE,
                              PDS_IPC_MSG_OP_PROCESS,
                              pds_flow_cfg_process);

    pds_ipc_register_callback(PDS_CFG_MSG_ID_SECURITY_PROFILE,
                              PDS_IPC_MSG_OP_ROLLBACK,
                              pds_flow_cfg_rollback);

    pds_ipc_register_callback(PDS_CFG_MSG_ID_SECURITY_PROFILE,
                              PDS_IPC_MSG_OP_COMMAND,
                              pds_flow_cfg_read);
}
