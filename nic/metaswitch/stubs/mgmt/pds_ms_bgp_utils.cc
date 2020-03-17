// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//Purpose: Helper APIs for metaswitch BGP-RM/NM components

#include "nic/metaswitch/stubs/mgmt/pds_ms_uuid_obj.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_state.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_utils.hpp"
#include "nic/metaswitch/stubs/pds_ms_stubs_init.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_util.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_state.hpp"
#include "qb0mib.h"
using namespace pds_ms;
using namespace types;

namespace pds_ms {

ApiStatus
bgp_peer_hard_reset (BGPPeerSpec& req)
{
    BGPPeerResetSpec proto_req;

    // if peer is already disabled, hard reset enables the peer
    // to trigger peer reset, disable the peer first and enable it
    // in a different CTM transaciton

    // populate Reset spec from Peer spec
    proto_req.set_entindex(PDS_MS_BGP_RM_ENT_INDEX);
    auto laddr = proto_req.mutable_localaddr();
    laddr->set_af(req.localaddr().af());
    laddr->set_v4addr(req.localaddr().v4addr());
    auto paddr = proto_req.mutable_peeraddr();
    paddr->set_af(req.peeraddr().af());
    paddr->set_v4addr(req.peeraddr().v4addr());

    // start CTM transaction to disable peer
    PDS_MS_START_TXN(PDS_MS_CTM_GRPC_CORRELATOR);

    // disable peer
    proto_req.set_state(ADMIN_STATE_DISABLE);
    pds_ms_set_bgppeerresetspec_amb_bgp_peer (proto_req, AMB_ROW_ACTIVE,
                                              PDS_MS_CTM_GRPC_CORRELATOR,
                                              false, false);

    // end CTM transaction
    PDS_MS_END_TXN(PDS_MS_CTM_GRPC_CORRELATOR);

    // blocking on response from MS
    auto ret = pds_ms::mgmt_state_t::ms_response_wait();
    if (ret != API_STATUS_OK) {
        PDS_TRACE_ERR ("Hard reset: Failed to disable peer");
        // disabling the peer failed
        return ret;
    }

    // start CTM transaction to enable peer
    PDS_MS_START_TXN(PDS_MS_CTM_GRPC_CORRELATOR);

    // enable peer
    proto_req.set_state(ADMIN_STATE_ENABLE);
    pds_ms_set_bgppeerresetspec_amb_bgp_peer (proto_req, AMB_ROW_ACTIVE,
                                              PDS_MS_CTM_GRPC_CORRELATOR,
                                              false, false);

    // end CTM transaction
    PDS_MS_END_TXN(PDS_MS_CTM_GRPC_CORRELATOR);

    // blocking on response from MS
    return pds_ms::mgmt_state_t::ms_response_wait();
}

ApiStatus
bgp_peer_route_refresh (BGPPeerSpec& req, BGPClearRouteOptions in_out)
{
    BGPPeerRtRefreshSpec proto_req;

    // start CTM transaction
    PDS_MS_START_TXN(PDS_MS_CTM_GRPC_CORRELATOR);

    // populate Reset spec from Peer spec
    proto_req.set_entindex(PDS_MS_BGP_RM_ENT_INDEX);
    auto laddr = proto_req.mutable_localaddr();
    laddr->set_af(req.localaddr().af());
    laddr->set_v4addr(req.localaddr().v4addr());
    auto paddr = proto_req.mutable_peeraddr();
    paddr->set_af(req.peeraddr().af());
    paddr->set_v4addr(req.peeraddr().v4addr());

    if ((in_out == BGP_CLEAR_ROUTE_REFRESH_IN) ||
        (in_out == BGP_CLEAR_ROUTE_REFRESH_BOTH)) {
        // flag to send route refresh to peer
        proto_req.set_rtrefreshin(true);
    }
    if ((in_out == BGP_CLEAR_ROUTE_REFRESH_OUT) ||
        (in_out == BGP_CLEAR_ROUTE_REFRESH_BOTH)) {
        // flag to send routes to peer
        proto_req.set_rtrefreshout(true);
    }

    pds_ms_set_bgppeerrtrefreshspec_amb_bgp_peer_status (
                                        proto_req,
                                        AMB_ROW_ACTIVE,
                                        PDS_MS_CTM_GRPC_CORRELATOR,
                                        false, false);
    // end CTM transaction
    PDS_MS_END_TXN(PDS_MS_CTM_GRPC_CORRELATOR);

    // blocking on response from MS
    return pds_ms::mgmt_state_t::ms_response_wait();
}

ApiStatus
bgp_peeraf_route_refresh (BGPPeerAfSpec &req, BGPClearRouteOptions in_out)
{
    BGPPeerAfRtRefreshSpec proto_req;

    // start CTM transaction
    PDS_MS_START_TXN(PDS_MS_CTM_GRPC_CORRELATOR);

    // populate Reset spec from Peer spec
    proto_req.set_entindex(PDS_MS_BGP_RM_ENT_INDEX);
    auto laddr = proto_req.mutable_localaddr();
    laddr->set_af(req.localaddr().af());
    laddr->set_v4addr(req.localaddr().v4addr());
    auto paddr = proto_req.mutable_peeraddr();
    paddr->set_af(req.peeraddr().af());
    paddr->set_v4addr(req.peeraddr().v4addr());
    proto_req.set_afi((NBB_LONG)req.afi());
    proto_req.set_safi((NBB_LONG)req.safi());

    if ((in_out == BGP_CLEAR_ROUTE_REFRESH_IN) ||
        (in_out == BGP_CLEAR_ROUTE_REFRESH_BOTH)) {
        // flag to send af route refresh to peer
        proto_req.set_rtrefreshin(true);
    }
    if ((in_out == BGP_CLEAR_ROUTE_REFRESH_OUT) ||
        (in_out == BGP_CLEAR_ROUTE_REFRESH_BOTH)) {
        // flag to send af routes to peer
        proto_req.set_rtrefreshout(true);
    }

    pds_ms_set_bgppeerafrtrefreshspec_amb_bgp_peer_afi_safi_stat (
                                        proto_req,
                                        AMB_ROW_ACTIVE,
                                        PDS_MS_CTM_GRPC_CORRELATOR,
                                        false, false);
    // end CTM transaction
    PDS_MS_END_TXN(PDS_MS_CTM_GRPC_CORRELATOR);

    // blocking on response from MS
    return pds_ms::mgmt_state_t::ms_response_wait();
}

ApiStatus
bgp_clear_route_action_func (const pds::BGPClearRouteRequest *req,
                             pds::BGPClearRouteResponse  *resp)
{
    pds_ms::BGPClearRouteOptions option;
    pds_ms::BGPPeerSpec peer;
    pds_ms::BGPPeerAfSpec peeraf;
    ApiStatus ret=API_STATUS_OK;

    // convert external proto to internal fields
    option = (pds_ms::BGPClearRouteOptions)req->option();
    if (option == BGP_CLEAR_ROUTE_NONE) {
        throw Error (std::string("Invalid BGP clear route request"),
                     SDK_RET_INVALID_ARG);
    }
    if (option == BGP_CLEAR_ROUTE_HARD && req->has_peeraf()) {
        throw Error (std::string("Hard reset connot be done on "
                     "PeerAF"), SDK_RET_INVALID_ARG);
    }

    if (req->has_peer()) {
        // get internal peer spec from keyhandle
        pds_ms_get_bgppeerspec_from_bgppeerkeyhandle (req->peer(),
                                                      peer);
        if (option == BGP_CLEAR_ROUTE_HARD) {
            ret = bgp_peer_hard_reset(peer);
        } else {
            ret = bgp_peer_route_refresh(peer, option);
        }
    } else if (req->has_peeraf()) {
        // get internal peeraf spec from keyhandle
        pds_ms_get_bgppeerafspec_from_bgppeerafkeyhandle (req->peeraf(),
                                                          peeraf);
        ret = bgp_peeraf_route_refresh(peeraf, option);
    } else {
        throw Error (std::string("Invalid Peer/PeerAF in BGP clear route "
                     "request"), SDK_RET_INVALID_ARG);
    }

    return ret;
}

NBB_VOID
update_bgp_route_map_table (NBB_ULONG correlator)
{
    BgpRouteMapSpec spec;
    std::string str;
    NBB_ULONG len = 0;

    auto state_ctxt = state_t::thread_context();
    // walk-through subnets
    state_ctxt.state()->subnet_store().
        walk([&str] (ms_bd_id_t bd_id, subnet_obj_t& subnet_obj) -> bool {
                // walk-through (import) rt list of each subnet
                subnet_obj.rt_store.walk([&str] (ms_rt_t &obj) -> void {
                    str=str+obj.ms_str()+',';
                    });
                return true;
                });
    // walk-through vpcs
    state_ctxt.state()->vpc_store().
        walk([&str] (ms_vrf_id_t vrf_id, vpc_obj_t& vpc_obj) -> bool {
                // walk-through (import) rt list of each vpc
                vpc_obj.rt_store.walk([&str] (ms_rt_t &obj) -> void {
                    str=str+obj.ms_str()+',';
                    });
                return true;
                });

    PDS_TRACE_DEBUG ("ORF: bgpRouteMapTable ext-comm:: %s (len=%d)\n",
                      str.c_str(), str.length());

    // set length appropriately
    if (!str.empty()) {
        len = str.length()-1;
    }

    spec.set_rmindex (PDS_MS_BGP_RM_ENT_INDEX);
    spec.set_index (PDS_MS_BGP_ROUTE_MAP_DEF_INDEX);
    spec.set_number (PDS_MS_BGP_ROUTE_MAP_DEF_NUBMER);
    spec.set_afi (AMB_BGP_AFI_L2VPN);
    spec.set_afidefined (true);
    spec.set_safi (AMB_BGP_EVPN);
    spec.set_safidefined (true);
    spec.set_orfassociation (AMB_BGP_ORF_ASSOC_LOCAL);
    spec.set_matchextcomm (str.c_str(), len);
    pds_ms_set_bgproutemapspec_amb_bgp_route_map (spec, AMB_ROW_ACTIVE,
                                                  correlator, FALSE);
}

static NBB_VOID 
bgp_peer_fill_keys_(BGPPeerSpec& req, bgp_peer_uuid_obj_t* uuid_obj)
{
    auto bgp_peer_uuid_obj = (bgp_peer_uuid_obj_t*) uuid_obj;
    auto localaddr = req.mutable_localaddr();
    ip_addr_to_spec(&bgp_peer_uuid_obj->ms_id().local_ip, localaddr);

    auto peeraddr = req.mutable_peeraddr();
    ip_addr_to_spec(&bgp_peer_uuid_obj->ms_id().peer_ip, peeraddr);

    PDS_TRACE_VERBOSE("BGP Peer Pre-set Keys UUID %s Local IP %s Peer IP %s",
                      uuid_obj->uuid().str(),
                      ipaddr2str(&bgp_peer_uuid_obj->ms_id().local_ip),
                      ipaddr2str(&bgp_peer_uuid_obj->ms_id().peer_ip));
}

static NBB_VOID 
bgp_peer_af_fill_keys_(BGPPeerAfSpec& req,
                       bgp_peer_af_uuid_obj_t* uuid_obj)
{
    auto bgp_peer_af_uuid_obj = (bgp_peer_af_uuid_obj_t*) uuid_obj;
    auto localaddr = req.mutable_localaddr();
    ip_addr_to_spec(&bgp_peer_af_uuid_obj->ms_id().local_ip, localaddr);

    auto peeraddr = req.mutable_peeraddr();
    ip_addr_to_spec(&bgp_peer_af_uuid_obj->ms_id().peer_ip, peeraddr);
    req.set_afi((BGPAfi) bgp_peer_af_uuid_obj->ms_id().afi);
    req.set_safi((BGPSafi) bgp_peer_af_uuid_obj->ms_id().safi);


    PDS_TRACE_VERBOSE("BGP Peer Pre-set Keys UUID %s Local IP %s Peer IP %s",
                      uuid_obj->uuid().str(),
                      ipaddr2str(&bgp_peer_af_uuid_obj->ms_id().local_ip),
                      ipaddr2str(&bgp_peer_af_uuid_obj->ms_id().peer_ip));
}

static NBB_VOID
populate_disable_peer_af_spec (BGPPeerSpec &peer, BGPPeerAfSpec *peer_af,
                               BGPAfi afi, BGPSafi safi)
{
    auto paddr = peer_af->mutable_peeraddr();
    auto laddr = peer_af->mutable_localaddr();
    paddr->set_af (peer.peeraddr().af());
    paddr->set_v4addr(peer.peeraddr().v4addr());
    laddr->set_af (peer.localaddr().af());
    laddr->set_v4addr(peer.localaddr().v4addr());
    peer_af->set_afi(afi);
    peer_af->set_safi(safi);
    peer_af->set_disable(true);
}

NBB_VOID
bgp_rm_ent_pre_get(BGPSpec &req, BGPGetResponse* resp, NBB_VOID* kh)
{
    BGPKeyHandle *key_spec = (BGPKeyHandle *)kh;

    // set UUID from key handle to req
    req.set_id(key_spec->id());
}

NBB_VOID
bgp_peer_pre_get(BGPPeerSpec &req, BGPPeerGetResponse* resp, NBB_VOID* kh)
{
    pds_obj_key_t uuid = {0};
    BGPPeerKeyHandle *key_handle = (BGPPeerKeyHandle *)kh;

    if (key_handle->id_or_key_case() == key_handle->kIdFieldNumber) {
        pds_ms_get_uuid(&uuid, req.id());
        auto mgmt_ctxt = mgmt_state_t::thread_context();
        auto uuid_obj = mgmt_ctxt.state()->lookup_uuid(uuid);

        if (uuid_obj == nullptr) {
            // Invalid UUID in get request
            throw Error (std::string("BGP Peer get with invalid key ").
                         append(uuid.str()), SDK_RET_ENTRY_NOT_FOUND);
        } else if (uuid_obj->obj_type() == uuid_obj_type_t::BGP_PEER) {
            // Fill keys
            bgp_peer_fill_keys_(req, (bgp_peer_uuid_obj_t*)uuid_obj);
        } else if (uuid_obj->obj_type() != uuid_obj_type_t::BGP) {
            // non-matching UUID in get request
            throw Error (std::string("BGP Peer get with non-matching UUID type ").
                         append(uuid.str()).append(" of type ").
                         append(uuid_obj_type_str(uuid_obj->obj_type())), 
                         SDK_RET_INVALID_ARG);
        }
    } else {
        // key is passed in the request. it should have required key parameters
        PDS_TRACE_VERBOSE("Received BGP Peer request with Key");
    }
}

NBB_VOID
bgp_peer_pre_set(BGPPeerSpec &req, NBB_LONG row_status,
                 NBB_ULONG correlator, NBB_VOID* kh, bool op_update)
{
    pds_obj_key_t uuid = {0};

    if (row_status == AMB_ROW_DESTROY) {
        BGPPeerKeyHandle *key_handle = (BGPPeerKeyHandle *)kh;
        if (key_handle->id_or_key_case() == key_handle->kIdFieldNumber) {
            pds_ms_get_uuid(&uuid, req.id());
        } 
    } else {
        // create/update should have valid UUID
        pds_ms_get_uuid(&uuid, req.id());
    }

    if (!is_pds_obj_key_invalid (uuid)) {
        auto mgmt_ctxt = mgmt_state_t::thread_context();
        auto uuid_obj = mgmt_ctxt.state()->lookup_uuid(uuid);

        if (uuid_obj == nullptr) {
            if (row_status == AMB_ROW_DESTROY) {
                // Should throw an error instead of return.
                // MS might delete entry if key params match
                throw Error (std::string("BGP Peer delete with Invalid key ").
                             append(uuid.str()), SDK_RET_ENTRY_NOT_FOUND);
            }
            if (op_update) {
                // Update without an existing entry
                throw Error (std::string("BGP Peer update with Invalid key ").
                             append(uuid.str()), SDK_RET_ENTRY_NOT_FOUND);
            }
            // BGP Peer Create - store UUID to key mapping pending confirmation
            ip_addr_t local_ipaddr, peer_ipaddr;
            ip_addr_spec_to_ip_addr (req.localaddr(), &local_ipaddr);
            ip_addr_spec_to_ip_addr (req.peeraddr(), &peer_ipaddr);
            bgp_peer_uuid_obj_uptr_t bgp_peer_uuid_obj 
                                     (new bgp_peer_uuid_obj_t (uuid,
                                      local_ipaddr,
                                      peer_ipaddr));
            {
                mgmt_ctxt.state()->set_pending_uuid_create(uuid,
                                   std::move(bgp_peer_uuid_obj));
            }
            PDS_TRACE_VERBOSE("BGP Peer Pre-set Create UUID %s Local IP %s Peer IP %s",
                               uuid.str(), ipaddr2str(&local_ipaddr),
                               ipaddr2str(&peer_ipaddr));
        } else if (uuid_obj->obj_type() == uuid_obj_type_t::BGP_PEER) {
            // BGP Peer Update - Fill keys
            bgp_peer_fill_keys_(req, (bgp_peer_uuid_obj_t*)uuid_obj);
            if (row_status == AMB_ROW_DESTROY) {
                mgmt_ctxt.state()->set_pending_uuid_delete(uuid);
            }
        } else if (uuid_obj->obj_type() == uuid_obj_type_t::BGP) {
            // venice may send peer config with BGP UUID. accept if the UUID
            // is of type BGP else return error
            PDS_TRACE_VERBOSE("BGP Peer request with BGP UUID %s",
                              uuid.str());
        } else {
            // non-matching UUID in get request
            throw Error (std::string("BGP Peer with non-matching UUID type ").
                         append(uuid.str()).append(" of type ").
                         append(uuid_obj_type_str(uuid_obj->obj_type())), 
                         SDK_RET_INVALID_ARG);
        }
    } else if (row_status != AMB_ROW_DESTROY) {
        // non-matching UUID in get request
        throw Error (std::string("BGP Peer create/update reruest with "
                     "invalid UUID ").append(uuid.str()));
    }

    if (row_status == AMB_ROW_ACTIVE && !op_update) {
        // Disable AFs for newly created Peer. User has to enable (create) address
        // families as per the peer connectivity
        BGPPeerAfSpec peer_af_spec;
        populate_disable_peer_af_spec (req, &peer_af_spec,
                                       BGP_AFI_IPV4, BGP_SAFI_UNICAST);
        pds_ms_set_bgppeerafspec_amb_bgp_peer_afi_safi(peer_af_spec,
                                                       AMB_ROW_ACTIVE,
                                                       correlator, FALSE);
        populate_disable_peer_af_spec (req, &peer_af_spec,
                                       BGP_AFI_L2VPN, BGP_SAFI_EVPN);
        pds_ms_set_bgppeerafspec_amb_bgp_peer_afi_safi(peer_af_spec,
                                                       AMB_ROW_ACTIVE,
                                                       correlator, FALSE);
    }

}

// API to verify whether given peer AF can be displayed to user or not
// for now only IPV4-unicast and L2VPN-EVPN AFs are supported
bool
bgp_peer_afi_safi_pre_fill_get (amb_bgp_peer_afi_safi *data)
{
    if ((data->afi == AMB_BGP_AFI_L2VPN && data->safi == AMB_BGP_EVPN) ||
        (data->afi == AMB_BGP_AFI_IPV4 && data->safi == AMB_BGP_UNICAST)) {
        // display only enabled AFs
        return (data->disable_afi_safi == AMB_FALSE);
    }
    return false;
}

// API to verify whether BGP rm ent can be displayed to user or not
bool
bgp_rm_ent_pre_fill_get (amb_bgp_rm_ent *data)
{
    return (data->admin_status == AMB_BGP_ADMIN_STATUS_UP);
}

NBB_VOID
bgp_peer_afi_safi_pre_get(BGPPeerAfSpec &req,
                          BGPPeerAfGetResponse* resp,
                          NBB_VOID* kh)
{
    pds_obj_key_t uuid = {0};
    BGPPeerAfKeyHandle *key_handle = (BGPPeerAfKeyHandle *)kh;

    if (key_handle->id_or_key_case() == key_handle->kIdFieldNumber) {
        pds_ms_get_uuid(&uuid, req.id());

        auto mgmt_ctxt = mgmt_state_t::thread_context();
        auto uuid_obj = mgmt_ctxt.state()->lookup_uuid(uuid);

        if (uuid_obj == nullptr) {
            // Invalid UUID in get request
            throw Error (std::string("BGP PeerAF get with invalid key ").
                         append(uuid.str()), SDK_RET_ENTRY_NOT_FOUND);
        } else if (uuid_obj->obj_type() == uuid_obj_type_t::BGP_PEER_AF) {
            // Fill keys
            bgp_peer_af_fill_keys_(req, (bgp_peer_af_uuid_obj_t*)uuid_obj);
        } else {
            // non-matching UUID in get request
            throw Error (std::string("BGP PeerAf get with non-matching UUID type ").
                         append(uuid.str()).append(" of type ").
                         append(uuid_obj_type_str(uuid_obj->obj_type())), 
                         SDK_RET_INVALID_ARG);
        }
    } else {
        // venice use case
        // key is passed in the request. it should have required key parameters
        PDS_TRACE_VERBOSE("Received BGP PeerAf request with Key");
    }
}

NBB_VOID
bgp_peer_afi_safi_pre_set(BGPPeerAfSpec &req, NBB_LONG row_status,
                          NBB_ULONG correlator, NBB_VOID* kh, bool op_update)
{
    pds_obj_key_t uuid = {0};

    if (row_status == AMB_ROW_DESTROY) {
        BGPPeerAfKeyHandle *key_handle = (BGPPeerAfKeyHandle *)kh;
        if (key_handle->id_or_key_case() == key_handle->kIdFieldNumber) {
            pds_ms_get_uuid(&uuid, req.id());
        } 
    } else {
        // create/update should have valid UUID
        pds_ms_get_uuid(&uuid, req.id());
    }

    if (!is_pds_obj_key_invalid (uuid)) {
        auto mgmt_ctxt = mgmt_state_t::thread_context();
        auto uuid_obj = mgmt_ctxt.state()->lookup_uuid(uuid);

        if (uuid_obj == nullptr) {
            if (row_status == AMB_ROW_DESTROY) {
                // Should throw an error instead of return.
                // MS might delete entry if key params match
                throw Error (std::string("BGP PeerAF delete with Invalid key ").
                             append(uuid.str()), SDK_RET_ENTRY_NOT_FOUND);
            }
            if (op_update) {
                // Update without an existing entry
                throw Error (std::string("BGP PeerAF update with Invalid key ").
                             append(uuid.str()), SDK_RET_ENTRY_NOT_FOUND);
            }
            // BGP PeerAF Create - store UUID to key mapping pending confirmation
            ip_addr_t local_ipaddr, peer_ipaddr;
            ip_addr_spec_to_ip_addr (req.localaddr(), &local_ipaddr);
            ip_addr_spec_to_ip_addr (req.peeraddr(), &peer_ipaddr);
            bgp_peer_af_uuid_obj_uptr_t bgp_peer_af_uuid_obj 
                (new bgp_peer_af_uuid_obj_t (uuid, local_ipaddr, peer_ipaddr,
                                             req.afi(), req.safi()));
            {
                mgmt_ctxt.state()->set_pending_uuid_create(uuid,
                                                           std::move(bgp_peer_af_uuid_obj));
            }
            PDS_TRACE_VERBOSE("BGP PeerAF Pre-set Create UUID %s Local IP %s "
                              "Peer IP %s AFI %d SAFI %d",
                              uuid.str(), ipaddr2str(&local_ipaddr),
                              ipaddr2str(&peer_ipaddr), req.afi(), req.safi());
        } else if (uuid_obj->obj_type() == uuid_obj_type_t::BGP_PEER_AF) {
            // BGP Peer Update - Fill keys
            bgp_peer_af_fill_keys_(req, (bgp_peer_af_uuid_obj_t*)uuid_obj);
            if (row_status == AMB_ROW_DESTROY) {
                mgmt_ctxt.state()->set_pending_uuid_delete(uuid);
            }
        } else if (uuid_obj->obj_type() == uuid_obj_type_t::BGP) {
            // venice may send peer-af config with BGP UUID. accept if the UUID
            // is of type BGP else return error
            PDS_TRACE_VERBOSE("BGP PeerAF request with BGP UUID %s",
                              uuid.str());
        } else {
            // non-matching UUID in get request
            throw Error (std::string("BGP PeerAF with non-matching UUID type ").
                         append(uuid.str()).append(" of type ").
                         append(uuid_obj_type_str(uuid_obj->obj_type())), 
                         SDK_RET_INVALID_ARG);
        }
    } else if (row_status != AMB_ROW_DESTROY) {
        // non-matching UUID in get request
        throw Error (std::string("BGP PeerAF with invalid UUID ").
                     append(uuid.str()));
    }

    // Address Family enable/disable should be set internally as per
    // create-update/delete operation. these rows cannot be destroyed
    // in metaswitch, so when user deletes an AF for a peer, we internally
    // disable the AF and enable when he creates it
    req.set_disable (row_status == AMB_ROW_DESTROY);
    ip_addr_t local_ipaddr, peer_ipaddr;
    ip_addr_spec_to_ip_addr (req.localaddr(), &local_ipaddr);
    ip_addr_spec_to_ip_addr (req.peeraddr(), &peer_ipaddr);
    PDS_TRACE_VERBOSE("BGP PeerAF for Local IP %s "
                      "Peer IP %s AFI %d SAFI %d is %s",
                      ipaddr2str(&local_ipaddr), ipaddr2str(&peer_ipaddr),
                      req.afi(), req.safi(),
                      (row_status == AMB_ROW_DESTROY)?"disabled":"enabled");
}

NBB_VOID
bgp_rm_ent_pre_set (BGPSpec &req, NBB_LONG row_status,
                    NBB_ULONG correlator, NBB_VOID* kh, bool op_update)
{
    AdminState state = ADMIN_STATE_ENABLE;
    BGPKeyHandle *key_spec = (BGPKeyHandle *)kh;

    // set UUID from key handle to req
    req.set_id(key_spec->id());

    // BGP global spec deletion should be sent down as
    // update with admin_status_down in order to keep all
    // joins and peer info (?) intact when user creates the
    // global spec again. this goes with the assumption
    // that there will be only one global spec at any time
    if (row_status == AMB_ROW_DESTROY) {
        state = ADMIN_STATE_DISABLE;
    }

    req.set_state (state);
    PDS_TRACE_VERBOSE ("BGP Rm Ent admin status is updated to %s",
                        (state == ADMIN_STATE_DISABLE) ? "Disable" : "Enable");
}

NBB_VOID
bgp_peer_get_fill_func (BGPPeerSpec&   req,
                        NBB_ULONG*           oid)
{
    oid[AMB_BGP_PER_RM_ENT_INDEX_INDEX] = PDS_MS_BGP_RM_ENT_INDEX;
}

NBB_VOID
bgp_peer_status_get_fill_func (BGPPeerSpec&   req,
                               NBB_ULONG*            oid)
{
    oid[AMB_BGP_PRST_RM_ENT_INDEX_INDEX] = PDS_MS_BGP_RM_ENT_INDEX;
}

NBB_VOID 
bgp_peer_set_fill_func (BGPPeerSpec& req,
                        AMB_GEN_IPS  *mib_msg,
                        AMB_BGP_PEER *v_amb_bgp_peer,
                        NBB_LONG     row_status)
{
    // Local variables
    NBB_ULONG *oid = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);

    // ASSUMPTION - There is only a single BGP instance ever.
    // So the BGP instance UUID is not included in the BGP Peer Proto. 
    // Harcoding the BGP entity index.
    v_amb_bgp_peer->rm_ent_index        = PDS_MS_BGP_RM_ENT_INDEX;
    oid[AMB_BGP_PER_RM_ENT_INDEX_INDEX] = v_amb_bgp_peer->rm_ent_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PER_RM_ENT_INDEX);

    // Always set admin status to UP
    if (row_status != AMB_ROW_DESTROY) {
        NBB_TRC_FLOW ((NBB_FORMAT "Not destroying peer: fill in field Local_NM"));
        v_amb_bgp_peer->local_nm = PDS_MS_BGP_NM_ENT_INDEX;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PER_LOCAL_NM);

        // Trap settings. Enabled by default for all Peers
        v_amb_bgp_peer->trap_estab = AMB_TRUE;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PER_TRAP_ESTAB);
        v_amb_bgp_peer->trap_backw = AMB_TRUE;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PER_TRAP_BACKW);

        // Avoid delay in advertising Type-2 withdrawal for all
        // IP addresses associated with a MAC in case of the EVPN Remote
        // to Local MAC/IP move scenario since it causes spurious IP add
        // for the invalid IP addresses
        v_amb_bgp_peer->min_asorigination_interval = 1;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PER_MIN_AS_ORIG_INT);
        v_amb_bgp_peer->min_route_advertise_interval = 1;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PEER_MIN_RT_ADVERT);
        v_amb_bgp_peer->min_route_withdraw_interval = 1;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PEER_MIN_RT_WDRW);
    }
}

NBB_VOID
bgp_rm_ent_get_fill_func (BGPSpec &req, NBB_ULONG *oid)
{
    pds_obj_key_t uuid = {0};
    bgp_uuid_obj_t::ms_id_t entity_index = 0;
    pds_ms_get_uuid(&uuid, req.id());
    {
        auto mgmt_ctxt = mgmt_state_t::thread_context();
        auto uuid_obj = mgmt_ctxt.state()->lookup_uuid(uuid);
        if (uuid_obj == nullptr) {
            bgp_uuid_obj_uptr_t bgp_uuid_obj(new bgp_uuid_obj_t (uuid));
            entity_index = bgp_uuid_obj->ms_id();
            mgmt_ctxt.state()->set_pending_uuid_create(uuid,
                                                       std::move(bgp_uuid_obj));
            PDS_TRACE_VERBOSE("BGP RM Pre-set Create UUID %s Entity %d",
                              uuid.str(), entity_index);
        } else if (uuid_obj->obj_type() == uuid_obj_type_t::BGP) {
            auto bgp_uuid_obj = (bgp_uuid_obj_t*)uuid_obj;
            entity_index = bgp_uuid_obj->ms_id();
        } else {
            PDS_TRACE_ERR("BGP RM Request with unknown UUID %s of type %d",
                          uuid.str(), uuid_obj_type_str(uuid_obj->obj_type()));
        }
    }
    oid[AMB_BGP_RM_INDEX_INDEX] = entity_index;
}

NBB_VOID 
bgp_rm_ent_set_fill_func (BGPSpec        &req,
                          AMB_GEN_IPS    *mib_msg,
                          AMB_BGP_RM_ENT *v_amb_bgp_rm_ent,
                          NBB_LONG       row_status)
{
    // Local variables
    NBB_ULONG *oid = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);

    pds_obj_key_t uuid = {0};
    bgp_uuid_obj_t::ms_id_t entity_index = 0;
    pds_ms_get_uuid(&uuid, req.id());
    {
        auto mgmt_ctxt = mgmt_state_t::thread_context();
        auto uuid_obj = mgmt_ctxt.state()->lookup_uuid(uuid);
        if (uuid_obj == nullptr) {
            bgp_uuid_obj_uptr_t bgp_uuid_obj(new bgp_uuid_obj_t (uuid));
            entity_index = bgp_uuid_obj->ms_id();
            mgmt_ctxt.state()->set_pending_uuid_create(uuid,
                                                       std::move(bgp_uuid_obj));
            PDS_TRACE_VERBOSE("BGP RM Pre-set Create UUID %s Entity %d",
                              uuid.str(), entity_index);
        } else if (uuid_obj->obj_type() == uuid_obj_type_t::BGP) {
            auto bgp_uuid_obj = (bgp_uuid_obj_t*)uuid_obj;
            entity_index = bgp_uuid_obj->ms_id();
        } else {
            throw Error (std::string("BGP RM Request with unknown UUID ")
                         .append(uuid.str()).append(" of type ")
                         .append(uuid_obj_type_str(uuid_obj->obj_type())),
                         SDK_RET_INVALID_ARG);
        }
    }

    // ASSUMPTION - There is only a single BGP instance ever.
    // Harcoding the BGP entity index.
    v_amb_bgp_rm_ent->index     = entity_index;
    oid[AMB_BGP_RM_INDEX_INDEX] = v_amb_bgp_rm_ent->index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_RM_INDEX);

    // Always set admin status to UP
    if (row_status != AMB_ROW_DESTROY) {
        NBB_TRC_FLOW ((NBB_FORMAT "Not destroying RM: fill in field admin_status and I3 in AMB_BGP_RM_ENT"));
        v_amb_bgp_rm_ent->i3_ent_index = PDS_MS_I3_ENT_INDEX;;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_RM_I3_ENT_INDEX);

#if 0
        // TODO: Update-groups to be enabled when ORF support for update-groups
        // is added. this should be enabled only on RRs
        v_amb_bgp_rm_ent->update_groups = AMB_TRUE;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_RM_UPDATE_GROUPS);
#endif
        v_amb_bgp_rm_ent->agg_split_horizon = AMB_FALSE;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_RM_AGG_SPLT_HORIZON);

        v_amb_bgp_rm_ent->admin_status = req.state();
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_RM_ADMIN_STATUS);
    }
}

NBB_VOID
bgp_peer_af_get_fill_func (BGPPeerAfSpec    &req,
                           NBB_ULONG*             oid)
{
    oid[AMB_BGP_PAS_RM_ENT_INDEX_INDEX] = PDS_MS_BGP_RM_ENT_INDEX;
}

NBB_VOID 
bgp_peer_af_set_fill_func (BGPPeerAfSpec         &req,
                           AMB_GEN_IPS           *mib_msg,
                           AMB_BGP_PEER_AFI_SAFI *v_amb_bgp_peer_af,
                           NBB_LONG              row_status)
{
    // Local variables
    NBB_ULONG *oid = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);

    // ASSUMPTION - There is only a single BGP instance ever.
    // So the BGP instance UUID is not included in the BGP Peer Proto. 
    // Harcoding the BGP entity index.
    v_amb_bgp_peer_af->rm_ent_index        = PDS_MS_BGP_RM_ENT_INDEX;
    oid[AMB_BGP_PAS_RM_ENT_INDEX_INDEX] = v_amb_bgp_peer_af->rm_ent_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PAS_RM_ENT_INDEX);

    if (req.afi() == AMB_BGP_AFI_L2VPN && req.safi() == AMB_BGP_EVPN &&
        req.disable() != true) {
        auto mgmt_ctxt = mgmt_state_t::thread_context();
        if (!mgmt_ctxt.state()->rr_mode()) {
            // always set import map for peer evpn af, if not in rr-mode
            v_amb_bgp_peer_af->import_map = PDS_MS_BGP_ROUTE_MAP_DEF_INDEX;
            AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PAS_IMP_MAP);
        }
    }
    
    v_amb_bgp_peer_af->disable_afi_safi = (req.disable()) ? AMB_TRUE : AMB_FALSE;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PAS_DISABLE);
    
}

// Fill bgpRmEntTable: AMB_BGP_RM_ENT
NBB_VOID
pds_ms_fill_amb_bgp_rm (AMB_GEN_IPS *mib_msg, pds_ms_config_t *conf)
{
    // Local variables
    NBB_ULONG       *oid = NULL; 
    AMB_BGP_RM_ENT  *data= NULL;

    NBB_TRC_ENTRY ("pds_ms_fill_amb_bgp_rm");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_BGP_RM_ENT *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_BGP_RM_OID_LEN;
    oid[1] = AMB_FAM_BGP_RM_ENT;

    // Set all incoming fields
    oid[AMB_BGP_RM_INDEX_INDEX] = conf->entity_index;
    data->index                 = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_RM_INDEX);

    data->row_status = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_RM_ROW_STATUS);

    if (conf->row_status != AMB_ROW_DESTROY)
    {
        NBB_TRC_FLOW ((NBB_FORMAT "Not destroying RM: fill in fields"));
        // RM Entity is always admin down during init
        data->admin_status = AMB_BGP_ADMIN_STATUS_DOWN;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_RM_ADMIN_STATUS);

        data->max_ibgp_ecmp_routes = 4;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_RM_MAX_INT_ECMP_RTS);      
        data->max_ebgp_ecmp_routes = 2;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_RM_MAX_EXT_ECMP_RTS);

        // Enable 4 byte ASN
        data->as_size = AMB_BGP_FOUR_OCTET;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_RM_AS_SIZE);

        // Enable Fast external fallover
        data->fast_ext_fallover = AMB_TRUE;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_RM_FAST_EXT_FALL);

        // Enable ORF support
        data->orf_supported  = AMB_TRUE;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_RM_ORF_SUPPORTED);
    }

    NBB_TRC_EXIT();
    return;
} 

// Fill  bgpNmEntTable: AMB_BGP_NM_ENT
NBB_VOID
pds_ms_fill_amb_bgp_nm (AMB_GEN_IPS *mib_msg, pds_ms_config_t *conf)
{
    // Local variables
    NBB_ULONG       *oid = NULL; 
    AMB_BGP_NM_ENT  *data= NULL;

    NBB_TRC_ENTRY ("pds_ms_fill_amb_bgp_nm");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_BGP_NM_ENT *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_BGP_NM_OID_LEN;
    oid[1] = AMB_FAM_BGP_NM_ENT;

    // Set all incoming fields
    oid[AMB_BGP_NM_INDEX_INDEX] = conf->entity_index;
    data->index                 = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_NM_INDEX);

    data->row_status = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_NM_ROW_STATUS);

    if (data->row_status != AMB_ROW_DESTROY)
    {
        NBB_TRC_FLOW ((NBB_FORMAT "Not destroying NM: fill in fields"));
        data->rm_index = PDS_MS_BGP_RM_ENT_INDEX;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_NM_RM_INDEX);

        data->admin_status = conf->admin_status;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_NM_ADMIN_STATUS);

        data->sck_index = PDS_MS_SCK_ENT_INDEX;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_NM_SCK_INDEX);
    }

    NBB_TRC_EXIT();
    return;
}


// Fill bgpRmAfiSafiTable: AMB_BGP_RM_AFI_SAFI
NBB_VOID
pds_ms_fill_amb_bgp_rm_afi_safi (AMB_GEN_IPS *mib_msg, pds_ms_config_t *conf)
{
    // Local variables
    NBB_ULONG           *oid = NULL; 
    AMB_BGP_RM_AFI_SAFI *data= NULL;

    NBB_TRC_ENTRY ("pds_ms_fill_amb_bgp_rm_afi_safi");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_BGP_RM_AFI_SAFI *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_BGP_AFI_OID_LEN;
    oid[1] = AMB_FAM_BGP_RM_AFI_SAFI;

    // Set all incoming fields
    oid[AMB_BGP_AFI_ENT_INDEX_INDEX]    = conf->entity_index;
    data->ent_index                     = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFI_ENT_INDEX);

    oid[AMB_BGP_AFI_AFI_INDEX]  = AMB_BGP_AFI_L2VPN;
    data->afi                   = AMB_BGP_AFI_L2VPN;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFI_AFI);

    oid[AMB_BGP_AFI_SAFI_INDEX] = AMB_BGP_EVPN;
    data->safi                  = AMB_BGP_EVPN;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFI_SAFI);

    data->next_hop_safi = AMB_BGP_UNICAST;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFI_NEXT_HOP_SAFI);

    data->admin_status = AMB_BGP_ADMIN_STATUS_UP;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFI_ADMIN_STATUS);

    data->state_kept = AMB_FALSE;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFI_STATE_KEPT);

    data->afm_required = AMB_FALSE;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFI_AFM_REQUIRED);

    NBB_TRC_EXIT();
    return;
}

// Fill bgpNmListenTable: AMB_BGP_NM_LISTEN
NBB_VOID
pds_ms_fill_amb_bgp_nm_listen (AMB_GEN_IPS *mib_msg, pds_ms_config_t *conf)
{
    // Local variables
    NBB_ULONG           *oid = NULL; 
    AMB_BGP_NM_LISTEN   *data= NULL;

    NBB_TRC_ENTRY ("pds_ms_fill_amb_bgp_nm_listen");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_BGP_NM_LISTEN *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_BGP_LIS_OID_LEN;
    oid[1] = AMB_FAM_BGP_NM_LISTEN;

    // Set all incoming fields
    oid[AMB_BGP_LIS_ENT_INDEX_INDEX]    = conf->entity_index;
    data->ent_index                     = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_LIS_ENT_INDEX);

    oid[AMB_BGP_LIS_INDEX_INDEX]    = PDS_MS_SCK_ENT_INDEX;
    data->index                     = PDS_MS_SCK_ENT_INDEX;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_LIS_INDEX);

    data->row_status = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_LIS_ROW_STATUS);

    if (data->row_status != AMB_ROW_DESTROY)
    {
        NBB_TRC_FLOW((NBB_FORMAT "Not destroying NM LISTEN: fill in fields"));
        data->admin_status = conf->admin_status;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_LIS_ADMIN_STATUS);

        data->addr_type = AMB_INETWK_ADDR_TYPE_IPV4;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_LIS_ADDR_TYPE);

        data->addr_len = 0;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_LIS_ADDR);
    }

    NBB_TRC_EXIT();
    return;
}

// Fill bgpRmAfmJoinTable: AMB_BGP_RM_AFM_JOIN
NBB_VOID
pds_ms_fill_amb_bgp_rm_afm_join (AMB_GEN_IPS *mib_msg, pds_ms_config_t *conf)
{
    // Local variables
    NBB_ULONG           *oid = NULL; 
    AMB_BGP_RM_AFM_JOIN *data= NULL;

    NBB_TRC_ENTRY ("pds_ms_fill_amb_bgp_rm_afm_join");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_BGP_RM_AFM_JOIN *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_BGP_AFM_OID_LEN;
    oid[1] = AMB_FAM_BGP_RM_AFM_JOIN;

    // Set all incoming fields
    oid[AMB_BGP_AFM_ENTITY_INDEX]   = conf->entity_index;
    data->ent_index                 = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFM_ENTITY);

    oid[AMB_BGP_AFM_JOIN_INDEX] = conf->join_index;
    data->join                  = conf->join_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFM_JOIN);

    data->row_status = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFM_ROW_STATUS);

    if (data->row_status != AMB_ROW_DESTROY)
    {
        NBB_TRC_FLOW((NBB_FORMAT "Not destroying RM AFM Join: fill in fields"));
        data->admin_status = conf->admin_status;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFM_ADMIN_STATUS);

        data->afi = conf->afi;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFM_AFI);

        data->safi = conf->safi;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFM_SAFI);

        data->partner_index = conf->partner_index;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFM_PARTNER_INDEX);
    }

    NBB_TRC_EXIT();
    return;
}

// Fill bgpOrfCapabilityTable: AMB_BGP_ORF_CAP
NBB_VOID
pds_ms_fill_amb_bgp_orf_cap (AMB_GEN_IPS *mib_msg, pds_ms_config_t *conf)
{
    // Local variables
    NBB_ULONG       *oid = NULL; 
    AMB_BGP_ORF_CAP *data= NULL;

    NBB_TRC_ENTRY ("pds_ms_fill_amb_bgp_orf_cap");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_BGP_ORF_CAP *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_BGP_ORF_OID_LEN;
    oid[1] = AMB_FAM_BGP_ORF_CAP;

    // Set all incoming fields
    oid[AMB_BGP_ORF_CAP_ENT_INDEX_INDEX] = conf->entity_index;
    data->ent_index                      = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_ORF_CAP_ENT_INDEX);

    oid[AMB_BGP_ORF_CAP_AFI_INDEX] = conf->afi;
    data->afi                      = conf->afi;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_ORF_CAP_AFI);

    oid [AMB_BGP_ORF_CAP_SAFI_INDEX] = conf->safi;
    data->safi                       = conf->safi;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_ORF_CAP_SAFI);

    oid[AMB_BGP_ORF_CAP_ORF_TYPE_INDEX] = AMB_BGP_ORF_CAP_TYPE_EXT_COM;
    data->orf_type                      = AMB_BGP_ORF_CAP_TYPE_EXT_COM;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_ORF_CAP_ORF_TYPE);

    data->admin_status = AMB_BGP_ADMIN_STATUS_UP;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_ORF_CAP_ADMIN_STAT);

    data->send_receive = AMB_BGP_ORF_CAP_SR_BOTH;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_ORF_CAP_SEND_RECV);

    PDS_TRACE_VERBOSE ("BGP ORF Cap is enbaled for Afi/Safi %d/%d",
                        conf->afi, conf->safi);

    NBB_TRC_EXIT();
    return;
}

// row_update for BGP-RM
NBB_VOID
pds_ms_row_update_bgp_rm (pds_ms_config_t *conf)
{
    NBB_TRC_ENTRY ("pds_ms_row_update_bgp_rm");

    // Set params
    conf->oid_len           = AMB_BGP_RM_OID_LEN;
    conf->data_len          = sizeof (AMB_BGP_RM_ENT);

    // Convert to row_update and send
    pds_ms_ctm_send_row_update_common (conf, pds_ms_fill_amb_bgp_rm); 

    NBB_TRC_EXIT();
    return;
}

// row_update for BGP-NM
NBB_VOID
pds_ms_row_update_bgp_nm (pds_ms_config_t *conf)
{
    NBB_TRC_ENTRY ("pds_ms_row_update_bgp_nm");

    // Set params
    conf->oid_len       = AMB_BGP_NM_OID_LEN;
    conf->data_len      = sizeof (AMB_BGP_NM_ENT);

    // Convert to row_update and send
    pds_ms_ctm_send_row_update_common (conf, pds_ms_fill_amb_bgp_nm); 

    NBB_TRC_EXIT();
    return;
}

// row_update for bgpRmAfiSafiTable
NBB_VOID
pds_ms_row_update_bgp_rm_afi_safi (pds_ms_config_t *conf)
{
    NBB_TRC_ENTRY ("pds_ms_row_update_bgp_rm_afi_safi");

    // Set params
    conf->oid_len       = AMB_BGP_AFI_OID_LEN;
    conf->data_len      = sizeof (AMB_BGP_RM_AFI_SAFI);

    // Convert to row_update and send
    pds_ms_ctm_send_row_update_common (conf, pds_ms_fill_amb_bgp_rm_afi_safi); 

    NBB_TRC_EXIT();
    return;
}

// row_update for bgpNmListenTable
NBB_VOID
pds_ms_row_update_bgp_nm_listen (pds_ms_config_t *conf)
{
    NBB_TRC_ENTRY ("pds_ms_row_update_bgp_nm_listen");

    // Set params
    conf->oid_len       = AMB_BGP_LIS_OID_LEN;
    conf->data_len      = sizeof (AMB_BGP_NM_LISTEN);

    // Convert to row_update and send
    pds_ms_ctm_send_row_update_common (conf, pds_ms_fill_amb_bgp_nm_listen); 

    NBB_TRC_EXIT();
    return;
}

// row_update for bgpRmAfmJoinTable
NBB_VOID
pds_ms_row_update_bgp_rm_afm_join (pds_ms_config_t *conf)
{
    NBB_TRC_ENTRY ("pds_ms_row_update_bgp_rm_afm_join");

    // Set params
    conf->oid_len       = AMB_BGP_AFM_OID_LEN;
    conf->data_len      = sizeof (AMB_BGP_RM_AFM_JOIN);

    // Convert to row_update and send
    pds_ms_ctm_send_row_update_common (conf, pds_ms_fill_amb_bgp_rm_afm_join); 

    NBB_TRC_EXIT();
    return;
}

// row_update for bgpOrfCapabilityTable
NBB_VOID
pds_ms_row_update_bgp_orf_cap (pds_ms_config_t *conf)
{
    NBB_TRC_ENTRY ("pds_ms_row_update_bgp_orf_cap");

    // Set params
    conf->oid_len       = AMB_BGP_ORF_OID_LEN;
    conf->data_len      = sizeof (AMB_BGP_ORF_CAP);

    // Convert to row_update and send
    pds_ms_ctm_send_row_update_common (conf, pds_ms_fill_amb_bgp_orf_cap); 

    NBB_TRC_EXIT();
    return;
}

NBB_VOID
pds_ms_bgp_create (pds_ms_config_t *conf)
{
    NBB_TRC_ENTRY ("pds_ms_bgp_create");

   // bgpRmEntTable
   conf->entity_index        = PDS_MS_BGP_RM_ENT_INDEX;
   conf->admin_status        = AMB_ADMIN_STATUS_DOWN;
   pds_ms_row_update_bgp_rm (conf);

    // bgpNmEntTable
    conf->entity_index       = PDS_MS_BGP_NM_ENT_INDEX;
    conf->admin_status       = AMB_ADMIN_STATUS_UP;
    pds_ms_row_update_bgp_nm (conf);

    // bgpRmAfmJoinTable
    conf->entity_index       = PDS_MS_BGP_RM_ENT_INDEX;
    conf->admin_status       = AMB_ADMIN_STATUS_UP;
    conf->partner_index      = PDS_MS_RTM_DEF_ENT_INDEX;

    // AFM: AMB_BGP_AFI_IPV4
    conf->join_index         = 1;
    conf->afi                = AMB_BGP_AFI_IPV4;
    conf->safi               = AMB_BGP_UNICAST;
    pds_ms_row_update_bgp_rm_afm_join (conf);

    // AFM: AMB_BGP_AFI_L2VPN
    conf->join_index         = 2;
    conf->afi                = AMB_BGP_AFI_L2VPN;
    conf->safi               = AMB_BGP_EVPN;
    conf->partner_index      = PDS_MS_EVPN_ENT_INDEX;
    pds_ms_row_update_bgp_rm_afm_join (conf);
    pds_ms_row_update_bgp_orf_cap (conf);

    // bgpRmAfiSafiTable
    conf->entity_index      = 1;
    pds_ms_row_update_bgp_rm_afi_safi (conf);

    // bgpNmListenTable
    conf->entity_index       = PDS_MS_BGP_NM_ENT_INDEX;
    conf->admin_status       = AMB_ADMIN_STATUS_UP;
    pds_ms_row_update_bgp_nm_listen (conf);

    NBB_TRC_EXIT();
    return;
}
}
