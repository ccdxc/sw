//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include <iostream>
#include <grpc++/grpc++.h>
#include "platform/src/lib/nicmgr/include/logger.hpp"
#include "l2seg.hpp"
#include "uplink.hpp"
#include "enic.hpp"
#include "utils.hpp"
#include "devapi_mem.hpp"
#include "hal_grpc.hpp"
#include "print.hpp"
#include "vrf.hpp"

namespace iris {

indexer *devapi_l2seg::allocator_ = indexer::factory(devapi_l2seg::max_l2segs,
                                                     false, true);
std::map<l2seg_key_t, devapi_l2seg*> devapi_l2seg::l2seg_db_;

devapi_l2seg *
devapi_l2seg::factory(devapi_vrf *vrf, vlan_t vlan)
{
    sdk_ret_t ret = SDK_RET_OK;
    void *mem = NULL;
    devapi_l2seg *l2seg = NULL;
    l2seg_key_t key(vrf->get_id(), vlan);

    api_trace("l2seg create");

    if (l2seg_db_.find(key) != l2seg_db_.end()) {
        NIC_LOG_WARN("Duplicate Create of l2seg with vrf: {}, vlan: {}",
                     vrf->get_id(), vlan);
        return NULL;
    }

    NIC_LOG_DEBUG("l2seg create: vrf_id: {}, vlan: {}", vrf->get_id(), vlan);

    mem = (devapi_l2seg *)DEVAPI_CALLOC(DEVAPI_MEM_ALLOC_L2SEG,
                                        sizeof(devapi_l2seg));
    if (mem) {
        l2seg = new (mem) devapi_l2seg();
        ret = l2seg->init_(vrf, vlan);
        if (ret != SDK_RET_OK) {
            goto end;
        }
        ret = l2seg->l2seg_halcreate();
        if (ret != SDK_RET_OK) {
            goto end;
        }
#if 0
        // if vlan is native, add l2seg as native to uplink and push uplink
        if (l2seg->get_vlan() == NATIVE_VLAN_ID) {
            if (l2seg->get_vrf()->get_uplink()) {
                l2seg->get_vrf()->get_uplink()->set_native_l2seg(l2seg);
                l2seg->get_vrf()->get_uplink()->update_hal_native_l2seg(l2seg->get_id());
            }
        }
#endif
    }

    // Store in DB
    l2seg_db_[key] = l2seg;
end:
    if (ret != SDK_RET_OK) {
        l2seg->deallocate_id();
        l2seg->~devapi_l2seg();
        DEVAPI_FREE(DEVAPI_MEM_ALLOC_L2SEG, mem);
        l2seg = NULL;
    }
    return l2seg;
}

void
devapi_l2seg::destroy(devapi_l2seg *l2seg)
{
    l2seg_key_t key(l2seg->get_vrf()->get_id(), l2seg->get_vlan());

    api_trace("l2seg delete");

#if 0
    if (l2seg->get_vlan() == NATIVE_VLAN_ID) {
        if (l2seg->get_vrf()->get_uplink()) {
            l2seg->get_vrf()->get_uplink()->update_hal_native_l2seg(0);
            l2seg->get_vrf()->get_uplink()->set_native_l2seg(NULL);
        }
    }
#endif

    // remove from hal
    l2seg->l2seg_haldelete();
    // remove from db
    l2seg_db_.erase(key);
    // free up id
    l2seg->deallocate_id();
    // destructor
    l2seg->~devapi_l2seg();
    // free mem
    DEVAPI_FREE(DEVAPI_MEM_ALLOC_L2SEG, l2seg);
}

sdk_ret_t
devapi_l2seg::init_(devapi_vrf *vrf, vlan_t vlan)
{
    NIC_LOG_DEBUG("L2seg create: vrf: {}, vlan: {}",
                    vrf->get_id(), vlan);

    if (allocator_->alloc(&id_) != indexer::SUCCESS) {
        NIC_LOG_ERR("Failed to allocate devapi_l2seg");
        id_ = indexer::INVALID_INDEXER;
        return SDK_RET_ERR;
    }
    vlan_ = vlan;
    vrf_ = vrf;
    if (vrf_->get_uplink()) {
        uplink_refs_[vrf->get_uplink()->get_id()] = vrf->get_uplink();
    }
    return SDK_RET_OK;
}

sdk_ret_t
devapi_l2seg::l2seg_halcreate(void)
{
    sdk_ret_t                           ret = SDK_RET_OK;
    grpc::ClientContext                 context;
    grpc::Status                        status;

    l2segment::L2SegmentSpec            *req;
    l2segment::L2SegmentResponse        rsp;
    l2segment::L2SegmentRequestMsg      req_msg;
    l2segment::L2SegmentResponseMsg     rsp_msg;

    NIC_LOG_DEBUG("HalL2seg create id: {}", id_);

    req = req_msg.add_request();
    // req->mutable_meta()->set_vrf_id(vrf->get_id());
    req->mutable_key_or_handle()->set_segment_id(id_);
    //req->mutable_vrf_key_handle()->set_vrf_handle(vrf->GetHandle());
    req->mutable_vrf_key_handle()->set_vrf_id(vrf_->get_id());
    req->set_mcast_fwd_policy(l2segment::MulticastFwdPolicy::MULTICAST_FWD_POLICY_FLOOD);
    req->set_bcast_fwd_policy(l2segment::BroadcastFwdPolicy::BROADCAST_FWD_POLICY_FLOOD);
    req->mutable_wire_encap()->set_encap_type(::types::ENCAP_TYPE_DOT1Q);
    req->mutable_wire_encap()->set_encap_value(vlan_);
    req->set_single_wire_management(single_wire_mgmt_);

    // status = hal->l2seg_stub_->devapi_l2segCreate(&context, req_msg, &rsp_msg);
    for (auto it = uplink_refs_.begin(); it != uplink_refs_.end(); ++it) {
        req->add_if_key_handle()->set_interface_id(it->first);
    }

    VERIFY_HAL();
    status = hal->l2segment_create(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            NIC_LOG_DEBUG("L2 segment create succeeded id: {}", id_);
        } else {
            NIC_LOG_ERR("Failed to create l2segment: id: {} err: {}", id_, rsp.api_status());
            ret = SDK_RET_ERR;
            goto end;
        }
    } else {
        NIC_LOG_ERR("Failed to create l2segment: id: {}, err: {}, err_msg: {}", id_,
                    status.error_code(),
                    status.error_message());
        ret = SDK_RET_ERR;
        goto end;
    }

end:
    return ret;
}

devapi_l2seg *
devapi_l2seg::lookup(devapi_vrf *vrf, vlan_t vlan)
{
    l2seg_key_t key(vrf->get_id(), vlan);

    if (l2seg_db_.find(key) != l2seg_db_.cend()) {
        return l2seg_db_[key];
    } else {
        return NULL;
    }
}

sdk_ret_t
devapi_l2seg::l2seg_haldelete(void)
{
    sdk_ret_t                               ret = SDK_RET_OK;
    grpc::ClientContext                     context;
    grpc::Status                            status;

    l2segment::L2SegmentDeleteRequest       *req;
    l2segment::L2SegmentDeleteResponse      rsp;
    l2segment::L2SegmentDeleteRequestMsg    req_msg;
    l2segment::L2SegmentDeleteResponseMsg   rsp_msg;

    NIC_LOG_DEBUG("L2seg delete : vrf: {}, vlan: {}. id: {}",
                    vrf_->get_id(), vlan_, id_);
    req = req_msg.add_request();
    req->mutable_key_or_handle()->set_segment_id(id_);
    req->mutable_vrf_key_handle()->set_vrf_id(vrf_->get_id());

    VERIFY_HAL();
    status = hal->l2segment_delete(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            allocator_->free(id_);
            NIC_LOG_DEBUG("L2 segment delete succeeded id: {}", id_);
        } else {
            NIC_LOG_ERR("Failed to delete l2segment: err: {}", rsp.api_status());
            ret = SDK_RET_ERR;
            goto end;
        }
    } else {
        NIC_LOG_ERR("Failed to delete l2segment: err: {}, err_msg: {}",
                      status.error_code(),
                      status.error_message());
        ret = SDK_RET_ERR;
        goto end;
    }

    allocator_->free(id_);
end:
    return ret;
}

void
devapi_l2seg::deallocate_id(void)
{
    if (id_ != indexer::INVALID_INDEXER) {
        allocator_->free(id_);
    }
}

uint64_t
devapi_l2seg::get_id()
{
    return id_;
}

devapi_vrf *
devapi_l2seg::get_vrf()
{
    return vrf_;
}

sdk_ret_t
devapi_l2seg::trigger_halupdate()
{
    sdk_ret_t                           ret = SDK_RET_OK;
    grpc::ClientContext                 context;
    grpc::Status                        status;

    l2segment::L2SegmentSpec            *req;
    l2segment::L2SegmentResponse        rsp;
    l2segment::L2SegmentRequestMsg      req_msg;
    l2segment::L2SegmentResponseMsg     rsp_msg;

    req = req_msg.add_request();
    // req->mutable_meta()->set_vrf_id(vrf->get_id());
    req->mutable_key_or_handle()->set_segment_id(id_);
    req->mutable_vrf_key_handle()->set_vrf_id(vrf_->get_id());
    req->set_mcast_fwd_policy(l2segment::MulticastFwdPolicy::MULTICAST_FWD_POLICY_FLOOD);
    req->set_bcast_fwd_policy(l2segment::BroadcastFwdPolicy::BROADCAST_FWD_POLICY_FLOOD);
    req->mutable_wire_encap()->set_encap_type(::types::ENCAP_TYPE_DOT1Q);
    req->mutable_wire_encap()->set_encap_value(vlan_);
    req->set_single_wire_management(single_wire_mgmt_);

    for (auto it = uplink_refs_.begin(); it != uplink_refs_.end(); ++it) {
        req->add_if_key_handle()->set_interface_id(it->first);
    }

    VERIFY_HAL();
    status = hal->l2segment_update(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            NIC_LOG_DEBUG("L2 segment update succeeded id: {}", id_);
        } else {
            NIC_LOG_ERR("Failed to update l2segment: err: {}", rsp.api_status());
            ret = SDK_RET_ERR;
        }
    } else {
        NIC_LOG_ERR("Failed to update l2segment: err: {}, err_msg: {}",
                      status.error_code(),
                      status.error_message());
            ret = SDK_RET_ERR;
    }
end:
    return ret;
}

//-----------------------------------------------------------------------------
// Adds uplink to l2segment
//-----------------------------------------------------------------------------
sdk_ret_t
devapi_l2seg::add_uplink(devapi_uplink *uplink)
{
    // Check for the presence of new uplink
    if (uplink_refs_.find(uplink->get_id()) != uplink_refs_.end()) {
        NIC_LOG_WARN("Duplicate uplink add : {}", uplink->get_id());
        return SDK_RET_ERR;
    }

    // Add new uplink to the map
    uplink_refs_[uplink->get_id()] = uplink;

    // Sends update to Hal
    return trigger_halupdate();

    return SDK_RET_OK;
}

//-----------------------------------------------------------------------------
// Deletes uplink to l2segment
//-----------------------------------------------------------------------------
sdk_ret_t
devapi_l2seg::del_uplink(devapi_uplink *uplink, bool trigger_hal_update)
{
    // Check for the presence of uplink
    if (uplink_refs_.find(uplink->get_id()) == uplink_refs_.end()) {
        NIC_LOG_ERR("Not able to find uplink: {}", uplink->get_id());
        return SDK_RET_ERR;
    }

    // Del uplink from the map
    uplink_refs_.erase(uplink->get_id());

    if (trigger_hal_update) {
        // Sends update to Hal
        return trigger_halupdate();
    }

    return SDK_RET_OK;
}

//-----------------------------------------------------------------------------
// Adds enic to l2segment
//-----------------------------------------------------------------------------
sdk_ret_t
devapi_l2seg::add_enic(devapi_enic *enic)
{
    // Check for the presence of new enic
    if (enic_refs_.find(enic->get_id()) != enic_refs_.end()) {
        NIC_LOG_WARN("Duplicate enic add : {}", enic->get_id());
        return SDK_RET_ERR;
    }

    // Add new enic to the map
    enic_refs_[enic->get_id()] = enic;

    return SDK_RET_OK;
}

//-----------------------------------------------------------------------------
// Deletes enic to l2segment
//-----------------------------------------------------------------------------
sdk_ret_t
devapi_l2seg::del_enic(devapi_enic *enic)
{
    // Check for the presence of enic
    if (enic_refs_.find(enic->get_id()) == enic_refs_.end()) {
        NIC_LOG_ERR("Not able to find enic: {}", enic->get_id());
        return SDK_RET_ERR;
    }

    // Del enic from the map
    enic_refs_.erase(enic->get_id());

    return SDK_RET_OK;
}
}    // namespace iris
