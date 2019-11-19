// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/iris/lif/lif_pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/plugins/cfg/nw/interface_api.hpp"
#include "nic/hal/pd/iris/nw/if_pd_utils.hpp"
#include "gen/p4gen/p4/include/p4pd.h"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/hal/pd/iris/telemetry/telemetry_pd.hpp"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/hal/pd/iris/internal/p4plus_pd_api.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nic/hal/pd/iris/telemetry/ipfix_pd.hpp"

namespace hal {
namespace pd {

telemetry_export_dest_t export_destinations[TELEMETRY_NUM_EXPORT_DEST];
char _deb_buf[TELEMETRY_EXPORT_BUFF_SIZE + 1];

hal_ret_t
pd_mirror_update_hw(uint32_t id, mirror_actiondata_t *action_data)
{
    hal_ret_t           ret = HAL_RET_OK;
    sdk_ret_t           sdk_ret;
    directmap           *session = NULL;
    p4pd_error_t        p4_err;
    char                buff[4096] = {0};

    session = g_hal_state_pd->dm_table(P4TBL_ID_MIRROR);
    SDK_ASSERT_RETURN((session != NULL), HAL_RET_ERR);

    sdk_ret = session->update(id, action_data);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}: programming sesion {} failed ({})",
                __FUNCTION__, id, ret);
        if (sdk_ret == SDK_RET_ENTRY_NOT_FOUND) {
            sdk_ret = session->insert_withid(action_data, id);
            ret = hal_sdk_ret_to_hal_ret(sdk_ret);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("{}: programming sesion {} failed ({})",
                              __FUNCTION__, id, ret);
            } else {
                HAL_TRACE_DEBUG("{}: programmed session {}: {}",
                                __FUNCTION__, id, buff);
            }
        }
    } else {
        p4_err =  p4pd_table_ds_decoded_string_get(P4TBL_ID_MIRROR, 0, NULL, NULL,
                action_data, buff, sizeof(buff));
        SDK_ASSERT(p4_err == P4PD_SUCCESS);
        HAL_TRACE_DEBUG("{}: programmed session {}: {}",
                __FUNCTION__, id, buff);
    }
    return ret;
}

hal_ret_t
pd_mirror_session_update (pd_func_args_t *pd_func_args)
{
    uint32_t                                dst_lport;
    hal_ret_t                               ret = HAL_RET_OK;
    p4pd_error_t                            pdret;
    mirror_actiondata_t                     action_data;
    pd_mirror_session_update_args_t         *args = pd_func_args->pd_mirror_session_update;

    if ((args == NULL) || (args->session == NULL)) {
        HAL_TRACE_ERR(" NULL argument");
        return HAL_RET_INVALID_ARG;
    }
    SDK_ASSERT((args->session->id >= 0) && (args->session->id <= 7));

    HAL_TRACE_DEBUG("Update call for session {}", args->session->id);
    pdret = p4pd_entry_read(P4TBL_ID_MIRROR, args->session->id, NULL,
                            NULL, (void *)&action_data);
    if (pdret != P4PD_SUCCESS) {
        HAL_TRACE_ERR("Session id {} read failed {}", args->session->id, pdret);
        return HAL_RET_ERR;
    }
    // Update the dest_if
    dst_lport = if_get_lport_id(args->session->dest_if);
    if (action_data.action_id == MIRROR_ERSPAN_MIRROR_ID) {
        action_data.action_u.mirror_erspan_mirror.dst_lport = dst_lport;
        ret = pd_mirror_update_hw(args->session->id, &action_data);
    }

    return ret;
}

hal_ret_t
pd_mirror_session_create (pd_func_args_t *pd_func_args)
{
    uint32_t dst_lport;
    pd_mirror_session_create_args_t *args = pd_func_args->pd_mirror_session_create;
    mirror_actiondata_t action_data;
    hal::pd::pd_tunnelif_get_rw_idx_args_t    tif_args = { 0 };
    pd_func_args_t pd_func_args1 = {0};

    if ((args == NULL) || (args->session == NULL)) {
        HAL_TRACE_ERR(" NULL argument");
        return HAL_RET_INVALID_ARG;
    }

    HAL_TRACE_DEBUG("{}: Create call for session {}",
            __FUNCTION__, args->session->id);

    // Add to a PD datastructure instead of stack.
    memset(&action_data, 0, sizeof(mirror_actiondata_t));
    SDK_ASSERT((args->session->id >= 0) && (args->session->id <= 7));

    switch (args->session->dest_if->if_type) {
        case intf::IF_TYPE_TUNNEL:
        case intf::IF_TYPE_ENIC:
        case intf::IF_TYPE_UPLINK_PC:
        case intf::IF_TYPE_UPLINK:
        case intf::IF_TYPE_APP_REDIR:
            dst_lport = if_get_lport_id(args->session->dest_if);
            break;
        default:
            return HAL_RET_INVALID_OP;
    }

    switch (args->session->type) {
    case MIRROR_DEST_LOCAL: {
        action_data.action_id = MIRROR_LOCAL_SPAN_ID;
        action_data.action_u.mirror_local_span.truncate_len = args->session->truncate_len;
        action_data.action_u.mirror_local_span.dst_lport = dst_lport;
        break;
    }
    case MIRROR_DEST_RSPAN: {
        action_data.action_id = MIRROR_REMOTE_SPAN_ID;
        action_data.action_u.mirror_remote_span.truncate_len = args->session->truncate_len;
        action_data.action_u.mirror_remote_span.dst_lport = dst_lport;
        action_data.action_u.mirror_remote_span.vlan = args->session->mirror_destination_u.r_span_dest.vlan;
        action_data.action_u.mirror_remote_span.tunnel_rewrite_index = g_hal_state_pd->tnnl_rwr_tbl_encap_vlan_idx();
        break;
    }
    case MIRROR_DEST_ERSPAN: {
        action_data.action_id = MIRROR_ERSPAN_MIRROR_ID;
        action_data.action_u.mirror_erspan_mirror.truncate_len = args->session->truncate_len;
        action_data.action_u.mirror_erspan_mirror.dst_lport = dst_lport;
        tif_args.hal_if = args->session->mirror_destination_u.er_span_dest.tunnel_if;
        pd_func_args1.pd_tunnelif_get_rw_idx = &tif_args;
        hal::pd::pd_tunnelif_get_rw_idx(&pd_func_args1);
        action_data.action_u.mirror_erspan_mirror.tunnel_rewrite_index =
            tif_args.tnnl_rw_idx;
        break;
    }
    default:
        HAL_TRACE_ERR(" unknown session type {}", args->session->type);
        return HAL_RET_INVALID_ARG;
    }

    // Do clock sync to P4 to make sure we have the latest time
    // Note that this is only needed once after NTPD is started in 
    // Naples. Today we dont have a notification from NMD to HAL on
    // that so we want to sync during mirror session create. This will
    // ensure clock delta to be written to P4 when we get decommisioned
    // from one venice and admitted to other.
    if (likely(is_platform_type_hw()))
        pd_clock_trigger_sync(pd_func_args);

    return pd_mirror_update_hw(args->session->id, &action_data);
}

hal_ret_t
pd_mirror_session_delete(pd_func_args_t *pd_func_args)
{
    mirror_actiondata_t action_data;
    pd_mirror_session_delete_args_t *args = pd_func_args->pd_mirror_session_delete;
    if ((args == NULL) || (args->session == NULL)) {
        HAL_TRACE_ERR("NULL argument");
        return HAL_RET_INVALID_ARG;
    }
    memset(&action_data, 0, sizeof(mirror_actiondata_t));
    SDK_ASSERT((args->session->id >= 0) && (args->session->id <= 7));
    action_data.action_id = MIRROR_DROP_MIRROR_ID;

    return pd_mirror_update_hw(args->session->id, &action_data);
}

hal_ret_t
pd_mirror_session_get(pd_func_args_t *pd_func_args)
{
    mirror_actiondata_t action_data;
    pd_mirror_session_get_args_t *args = pd_func_args->pd_mirror_session_get;
    if ((args == NULL) || (args->session == NULL)) {
        HAL_TRACE_ERR("NULL argument");
        return HAL_RET_INVALID_ARG;
    }
    memset(&action_data, 0, sizeof(mirror_actiondata_t));
    SDK_ASSERT((args->session->id >= 0) && (args->session->id <= 7));

    p4pd_error_t pdret;
    pdret = p4pd_entry_read(P4TBL_ID_MIRROR, args->session->id, NULL, NULL, (void *)&action_data);
    if (pdret == P4PD_SUCCESS) {
        switch (action_data.action_id) {
        case MIRROR_LOCAL_SPAN_ID:
            args->session->type = MIRROR_DEST_LOCAL;
            args->session->truncate_len = action_data.action_u.mirror_local_span.truncate_len;
            // args-> dst_if // TBD
        case MIRROR_REMOTE_SPAN_ID:
            args->session->type = MIRROR_DEST_RSPAN;
            args->session->truncate_len = action_data.action_u.mirror_remote_span.truncate_len;
            args->session->mirror_destination_u.r_span_dest.vlan = action_data.action_u.mirror_remote_span.vlan;
        case MIRROR_ERSPAN_MIRROR_ID:
            args->session->type = MIRROR_DEST_ERSPAN;
            args->session->truncate_len = action_data.action_u.mirror_erspan_mirror.truncate_len;
            // Get tunnel if ID - TBD
            //args->session->mirror_destination_u.r_span_dest.tunnel_if_id =
        case MIRROR_NOP_ID:
            args->session->type = MIRROR_DEST_NONE;
        default:
            return HAL_RET_INVALID_OP;
        }
    } else {
        return HAL_RET_HW_PROG_ERR;
    }
    return HAL_RET_OK;
}

hal_ret_t
telemetry_export_dest_init(telemetry_export_dest_t *d)
{
    HAL_TRACE_DEBUG("{}: Export Destination Init {}", __FUNCTION__, d->id);
    uint64_t hbm_start = get_mem_addr(JP4_IPFIX);
    d->base_addr = hbm_start + (d->id * TELEMETRY_IPFIX_BUFSIZE);
    d->buf_hdr.packet_start = sizeof(telemetry_pd_export_buf_header_t);
    d->buf_hdr.payload_start = sizeof(telemetry_pd_export_buf_header_t) + sizeof(telemetry_pd_ipfix_header_t);
    d->buf_hdr.payload_length = TELEMETRY_IPFIX_BUFSIZE - d->buf_hdr.payload_start;
    d->buf_hdr.ip_hdr_start = sizeof(telemetry_pd_export_buf_header_t) + offsetof(telemetry_pd_ipfix_header_t, iphdr);
    memset(&d->ipfix_hdr, 0, sizeof(telemetry_pd_ipfix_header_t));
    d->ipfix_hdr.vlan.tpid = htons(0x8100);
    d->ipfix_hdr.vlan.etype = htons(0x0800);
    d->ipfix_hdr.iphdr.version = 4;
    d->ipfix_hdr.iphdr.ihl = 5;
    d->ipfix_hdr.iphdr.protocol = 17;
    ipfix_init(d->id, d->base_addr + d->buf_hdr.packet_start,
               sizeof(telemetry_pd_ipfix_header_t), d->buf_hdr.payload_length);
    HAL_TRACE_DEBUG("{}: Export Destination Init Done {}", __FUNCTION__, d->id);
    return HAL_RET_OK;
}

void
telemetry_export_dest_get_mac (telemetry_export_dest_t *d,
                               collector_config_t *cfg, bool src)
{
    if (src) {
        memcpy(cfg->src_mac, d->ipfix_hdr.vlan.smac, sizeof(uint8_t) * ETH_ADDR_LEN);
    } else {
        memcpy(cfg->dest_mac, d->ipfix_hdr.vlan.dmac, sizeof(uint8_t) * ETH_ADDR_LEN);
    }
    return;
}

void
telemetry_export_dest_set_mac(telemetry_export_dest_t *d, mac_addr_t in,
                              bool src)
{
    if (src) {
        memcpy(d->ipfix_hdr.vlan.smac, in, sizeof(uint8_t) * ETH_ADDR_LEN);
    } else {
        memcpy(d->ipfix_hdr.vlan.dmac, in, sizeof(uint8_t) * ETH_ADDR_LEN);
    }
    return;
}

hal_ret_t
telemetry_export_dest_get_ip (telemetry_export_dest_t *d,
                              collector_config_t *cfg, bool src)
{
    if (src) {
        cfg->src_ip.af = IP_AF_IPV4;
        cfg->src_ip.addr.v4_addr = ntohl(d->ipfix_hdr.iphdr.saddr);
    } else {
        cfg->dst_ip.af = IP_AF_IPV4;
        cfg->dst_ip.addr.v4_addr = ntohl(d->ipfix_hdr.iphdr.daddr);
    }
    return HAL_RET_OK;
}

hal_ret_t
telemetry_export_dest_set_ip(telemetry_export_dest_t *d, ip_addr_t in,
                             bool src)
{
    if (in.af != IP_AF_IPV4) {
        HAL_TRACE_ERR("Non IPV4 source");
        return HAL_RET_INVALID_OP;
    }
    if (src) {
        d->ipfix_hdr.iphdr.saddr = htonl(in.addr.v4_addr);
    } else {
        d->ipfix_hdr.iphdr.daddr = htonl(in.addr.v4_addr);
    }
    return HAL_RET_OK;
}

// helper to dump Packet buffer
void
print_buffer(char *outbuf, int max_size, uint8_t *inbuf, int size)
{
    int i, ofset = 0;

    for (i = 0; i < size; ++i) {
        ofset += snprintf(outbuf + ofset, max_size - ofset, "0x%02x ", inbuf[i]);
        if ((i % 32) == 0) {
            ofset += snprintf(outbuf + ofset, max_size - ofset, "\n");
        }
    }
}

hal_ret_t
telemetry_export_dest_commit(telemetry_export_dest_t *d)
{
    HAL_TRACE_DEBUG("{}: Export Destination commit {}-> {}", __FUNCTION__, d->id, d->base_addr);
    p4plus_hbm_write(d->base_addr, (uint8_t*)&d->buf_hdr, sizeof(d->buf_hdr),
            P4PLUS_CACHE_ACTION_NONE);
    print_buffer(_deb_buf, TELEMETRY_EXPORT_BUFF_SIZE, (uint8_t*)&d->buf_hdr,
                 sizeof(d->buf_hdr));
    HAL_TRACE_DEBUG("{} : Buffer Header: Wrote: {}", __FUNCTION__, _deb_buf);
    // memcpy(d->base_addr, &d->buf_hdr, sizeof(d->buf_hdr));
    uint64_t hdr = d->base_addr + sizeof(d->buf_hdr);
    p4plus_hbm_write(hdr, (uint8_t*)&d->ipfix_hdr, sizeof(d->ipfix_hdr),
            P4PLUS_CACHE_ACTION_NONE);
    print_buffer(_deb_buf, TELEMETRY_EXPORT_BUFF_SIZE, (uint8_t*)&d->ipfix_hdr,
                 sizeof(d->ipfix_hdr));
    HAL_TRACE_DEBUG("{} : IPFIX-Header: Wrote: {}", __FUNCTION__, _deb_buf);
    return HAL_RET_OK;
}

hal_ret_t
pd_collector_create(pd_func_args_t *pd_func_args)
{
    pd_collector_create_args_t          *c_args;
    collector_config_t                  *cfg;
    hal_cfg_t                           *hal_cfg = NULL;
    pd_l2seg_get_fromcpu_vlanid_args_t  args;
    pd_func_args_t                      pd_func_args1 = {0};
    telemetry_export_dest_t             *d;

    c_args = pd_func_args->pd_collector_create;
    cfg = c_args->cfg;
    HAL_TRACE_DEBUG("{}: CollectorID {}", __FUNCTION__, cfg->collector_id);
    
    if (cfg->collector_id >= (TELEMETRY_NUM_EXPORT_DEST)) {
        HAL_TRACE_ERR(" invalid Id {}", cfg->collector_id );
        return HAL_RET_INVALID_ARG;
    }

    d = &export_destinations[cfg->collector_id];
    if (d->valid) {
        HAL_TRACE_ERR(" Already exists Id {}", cfg->collector_id );
        return HAL_RET_INVALID_ARG;
    }
    d->id = cfg->collector_id;
    telemetry_export_dest_init(d);

    args.l2seg = cfg->l2seg;
    args.vid = &cfg->vlan;
    pd_func_args1.pd_l2seg_get_fromcpu_vlanid = &args;

    if (pd_l2seg_get_fromcpu_vlanid(&pd_func_args1) != HAL_RET_OK) { 
        HAL_TRACE_DEBUG("{}: Could not retrieve CPU VLAN", __FUNCTION__);
        return HAL_RET_INVALID_ARG;
    }
    HAL_TRACE_DEBUG("{}: CPU VLAN {}", __FUNCTION__, cfg->vlan);

    d->ipfix_hdr.vlan.vlan_tag = htons(cfg->vlan);
    d->ipfix_hdr.iphdr.tos = 0;
    d->ipfix_hdr.iphdr.ttl = 64;
    // Total len will be updated correctly in the P4 datapath
    d->ipfix_hdr.iphdr.tot_len = htons(0xFFFF);
    d->ipfix_hdr.udphdr.sport = htons(UDP_SRC_PORT_TELEMETRY);
    d->ipfix_hdr.udphdr.dport = htons(cfg->dport);
    d->template_id = cfg->template_id;
    d->export_intvl = cfg->export_intvl;
    d->valid = true;
    
    telemetry_export_dest_set_ip(d, cfg->src_ip, true);
    telemetry_export_dest_set_ip(d, cfg->dst_ip, false);
    telemetry_export_dest_set_mac(d, cfg->src_mac, true);
    telemetry_export_dest_set_mac(d, cfg->dest_mac, false);
    telemetry_export_dest_commit(d);
    
    hal_cfg = g_hal_state_pd->hal_cfg();
    SDK_ASSERT(hal_cfg);
    // Start timer for the collector, only in HW mode
    d->db_timer = 
        sdk::lib::timer_schedule((HAL_TIMER_ID_IPFIX_MIN + d->id),
                                 (d->export_intvl * TIME_MSECS_PER_SEC),
                                 (void *) 0,
                                 ipfix_doorbell_ring_cb,
                                 true);
    if (!d->db_timer) {
        HAL_TRACE_ERR("Failed to start periodic doorbell ring timer");
        return HAL_RET_ERR;
    }
    HAL_TRACE_DEBUG("Started periodic doorbell ring timer with "
                    "{} ms interval", (d->export_intvl * TIME_MSECS_PER_SEC));
    return HAL_RET_OK;
}

hal_ret_t
pd_collector_delete(pd_func_args_t *pd_func_args)
{
    hal_cfg_t                           *hal_cfg = NULL;
    collector_config_t                  *cfg;
    telemetry_export_dest_t             *d;
    pd_collector_delete_args_t          *c_args;

    c_args = pd_func_args->pd_collector_delete;
    cfg = c_args->cfg;
    HAL_TRACE_DEBUG("{}: CollectorID {}", __FUNCTION__, cfg->collector_id);
    
    if (cfg->collector_id >= (TELEMETRY_NUM_EXPORT_DEST)) {
        HAL_TRACE_ERR(" invalid Id {}", cfg->collector_id );
        return HAL_RET_INVALID_ARG;
    }
    hal_cfg = g_hal_state_pd->hal_cfg();
    SDK_ASSERT(hal_cfg);
    d = &export_destinations[cfg->collector_id];
    if (!d->valid) {
        HAL_TRACE_ERR("Collector does not exist, id {}", cfg->collector_id);
        return HAL_RET_INVALID_ARG;
    } else {
        // Delete the timer which rings the doorbell for export
        // No other cleanup is required. New collector will overwrite the
        // deleted collector based on the valid flag
        if (!d->db_timer) {
            HAL_TRACE_ERR("Timer doesnt exist for this collector id {}", d->id);
            return HAL_RET_ERR;
        }
        HAL_TRACE_DEBUG("Deleting periodic doorbell ring timer for id {}",
                         d->id);
        sdk::lib::timer_delete(d->db_timer);
        d->valid = false;
    }
    return HAL_RET_OK;
}

hal_ret_t
pd_collector_get(pd_func_args_t *pd_func_args)
{
    pd_collector_get_args_t             *c_args;
    collector_config_t                  *cfg;
    telemetry_export_dest_t             *d;

    c_args = pd_func_args->pd_collector_get;
    cfg = c_args->cfg;
    HAL_TRACE_DEBUG("{}: CollectorID {}", __FUNCTION__, cfg->collector_id);

    if (cfg->collector_id >= (TELEMETRY_NUM_EXPORT_DEST)) {
        HAL_TRACE_ERR(" invalid Id {}", cfg->collector_id );
        return HAL_RET_INVALID_ARG;
    }
    d = &export_destinations[cfg->collector_id];
    if (!d->valid) {
        HAL_TRACE_ERR("Collector does not exist: Id {}", cfg->collector_id);
        return HAL_RET_INVALID_ARG;
    }
    cfg->vlan = d->ipfix_hdr.vlan.vlan_tag;
    cfg->dport = ntohs(d->ipfix_hdr.udphdr.dport);
    cfg->template_id = d->template_id;
    cfg->export_intvl = d->export_intvl;
    telemetry_export_dest_get_ip(d, cfg, true);
    telemetry_export_dest_get_ip(d, cfg, false);
    telemetry_export_dest_get_mac(d, cfg, true);
    telemetry_export_dest_get_mac(d, cfg, false);

    return HAL_RET_OK;
}

static hal_ret_t
program_drop_stats_actiondata_table (drop_stats_actiondata_t *data,
                                     uint8_t sessid_bitmap, bool reason, int code)
{
    hal_ret_t   ret = HAL_RET_OK;
    sdk_ret_t   sdk_ret;
    tcam        *tcam;

    tcam = g_hal_state_pd->tcam_table(P4TBL_ID_DROP_STATS);
    SDK_ASSERT(tcam != NULL);

    data->action_u.drop_stats_drop_stats.mirror_en = reason;
    data->action_u.drop_stats_drop_stats.mirror_session_id = sessid_bitmap;
    data->action_id = DROP_STATS_DROP_STATS_ID;
    sdk_ret = tcam->update(code, data);
    if (sdk_ret != sdk::SDK_RET_OK) {
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        HAL_TRACE_ERR("drop stats table write failure, idx : {}, err : {}",
                       code, ret);
        goto end;
    }

end:
    return ret;
}

#define PROGRAM_DROP_STATS_TABLE(arg1, arg2) \
    if (arg1) { \
        ret = program_drop_stats_actiondata_table(&data, sessid_bitmap, arg1, arg2); \
        if (ret != HAL_RET_OK) goto end; \
    }

hal_ret_t
pd_drop_monitor_rule_create(pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret = HAL_RET_OK;
    pd_drop_monitor_rule_create_args_t *args = pd_func_args->pd_drop_monitor_rule_create;
    uint8_t                 sessid_bitmap = 0;
    drop_stats_actiondata_t   data = { 0 };

    for (int i = 0; i < MAX_MIRROR_SESSION_DEST; i++) {
        sessid_bitmap |= args->rule->mirror_destinations[i] ? (1 << i) : 0;
    }
    PROGRAM_DROP_STATS_TABLE(args->rule->codes.drop_malformed_pkt, DROP_MALFORMED_PKT);
    PROGRAM_DROP_STATS_TABLE(args->rule->codes.drop_input_mapping, DROP_INPUT_MAPPING);
    PROGRAM_DROP_STATS_TABLE(args->rule->codes.drop_input_mapping_dejavu, DROP_INPUT_MAPPING_DEJAVU);
    PROGRAM_DROP_STATS_TABLE(args->rule->codes.drop_flow_hit, DROP_FLOW_HIT);
    PROGRAM_DROP_STATS_TABLE(args->rule->codes.drop_flow_miss, DROP_FLOW_MISS);
    PROGRAM_DROP_STATS_TABLE(args->rule->codes.drop_nacl, DROP_NACL);
    PROGRAM_DROP_STATS_TABLE(args->rule->codes.drop_ipsg, DROP_IPSG);
    PROGRAM_DROP_STATS_TABLE(args->rule->codes.drop_ip_normalization, DROP_IP_NORMALIZATION);
    PROGRAM_DROP_STATS_TABLE(args->rule->codes.drop_tcp_normalization, DROP_TCP_NORMALIZATION);
    PROGRAM_DROP_STATS_TABLE(args->rule->codes.drop_tcp_rst_with_invalid_ack_num, DROP_TCP_RST_WITH_INVALID_ACK_NUM);
    PROGRAM_DROP_STATS_TABLE(args->rule->codes.drop_tcp_non_syn_first_pkt, DROP_TCP_NON_SYN_FIRST_PKT);
    PROGRAM_DROP_STATS_TABLE(args->rule->codes.drop_icmp_normalization, DROP_ICMP_NORMALIZATION);
    PROGRAM_DROP_STATS_TABLE(args->rule->codes.drop_input_properties_miss, DROP_INPUT_PROPERTIES_MISS);
    PROGRAM_DROP_STATS_TABLE(args->rule->codes.drop_tcp_out_of_window, DROP_TCP_OUT_OF_WINDOW);
    PROGRAM_DROP_STATS_TABLE(args->rule->codes.drop_tcp_split_handshake, DROP_TCP_SPLIT_HANDSHAKE);
    PROGRAM_DROP_STATS_TABLE(args->rule->codes.drop_tcp_win_zero_drop, DROP_TCP_WIN_ZERO_DROP);
    PROGRAM_DROP_STATS_TABLE(args->rule->codes.drop_tcp_data_after_fin, DROP_TCP_DATA_AFTER_FIN);
    PROGRAM_DROP_STATS_TABLE(args->rule->codes.drop_tcp_non_rst_pkt_after_rst, DROP_TCP_NON_RST_PKT_AFTER_RST);
    PROGRAM_DROP_STATS_TABLE(args->rule->codes.drop_tcp_invalid_responder_first_pkt, DROP_TCP_INVALID_RESPONDER_FIRST_PKT);
    PROGRAM_DROP_STATS_TABLE(args->rule->codes.drop_tcp_unexpected_pkt, DROP_TCP_UNEXPECTED_PKT);
    PROGRAM_DROP_STATS_TABLE(args->rule->codes.drop_src_lif_mismatch, DROP_SRC_LIF_MISMATCH);
    PROGRAM_DROP_STATS_TABLE(args->rule->codes.drop_parser_icrc_error, DROP_PARSER_ICRC_ERR);
    PROGRAM_DROP_STATS_TABLE(args->rule->codes.drop_parse_len_error, DROP_PARSER_LEN_ERR);
    PROGRAM_DROP_STATS_TABLE(args->rule->codes.drop_hardware_error, DROP_HARDWARE_ERR);

end:
    return ret;
}

hal_ret_t
pd_drop_monitor_rule_delete(pd_func_args_t *pd_func_args)
{
    hal_ret_t ret = HAL_RET_OK;

    return ret;
}

hal_ret_t
pd_drop_monitor_rule_get(pd_func_args_t *pd_func_args)
{
    hal_ret_t ret = HAL_RET_OK;

    return ret;
}

}    // namespace pd
}    // namespace hal

