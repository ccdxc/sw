// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/hal_lock.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/iris/lif/lif_pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/include/interface_api.hpp"
#include "nic/hal/pd/iris/nw/if_pd_utils.hpp"
#include "nic/gen/iris/include/p4pd.h"
#include "nic/hal/pd/p4pd_api.hpp"
#include "nic/hal/pd/iris/telemetry/telemetry_pd.hpp"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/hal/pd/iris/internal/p4plus_pd_api.h"
#include "nic/hal/pd/iris/telemetry/ipfix_pd.hpp"

namespace hal {
namespace pd {

hal_ret_t
pd_mirror_update_hw(uint32_t id, mirror_actiondata *action_data)
{
    hal_ret_t       ret = HAL_RET_OK;
    sdk_ret_t       sdk_ret;
    directmap       *session = NULL;
    p4pd_error_t    p4_err;
    char            buff[4096] = {0};

    session = g_hal_state_pd->dm_table(P4TBL_ID_MIRROR);
    HAL_ASSERT_RETURN((session != NULL), HAL_RET_ERR);

    sdk_ret = session->update(id, action_data);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}: programming sesion {} failed ({})",
                __FUNCTION__, id, ret);
    } else {
        p4_err =  p4pd_table_ds_decoded_string_get(P4TBL_ID_MIRROR, 0, NULL, NULL,
                action_data, buff, sizeof(buff));
        HAL_ASSERT(p4_err == P4PD_SUCCESS);
        HAL_TRACE_DEBUG("{}: programmed session {}: {}",
                __FUNCTION__, id, buff);
    }
    return ret;
}


hal_ret_t
pd_mirror_session_create(pd_mirror_session_create_args_t *args)
{
    uint32_t dst_lport;
    mirror_actiondata action_data;
    hal::pd::pd_tunnelif_get_rw_idx_args_t    tif_args = { 0 };

    if ((args == NULL) || (args->session == NULL)) {
        HAL_TRACE_ERR(" NULL argument");
        return HAL_RET_INVALID_ARG;
    }

    HAL_TRACE_DEBUG("{}: Create call for session {}",
            __FUNCTION__, args->session->id);

    // Add to a PD datastructure instead of stack.
    memset(&action_data, 0, sizeof(mirror_actiondata));
    HAL_ASSERT((args->session->id >= 0) && (args->session->id <= 7));

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
        action_data.actionid = MIRROR_LOCAL_SPAN_ID;
        action_data.mirror_action_u.mirror_local_span.truncate_len = args->session->truncate_len;
        action_data.mirror_action_u.mirror_local_span.dst_lport = dst_lport;
        break;
    }
    case MIRROR_DEST_RSPAN: {
        action_data.actionid = MIRROR_REMOTE_SPAN_ID;
        action_data.mirror_action_u.mirror_remote_span.truncate_len = args->session->truncate_len;
        action_data.mirror_action_u.mirror_remote_span.dst_lport = dst_lport;
        action_data.mirror_action_u.mirror_remote_span.vlan = args->session->mirror_destination_u.r_span_dest.vlan;
        action_data.mirror_action_u.mirror_remote_span.tunnel_rewrite_index = g_hal_state_pd->tnnl_rwr_tbl_encap_vlan_idx();
        break;
    }
    case MIRROR_DEST_ERSPAN: {
        action_data.actionid = MIRROR_ERSPAN_MIRROR_ID;
        action_data.mirror_action_u.mirror_erspan_mirror.truncate_len = args->session->truncate_len;
        action_data.mirror_action_u.mirror_erspan_mirror.dst_lport = dst_lport;
        tif_args.hal_if = args->session->mirror_destination_u.er_span_dest.tunnel_if;
        hal::pd::pd_tunnelif_get_rw_idx(&tif_args);
        action_data.mirror_action_u.mirror_erspan_mirror.tunnel_rewrite_index =
            tif_args.tnnl_rw_idx;
        break;
    }
    default:
        HAL_TRACE_ERR(" unknown session type {}", args->session->type);
        return HAL_RET_INVALID_ARG;
    }

    return pd_mirror_update_hw(args->session->id, &action_data);
}

hal_ret_t
pd_mirror_session_delete(pd_mirror_session_delete_args_t *args)
{
    mirror_actiondata action_data;
    if ((args == NULL) || (args->session == NULL)) {
        HAL_TRACE_ERR("NULL argument");
        return HAL_RET_INVALID_ARG;
    }
    memset(&action_data, 0, sizeof(mirror_actiondata));
    HAL_ASSERT((args->session->id >= 0) && (args->session->id <= 7));
    action_data.actionid = MIRROR_DROP_MIRROR_ID;

    return pd_mirror_update_hw(args->session->id, &action_data);
}

hal_ret_t
pd_mirror_session_get(pd_mirror_session_get_args_t *args)
{
    mirror_actiondata action_data;
    if ((args == NULL) || (args->session == NULL)) {
        HAL_TRACE_ERR("NULL argument");
        return HAL_RET_INVALID_ARG;
    }
    memset(&action_data, 0, sizeof(mirror_actiondata));
    HAL_ASSERT((args->session->id >= 0) && (args->session->id <= 7));

    p4pd_error_t pdret;
    pdret = p4pd_entry_read(P4TBL_ID_MIRROR, args->session->id, NULL, NULL, (void *)&action_data);
    if (pdret == P4PD_SUCCESS) {
        switch (action_data.actionid) {
        case MIRROR_LOCAL_SPAN_ID:
            args->session->type = MIRROR_DEST_LOCAL;
            args->session->truncate_len = action_data.mirror_action_u.mirror_local_span.truncate_len;
            // args-> dst_if // TBD
        case MIRROR_REMOTE_SPAN_ID:
            args->session->type = MIRROR_DEST_RSPAN;
            args->session->truncate_len = action_data.mirror_action_u.mirror_remote_span.truncate_len;
            args->session->mirror_destination_u.r_span_dest.vlan = action_data.mirror_action_u.mirror_remote_span.vlan;
        case MIRROR_ERSPAN_MIRROR_ID:
            args->session->type = MIRROR_DEST_ERSPAN;
            args->session->truncate_len = action_data.mirror_action_u.mirror_erspan_mirror.truncate_len;
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

#define IPFIX_BUFSIZE 2048
#define IPFIX_HBM_MEMSIZE (64 * 1024)
telemetry_export_dest *_export_destinations[IPFIX_HBM_MEMSIZE/IPFIX_BUFSIZE];
hal_ret_t
// pd_collector_create(collector_config_t *cfg)
pd_collector_create(pd_collector_create_args_t *c_args)
{
    collector_config_t *cfg = c_args->cfg;
    pd_l2seg_get_fromcpu_vlanid_args_t args;
    HAL_TRACE_DEBUG("{}: ExportID {}", __FUNCTION__,
    cfg->exporter_id);
    // Id is less than max size allows.
    if (cfg->exporter_id >= (IPFIX_HBM_MEMSIZE/IPFIX_BUFSIZE)) {
        HAL_TRACE_ERR(" invalid Id {}", cfg->exporter_id );
        return HAL_RET_INVALID_ARG;
    }

    telemetry_export_dest *d = _export_destinations[cfg->exporter_id];
    if (d != NULL) {
        HAL_TRACE_ERR(" Already exists Id {}", cfg->exporter_id );
        return HAL_RET_INVALID_ARG;
    }
    d = new(telemetry_export_dest);
    _export_destinations[cfg->exporter_id] = d;
    d->init(cfg->exporter_id);

    args.l2seg = cfg->l2seg;
    args.vid = &cfg->vlan;
    // if (pd_l2seg_get_fromcpu_vlanid(cfg->l2seg, &cfg->vlan) != HAL_RET_OK)
    if (hal_pd_call(hal::pd::PD_FUNC_ID_L2SEG_GET_FRCPU_VLANID, (void *)&args) != HAL_RET_OK) {
        HAL_TRACE_DEBUG("{}: Could not retrieve CPU VLAN", __FUNCTION__);
        return HAL_RET_INVALID_ARG;
    }
    HAL_TRACE_DEBUG("{}: CPU VLAN {}", __FUNCTION__, cfg->vlan);

    d->set_vlan(cfg->vlan);
    d->set_dscp(0);
    d->set_ttl(64);
    d->set_src_ip(cfg->src_ip);
    d->set_dst_ip(cfg->dst_ip);
    d->set_sport(32007);
    d->set_dport(cfg->dport);
    d->set_dst_mac(cfg->dest_mac);
    d->set_src_mac(cfg->src_mac);
    d->commit();
    return HAL_RET_OK;
}

hal_ret_t
telemetry_export_dest::init(uint16_t id)
{
    HAL_TRACE_DEBUG("{}: Export Destination Init {}", __FUNCTION__, id);
    uint64_t hbm_start = get_start_offset(JP4_IPFIX);
    base_addr_ = hbm_start + (id * IPFIX_BUFSIZE);
    buf_hdr_.packet_start = sizeof(telemetry_pd_export_buf_header_t);
    buf_hdr_.payload_start = sizeof(telemetry_pd_export_buf_header_t) + sizeof(telemetry_pd_ipfix_header_t);
    buf_hdr_.payload_length = IPFIX_BUFSIZE - buf_hdr_.payload_start;
    buf_hdr_.ip_hdr_start = sizeof(telemetry_pd_export_buf_header_t) + offsetof(telemetry_pd_ipfix_header_t, iphdr);
    memset(&ipfix_hdr_, 0, sizeof(telemetry_pd_ipfix_header_t));
    ipfix_hdr_.vlan.tpid = htons(0x8100);
    ipfix_hdr_.vlan.etype = htons(0x0800);
    ipfix_hdr_.iphdr.version = 4;
    ipfix_hdr_.iphdr.ihl = 5;
    ipfix_hdr_.iphdr.protocol = 17;
    ipfix_init(id, base_addr_ + buf_hdr_.packet_start,
               sizeof(telemetry_pd_ipfix_header_t), buf_hdr_.payload_length);
    HAL_TRACE_DEBUG("{}: Export Destination Init Done {}", __FUNCTION__, id);
    return HAL_RET_OK;
}

hal_ret_t
telemetry_export_dest::set_src_mac(mac_addr_t in)
{
    memcpy(ipfix_hdr_.vlan.smac, in, sizeof(uint8_t) * ETH_ADDR_LEN);
    return HAL_RET_OK;
}

hal_ret_t
telemetry_export_dest::set_dst_mac(mac_addr_t in)
{
    memcpy(ipfix_hdr_.vlan.dmac, in, sizeof(uint8_t) * ETH_ADDR_LEN);
    return HAL_RET_OK;
}

hal_ret_t
telemetry_export_dest::set_vlan(uint16_t in)
{
    ipfix_hdr_.vlan.vlan_tag = htons(in);
    return HAL_RET_OK;
}

hal_ret_t
telemetry_export_dest::set_dscp(uint8_t in)
{
    ipfix_hdr_.iphdr.tos = in;
    return HAL_RET_OK;
}

hal_ret_t
telemetry_export_dest::set_ttl(uint8_t in)
{
    ipfix_hdr_.iphdr.ttl = in;
    return HAL_RET_OK;
}

hal_ret_t
telemetry_export_dest::set_src_ip(ip_addr_t in)
{
    if (in.af != IP_AF_IPV4) {
        HAL_TRACE_ERR("Non IPV4 source");
        return HAL_RET_INVALID_OP;
    }
    ipfix_hdr_.iphdr.saddr = htonl(in.addr.v4_addr);
    return HAL_RET_OK;
}

hal_ret_t
telemetry_export_dest::set_dst_ip(ip_addr_t in)
{
    if (in.af != IP_AF_IPV4) {
        HAL_TRACE_ERR("Non IPV4 destination");
        return HAL_RET_INVALID_OP;
    }
    ipfix_hdr_.iphdr.daddr = htonl(in.addr.v4_addr);
    return HAL_RET_OK;
}

hal_ret_t
telemetry_export_dest::set_sport(uint16_t in)
{
    ipfix_hdr_.udphdr.sport = htons(in);
    return HAL_RET_OK;
}

hal_ret_t
telemetry_export_dest::set_dport(uint16_t in)
{
    ipfix_hdr_.udphdr.dport = htons(in);
    return HAL_RET_OK;
}

uint16_t
telemetry_export_dest::get_exporter_id()
{
    return id_;
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

char _deb_buf[2048];
hal_ret_t
telemetry_export_dest::commit()
{
    HAL_TRACE_DEBUG("{}: Export Destination commit {}-> {}", __FUNCTION__, id_, base_addr_);
    p4plus_hbm_write(base_addr_, (uint8_t*)&buf_hdr_, sizeof(buf_hdr_),
            P4PLUS_CACHE_ACTION_NONE);
    print_buffer(_deb_buf, 2047, (uint8_t*)&buf_hdr_, sizeof(buf_hdr_));
    HAL_TRACE_DEBUG("{} : Buffer Header: Wrote: {}", __FUNCTION__, _deb_buf);
    // memcpy(base_addr_, &buf_hdr_, sizeof(buf_hdr_));
    uint64_t hdr = base_addr_ + sizeof(buf_hdr_);
    p4plus_hbm_write(hdr, (uint8_t*)&ipfix_hdr_, sizeof(ipfix_hdr_),
            P4PLUS_CACHE_ACTION_NONE);
    print_buffer(_deb_buf, 2047, (uint8_t*)&ipfix_hdr_, sizeof(ipfix_hdr_));
    HAL_TRACE_DEBUG("{} : IPFIX-Header: Wrote: {}", __FUNCTION__, _deb_buf);
    return HAL_RET_OK;
}

hal_ret_t
pd_flow_monitor_rule_create(pd_flow_monitor_rule_create_args_t *args)
{
    // TODO: Add rules to itree
    hal_ret_t ret = HAL_RET_OK;

    return ret;
}

hal_ret_t
pd_flow_monitor_rule_delete(pd_flow_monitor_rule_delete_args_t *args)
{
    hal_ret_t ret = HAL_RET_OK;

    return ret;
}

hal_ret_t
pd_flow_monitor_rule_get(pd_flow_monitor_rule_get_args_t *args)
{
    hal_ret_t ret = HAL_RET_OK;

    return ret;
}

hal_ret_t
pd_drop_monitor_rule_create(pd_drop_monitor_rule_create_args_t *args)
{
    // TODO: Add rules to itree
    hal_ret_t ret = HAL_RET_OK;

    return ret;
}

hal_ret_t
pd_drop_monitor_rule_delete(pd_drop_monitor_rule_delete_args_t *args)
{
    hal_ret_t ret = HAL_RET_OK;

    return ret;
}

hal_ret_t
pd_drop_monitor_rule_get(pd_drop_monitor_rule_get_args_t *args)
{
    hal_ret_t ret = HAL_RET_OK;

    return ret;
}

}    // namespace pd
}    // namespace hal

