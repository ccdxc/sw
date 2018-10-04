
#include <iostream>
#include <grpc++/grpc++.h>

#include "l2seg.hpp"
#include "uplink.hpp"

using namespace std;


sdk::lib::indexer *L2Segment::allocator = sdk::lib::indexer::factory(L2Segment::max_l2segs, false, true);
std::map<l2seg_key_t, L2Segment*> L2Segment::l2seg_db;

L2Segment *
L2Segment::Factory(Vrf *vrf, uint16_t vlan)
{
    l2seg_key_t key(vrf->GetId(), vlan);
    L2Segment *l2seg = new L2Segment(vrf, vlan);

    // Store in DB for disruptive restart
    l2seg_db[key] = l2seg;

    return l2seg;
}

void
L2Segment::Destroy(L2Segment *l2seg)
{
    l2seg_key_t key(l2seg->vrf->GetId(), l2seg->_vlan);
    if (l2seg) {
        l2seg->~L2Segment();
    }

    l2seg_db.erase(key);
}

L2Segment::L2Segment(Vrf *vrf, uint16_t vlan)
{
    grpc::ClientContext                 context;
    grpc::Status                        status;

    l2segment::L2SegmentSpec            *req;
    l2segment::L2SegmentResponse        rsp;
    l2segment::L2SegmentRequestMsg      req_msg;
    l2segment::L2SegmentResponseMsg     rsp_msg;

    if (allocator->alloc(&id) != sdk::lib::indexer::SUCCESS) {
        HAL_TRACE_ERR("Failed to allocate L2Segment");
        return;
    }

    HAL_TRACE_DEBUG("L2seg create: vrf: {}, vlan: {}. id: {}",
                    vrf->GetId(), vlan, id);

    req = req_msg.add_request();
    req->mutable_key_or_handle()->set_segment_id(id);
    //req->mutable_vrf_key_handle()->set_vrf_handle(vrf->GetHandle());
    req->mutable_vrf_key_handle()->set_vrf_id(vrf->GetId());
    req->set_mcast_fwd_policy(l2segment::MulticastFwdPolicy::MULTICAST_FWD_POLICY_FLOOD);
    req->set_bcast_fwd_policy(l2segment::BroadcastFwdPolicy::BROADCAST_FWD_POLICY_FLOOD);
    req->mutable_wire_encap()->set_encap_type(::types::ENCAP_TYPE_DOT1Q);
    req->mutable_wire_encap()->set_encap_value(vlan);

    // status = hal->l2seg_stub_->L2SegmentCreate(&context, req_msg, &rsp_msg);
    status = hal->l2segment_create(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            handle = rsp.l2segment_status().l2segment_handle();
            HAL_TRACE_DEBUG("L2 segment create succeeded id: {}, handle: {}",
                            id, handle);
            _vlan = vlan;
            this->vrf = vrf;
        } else {
            HAL_TRACE_ERR("Failed to create l2segment: id: {} err: {}", id, rsp.api_status());
        }
    } else {
        HAL_TRACE_ERR("Failed to create l2segment: id: {}, err: {}, err_msg: {}", id,
                      status.error_code(),
                      status.error_message());
    }
}

L2Segment *
L2Segment::Lookup(Vrf *vrf, uint16_t vlan)
{
    l2seg_key_t key(vrf->GetId(), vlan);

    if (l2seg_db.find(key) != l2seg_db.cend()) {
        return l2seg_db[key];
    } else {
        return NULL;
    }
}

#if 0
shared_ptr<L2Segment>
L2Segment::GetInstance(shared_ptr<Vrf> vrf, uint16_t vlan)
{
    l2seg_key_t key(vrf->GetId(), vlan);

    if (registry.find(key) == registry.cend()) {
        registry[key] = make_shared<L2Segment>(vrf, vlan);
    }

    return registry[key];
}
#endif

L2Segment::~L2Segment()
{
    grpc::ClientContext                     context;
    grpc::Status                            status;

    l2segment::L2SegmentDeleteRequest       *req;
    l2segment::L2SegmentDeleteResponse      rsp;
    l2segment::L2SegmentDeleteRequestMsg    req_msg;
    l2segment::L2SegmentDeleteResponseMsg   rsp_msg;

    HAL_TRACE_DEBUG("L2seg delete : vrf: {}, vlan: {}. id: {}",
                    vrf->GetId(), _vlan, id);
    req = req_msg.add_request();
    req->mutable_key_or_handle()->set_segment_id(id);
    req->mutable_vrf_key_handle()->set_vrf_id(vrf->GetId());

    // status = hal->l2seg_stub_->L2SegmentDelete(&context, req_msg, &rsp_msg);
    status = hal->l2segment_delete(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            HAL_TRACE_DEBUG("L2 segment delete succeeded id: {}, handle: {}",
                            id, handle);
        } else {
            HAL_TRACE_ERR("Failed to delete l2segment: err: {}", rsp.api_status());
        }
    } else {
        HAL_TRACE_ERR("Failed to delete l2segment: err: {}, err_msg: {}",
                      status.error_code(),
                      status.error_message());
    }
}

uint64_t
L2Segment::GetId()
{
    return id;
}


uint64_t
L2Segment::GetHandle()
{
    return handle;
}

Vrf *
L2Segment::GetVrf()
{
    return vrf;
}

void
L2Segment::TriggerHalUpdate()
{
    grpc::ClientContext                 context;
    grpc::Status                        status;

    l2segment::L2SegmentSpec            *req;
    l2segment::L2SegmentResponse        rsp;
    l2segment::L2SegmentRequestMsg      req_msg;
    l2segment::L2SegmentResponseMsg     rsp_msg;

    req = req_msg.add_request();
    req->mutable_key_or_handle()->set_segment_id(id);
    req->mutable_vrf_key_handle()->set_vrf_id(vrf->GetId());
    req->set_mcast_fwd_policy(l2segment::MulticastFwdPolicy::MULTICAST_FWD_POLICY_FLOOD);
    req->set_bcast_fwd_policy(l2segment::BroadcastFwdPolicy::BROADCAST_FWD_POLICY_FLOOD);
    req->mutable_wire_encap()->set_encap_type(::types::ENCAP_TYPE_DOT1Q);
    req->mutable_wire_encap()->set_encap_value(_vlan);

    for (auto it = uplink_refs.begin(); it != uplink_refs.end(); ++it) {
        req->add_if_key_handle()->set_interface_id(it->first);
    }

    status = hal->l2segment_update(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            handle = rsp.l2segment_status().l2segment_handle();
            HAL_TRACE_DEBUG("L2 segment update succeeded id: {}, handle: {}",
                            id, handle);
        } else {
            HAL_TRACE_ERR("Failed to update l2segment: err: {}", rsp.api_status());
        }
    } else {
        HAL_TRACE_ERR("Failed to update l2segment: err: {}, err_msg: {}",
                      status.error_code(),
                      status.error_message());
    }
}

//-----------------------------------------------------------------------------
// Adds uplink to l2segment
//-----------------------------------------------------------------------------
void
L2Segment::AddUplink (Uplink *uplink)
{
    // Check for the presence of new uplink
    if (uplink_refs.find(uplink->GetId()) != uplink_refs.end()) {
        HAL_TRACE_WARN("Duplicate uplink add : {}", uplink->GetId());
        return;
    }

    // Add new uplink to the map
    uplink_refs[uplink->GetId()] = uplink;

    // Sends update to Hal
    TriggerHalUpdate();
}

//-----------------------------------------------------------------------------
// Deletes uplink to l2segment
//-----------------------------------------------------------------------------
void
L2Segment::DelUplink (Uplink *uplink)
{
    // Check for the presence of uplink
    if (uplink_refs.find(uplink->GetId()) == uplink_refs.end()) {
        HAL_TRACE_ERR("Not able to find uplink: {}", uplink->GetId());
        return;
    }

    // Del uplink from the map
    uplink_refs.erase(uplink->GetId());

    // Sends update to Hal
    TriggerHalUpdate();
}
