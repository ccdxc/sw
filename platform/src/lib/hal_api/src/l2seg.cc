
#include <iostream>
#include <grpc++/grpc++.h>

#include "l2seg.hpp"
#include "uplink.hpp"

using namespace std;


sdk::lib::indexer *HalL2Segment::allocator = sdk::lib::indexer::factory(HalL2Segment::max_l2segs, false, true);
std::map<l2seg_key_t, HalL2Segment*> HalL2Segment::l2seg_db;

HalL2Segment *
HalL2Segment::Factory(HalVrf *vrf, uint16_t vlan)
{
    l2seg_key_t key(vrf->GetId(), vlan);
    HalL2Segment *l2seg = new HalL2Segment(vrf, vlan);

    // Store in DB for disruptive restart
    l2seg_db[key] = l2seg;

    return l2seg;
}

void
HalL2Segment::Destroy(HalL2Segment *l2seg)
{
    l2seg_key_t key(l2seg->vrf->GetId(), l2seg->_vlan);
    if (l2seg) {
        l2seg->~HalL2Segment();
    }

    l2seg_db.erase(key);
}

HalL2Segment::HalL2Segment(HalVrf *vrf, uint16_t vlan)
{
    grpc::ClientContext                 context;
    grpc::Status                        status;

    l2segment::L2SegmentSpec            *req;
    l2segment::L2SegmentResponse        rsp;
    l2segment::L2SegmentRequestMsg      req_msg;
    l2segment::L2SegmentResponseMsg     rsp_msg;

    if (allocator->alloc(&id) != sdk::lib::indexer::SUCCESS) {
        NIC_LOG_ERR("Failed to allocate HalL2Segment");
        return;
    }

    NIC_LOG_INFO("L2seg create: vrf: {}, vlan: {}. id: {}",
                    vrf->GetId(), vlan, id);


    this->_vlan = vlan;
    this->vrf = vrf;
    if (vrf->GetUplink()) {
        uplink_refs[vrf->GetUplink()->GetId()] = vrf->GetUplink();
    }

    req = req_msg.add_request();
    // req->mutable_meta()->set_vrf_id(vrf->GetId());
    req->mutable_key_or_handle()->set_segment_id(id);
    //req->mutable_vrf_key_handle()->set_vrf_handle(vrf->GetHandle());
    req->mutable_vrf_key_handle()->set_vrf_id(vrf->GetId());
    req->set_mcast_fwd_policy(l2segment::MulticastFwdPolicy::MULTICAST_FWD_POLICY_FLOOD);
    req->set_bcast_fwd_policy(l2segment::BroadcastFwdPolicy::BROADCAST_FWD_POLICY_FLOOD);
    req->mutable_wire_encap()->set_encap_type(::types::ENCAP_TYPE_DOT1Q);
    req->mutable_wire_encap()->set_encap_value(vlan);

    // status = hal->l2seg_stub_->HalL2SegmentCreate(&context, req_msg, &rsp_msg);
    for (auto it = uplink_refs.begin(); it != uplink_refs.end(); ++it) {
        req->add_if_key_handle()->set_interface_id(it->first);
    }

    status = hal->l2segment_create(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            handle = rsp.l2segment_status().key_or_handle().l2segment_handle();
            NIC_LOG_INFO("L2 segment create succeeded id: {}, handle: {}",
                            id, handle);
        } else {
            NIC_LOG_ERR("Failed to create l2segment: id: {} err: {}", id, rsp.api_status());
        }
    } else {
        NIC_LOG_ERR("Failed to create l2segment: id: {}, err: {}, err_msg: {}", id,
                      status.error_code(),
                      status.error_message());
    }
}

HalL2Segment *
HalL2Segment::Lookup(HalVrf *vrf, uint16_t vlan)
{
    l2seg_key_t key(vrf->GetId(), vlan);

    if (l2seg_db.find(key) != l2seg_db.cend()) {
        return l2seg_db[key];
    } else {
        return NULL;
    }
}

#if 0
shared_ptr<HalL2Segment>
HalL2Segment::GetInstance(shared_ptr<HalVrf> vrf, uint16_t vlan)
{
    l2seg_key_t key(vrf->GetId(), vlan);

    if (registry.find(key) == registry.cend()) {
        registry[key] = make_shared<HalL2Segment>(vrf, vlan);
    }

    return registry[key];
}
#endif

HalL2Segment::~HalL2Segment()
{
    grpc::ClientContext                     context;
    grpc::Status                            status;

    l2segment::L2SegmentDeleteRequest       *req;
    l2segment::L2SegmentDeleteResponse      rsp;
    l2segment::L2SegmentDeleteRequestMsg    req_msg;
    l2segment::L2SegmentDeleteResponseMsg   rsp_msg;

    NIC_LOG_INFO("L2seg delete : vrf: {}, vlan: {}. id: {}",
                    vrf->GetId(), _vlan, id);
    req = req_msg.add_request();
    req->mutable_key_or_handle()->set_segment_id(id);
    req->mutable_vrf_key_handle()->set_vrf_id(vrf->GetId());

    // status = hal->l2seg_stub_->HalL2SegmentDelete(&context, req_msg, &rsp_msg);
    status = hal->l2segment_delete(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            allocator->free(id);
            NIC_LOG_INFO("L2 segment delete succeeded id: {}, handle: {}",
                            id, handle);
        } else {
            NIC_LOG_ERR("Failed to delete l2segment: err: {}", rsp.api_status());
        }
    } else {
        NIC_LOG_ERR("Failed to delete l2segment: err: {}, err_msg: {}",
                      status.error_code(),
                      status.error_message());
    }
}

uint64_t
HalL2Segment::GetId()
{
    return id;
}


uint64_t
HalL2Segment::GetHandle()
{
    return handle;
}

HalVrf *
HalL2Segment::GetVrf()
{
    return vrf;
}

void
HalL2Segment::TriggerHalUpdate()
{
    grpc::ClientContext                 context;
    grpc::Status                        status;

    l2segment::L2SegmentSpec            *req;
    l2segment::L2SegmentResponse        rsp;
    l2segment::L2SegmentRequestMsg      req_msg;
    l2segment::L2SegmentResponseMsg     rsp_msg;

    req = req_msg.add_request();
    // req->mutable_meta()->set_vrf_id(vrf->GetId());
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
            handle = rsp.l2segment_status().key_or_handle().l2segment_handle();
            NIC_LOG_INFO("L2 segment update succeeded id: {}, handle: {}",
                            id, handle);
        } else {
            NIC_LOG_ERR("Failed to update l2segment: err: {}", rsp.api_status());
        }
    } else {
        NIC_LOG_ERR("Failed to update l2segment: err: {}, err_msg: {}",
                      status.error_code(),
                      status.error_message());
    }
}

//-----------------------------------------------------------------------------
// Adds uplink to l2segment
//-----------------------------------------------------------------------------
void
HalL2Segment::AddUplink (Uplink *uplink)
{
    // Check for the presence of new uplink
    if (uplink_refs.find(uplink->GetId()) != uplink_refs.end()) {
        NIC_LOG_WARN("Duplicate uplink add : {}", uplink->GetId());
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
HalL2Segment::DelUplink (Uplink *uplink)
{
    // Check for the presence of uplink
    if (uplink_refs.find(uplink->GetId()) == uplink_refs.end()) {
        NIC_LOG_ERR("Not able to find uplink: {}", uplink->GetId());
        return;
    }

    // Del uplink from the map
    uplink_refs.erase(uplink->GetId());

    // Sends update to Hal
    TriggerHalUpdate();
}
