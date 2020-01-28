//
//  {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
//
// Handlers for all messages from PDS Agent

#include <arpa/inet.h>
#include "nic/vpp/infra/cfg/pdsa_db.hpp"
#include <nic/sdk/include/sdk/table.hpp>
#include <gen/p4gen/p4/include/ftl.h>
#include <ftl_wrapper.h>
#include "nic/vpp/infra/ipc/pdsa_vpp_hdlr.h"
#include "nic/vpp/infra/ipc/pdsa_hdlr.hpp"
#include "pdsa_hdlr.h"
#include "pdsa_uds_hdlr.h"

extern "C" int clear_all_flow_entries();

int
clear_flow_entry (pds_flow_key_t *key)
{
    int ret;
    ipv4_flow_hash_entry_t v4entry;
    flow_hash_entry_t entry;

    if (key->src_ip.af == IP_AF_IPV4) {
        // Clear both ingress and egress flows
        ftlv4 *table4 = (ftlv4 *)pds_flow_get_table4();
        v4entry.clear();
        ftlv4_set_key(&v4entry, key->src_ip.addr.v4_addr, 
                      key->dst_ip.addr.v4_addr, key->proto, 
                      key->sport, key->dport, key->lookup_id);
        ret = ftlv4_remove(table4, &v4entry, 0);
        
        // For the reverse flow, just swap src and dst addr and port
        v4entry.clear();
        ftlv4_set_key(&v4entry, key->dst_ip.addr.v4_addr,
                      key->src_ip.addr.v4_addr, key->proto,
                      key->sport, key->dport, key->lookup_id);
        ret = ftlv4_remove(table4, &v4entry, 0);
    } else {
        // Clear both ingress and egress flows
        ftlv6 *table6 = (ftlv6 *)pds_flow_get_table6();
        entry.clear();
        ftlv6_set_key(&entry, key->src_ip.addr.v6_addr.addr8, 
                      key->dst_ip.addr.v6_addr.addr8, key->proto, 
                      key->sport, key->dport, key->lookup_id, 0);
        ret = ftlv6_remove(table6, &entry, 0);

        // For the reverse flow, just swap src and dst addr and ports
        entry.clear();
        ftlv6_set_key(&entry, key->dst_ip.addr.v6_addr.addr8,
                      key->src_ip.addr.v6_addr.addr8, key->proto,
                      key->sport, key->dport, key->lookup_id, 0);
        ret = ftlv6_remove(table6, &entry, 0);
    }
    return ret;
}

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

static sdk::sdk_ret_t
pdsa_flow_clear_cmd(const pds_cmd_msg_t *msg)
{
    int ret;
    pds_flow_key_t key = msg->flow_clear.key;

    // currently we support all filters or none, so just checking for one
    if (key.sport != 0) {
        ret = clear_flow_entry(&key);
    } else {
        ret = clear_all_flow_entries();
    }

    if (ret) {
        return SDK_RET_ERR;
    }

    return sdk::SDK_RET_OK;
}

void
pdsa_flow_hdlr_init (void)
{
    // initialize callbacks for cfg/oper messages received from pds-agent

    pds_cfg_register_callbacks(OBJ_ID_SECURITY_PROFILE,
                               pdsa_flow_cfg_set, 
                               pdsa_flow_cfg_clear,
                               NULL);
    pds_ipc_register_cmd_callbacks(PDS_CMD_MSG_FLOW_CLEAR,
                                   pdsa_flow_clear_cmd);
}
