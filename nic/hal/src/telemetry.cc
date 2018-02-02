#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/src/interface.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/src/telemetry.hpp"
#include "nic/gen/proto/hal/telemetry.pb.h"
// #include "nic/hal/svc/telemetry_svc.hpp"
#include "nic/gen/proto/hal/types.pb.h"

using telemetry::MirrorSession;
using telemetry::MirrorSessionStatus;
using telemetry::MirrorSessionSpec;
using telemetry::MirrorSessionId;
using hal::pd::pd_mirror_session_create_args_t;
using hal::pd::pd_mirror_session_delete_args_t;
using hal::pd::pd_mirror_session_get_args_t;
using hal::mirror_session_t;

namespace hal {

typedef struct flow_monitor_spec_ {
    mac_addr_t src_mac;
    bool src_mac_valid;
    mac_addr_t dst_mac;
    bool dst_mac_valid;
    ip_prefix_t src_ip;
    bool src_ip_valid;
    ip_prefix_t dst_ip;
    bool dst_ip_valid;
    uint8_t protocol;
    bool protocol_valid;
    uint16_t src_port;
    bool src_port_valid;
    uint16_t dst_port;
    bool dst_port_valid;
    // Add Source and Dest Tags.
    bool collect_flow_action;
    uint8_t mirror_destinations;
} flow_monitor_spec_t;


if_t
*get_if_from_key_or_handle(kh::InterfaceKeyHandle ifid)
{
    if_t *ift;

    ift = (ifid.key_or_handle_case() == kh::InterfaceKeyHandle::kInterfaceId)
        ? find_if_by_id(ifid.interface_id())
        : find_if_by_handle(ifid.if_handle());
    if (!ift) {
        HAL_TRACE_DEBUG(
                "PI-MirrorSession:{}: interface id {} not found", __FUNCTION__,
                (ifid.key_or_handle_case() == kh::InterfaceKeyHandle::kInterfaceId)
                ? ifid.interface_id()
                : ifid.if_handle());
        return NULL;
    }
    return ift;
}

hal_ret_t
mirror_session_create(MirrorSessionSpec *spec, MirrorSession *rsp)
{

    pd_mirror_session_create_args_t args;
    mirror_session_t session;
    kh::InterfaceKeyHandle ifid;
    hal_ret_t ret;
    if_t *id;

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("PI-MirrorSession:{}: Mirror session ID {}/{}", __FUNCTION__,
            spec->id().session_id(), spec->snaplen());
    // Eventually the CREATE API will differ from the Update API in the way it is
    // enabled. In a create invocation,
    //   the session is created only after all the flows using a previous
    //   incarnation of the mirror session have been cleanedup (i.e. mirror
    //   session removed by the periodic thread). Update is treated as an
    //   incremental update.
    session.id = spec->id().session_id();
    session.truncate_len = spec->snaplen();
    if (spec->has_local_span_if()) {
    }
    if (spec->has_rspan_spec()) {
    }
    if (spec->has_erspan_spec()) {
    }
    rsp->mutable_status()->set_code(MirrorSessionStatus::SUCCESS);
    rsp->mutable_status()->set_status("ok");
    rsp->mutable_status()->set_active_flows(0);
    switch (spec->destination_case()) {
        case MirrorSessionSpec::kLocalSpanIf: {
            HAL_TRACE_DEBUG("PI-MirrorSession:{}: Local Span IF is true", __FUNCTION__);
            ifid = spec->local_span_if();
            id = get_if_from_key_or_handle(ifid);
            if (id != NULL) {
                session.dest_if = id;
            } else {
                rsp->mutable_status()->set_code(MirrorSessionStatus::PERM_FAILURE);
                rsp->mutable_status()->set_status("get if from interface id failed");
                return HAL_RET_INVALID_ARG;
            }
            session.type = hal::MIRROR_DEST_LOCAL;
            break;
        }
        case MirrorSessionSpec::kRspanSpec: {
            HAL_TRACE_DEBUG("PI-MirrorSession:{}: RSpan IF is true", __FUNCTION__);
            auto rspan = spec->rspan_spec();
            ifid = rspan.intf();
            session.dest_if = get_if_from_key_or_handle(ifid);
            auto encap = rspan.rspan_encap();
            if (encap.encap_type() == types::ENCAP_TYPE_DOT1Q) {
                session.mirror_destination_u.r_span_dest.vlan = encap.encap_value();
            }
            session.type = hal::MIRROR_DEST_RSPAN;
            break;
        }
        case MirrorSessionSpec::kErspanSpec: {
            HAL_TRACE_DEBUG("PI-MirrorSession:{}: ERSpan IF is true", __FUNCTION__);
            auto erspan = spec->erspan_spec();
            ip_addr_t src_addr, dst_addr;
            ip_addr_spec_to_ip_addr(&src_addr, erspan.src_ip());
            ip_addr_spec_to_ip_addr(&dst_addr, erspan.dest_ip());
            ep_t *ep;
            switch (dst_addr.af) {
                case IP_AF_IPV4:
                    ep = find_ep_by_v4_key(spec->meta().vrf_id(), dst_addr.addr.v4_addr);
                    break;
                case IP_AF_IPV6:
                    ep = find_ep_by_v6_key(spec->meta().vrf_id(), &dst_addr);
                    break;
                default:
                    HAL_TRACE_ERR("PI-MirrorSession:{}: unknown ERSPAN dest AF{}",
                           __FUNCTION__, dst_addr.af);
                    return HAL_RET_INVALID_ARG;
            }
            if (ep == NULL) {
                HAL_TRACE_ERR("PI-MirrorSession:{}: unknown ERSPAN dest {}, vrfId {}",
                        __FUNCTION__, ipaddr2str(&dst_addr), spec->meta().vrf_id());
                return HAL_RET_INVALID_ARG;
            }
            auto dest_if = find_if_by_handle(ep->if_handle);
            if (dest_if == NULL) {
                HAL_TRACE_ERR("PI-MirrorSession:{}: could not find if ERSPAN dest {}",
                        __FUNCTION__, ipaddr2str(&dst_addr));
                return HAL_RET_INVALID_ARG;
            }
            session.dest_if = dest_if;
            auto ift = find_if_by_handle(ep->gre_if_handle);
            if (ift == NULL) {
                HAL_TRACE_ERR("PI-MirrorSession:{}: could not find ERSPAN tunnel dest if {}",
                        __FUNCTION__, ipaddr2str(&dst_addr));
                return HAL_RET_INVALID_ARG;
            }
            if (ift->if_type != intf::IF_TYPE_TUNNEL) {
                HAL_TRACE_ERR("PI-MirrorSession:{}: no tunnel to ERSPAN dest {}",
                        __FUNCTION__, ipaddr2str(&dst_addr));
                return HAL_RET_INVALID_ARG;
            }
            if (!is_if_type_tunnel(ift)) {
                HAL_TRACE_ERR("PI-MirrorSession:{}: not GRE tunnel to ERSPAN dest {}",
                        __FUNCTION__, ipaddr2str(&dst_addr));
                return HAL_RET_INVALID_ARG;
            }
            session.mirror_destination_u.er_span_dest.tunnel_if = ift; 
            session.type = hal::MIRROR_DEST_ERSPAN;
            break;
        }
        default: {
            HAL_TRACE_ERR("PI-MirrorSession:{}: unknown session type{}", __FUNCTION__,
                    spec->destination_case());
            return HAL_RET_INVALID_ARG;
        }
    }
    args.session = &session;
    // ret = pd_mirror_session_create(&args);
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_MIRROR_SESSION_CREATE, (void *)&args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PI-MirrorSession:{}: Create failed {}", __FUNCTION__, ret);
    } else {
        HAL_TRACE_DEBUG("PI-MirrorSession:{}: Create Succeeded {}", __FUNCTION__,
                session.id);
    }
    rsp->mutable_spec()->CopyFrom(*spec);
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return ret;
}

hal_ret_t
mirror_session_get(MirrorSessionId *id, MirrorSession *rsp)
{
    pd_mirror_session_get_args_t args;
    mirror_session_t session;
    hal_ret_t ret;

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("PI-MirrorSession:{}: Mirror Session ID {}", __FUNCTION__,
            id->session_id());
    memset(&session, 0, sizeof(session));
    session.id = id->session_id();
    args.session = &session;
    // ret = pd_mirror_session_get(&args);
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_MIRROR_SESSION_GET, (void *)&args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PI-MirrorSession:{}: PD API failed {}", __FUNCTION__, ret);
        rsp->mutable_status()->set_code(MirrorSessionStatus::PERM_FAILURE);
        rsp->mutable_status()->set_status("pd action failed");
        rsp->mutable_status()->set_active_flows(0);
        return ret;
    }

    rsp->mutable_status()->set_code(MirrorSessionStatus::SUCCESS);
    rsp->mutable_status()->set_status("ok");
    rsp->mutable_status()->set_active_flows(0);
    rsp->mutable_spec()->mutable_id()->set_session_id(id->session_id());
    rsp->mutable_spec()->set_snaplen(session.truncate_len);
    /* Find the interface ID depending on interface type.
       verify against local cache of session.
       switch (session->type) {
       case hal::MIRROR_LOCAL_SPAN_ID:
       break
       case hal::MIRROR_DEST_RSPAN:
       break
       case hal::MIRROR_DEST_ERSPAN:
       break
       case hal::MIRROR_DEST_NONE:
       break
       } */

    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return HAL_RET_OK;
}

hal_ret_t
mirror_session_delete(MirrorSessionId *id, MirrorSession *rsp)
{
    pd_mirror_session_delete_args_t args;
    mirror_session_t session;
    hal_ret_t ret;

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("PI-MirrorSession:{}: Delete Mirror Session ID {}", __FUNCTION__,
            id->session_id());
    memset(&session, 0, sizeof(session));
    session.id = id->session_id();
    args.session = &session;
    // ret = pd_mirror_session_delete(&args);
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_MIRROR_SESSION_DELETE, (void *)&args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PI-MirrorSession:{}: PD API failed {}", __FUNCTION__, ret);
        rsp->mutable_status()->set_code(MirrorSessionStatus::PERM_FAILURE);
        rsp->mutable_status()->set_status("pd action failed");
    }
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return ret;
}

hal_ret_t
collector_create(CollectorSpec *spec, Collector *resp) {
    collector_config_t cfg;
    pd::pd_collector_create_args_t args;
    hal_ret_t ret = HAL_RET_OK;

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("PI-ExportControl:{}: ExportID {}", __FUNCTION__,
            spec->export_controlid().id());
    
    cfg.exporter_id = spec->export_controlid().id();
    ip_addr_spec_to_ip_addr(&cfg.src_ip, spec->src_ip());
    ip_addr_spec_to_ip_addr(&cfg.dst_ip, spec->dest_ip());
    auto ep = find_ep_by_v4_key(spec->meta().vrf_id(), cfg.dst_ip.addr.v4_addr);
    if (ep == NULL) {
        HAL_TRACE_ERR("PI-Collector:{}: Unknown endpoint {} : {}", __FUNCTION__,
            spec->meta().vrf_id(), ipaddr2str(&cfg.dst_ip));
        return HAL_RET_INVALID_ARG;
    }
    memcpy(cfg.dest_mac, ep->l2_key.mac_addr, sizeof(cfg.dest_mac));
    cfg.template_id = spec->template_id();
    switch (spec->format()) {
        case telemetry::ExportFormat::IPFIX:
            cfg.format = EXPORT_FORMAT_IPFIX;
            break;
        case telemetry::ExportFormat::NETFLOWV9:
            cfg.format = EXPORT_FORMAT_NETFLOW9;
            break;
        default:
            HAL_TRACE_DEBUG("PI-Collector:{}: Unknown format type {}", __FUNCTION__, spec->template_id());
            return HAL_RET_INVALID_ARG;
    }
    cfg.protocol = spec->protocol();
    cfg.dport = spec->dest_port().port();
    auto encap = spec->encap();
    if (encap.encap_type() == types::ENCAP_TYPE_DOT1Q) {
        cfg.vlan = encap.encap_value();
    } else {
        HAL_TRACE_DEBUG("PI-Collector:{}: Unsupport Encap {}", __FUNCTION__, encap.encap_type());
        return HAL_RET_INVALID_ARG;
    }
    cfg.l2seg = l2seg_lookup_by_handle(spec->l2seg_handle());
    if (cfg.l2seg == NULL) {
        HAL_TRACE_DEBUG("PI-Collector:{}: Could not retrieve L2 segment", __FUNCTION__);
        return HAL_RET_INVALID_ARG;
    }
    auto dmac = l2seg_get_rtr_mac(cfg.l2seg);
    if (dmac == NULL) {
        HAL_TRACE_DEBUG("PI-Collector:{}: Could not retrieve L2 segment source mac", __FUNCTION__);
        return HAL_RET_INVALID_ARG;
    }
    memcpy(cfg.src_mac, *dmac, sizeof(cfg.src_mac));
    args.cfg = &cfg;
    // ret = hal::pd::pd_collector_create(&cfg);
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_COLLECTOR_CREATE, (void *)&args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PI-Collector:{}: PD API failed {}", __FUNCTION__, ret);
    }
    HAL_TRACE_DEBUG("PI-ExportControl:{}: SUCCESS: ExportID {}, dest {}, source {},  port {}", __FUNCTION__,
    spec->export_controlid().id(), ipaddr2str(&cfg.dst_ip), ipaddr2str(&cfg.src_ip), cfg.dport);
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return ret;
}

hal_ret_t
collector_update(CollectorSpec *spec, Collector *resp) {
    // implementation TBD
    return HAL_RET_OK;
}

hal_ret_t
collector_get(ExportControlId *id, Collector *resp) {
    
    return HAL_RET_OK;
}

hal_ret_t
collector_delete(ExportControlId *id, Collector *resp) {
    
    return HAL_RET_OK;
}

} // namespace hal
