// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
// Purpose: Configure Metaswitch to track underlay reachability for
// a destination IP address

#include "nic/metaswitch/stubs/mgmt/pds_ms_ip_track.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_utils.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_state.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_state.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_util.hpp"
#include "nic/metaswitch/stubs/mgmt/gen/svc/internal_gen.hpp"
#include "gen/proto/internal_cp_route.pb.h"
#include "nic/metaswitch/stubs/mgmt/gen/mgmt/pds_ms_internal_cp_route_utils_gen.hpp"

namespace pds_ms {

static void
release_ip_track_obj_ (ip_addr_t& destip)
{
    auto state_ctxt = state_t::thread_context();
    auto state = state_ctxt.state();

    auto ip_track_obj = state->ip_track_store().get(destip);
    if (ip_track_obj == nullptr) return;

    state->ip_track_internalip_store().
        erase(ip_track_obj->internal_ip_prefix());
    state->ip_track_store().erase(destip);
}


static sdk_ret_t
configure_static_tracking_route_ (ip_addr_t& destip,
                                  obj_id_t pds_obj_id, bool stop)
{
    CPStaticRouteSpec static_route_spec;
    ip_track_obj_t *ip_track_obj = nullptr;
    ip_prefix_t internal_ip_pfx;
    types::ApiStatus  ret;

    if (destip.af != IP_AF_IPV4) {
        PDS_TRACE_ERR("Dest IP tracking is only supported for IPv4");
        return SDK_RET_INVALID_ARG;
    }

    try {

        uint correlator = PDS_MS_CTM_GRPC_CORRELATOR;
        PDS_MS_START_TXN(correlator);

        { // Enter State context
            auto state_ctxt = state_t::thread_context();
            auto state = state_ctxt.state();
            ip_track_obj = state->ip_track_store().get(destip);

            if (ip_track_obj == nullptr && stop) {
                PDS_TRACE_INFO("Dest IP track %s stop entry not found",
                               ipaddr2str(&destip));
                return SDK_RET_ENTRY_NOT_FOUND;
            }
            if (ip_track_obj != nullptr && !stop) {
                PDS_TRACE_INFO("Dest IP track %s start entry already exists",
                               ipaddr2str(&destip));
                return SDK_RET_ENTRY_EXISTS;
            }
            if (stop) {
                internal_ip_pfx = ip_track_obj->internal_ip_prefix();
                pds_obj_id = ip_track_obj->pds_obj_id();
            } else {
                std::unique_ptr<ip_track_obj_t> ip_track_obj_uptr
                    (new ip_track_obj_t(destip, pds_obj_id));
                ip_track_obj = ip_track_obj_uptr.get();

                internal_ip_pfx = ip_track_obj->internal_ip_prefix();
                state->ip_track_store().add_upd(destip, std::move(ip_track_obj_uptr));
                state->ip_track_internalip_store()[internal_ip_pfx] = destip;
            }
        }

        PDS_TRACE_INFO("Dest IP track %s %s internal ip %s OBJ ID %d",
                       ipaddr2str(&destip), (stop) ? "stop" : "start",
                       ippfx2str(&internal_ip_pfx), pds_obj_id);

        // Route Table ID is unused since Static route is only supported
        // for underlay
        auto dest_addr_spec = static_route_spec.mutable_destaddr();
        dest_addr_spec->set_af(types::IP_AF_INET);
        dest_addr_spec->set_v4addr(internal_ip_pfx.addr.addr.v4_addr);

        static_route_spec.set_prefixlen(internal_ip_pfx.len);

        NBB_LONG row_status;
        auto nh_addr_spec = static_route_spec.mutable_nexthopaddr();
        nh_addr_spec->set_af(types::IP_AF_INET);
        nh_addr_spec->set_v4addr(destip.addr.v4_addr);

        if (!stop) {
            static_route_spec.set_admindist(10);
            static_route_spec.set_state(types::ADMIN_STATE_ENABLE);
            static_route_spec.set_override(true);

            row_status = AMB_ROW_ACTIVE;
        } else {
            row_status = AMB_ROW_DESTROY;
        }

        pds_ms_validate_cpstaticroutespec(static_route_spec);
        pds_ms_pre_set_cpstaticroutespec_amb_cipr_rtm_static_rt(static_route_spec,
                                                                row_status,
                                                                correlator, nullptr);
        pds_ms_dump_cpstaticroutespec(static_route_spec);
        pds_ms_set_cpstaticroutespec_amb_cipr_rtm_static_rt(static_route_spec,
                                                            row_status, correlator,
                                                            FALSE);
        pds_ms_post_set_cpstaticroutespec_amb_cipr_rtm_static_rt(static_route_spec,
                                                                 row_status,
                                                                 correlator, nullptr);

        PDS_MS_END_TXN(correlator);
        ret = pds_ms::mgmt_state_t::ms_response_wait();

        if (stop) {
            release_ip_track_obj_(destip);
        }
    } catch (const pds_ms::Error& e) {
        if (!stop) {
            release_ip_track_obj_(destip);
        }
        PDS_TRACE_ERR ("Dest IP track %s failed %s CTM Transaction aborted",
                        ipaddr2str(&destip), e.what());
        return e.rc();
    }

    if (ret != types::API_STATUS_OK) {
        if (!stop) {
            release_ip_track_obj_(destip);
        }
        PDS_TRACE_ERR ("Dest IP track %s internal route %s failed %d",
                        ipaddr2str(&destip), (stop) ? "delete" : "create",
                        pds_ms_api_ret_str(ret));
    } else {
        PDS_TRACE_DEBUG ("Dest IP track %s internal route %s successful",
                        ipaddr2str(&destip), (stop) ? "delete" : "create");
    }
    return pds_ms_api_to_sdk_ret(ret);
}

// API called when a Dest IP needs to be tracked
sdk_ret_t
ip_track_add (ip_addr_t& destip, obj_id_t pds_obj_id)
{
    std::lock_guard<std::mutex> lck(pds_ms::mgmt_state_t::grpc_lock());
    return configure_static_tracking_route_(destip, pds_obj_id, false);
}

// API called when a Dest IP no longer needs to be tracked
sdk_ret_t
ip_track_del (ip_addr_t& destip)
{
    std::lock_guard<std::mutex> lck(pds_ms::mgmt_state_t::grpc_lock());
    return configure_static_tracking_route_(destip, OBJ_ID_NONE, true);
}

// API called from gRPC test
types::ApiStatus
ip_track_add (const CPIPTrackTestCreateSpec   *req,
			  CPIPTrackTestResponse *resp)
{
    ip_addr_t destip;
    ip_addr_spec_to_ip_addr (req->destip(), &destip);

    auto pds_obj_id = (obj_id_t) req->pdsobjid();
    if (pds_obj_id != OBJ_ID_TEP && pds_obj_id != OBJ_ID_MIRROR_SESSION) {
        pds_obj_id = OBJ_ID_MIRROR_SESSION;
    }

    return pds_ms_sdk_ret_to_api_status(
        pds_ms::configure_static_tracking_route_(destip, pds_obj_id, false));
}

// API called from gRPC test
types::ApiStatus
ip_track_del (const CPIPTrackTestDeleteSpec   *req,
			  CPIPTrackTestResponse *resp)
{
    ip_addr_t destip;
    ip_addr_spec_to_ip_addr (req->destip(), &destip);

    return pds_ms_sdk_ret_to_api_status(
        pds_ms::configure_static_tracking_route_(destip, OBJ_ID_NONE,
                                                 true));
}

} // End namespace
