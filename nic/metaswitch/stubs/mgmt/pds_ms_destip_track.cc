// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: Helper APIs for metaswitch BGP-RM/NM components

#include "nic/metaswitch/stubs/mgmt/pds_ms_destip_track.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_utils.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_state.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_state.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_util.hpp"
#include "nic/metaswitch/stubs/mgmt/gen/svc/internal_gen.hpp"
#include "gen/proto/internal_cp_route.pb.h"
#include "nic/metaswitch/stubs/mgmt/gen/mgmt/pds_ms_internal_cp_route_utils_gen.hpp"

namespace pds_ms {

static void
release_destip_track_ (ip_addr_t& destip_track)
{
    auto state_ctxt = state_t::thread_context();
    auto state = state_ctxt.state();

    auto destip_track_obj = state->destip_track_store().get(destip_track);
    if (destip_track_obj == nullptr) return;

    state->destip_track_internalip_store().erase(destip_track_obj->internal_ip_prefix());
    state->destip_track_store().erase(destip_track);
}


static sdk_ret_t
destip_track_ (ip_addr_t& destip_track, obj_id_t pds_obj_id, bool stop)
{
    CPStaticRouteSpec static_route_spec;
    destip_track_obj_t *destip_track_obj = nullptr;
    ip_prefix_t internal_ip_pfx;
    types::ApiStatus  ret;

    if (destip_track.af != IP_AF_IPV4) {
        PDS_TRACE_ERR("Dest IP tracking is only supported for IPv4");
        return SDK_RET_INVALID_ARG;
    }

    try {

        uint correlator = PDS_MS_CTM_GRPC_CORRELATOR;
        PDS_MS_START_TXN(correlator);

        { // Enter State context
            auto state_ctxt = state_t::thread_context();
            auto state = state_ctxt.state();
            destip_track_obj = state->destip_track_store().get(destip_track);

            if (destip_track_obj == nullptr && stop) {
                PDS_TRACE_INFO("Dest IP track %s stop entry not found",
                               ipaddr2str(&destip_track));
                return SDK_RET_ENTRY_NOT_FOUND;
            }
            if (destip_track_obj != nullptr && !stop) {
                PDS_TRACE_INFO("Dest IP track %s start entry already exists",
                               ipaddr2str(&destip_track));
                return SDK_RET_ENTRY_EXISTS;
            }
            if (stop) {
                internal_ip_pfx = destip_track_obj->internal_ip_prefix();
                pds_obj_id = destip_track_obj->pds_obj_id();
            } else {
                std::unique_ptr<destip_track_obj_t> destip_track_obj_uptr
                    (new destip_track_obj_t(destip_track, pds_obj_id));
                destip_track_obj = destip_track_obj_uptr.get();

                internal_ip_pfx = destip_track_obj->internal_ip_prefix();
                state->destip_track_store().add_upd(destip_track, std::move(destip_track_obj_uptr));
                state->destip_track_internalip_store()[internal_ip_pfx] = destip_track;
            }
        }

        PDS_TRACE_INFO("Dest IP track %s %s internal ip %s OBJ ID %d",
                       ipaddr2str(&destip_track), (stop) ? "stop" : "start",
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
        nh_addr_spec->set_v4addr(destip_track.addr.v4_addr);

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
            release_destip_track_(destip_track);
        }
    } catch (const pds_ms::Error& e) {
        if (!stop) {
            release_destip_track_(destip_track);
        }
        PDS_TRACE_ERR ("Dest IP track %s failed %s CTM Transaction aborted",
                        ipaddr2str(&destip_track), e.what());
        return e.rc();
    }

    if (ret != types::API_STATUS_OK) {
        if (!stop) {
            release_destip_track_(destip_track);
        }
        PDS_TRACE_ERR ("Dest IP track %s internal route %s failed %d",
                        ipaddr2str(&destip_track), (stop) ? "delete" : "create",
                        pds_ms_api_ret_str(ret));
    } else {
        PDS_TRACE_DEBUG ("Dest IP track %s internal route %s successful",
                        ipaddr2str(&destip_track), (stop) ? "delete" : "create");
    }
    return pds_ms_api_to_sdk_ret(ret);
}

sdk_ret_t
destip_track_start (ip_addr_t& destip_track, obj_id_t pds_obj_id)
{
    std::lock_guard<std::mutex> lck(pds_ms::mgmt_state_t::grpc_lock());
    return destip_track_(destip_track, pds_obj_id, false);
}

sdk_ret_t
destip_track_stop (ip_addr_t& destip_track)
{
    std::lock_guard<std::mutex> lck(pds_ms::mgmt_state_t::grpc_lock());
    return destip_track_(destip_track, OBJ_ID_NONE, true);
}

// Call-back from MS underlay route update
sdk_ret_t
destip_track_reachability_change(ip_addr_t& destip_track,
                                 ms_hw_tbl_id_t nhgroup_id,
                                 obj_id_t pds_obj_id) {

    PDS_TRACE_DEBUG("++++ Dest IP Track %s reachability change to "
                    "Underlay NHgroup %d ++++",
                    ipaddr2str(&destip_track), nhgroup_id);

    // TODO Call HAL API based on obj type
    // pds_obj_key_t nh_group = msidx2pdsobjkey(nhgroup_id);

    switch (pds_obj_id) {
        case OBJ_ID_MIRROR_SESSION:
            break;
        case OBJ_ID_TEP:
            break;
        default:
            SDK_ASSERT(0);
    }
    return SDK_RET_OK;
}

types::ApiStatus
destip_track_start (const CPDestIPTrackTestCreateSpec   *req,
                         CPDestIPTrackTestResponse *resp)
{
    ip_addr_t destip_track;
    ip_addr_spec_to_ip_addr (req->destip(), &destip_track);

    auto pds_obj_id = (obj_id_t) req->pdsobjid();
    if (pds_obj_id != OBJ_ID_TEP && pds_obj_id != OBJ_ID_MIRROR_SESSION) {
        pds_obj_id = OBJ_ID_MIRROR_SESSION;
    }

    return pds_ms_sdk_ret_to_api_status(
        pds_ms::destip_track_(destip_track, pds_obj_id, false));
}

types::ApiStatus
destip_track_stop (const CPDestIPTrackTestDeleteSpec   *req,
                        CPDestIPTrackTestResponse *resp)
{
    ip_addr_t destip_track;
    ip_addr_spec_to_ip_addr (req->destip(), &destip_track);

    return pds_ms_sdk_ret_to_api_status(
        pds_ms::destip_track_(destip_track, OBJ_ID_NONE, true));
}

} // End namespace
