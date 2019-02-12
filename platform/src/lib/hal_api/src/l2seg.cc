
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
    hal_irisc_ret_t ret = HAL_IRISC_RET_SUCCESS;
    l2seg_key_t key(vrf->GetId(), vlan);
    HalL2Segment *l2seg = new HalL2Segment(vrf, vlan);

    ret = l2seg->HalL2SegmentCreate();
    if (ret != HAL_IRISC_RET_SUCCESS) {
        NIC_LOG_DEBUG("L2seg Create failed. ret: {}", ret);
        goto end;
    }

    // Store in DB
    l2seg_db[key] = l2seg;

end:
    if (ret != HAL_IRISC_RET_SUCCESS) {
        if (l2seg) {
            delete l2seg;
            l2seg = NULL;
        }
    }
    return l2seg;
}

hal_irisc_ret_t
HalL2Segment::Destroy(HalL2Segment *l2seg)
{
    hal_irisc_ret_t ret = HAL_IRISC_RET_SUCCESS;
    if (!l2seg) {
        return HAL_IRISC_RET_FAIL;
    }
    l2seg_key_t key(l2seg->vrf->GetId(), l2seg->_vlan);

    ret = l2seg->HalL2SegmentDelete();
    if (ret != HAL_IRISC_RET_SUCCESS) {
        NIC_LOG_CRIT("L2seg Delete failed. ret: {}", ret);
        goto end;
    }
    delete l2seg;

    l2seg_db.erase(key);

end:
    return ret;
}

HalL2Segment::HalL2Segment(HalVrf *vrf, uint16_t vlan)
{
    NIC_LOG_DEBUG("L2seg create: vrf: {}, vlan: {}",
                    vrf->GetId(), vlan);

    this->id = 0;
    this->_vlan = vlan;
    this->vrf = vrf;
    if (vrf->GetUplink()) {
        uplink_refs[vrf->GetUplink()->GetId()] = vrf->GetUplink();
    }
}

hal_irisc_ret_t
HalL2Segment::HalL2SegmentCreate()
{
    hal_irisc_ret_t                     ret = HAL_IRISC_RET_SUCCESS;
    grpc::ClientContext                 context;
    grpc::Status                        status;

    l2segment::L2SegmentSpec            *req;
    l2segment::L2SegmentResponse        rsp;
    l2segment::L2SegmentRequestMsg      req_msg;
    l2segment::L2SegmentResponseMsg     rsp_msg;

    if (allocator->alloc(&id) != sdk::lib::indexer::SUCCESS) {
        NIC_LOG_ERR("Failed to allocate HalL2Segment");
        return HAL_IRISC_RET_FAIL;
    }

    NIC_LOG_DEBUG("HalL2seg create id: {}", id);

    req = req_msg.add_request();
    // req->mutable_meta()->set_vrf_id(vrf->GetId());
    req->mutable_key_or_handle()->set_segment_id(id);
    //req->mutable_vrf_key_handle()->set_vrf_handle(vrf->GetHandle());
    req->mutable_vrf_key_handle()->set_vrf_id(vrf->GetId());
    req->set_mcast_fwd_policy(l2segment::MulticastFwdPolicy::MULTICAST_FWD_POLICY_FLOOD);
    req->set_bcast_fwd_policy(l2segment::BroadcastFwdPolicy::BROADCAST_FWD_POLICY_FLOOD);
    req->mutable_wire_encap()->set_encap_type(::types::ENCAP_TYPE_DOT1Q);
    req->mutable_wire_encap()->set_encap_value(_vlan);

    // status = hal->l2seg_stub_->HalL2SegmentCreate(&context, req_msg, &rsp_msg);
    for (auto it = uplink_refs.begin(); it != uplink_refs.end(); ++it) {
        req->add_if_key_handle()->set_interface_id(it->first);
    }

    status = hal->l2segment_create(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            NIC_LOG_DEBUG("L2 segment create succeeded id: {}", id);
        } else {
            NIC_LOG_ERR("Failed to create l2segment: id: {} err: {}", id, rsp.api_status());
            ret = HAL_IRISC_RET_FAIL;
            goto end;
        }
    } else {
        NIC_LOG_ERR("Failed to create l2segment: id: {}, err: {}, err_msg: {}", id,
                    status.error_code(),
                    status.error_message());
        ret = HAL_IRISC_RET_FAIL;
        goto end;
    }

end:
    if (ret != HAL_IRISC_RET_SUCCESS) {
        allocator->free(id);
    }
    return ret;
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

hal_irisc_ret_t
HalL2Segment::HalL2SegmentDelete()
{
    hal_irisc_ret_t                         ret = HAL_IRISC_RET_SUCCESS;
    grpc::ClientContext                     context;
    grpc::Status                            status;

    l2segment::L2SegmentDeleteRequest       *req;
    l2segment::L2SegmentDeleteResponse      rsp;
    l2segment::L2SegmentDeleteRequestMsg    req_msg;
    l2segment::L2SegmentDeleteResponseMsg   rsp_msg;

    NIC_LOG_DEBUG("L2seg delete : vrf: {}, vlan: {}. id: {}",
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
            NIC_LOG_DEBUG("L2 segment delete succeeded id: {}", id);
        } else {
            NIC_LOG_ERR("Failed to delete l2segment: err: {}", rsp.api_status());
            ret = HAL_IRISC_RET_FAIL;
            goto end;
        }
    } else {
        NIC_LOG_ERR("Failed to delete l2segment: err: {}, err_msg: {}",
                      status.error_code(),
                      status.error_message());
        ret = HAL_IRISC_RET_FAIL;
        goto end;
    }

    allocator->free(id);
end:
    return ret;
}

uint64_t
HalL2Segment::GetId()
{
    return id;
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
            NIC_LOG_DEBUG("L2 segment update succeeded id: {}", id);
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
