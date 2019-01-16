
#include <iostream>
#include <grpc++/grpc++.h>

#include "enic.hpp"
#include "ethlif.hpp"

using namespace std;

#define HAL_NON_RSVD_IF_OFFSET 256

sdk::lib::indexer *Enic::allocator = sdk::lib::indexer::factory(Enic::max_enics, false, true);

Enic *
Enic::Factory(EthLif *ethlif)
{
    Enic *enic = new Enic(ethlif);

    return enic;
}

void
Enic::Destroy(Enic *enic)
{
    if (enic) {
        enic->~Enic();
    }
}

// Classic ENIC constructor
Enic::Enic(EthLif *ethlif)
{
    grpc::ClientContext             context;
    grpc::Status                    status;

    intf::InterfaceSpec             *req;
    intf::InterfaceResponse         rsp;
    intf::InterfaceRequestMsg       req_msg;
    intf::InterfaceResponseMsg      rsp_msg;

    if (allocator->alloc(&id) != sdk::lib::indexer::SUCCESS) {
        NIC_LOG_ERR("Failed to allocate ENIC. Resource exhaustion");
        return;
    }

    id += HAL_NON_RSVD_IF_OFFSET;
    NIC_LOG_DEBUG("Enic create id: {}", id);

    this->ethlif = ethlif;

    req = req_msg.add_request();
    req->mutable_key_or_handle()->set_interface_id(id);
    req->set_type(::intf::IfType::IF_TYPE_ENIC);
    req->set_admin_status(::intf::IfStatus::IF_STATUS_UP);
    req->mutable_if_enic_info()->set_enic_type(::intf::IF_ENIC_TYPE_CLASSIC);
    req->mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(ethlif->GetId());
    req->mutable_if_enic_info()->mutable_classic_enic_info()->
        set_native_l2segment_id(ethlif->GetNativeL2Seg()->GetId());
    // req->mutable_if_enic_info()->mutable_classic_enic_info()->add_l2segment_key_handle()->set_l2segment_handle(l2seg->GetHandle());

    status = hal->interface_create(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            handle = rsp.status().if_handle();
            NIC_LOG_DEBUG("Created Enic id: {} for Lif: {} handle: {}",
                            id, ethlif->GetId(), handle);
        } else {
            NIC_LOG_ERR("Failed to create Enic for Lif: {}. err: {}",
                          ethlif->GetId(), rsp.api_status());
        }
    } else {
        NIC_LOG_ERR("Failed to create Enic for Lif: {}. err: {}:{}",
                      ethlif->GetId(), status.error_code(), status.error_message());
    }

    // Store spec
    spec.CopyFrom(*req);
}

Enic::~Enic()
{
    grpc::ClientContext                   context;
    grpc::Status                          status;

    intf::InterfaceDeleteRequest          *req;
    intf::InterfaceDeleteResponse         rsp;
    intf::InterfaceDeleteRequestMsg       req_msg;
    intf::InterfaceDeleteResponseMsg      rsp_msg;

    NIC_LOG_DEBUG("Enic delete id: {}", id);

    req = req_msg.add_request();
    req->mutable_key_or_handle()->set_interface_id(id);
    status = hal->interface_delete(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            NIC_LOG_DEBUG("Deleted Enic id: {} handle: {}",
                            id, handle);
        } else {
            NIC_LOG_ERR("Failed to delete Enic for id: {}. err: {}",
                          id, rsp.api_status());
        }
    } else {
        NIC_LOG_ERR("Failed to delete Enic for id: {}. err: {}:{}",
                      id, status.error_code(), status.error_message());
    }
}

void
Enic::TriggerHalUpdate()
{
    grpc::ClientContext             context;
    grpc::Status                    status;

    intf::InterfaceSpec             *spec;
    intf::InterfaceResponse         rsp;
    intf::InterfaceRequestMsg       req_msg;
    intf::InterfaceResponseMsg      rsp_msg;

    HalL2Segment *l2seg;

    spec = req_msg.add_request();
    spec->mutable_key_or_handle()->set_interface_id(id);
    spec->set_type(::intf::IfType::IF_TYPE_ENIC);
    spec->set_admin_status(::intf::IfStatus::IF_STATUS_UP);
    spec->mutable_if_enic_info()->set_enic_type(::intf::IF_ENIC_TYPE_CLASSIC);
    spec->mutable_if_enic_info()->mutable_lif_key_or_handle()->
        set_lif_id(ethlif->GetId());
    spec->mutable_if_enic_info()->mutable_classic_enic_info()->
        set_native_l2segment_id(ethlif->GetNativeL2Seg()->GetId());
    for (auto l2seg_it = l2seg_refs.begin(); l2seg_it != l2seg_refs.end(); l2seg_it++) {
        l2seg = l2seg_it->second->l2seg;
        spec->mutable_if_enic_info()->mutable_classic_enic_info()->
            add_l2segment_key_handle()->set_segment_id(l2seg->GetId());
    }

    status = hal->interface_update(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            handle = rsp.status().if_handle();
            NIC_LOG_DEBUG("Enic update succeeded id: {}, handle: {}",
                            id, handle);
        } else {
            NIC_LOG_ERR("Failed to update Enic: err: {}", rsp.api_status());
        }
    } else {
        NIC_LOG_ERR("Failed to update Enic: err: {}, err_msg: {}",
                      status.error_code(),
                      status.error_message());
    }
}

void
Enic::AddVlan(vlan_t vlan)
{
    std::map<vlan_t, l2seg_info_t *>::iterator it;
    l2seg_info_t *l2seg_info;

    HalL2Segment *l2seg = HalL2Segment::Lookup(ethlif->GetVrf(), vlan);
    if (!l2seg) {
        // Create L2seg
        l2seg = HalL2Segment::Factory(ethlif->GetVrf(), vlan);

    }

    NIC_LOG_DEBUG("Adding vlan {} on Enic {}", vlan, id);

    /*
     * Native vlan is added at the time of enic create. Native vlan
     * doesnt go into l2seg list. So noop here.
     */
    if (vlan != NATIVE_VLAN_ID) {
        // Check for the presence of new vlan
        it = l2seg_refs.find(vlan);
        if (it != l2seg_refs.end()) {
            NIC_LOG_WARN("Enic already has L2seg {} with Vlan: {}",
                           it->second->l2seg->GetId(),
                           vlan);
            it->second->filter_ref_cnt++;
            return;
        }

        // Allocate l2seg info
        l2seg_info = new l2seg_info_t();

        l2seg_info->filter_ref_cnt++;
        l2seg_info->l2seg = l2seg;

        l2seg_refs[vlan] = l2seg_info;

        // Sends update to Hal
        TriggerHalUpdate();
    }
}

void
Enic::DelVlan(vlan_t vlan)
{
    std::map<vlan_t, l2seg_info_t *>::iterator it;
    l2seg_info_t *l2seg_info;
    HalL2Segment *l2seg = HalL2Segment::Lookup(ethlif->GetVrf(), vlan);

    NIC_LOG_DEBUG("Deleting vlan {} on Enic {}", vlan, id);

    if (vlan != NATIVE_VLAN_ID) {
        // Check for the presence of vlan
        it = l2seg_refs.find(vlan);
        if (it == l2seg_refs.end()) {
            NIC_LOG_ERR("Not able to find vlan: {}", l2seg->GetId());
            return;
        }

        l2seg_info = it->second;

        // Decrement ref count
        l2seg_info->filter_ref_cnt--;

        if (!l2seg_info->filter_ref_cnt) {
            // Del vlan from the map
            l2seg_refs.erase(vlan);

            // Sends update to Hal
            TriggerHalUpdate();

            // Delete L2seg
            HalL2Segment::Destroy(l2seg);

            // Free up l2seg_info
            delete(l2seg_info);
        }
    }
}


uint64_t
Enic::GetId()
{
    return id;
}

uint64_t
Enic::GetHandle()
{
    return handle;
}

HalL2Segment *
Enic::GetL2seg(vlan_t vlan)
{
    if (vlan == NATIVE_VLAN_ID) {
        return ethlif->GetNativeL2Seg();
    }

    std::map<vlan_t, l2seg_info_t *>::iterator it;
    it = l2seg_refs.find(vlan);
    if (it != l2seg_refs.end()) {
        return it->second->l2seg;
    }
    return NULL;
}
