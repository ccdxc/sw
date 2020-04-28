//
//  {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
//
// Handlers for all messages from PDS Agent

#include <arpa/inet.h>
#include <nic/sdk/lib/metrics/metrics.hpp>
#include <gen/p4gen/p4/include/ftl.h>
#include <nic/vpp/infra/cfg/pdsa_db.hpp>
#include <nic/vpp/infra/ipc/pdsa_vpp_hdlr.h>
#include <nic/vpp/infra/ipc/pdsa_hdlr.hpp>
#include <feature.h>
#include <ftl_wrapper.h>
#include "session.h"
#include "pdsa_hdlr.h"
#include "pdsa_uds_hdlr.h"
#include "log.h"

int
clear_flow_entry (pds_flow_key_t *key)
{
    int ret;
    ipv4_flow_hash_entry_t iv4entry, rv4entry;
    flow_hash_entry_t entry;

    if (key->src_ip.af == IP_AF_IPV4) {
        // Clear both ingress and egress flows
        ftlv4 *table4 = (ftlv4 *)pds_flow_get_table4();
        iv4entry.clear();
        ftlv4_set_key(&iv4entry, key->src_ip.addr.v4_addr,
                      key->dst_ip.addr.v4_addr, key->proto,
                      key->sport, key->dport, key->lookup_id);
        ret = ftlv4_remove(table4, &iv4entry, 0);

        // For the reverse flow, just swap src and dst addr and port
        rv4entry.clear();
        ftlv4_set_key(&rv4entry, key->dst_ip.addr.v4_addr,
                      key->src_ip.addr.v4_addr, key->proto,
                      key->sport, key->dport, key->lookup_id);
        ret = ftlv4_remove(table4, &rv4entry, 0);
    } else {
        // Clear both ingress and egress flows
        ftlv6 *table6 = (ftlv6 *)pds_flow_get_table6_or_l2();
        entry.clear();
        ftlv6_set_key(&entry, key->src_ip.addr.v6_addr.addr8,
                      key->dst_ip.addr.v6_addr.addr8, key->proto,
                      key->sport, key->dport, key->lookup_id);
        ret = ftlv6_remove(table6, &entry, 0);

        // For the reverse flow, just swap src and dst addr and ports
        entry.clear();
        ftlv6_set_key(&entry, key->dst_ip.addr.v6_addr.addr8,
                      key->src_ip.addr.v6_addr.addr8, key->proto,
                      key->sport, key->dport, key->lookup_id);
        ret = ftlv6_remove(table6, &entry, 0);
    }
    return ret;
}

static sdk::sdk_ret_t
pdsa_flow_vnic_stats_get(const pds_cmd_msg_t *msg, pds_cmd_ctxt_t *ctxt) {
    uint32_t active_sessions;
    bool ret;

    ret = pds_session_active_on_vnic_get(msg->vnic_stats_get.vnic_hw_id,
                                         &active_sessions);
    if (!ret) {
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }
    ctxt->vnic_stats->active_sessions = active_sessions;
    return sdk::SDK_RET_OK;
}

static sdk::sdk_ret_t
pdsa_flow_vnic_stats_init(const pds_cmd_msg_t *msg, pds_cmd_ctxt_t *ctxt)
{
    ctxt->vnic_stats = (pds_vnic_stats_t *)calloc(1, sizeof(pds_vnic_stats_t));
    if (ctxt->vnic_stats == NULL) {
        return sdk::SDK_RET_OOM;
    }
    return sdk::SDK_RET_OK;
}

static sdk::sdk_ret_t
pdsa_flow_vnic_stats_destroy(const pds_cmd_msg_t *msg, pds_cmd_ctxt_t *ctxt)
{
    free(ctxt->vnic_stats);
    return sdk::SDK_RET_OK;
}


static sdk::sdk_ret_t
pdsa_flow_cfg_set (const pds_cfg_msg_t *msg)
{
    uint32_t new_flow_idle_timeouts[PDS_FLOW_PROTO_END];
    uint32_t new_flow_drop_timeouts[PDS_FLOW_PROTO_END];
    auto sp_msg = &msg->security_profile;

    new_flow_idle_timeouts[PDS_FLOW_PROTO_TCP] =
                                        sp_msg->spec.tcp_idle_timeout;
    new_flow_idle_timeouts[PDS_FLOW_PROTO_UDP] =
                                        sp_msg->spec.udp_idle_timeout;
    new_flow_idle_timeouts[PDS_FLOW_PROTO_ICMP] =
                                        sp_msg->spec.icmp_idle_timeout;
    new_flow_idle_timeouts[PDS_FLOW_PROTO_OTHER] =
                                        sp_msg->spec.other_idle_timeout;
    new_flow_drop_timeouts[PDS_FLOW_PROTO_TCP] =
                                        sp_msg->spec.tcp_drop_timeout;
    new_flow_drop_timeouts[PDS_FLOW_PROTO_UDP] =
                                        sp_msg->spec.udp_drop_timeout;
    new_flow_drop_timeouts[PDS_FLOW_PROTO_ICMP] = 
                                        sp_msg->spec.icmp_drop_timeout;
    new_flow_drop_timeouts[PDS_FLOW_PROTO_OTHER] =
                                        sp_msg->spec.other_drop_timeout;

    pds_flow_cfg_set(sp_msg->spec.conn_track_en,
                     sp_msg->spec.tcp_syn_timeout,
                     sp_msg->spec.tcp_halfclose_timeout,
                     sp_msg->spec.tcp_close_timeout,
                     new_flow_idle_timeouts,
                     new_flow_drop_timeouts);

    return sdk::SDK_RET_OK;
}

static sdk::sdk_ret_t
pdsa_flow_cfg_clear (const pds_cfg_msg_t *msg)
{
    uint32_t new_flow_idle_timeouts[PDS_FLOW_PROTO_END] = {0};

    pds_flow_cfg_set(0, 0, 0, 0,
                     new_flow_idle_timeouts, new_flow_idle_timeouts);

    return sdk::SDK_RET_OK;
}

static sdk::sdk_ret_t
pdsa_flow_clear_cmd(const pds_cmd_msg_t *msg, pds_cmd_ctxt_t *ctxt)
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

    if (pds_flow_age_supported()) {
        pds_cfg_register_callbacks(OBJ_ID_SECURITY_PROFILE,
                                   pdsa_flow_cfg_set, 
                                   pdsa_flow_cfg_clear,
                                   NULL);
    }
    pds_ipc_register_cmd_callbacks(PDS_CMD_MSG_FLOW_CLEAR,
                                   pdsa_flow_clear_cmd);
    pds_ipc_register_cmd_callbacks(PDS_CMD_MSG_VNIC_STATS_GET,
                                   pdsa_flow_vnic_stats_get,
                                   pdsa_flow_vnic_stats_init,
                                   pdsa_flow_vnic_stats_destroy);
}
