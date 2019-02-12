
#include <iostream>
#include <grpc++/grpc++.h>

#include "enic.hpp"
#include "lif.hpp"

using namespace std;

#define HAL_NON_RSVD_IF_OFFSET 256

sdk::lib::indexer *Enic::allocator = sdk::lib::indexer::factory(Enic::max_enics, false, true);

Enic *
Enic::Factory(Lif *ethlif)
{
    hal_irisc_ret_t ret = HAL_IRISC_RET_SUCCESS;
    Enic *enic = new Enic(ethlif);

    ret = enic->HalEnicCreate();
    if (ret != HAL_IRISC_RET_SUCCESS) {
        NIC_LOG_DEBUG("Enic Create failed. ret: {}", ret);
        goto end;
    }

end:
    if (ret != HAL_IRISC_RET_SUCCESS) {
        if (enic) {
            delete enic;
            enic = NULL;
        }
    }
    return enic;
}

hal_irisc_ret_t
Enic::Destroy(Enic *enic)
{
    hal_irisc_ret_t ret = HAL_IRISC_RET_SUCCESS;
    ret = enic->HalEnicDelete();
    if (ret != HAL_IRISC_RET_SUCCESS) {
        NIC_LOG_CRIT("FATAL:Enic Delete failed. ret: {}", ret);
        goto end;
    }
    delete enic;

end:
    return ret;
}

// Classic ENIC constructor
Enic::Enic(Lif *ethlif)
{
    NIC_LOG_DEBUG("Enic create for lif: {}", ethlif->GetId());

    this->id = 0;
    this->_mac = 0;
    this->_vlan = 0;
    this->ethlif = ethlif;
}

// Classic ENIC constructor
hal_irisc_ret_t
Enic::HalEnicCreate()
{
    hal_irisc_ret_t                 ret = HAL_IRISC_RET_SUCCESS;
    grpc::ClientContext             context;
    grpc::Status                    status;

    intf::InterfaceSpec             *req;
    intf::InterfaceResponse         rsp;
    intf::InterfaceRequestMsg       req_msg;
    intf::InterfaceResponseMsg      rsp_msg;

    if (allocator->alloc(&id) != sdk::lib::indexer::SUCCESS) {
        NIC_LOG_ERR("Failed to allocate ENIC. Resource exhaustion");
        return HAL_IRISC_RET_FAIL;
    }

    id += HAL_NON_RSVD_IF_OFFSET;
    NIC_LOG_DEBUG("Enic create id: {}", id);

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
            NIC_LOG_DEBUG("Created Enic id: {} for Lif: {}",
                          id, ethlif->GetId());
        } else {
            NIC_LOG_ERR("Failed to create Enic for Lif: {}. err: {}",
                        ethlif->GetId(), rsp.api_status());
            ret = HAL_IRISC_RET_FAIL;
            goto end;
        }
    } else {
        NIC_LOG_ERR("Failed to create Enic for Lif: {}. err: {}:{}",
                    ethlif->GetId(), status.error_code(), status.error_message());
        ret = HAL_IRISC_RET_FAIL;
        goto end;
    }

    // Store spec
    spec.CopyFrom(*req);

end:
    return ret;
}

hal_irisc_ret_t
Enic::HalEnicDelete()
{
    hal_irisc_ret_t                       ret = HAL_IRISC_RET_SUCCESS;
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
            NIC_LOG_DEBUG("Deleted Enic id: {}", id);
        } else {
            NIC_LOG_ERR("Failed to delete Enic for id: {}. err: {}",
                          id, rsp.api_status());
            ret = HAL_IRISC_RET_FAIL;
        }
    } else {
        NIC_LOG_ERR("Failed to delete Enic for id: {}. err: {}:{}",
                      id, status.error_code(), status.error_message());
        ret = HAL_IRISC_RET_FAIL;
    }

    return ret;
}

hal_irisc_ret_t
Enic::TriggerHalUpdate()
{
    hal_irisc_ret_t                 ret = HAL_IRISC_RET_SUCCESS;
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
            NIC_LOG_DEBUG("Enic update succeeded id: {}", id);
        } else {
            NIC_LOG_ERR("Failed to update Enic: err: {}", rsp.api_status());
            ret = HAL_IRISC_RET_FAIL;
            goto end;
        }
    } else {
        NIC_LOG_ERR("Failed to update Enic: err: {}, err_msg: {}",
                      status.error_code(),
                      status.error_message());
        ret = HAL_IRISC_RET_FAIL;
        goto end;
    }

end:
    return ret;
}

hal_irisc_ret_t
Enic::AddVlan(vlan_t vlan)
{
    hal_irisc_ret_t                            ret = HAL_IRISC_RET_SUCCESS;
    std::map<vlan_t, l2seg_info_t *>::iterator it;
    l2seg_info_t                               *l2seg_info;

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
            goto end;
        }

        // Allocate l2seg info
        l2seg_info = new l2seg_info_t();

        l2seg_info->filter_ref_cnt++;
        l2seg_info->l2seg = l2seg;

        l2seg_refs[vlan] = l2seg_info;

        // Sends update to Hal
        ret = TriggerHalUpdate();
        if (ret != HAL_IRISC_RET_SUCCESS) {
            NIC_LOG_ERR("Unable to add vlan to Enic. ret: {}", ret);
            // Cleaning up
            DelVlan(vlan, true /* skip_hal */);
        }
    }

end:
    return ret;
}

void
Enic::DelVlan(vlan_t vlan, bool skip_hal)
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

            if (!skip_hal) {
                // Sends update to Hal
                TriggerHalUpdate();
            }

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
