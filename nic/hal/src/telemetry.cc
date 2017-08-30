#include <base.h>
#include <hal.hpp>
#include <hal_state.hpp>
#include <interface.hpp>
#include <pd.hpp>
#include <pd_api.hpp>
#include <telemetry.hpp>
#include <telemetry.pb.h>
#include <telemetry_svc.hpp>
#include <types.pb.h>

using telemetry::MirrorSession;
using telemetry::MirrorSessionStatus;
using telemetry::MirrorSessionSpec;
using telemetry::MirrorSessionId;
using hal::pd::pd_mirror_session_args_t;
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
*get_if_from_key_or_handle(intf::InterfaceKeyHandle ifid)
{
    if_t *ift;

    ift = (ifid.key_or_handle_case() == intf::InterfaceKeyHandle::kInterfaceId)
        ? find_if_by_id(ifid.interface_id())
        : find_if_by_handle(ifid.if_handle());
    if (!ift) {
        HAL_TRACE_DEBUG(
                "PI-MirrorSession:{}: interface id {} not found", __FUNCTION__,
                (ifid.key_or_handle_case() == intf::InterfaceKeyHandle::kInterfaceId)
                ? ifid.interface_id()
                : ifid.if_handle());
        return NULL;
    }
    return ift;
}

hal_ret_t
mirror_session_create(MirrorSessionSpec *spec, MirrorSession *rsp)
{

    pd_mirror_session_args_t args;
    mirror_session_t session;
    intf::InterfaceKeyHandle ifid;
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
                session.dest_if = *id;
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
            ifid = rspan.if_();
            session.dest_if = *(get_if_from_key_or_handle(ifid));
            auto encap = rspan.rspan_encap();
            if (encap.encap_type() == types::ENCAP_TYPE_DOT1Q) {
                session.mirror_destination_u.r_span_dest.vlan = encap.encap_value();
            }
            session.type = hal::MIRROR_DEST_RSPAN;
            break;
        }
        case MirrorSessionSpec::kErspanSpec: {
            HAL_TRACE_DEBUG("PI-MirrorSession:{}: ERSpan IF is true", __FUNCTION__);
            rsp->mutable_status()->set_code(MirrorSessionStatus::PERM_FAILURE);
            rsp->mutable_status()->set_status("ERSPAN not implemented yet");
            HAL_TRACE_DEBUG("PI-MirrorSession:{}: ERSPAN not implemented yet",
                    __FUNCTION__);
            return HAL_RET_INVALID_ARG;
        }
        default: {
            HAL_TRACE_ERR("PI-MirrorSession:{}: unknown session type{}", __FUNCTION__,
                    spec->destination_case());
            return HAL_RET_INVALID_ARG;
        }
    }
    args.session = &session;
    ret = pd_mirror_session_create(&args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PI-MirrorSession:{}: Create failed {}", __FUNCTION__, ret);
    } else {
        HAL_TRACE_DEBUG("PI-MirrorSession:{}: Create Succeeded {}", __FUNCTION__,
                session.id);
    }
    rsp->mutable_spec()->CopyFrom(*spec);
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return HAL_RET_OK;
}

hal_ret_t
mirror_session_get(MirrorSessionId *id, MirrorSession *rsp)
{
    pd_mirror_session_args_t args;
    mirror_session_t session;
    hal_ret_t ret;

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("PI-MirrorSession:{}: Mirror Session ID {}", __FUNCTION__,
            id->session_id());
    memset(&session, 0, sizeof(session));
    args.session = &session;
    ret = pd_mirror_session_get(&args);
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

} // namespace hal
