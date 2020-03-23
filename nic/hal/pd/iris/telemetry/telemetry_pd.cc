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
#include "nic/hal/pd/iris/nw/tnnl_rw_pd.hpp"
#include "nic/sdk/lib/pal/pal.hpp"

#define IPFIX_STATS_SHIFT 6

using sdk::lib::indexer;

namespace hal {
extern int telemetry_collector_id_db[HAL_MAX_TELEMETRY_COLLECTORS];
namespace pd {

telemetry_export_dest_t export_destinations[TELEMETRY_NUM_EXPORT_DEST];
char _deb_buf[TELEMETRY_EXPORT_BUFF_SIZE + 1];

hal_ret_t
pd_mirror_update_hw(uint32_t id, mirror_actiondata_t *action_data)
{
    hal_ret_t           ret = HAL_RET_OK;
    sdk_ret_t           sdk_ret;
    directmap           *session = NULL;
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
#if 0
        p4pd_error_t        p4_err;
        p4_err =  p4pd_table_ds_decoded_string_get(P4TBL_ID_MIRROR, 0, NULL, NULL,
                action_data, buff, sizeof(buff));
        SDK_ASSERT(p4_err == P4PD_SUCCESS);
#endif
        HAL_TRACE_DEBUG("{}: programmed session {}: {}",
                __FUNCTION__, id, buff);
    }
    return ret;
}

hal_ret_t
pd_mirror_session_update (pd_func_args_t *pd_func_args)
{
    uint32_t                                dst_lport, tnnl_rw_idx;
    hal_ret_t                               ret = HAL_RET_OK;
    p4pd_error_t                            pdret;
    mirror_actiondata_t                     action_data;
    pd_mirror_session_update_args_t         *args = pd_func_args->pd_mirror_session_update;
    pd_func_args_t                          pd_func_args1 = {0};
    hal::pd::pd_tunnelif_get_rw_idx_args_t  tif_args = { 0 };

    if ((args == NULL) || (args->session == NULL) || (args->session->pd == NULL)) {
        HAL_TRACE_ERR(" NULL argument");
        return HAL_RET_INVALID_ARG;
    }

    HAL_TRACE_DEBUG("dst_if: {}, tnnl_if: {}, rtep_ep: {}",
                    args->dst_if ? if_keyhandle_to_str(args->dst_if) : "NULL",
                    args->tunnel_if ? if_keyhandle_to_str(args->tunnel_if) : "NULL",
                    args->rtep_ep ? ep_l2_key_to_str(args->rtep_ep) : "NULL");

    mirror_session_pd_t *session_pd = (mirror_session_pd_t *)args->session->pd;
    auto hw_id = session_pd->hw_id;

    SDK_ASSERT(hw_id < MAX_MIRROR_SESSION_DEST);

    HAL_TRACE_DEBUG("Update call for session {} with hw_id: {}", 
                    args->session->sw_id, hw_id);
    pdret = p4pd_entry_read(P4TBL_ID_MIRROR, hw_id, NULL,
                            NULL, (void *)&action_data);
    if (pdret != P4PD_SUCCESS) {
        HAL_TRACE_ERR("Session id {} read from hw id {} failed {}",
                      args->session->sw_id, hw_id,
                      pdret);
        return HAL_RET_ERR;
    }

    if (args->dst_if) {
        dst_lport = if_get_lport_id(args->dst_if);
    } else {
        dst_lport = 0;
    }
    if (args->tunnel_if) {
        tif_args.hal_if = args->tunnel_if;
        pd_func_args1.pd_tunnelif_get_rw_idx = &tif_args;
        hal::pd::pd_tunnelif_get_rw_idx(&pd_func_args1);
        tnnl_rw_idx = tif_args.tnnl_rw_idx;
    } else {
        // Tunnel If not present.
        dst_lport = 0;
        tnnl_rw_idx = 0;
    }
    if (!args->rtep_ep) {
        dst_lport = 0;
    }
    action_data.action_u.mirror_erspan_mirror.dst_lport = dst_lport;
    action_data.action_u.mirror_erspan_mirror.tunnel_rewrite_index = tnnl_rw_idx;

    // Update the dest_if
    if (action_data.action_id == MIRROR_ERSPAN_MIRROR_ID) {
        ret = pd_mirror_update_hw(hw_id, &action_data);
    }

    return ret;
}

#if 0
hal_ret_t
pd_mirror_session_update (pd_func_args_t *pd_func_args)
{
    uint32_t                                dst_lport;
    hal_ret_t                               ret = HAL_RET_OK;
    p4pd_error_t                            pdret;
    mirror_actiondata_t                     action_data;
    pd_mirror_session_update_args_t         *args = pd_func_args->pd_mirror_session_update;
    pd_func_args_t                          pd_func_args1 = {0};
    hal::pd::pd_tunnelif_get_rw_idx_args_t  tif_args = { 0 };

    if ((args == NULL) || (args->session == NULL) || (args->session->pd == NULL)) {
        HAL_TRACE_ERR(" NULL argument");
        return HAL_RET_INVALID_ARG;
    }

    HAL_TRACE_DEBUG("dst_if_change: {}, dst_if: {}, tunnel_if_change: {}, tnnl_if: {}, "
                    "rtep_ep_exists: {}",
                    args->dst_if_change, 
                    args->dst_if ? if_keyhandle_to_str(args->dst_if) : "NULL",
                    args->tunnel_if_change,
                    args->tunnel_if ? if_keyhandle_to_str(args->tunnel_if) : "NULL",
                    args->session->rtep_ep_exists);

    mirror_session_pd_t *session_pd = (mirror_session_pd_t *)args->session->pd;
    auto hw_id = session_pd->hw_id;

    SDK_ASSERT(hw_id < MAX_MIRROR_SESSION_DEST);

    HAL_TRACE_DEBUG("Update call for session {} with hw_id: {}", 
                    args->session->sw_id, hw_id);
    pdret = p4pd_entry_read(P4TBL_ID_MIRROR, hw_id, NULL,
                            NULL, (void *)&action_data);
    if (pdret != P4PD_SUCCESS) {
        HAL_TRACE_ERR("Session id {} read from hw id {} failed {}",
                      args->session->sw_id, hw_id,
                      pdret);
        return HAL_RET_ERR;
    }

    if (args->dst_if_change) {
        if (args->dst_if) {
            dst_lport = if_get_lport_id(args->dst_if);
        } else {
            dst_lport = 0;
        } 
        action_data.action_u.mirror_erspan_mirror.dst_lport = dst_lport;
    }
    if (args->tunnel_if_change) {
        if (args->tunnel_if) {
            tif_args.hal_if = args->tunnel_if;
            pd_func_args1.pd_tunnelif_get_rw_idx = &tif_args;
            hal::pd::pd_tunnelif_get_rw_idx(&pd_func_args1);
            action_data.action_u.mirror_erspan_mirror.tunnel_rewrite_index =
                tif_args.tnnl_rw_idx;
        } else {
            action_data.action_u.mirror_erspan_mirror.dst_lport = 0;
            action_data.action_u.mirror_erspan_mirror.tunnel_rewrite_index = 0;
        }
    }
    if (!args->session->rtep_ep_exists) {
        action_data.action_u.mirror_erspan_mirror.dst_lport = 0;
    }

    // Update the dest_if
    if (action_data.action_id == MIRROR_ERSPAN_MIRROR_ID) {
        ret = pd_mirror_update_hw(hw_id, &action_data);
    }

    return ret;
}
#endif

hal_ret_t
pd_mirror_session_create (pd_func_args_t *pd_func_args)
{
    uint32_t dst_lport;
    pd_mirror_session_create_args_t *args = pd_func_args->pd_mirror_session_create;
    mirror_actiondata_t action_data;
    hal::pd::pd_tunnelif_get_rw_idx_args_t    tif_args = { 0 };
    pd_func_args_t pd_func_args1 = {0};
    mirror_session_pd_t *session_pd;
    uint32_t hw_id;

    if ((args == NULL) || (args->session == NULL)) {
        HAL_TRACE_ERR(" NULL argument");
        return HAL_RET_INVALID_ARG;
    }

    SDK_ASSERT(args->session->pd == NULL);

    HAL_TRACE_DEBUG("Create call for mirror session {}",
                    args->session->sw_id);

    // Add to a PD datastructure instead of stack.
    memset(&action_data, 0, sizeof(mirror_actiondata_t));

    if (args->session->dest_if) {
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
        if (likely(is_platform_type_hw())) {
            action_data.action_u.mirror_erspan_mirror.gre_seq_en = 1;
        }
        action_data.action_u.mirror_erspan_mirror.truncate_len = args->session->truncate_len;
        if (args->dst_if) {
            dst_lport = if_get_lport_id(args->dst_if);
        } else {
            dst_lport = 0;
        }
        if (args->tunnel_if) {
            tif_args.hal_if = args->tunnel_if;
            pd_func_args1.pd_tunnelif_get_rw_idx = &tif_args;
            hal::pd::pd_tunnelif_get_rw_idx(&pd_func_args1);
            action_data.action_u.mirror_erspan_mirror.tunnel_rewrite_index =
                tif_args.tnnl_rw_idx;
        } else {
            // Tunnel If not present.
            dst_lport = 0;
        }
        if (!args->rtep_ep) {
            dst_lport = 0;
        }
        action_data.action_u.mirror_erspan_mirror.dst_lport = dst_lport;
        break;
    }
    default:
        HAL_TRACE_ERR("unknown session type {}", args->session->type);
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

    // alloc mirror session hw index
    auto status = g_hal_state_pd->mirror_session_idxr()->alloc(&hw_id);
    if (status == indexer::INDEXER_FULL) {
        HAL_TRACE_ERR("Mirror session ID {} hw id alloc failed. "
                      "Mirror session indexer is full",
                      args->session->sw_id);
        return HAL_RET_TABLE_FULL;
    } else if (status != indexer::SUCCESS) {
        HAL_TRACE_ERR("Mirror session ID {} hw id alloc failed {}",
                      args->session->sw_id, status);
        return HAL_RET_ERR;
    }
    HAL_TRACE_DEBUG("Mirror session ID {} allocated hw id {}",
                    args->session->sw_id, hw_id);
    SDK_ASSERT(hw_id < MAX_MIRROR_SESSION_DEST);

    session_pd = (mirror_session_pd_t*)g_hal_state_pd->mirror_session_pd_slab()->alloc();
    SDK_ASSERT(session_pd != NULL);
    // program the hw
    auto ret = pd_mirror_update_hw(hw_id, &action_data);
    if (ret != HAL_RET_OK) {
        g_hal_state_pd->mirror_session_idxr()->free(hw_id);
        g_hal_state_pd->mirror_session_pd_slab()->free(session_pd);
        return ret;
    } else {
        session_pd->hw_id = hw_id;
        args->session->pd = session_pd;
        args->hw_id = hw_id;
    }
    return ret;
}

hal_ret_t
pd_mirror_session_delete(pd_func_args_t *pd_func_args)
{
    mirror_actiondata_t action_data;
    pd_mirror_session_delete_args_t *args = pd_func_args->pd_mirror_session_delete;
    if ((args == NULL) || (args->session == NULL) || (args->session->pd == NULL)) {
        HAL_TRACE_ERR("NULL argument");
        return HAL_RET_INVALID_ARG;
    }
    mirror_session_pd_t *session_pd = (mirror_session_pd_t *)args->session->pd;
    auto hw_id = session_pd->hw_id;

    HAL_TRACE_DEBUG("Delete call for mirror session ID {}",
                    args->session->sw_id);

    memset(&action_data, 0, sizeof(mirror_actiondata_t));
    SDK_ASSERT(hw_id < MAX_MIRROR_SESSION_DEST);
    action_data.action_id = MIRROR_DROP_MIRROR_ID;

    auto ret = pd_mirror_update_hw(hw_id, &action_data);
    if (ret == HAL_RET_OK) {
        HAL_TRACE_DEBUG("Mirror session ID {} freed hw id {}",
                        args->session->sw_id, hw_id);
        g_hal_state_pd->mirror_session_idxr()->free(hw_id);
        g_hal_state_pd->mirror_session_pd_slab()->free(args->session->pd);
        args->session->pd = NULL;
    }
    return ret;
}

hal_ret_t
pd_mirror_session_get_hw_id (pd_func_args_t *pd_func_args)
{
    pd_mirror_session_get_hw_id_args_t *args =
        pd_func_args->pd_mirror_session_get_hw_id;
    if ((args == NULL) || (args->session == NULL) ||
        (args->session->pd == NULL)) {
        HAL_TRACE_ERR("NULL argument");
        return HAL_RET_INVALID_ARG;
    }

    mirror_session_pd_t *session_pd =
        (mirror_session_pd_t *)args->session->pd;
    args->hw_id = session_pd->hw_id;
    return HAL_RET_OK;
}

hal_ret_t
telemetry_export_dest_init(telemetry_export_dest_t *d)
{
    HAL_TRACE_DEBUG("{}: Export Destination Init {}", __FUNCTION__, d->id);
    uint64_t hbm_start = get_mem_addr(JP4_IPFIX);
    d->skip_doorbell = false;
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
pd_collector_ep_update (pd_func_args_t *pd_func_args)
{
    hal_ret_t                        ret = HAL_RET_OK;
    pd_collector_ep_update_args_t    *args = pd_func_args->pd_collector_ep_update;
    ep_t                             *ep;
    telemetry_export_dest_t          *dst;
    collector_config_t               cfg;
    bool                             skip_doorbell = false;
    mac_addr_t                       *dmac = NULL;

    ep = args->ep;
    if (ep) {
        dmac = ep_get_mac_addr(ep);
    } else {
        skip_doorbell = true;
    }

    for (int i = 0; i < HAL_MAX_TELEMETRY_COLLECTORS; i++) {
        if (telemetry_collector_id_db[i] > 0) {
            dst = &export_destinations[i];
            telemetry_export_dest_get_ip(dst, &cfg, false);
            HAL_TRACE_DEBUG("Processing collector: {}, EP: {}", 
                            ipaddr2str(&cfg.dst_ip), ipaddr2str(args->ip));
            if (!memcmp(&cfg.dst_ip, args->ip, sizeof(ip_addr_t))) {
                HAL_TRACE_DEBUG("Updating collector: {}, skip_doorbell: {}", 
                                ipaddr2str(args->ip), skip_doorbell);
                dst->skip_doorbell = skip_doorbell;
                if (!skip_doorbell) {
                    telemetry_export_dest_set_mac(dst, *dmac, false);
                    telemetry_export_dest_commit(dst);
                }
            }
        }
    }

    return ret;
}

hal_ret_t
pd_collector_populate_export_info (collector_config_t *cfg, 
                                   ep_t *ep, 
                                   telemetry_export_dest_t *dst)
{
    hal_ret_t ret = HAL_RET_OK;
    mac_addr_t *dmac = NULL;

    dst->ipfix_hdr.vlan.vlan_tag = htons(cfg->vlan);
    dst->ipfix_hdr.iphdr.tos = 0;
    dst->ipfix_hdr.iphdr.ttl = 64;
    // Total len will be updated correctly in the P4 datapath
    dst->ipfix_hdr.iphdr.tot_len = htons(0xFFFF);
    dst->ipfix_hdr.udphdr.sport = htons(UDP_SRC_PORT_TELEMETRY);
    dst->ipfix_hdr.udphdr.dport = htons(cfg->dport);
    dst->template_id = cfg->template_id;
    dst->export_intvl = cfg->export_intvl;
    dst->valid = true;
    if (ep) {
        dmac = ep_get_mac_addr(ep);
    } else {
        dst->skip_doorbell = true;
    }
    
    telemetry_export_dest_set_ip(dst, cfg->src_ip, true);
    telemetry_export_dest_set_ip(dst, cfg->dst_ip, false);
    telemetry_export_dest_set_mac(dst, cfg->src_mac, true);
    if (dmac) {
        telemetry_export_dest_set_mac(dst, *dmac, false);
    }
    telemetry_export_dest_commit(dst);

    return ret;
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

    pd_collector_populate_export_info(cfg, c_args->ep, d);

#if 0
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
#endif
    
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
    collector_config_t                  *cfg = NULL;
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

    sdk::types::mem_addr_t vaddr;
    sdk::types::mem_addr_t stats_mem_addr = 
                        get_mem_addr(CAPRI_HBM_REG_IPFIX_STATS);
    SDK_ASSERT(stats_mem_addr != INVALID_MEM_ADDRESS);

    stats_mem_addr += ((1 << IPFIX_STATS_SHIFT)*cfg->collector_id);
    sdk::lib::pal_ret_t ret = sdk::lib::pal_physical_addr_to_virtual_addr(stats_mem_addr, &vaddr);
    SDK_ASSERT(ret == sdk::lib::PAL_RET_OK);
    c_args->stats = (collector_stats_t *)vaddr;

    return HAL_RET_OK;
}

#define	lif_info   data.action_u.lif_lif_info
#define om_tmoport data.action_u.output_mapping_set_tm_oport_enforce_src_lport
hal_ret_t
pd_uplink_erspan_enable (pd_func_args_t *pd_func_args)
{
    pd_uplink_erspan_enable_args_t *args = pd_func_args->
                                           pd_uplink_erspan_enable;
    hal_ret_t                       ret = HAL_RET_OK;
    p4pd_error_t                    pdret;
    sdk_ret_t                       sdk_ret;
    directmap                       *lif_table = NULL;
    uint32_t                        hw_lif_id;
    uint8_t                         sessid_bitmap = 0;

    // Retrieve If hw_lif_id
    hw_lif_id = if_get_hw_lif_id(args->if_p);

    if (args->if_p->direction == UPLINK_ERSPAN_DIRECTION_INGRESS) {
        lif_actiondata_t data;

        for (int i = 0; i < args->if_p->mirror_cfg.rx_sessions_count; i++) {
            sessid_bitmap |= (1 << args->if_p->rx_mirror_session_id[i]);
        }

        lif_table = g_hal_state_pd->dm_table(P4TBL_ID_LIF);
        SDK_ASSERT_RETURN((lif_table != NULL), HAL_RET_ERR);

        memset(&data, 0, sizeof(data));
        data.action_id = LIF_LIF_INFO_ID;
        lif_info.ingress_mirror_en = 1;
        lif_info.ingress_mirror_session_id = sessid_bitmap;
        if (args->if_p->programmed_rx_session_id_bitmap == 0) {
            // Entry-not-present case
            HAL_TRACE_DEBUG("LIF-Table Entry Not present {}", hw_lif_id);
            sdk_ret = lif_table->insert_withid(&data, hw_lif_id);
        }
        else {
            // Entry present case
            HAL_TRACE_DEBUG("LIF-Table Entry present {}", hw_lif_id);
            sdk_ret = lif_table->update(hw_lif_id, &data);
        }
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("unable to program for lif_id: {} {} {}",
                          hw_lif_id, sdk_ret, ret);
            return ret;
        } else {
            HAL_TRACE_DEBUG("programmed lif_id: {}", hw_lif_id);
        }
        args->if_p->programmed_rx_session_id_bitmap = sessid_bitmap;
    }

    if (args->if_p->direction == UPLINK_ERSPAN_DIRECTION_EGRESS) {
        directmap                   *omap_table = NULL;
        output_mapping_actiondata_t data;
        uint32_t                    lport_id;

        // Retrieve If lport_id
        lport_id = if_get_lport_id(args->if_p);

        for (int i = 0; i < args->if_p->mirror_cfg.tx_sessions_count; i++) {
            sessid_bitmap |= (1 << args->if_p->tx_mirror_session_id[i]);
        }

        omap_table = g_hal_state_pd->dm_table(P4TBL_ID_OUTPUT_MAPPING);
        SDK_ASSERT_RETURN((omap_table != NULL), HAL_RET_ERR);

        pdret = p4pd_entry_read(P4TBL_ID_OUTPUT_MAPPING, lport_id, 
                                NULL, NULL, (void *) &data);
        if (pdret != P4PD_SUCCESS) {
            // Entry-not found case
            HAL_TRACE_ERR("OMAP Entry not present {}", lport_id);
            ret = HAL_RET_ERR;
        }
        else {
            // Entry found case
            if (data.action_id != OUTPUT_MAPPING_SET_TM_OPORT_ID &&
                data.action_id != 
                OUTPUT_MAPPING_SET_TM_OPORT_ENFORCE_SRC_LPORT_ID) {
                HAL_TRACE_ERR(
                "OMAP Entry present with conflicting action {} {}",
                 lport_id, data.action_id);
                ret = HAL_RET_ERR;
                goto end;
            }

            if (om_tmoport.dst_lif != hw_lif_id) {
                HAL_TRACE_ERR(
                "OMAP Entry present with conflicting dst_lif {} {} {}",
                 lport_id, om_tmoport.dst_lif, hw_lif_id);
                ret = HAL_RET_ERR;
                goto end;
            }

            om_tmoport.mirror_en = 1;
            om_tmoport.mirror_session_id = sessid_bitmap;
            sdk_ret = omap_table->update(lport_id, &data);
            ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        }

end:
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("unable to program for lport_id: {} {} {}",
                          lport_id, sdk_ret, ret);
            return ret;
        } else {
            HAL_TRACE_DEBUG("programmed lport_id: {}", lport_id);
        }
        args->if_p->programmed_tx_session_id_bitmap = sessid_bitmap;
    }

    return ret;
}

hal_ret_t
pd_uplink_erspan_disable (pd_func_args_t *pd_func_args)
{
    pd_uplink_erspan_disable_args_t *args = pd_func_args->
                                            pd_uplink_erspan_disable;
    hal_ret_t                        ret = HAL_RET_OK;
    sdk_ret_t                        sdk_ret;

    if (args->if_p->direction == UPLINK_ERSPAN_DIRECTION_INGRESS) {
        directmap *lif_table;
        uint32_t  hw_lif_id;

        // Retrieve If hw_lif_id
        hw_lif_id = if_get_hw_lif_id(args->if_p);

        lif_table = g_hal_state_pd->dm_table(P4TBL_ID_LIF);
        SDK_ASSERT_RETURN((lif_table != NULL), HAL_RET_ERR);

        sdk_ret = lif_table->remove(hw_lif_id);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("unable to deprogram for lif_id: {} {} {}",
                          hw_lif_id, sdk_ret, ret);
            return ret;
        } else {
            HAL_TRACE_DEBUG("deprogrammed lif_id: {}", hw_lif_id);
        }
        args->if_p->programmed_rx_session_id_bitmap = 0;
    }

    if (args->if_p->direction == UPLINK_ERSPAN_DIRECTION_EGRESS) {
        directmap                   *omap_table = NULL;
        output_mapping_actiondata_t  data;
        p4pd_error_t                 pdret;
        uint32_t                    lport_id;

        // Retrieve If lport_id
        lport_id = if_get_lport_id(args->if_p);


        omap_table = g_hal_state_pd->dm_table(P4TBL_ID_OUTPUT_MAPPING);
        SDK_ASSERT_RETURN((omap_table != NULL), HAL_RET_ERR);

        pdret = p4pd_entry_read(P4TBL_ID_OUTPUT_MAPPING, lport_id, 
                                NULL, NULL, (void *) &data);
        if (pdret != P4PD_SUCCESS) {
            HAL_TRACE_ERR("unable to read OMAP for lif_id: {} {}",
                          lport_id, pdret);
            return HAL_RET_ERR;
        }

        if (data.action_id != OUTPUT_MAPPING_SET_TM_OPORT_ID &&
            data.action_id != 
            OUTPUT_MAPPING_SET_TM_OPORT_ENFORCE_SRC_LPORT_ID) {
            HAL_TRACE_ERR(
            "OMAP Entry present with conflicting action {} {}",
             lport_id, data.action_id);
            return HAL_RET_ERR;
        }

        om_tmoport.mirror_en = 0;
        om_tmoport.mirror_session_id = 0;
        sdk_ret = omap_table->update(lport_id, &data);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("unable to deprogram for lport_id: {} {} {}",
                          lport_id, sdk_ret, ret);
            return ret;
        } else {
            HAL_TRACE_DEBUG("deprogrammed lif_id: {}", lport_id);
        }
        args->if_p->programmed_tx_session_id_bitmap = 0;
    }

    return ret;
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
    PROGRAM_DROP_STATS_TABLE(args->rule->codes.drop_icmp_frag_pkt, DROP_ICMP_FRAGMENT_PKT);

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

