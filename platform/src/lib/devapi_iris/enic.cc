//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include <iostream>
#include <grpc++/grpc++.h>
#include "platform/src/lib/nicmgr/include/logger.hpp"
#include "print.hpp"
#include "utils.hpp"
#include "enic.hpp"
#include "lif.hpp"
#include "utils.hpp"
#include "l2seg.hpp"
#include "devapi_mem.hpp"

namespace iris {

#define HAL_NON_RSVD_IF_OFFSET 256

indexer *devapi_enic::allocator_ = indexer::factory(devapi_enic::max_enics, false, true);

devapi_enic *
devapi_enic::factory(devapi_lif *lif)
{
    sdk_ret_t ret = SDK_RET_OK;
    void *mem = NULL;
    devapi_enic *enic = NULL;

    api_trace("enic create");

    mem = (devapi_enic *)DEVAPI_CALLOC(DEVAPI_MEM_ALLOC_ENIC,
                                       sizeof(devapi_enic));
    if (mem) {
        enic = new (mem) devapi_enic();
        ret = enic->init_(lif);
        if (ret != SDK_RET_OK) {
            goto end;
        }
        ret = enic->enic_halcreate();
        if (ret != SDK_RET_OK) {
            goto end;
        }
    }

end:
    if (ret != SDK_RET_OK) {
        enic->~devapi_enic();
        DEVAPI_FREE(DEVAPI_MEM_ALLOC_ENIC, mem);
        enic = NULL;
    }
    return enic;
}

sdk_ret_t
devapi_enic::init_(devapi_lif *lif)
{
    NIC_LOG_DEBUG("enic create for lif: {}", lif->get_id());

    if (allocator_->alloc(&id_) != indexer::SUCCESS) {
        NIC_LOG_ERR("Failed to allocate ENIC. Resource exhaustion");
        id_ = indexer::INVALID_INDEXER;
        return SDK_RET_ERR;
    }

    id_ += HAL_NON_RSVD_IF_OFFSET;
    mac_ = 0;
    vlan_ = 0;
    lif_ = lif;
    return SDK_RET_OK;
}

void
devapi_enic::destroy(devapi_enic *enic)
{
    enic->enic_haldelete();
    enic->deallocate_id();
    enic->~devapi_enic();
    DEVAPI_FREE(DEVAPI_MEM_ALLOC_ENIC, enic);
}

void
devapi_enic::deallocate_id(void)
{
    if (id_ != indexer::INVALID_INDEXER) {
        allocator_->free(id_);
    }
}

sdk_ret_t
devapi_enic::enic_halcreate(void)
{
    sdk_ret_t             ret = SDK_RET_OK;
    grpc::ClientContext   context;
    grpc::Status          status;
    InterfaceSpec         *req;
    InterfaceResponse     rsp;
    InterfaceRequestMsg   req_msg;
    InterfaceResponseMsg  rsp_msg;

    NIC_LOG_DEBUG("devapi_enic create id: {}", id_);

    req = req_msg.add_request();
    req->mutable_key_or_handle()->set_interface_id(id_);
    req->set_type(::intf::IfType::IF_TYPE_ENIC);
    req->set_admin_status(::intf::IfStatus::IF_STATUS_UP);
    req->mutable_if_enic_info()->set_enic_type(::intf::IF_ENIC_TYPE_CLASSIC);
    req->mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(lif_->get_id());
    if (lif_->get_nativel2seg()) {
        req->mutable_if_enic_info()->mutable_classic_enic_info()->
            set_native_l2segment_id(lif_->get_nativel2seg()->get_id());
    }

    VERIFY_HAL();
    status = hal->interface_create(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            NIC_LOG_DEBUG("Created devapi_enic id: {} for Lif: {}",
                          id_, lif_->get_id());
        } else {
            NIC_LOG_ERR("Failed to create devapi_enic for Lif: {}. err: {}",
                        lif_->get_id(), rsp.api_status());
            ret = SDK_RET_ERR;
            goto end;
        }
    } else {
        NIC_LOG_ERR("Failed to create devapi_enic for Lif: {}. err: {}:{}",
                    lif_->get_id(), status.error_code(), status.error_message());
        ret = SDK_RET_ERR;
        goto end;
    }

end:
    return ret;
}

sdk_ret_t
devapi_enic::enic_haldelete(void)
{
    sdk_ret_t                       ret = SDK_RET_OK;
    grpc::ClientContext             context;
    grpc::Status                    status;
    InterfaceDeleteRequest          *req;
    InterfaceDeleteResponse         rsp;
    InterfaceDeleteRequestMsg       req_msg;
    InterfaceDeleteResponseMsg      rsp_msg;

    NIC_LOG_DEBUG("enic delete id: {}", id_);

    req = req_msg.add_request();
    req->mutable_key_or_handle()->set_interface_id(id_);
    VERIFY_HAL();
    status = hal->interface_delete(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            NIC_LOG_DEBUG("Deleted devapi_enic id: {}", id_);
        } else {
            NIC_LOG_ERR("Failed to delete devapi_enic for id: {}. err: {}",
                        id_, rsp.api_status());
            ret = SDK_RET_ERR;
        }
    } else {
        NIC_LOG_ERR("Failed to delete devapi_enic for id: {}. err: {}:{}",
                    id_, status.error_code(), status.error_message());
        ret = SDK_RET_ERR;
    }

end:
    return ret;
}

sdk_ret_t
devapi_enic::trigger_halupdate(void)
{
    sdk_ret_t                 ret = SDK_RET_OK;
    grpc::ClientContext       context;
    grpc::Status              status;
    InterfaceSpec             *spec;
    InterfaceResponse         rsp;
    InterfaceRequestMsg       req_msg;
    InterfaceResponseMsg      rsp_msg;
    devapi_l2seg              *l2seg;

    spec = req_msg.add_request();
    spec->mutable_key_or_handle()->set_interface_id(id_);
    spec->set_type(::intf::IfType::IF_TYPE_ENIC);
    spec->set_admin_status(::intf::IfStatus::IF_STATUS_UP);
    spec->mutable_if_enic_info()->set_enic_type(::intf::IF_ENIC_TYPE_CLASSIC);
    spec->mutable_if_enic_info()->mutable_lif_key_or_handle()->
        set_lif_id(lif_->get_id());
    spec->mutable_if_enic_info()->mutable_classic_enic_info()->
        set_native_l2segment_id(lif_->get_nativel2seg()->get_id());
    for (auto l2seg_it = l2seg_refs_.begin(); l2seg_it != l2seg_refs_.end(); l2seg_it++) {
        l2seg = l2seg_it->second->l2seg;
        spec->mutable_if_enic_info()->mutable_classic_enic_info()->
            add_l2segment_key_handle()->set_segment_id(l2seg->get_id());
    }

    VERIFY_HAL();
    status = hal->interface_update(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            NIC_LOG_DEBUG("devapi_enic update succeeded id: {}", id_);
        } else {
            NIC_LOG_ERR("Failed to update devapi_enic: err: {}", rsp.api_status());
            ret = SDK_RET_ERR;
            goto end;
        }
    } else {
        NIC_LOG_ERR("Failed to update devapi_enic: err: {}, err_msg: {}",
                      status.error_code(),
                      status.error_message());
        ret = SDK_RET_ERR;
        goto end;
    }

end:
    return ret;
}

sdk_ret_t
devapi_enic::add_vlan(vlan_t vlan)
{
    sdk_ret_t ret = SDK_RET_OK;
    l2seg_info_t *l2seg_info;
    std::map<vlan_t, l2seg_info_t *>::iterator it;

    devapi_l2seg *l2seg = devapi_l2seg::lookup(lif_->get_vrf(), vlan);
    if (!l2seg) {
        // Create L2seg
        l2seg = devapi_l2seg::factory(lif_->get_vrf(), vlan);
    }

    NIC_LOG_DEBUG("Adding vlan {} on devapi_enic {}", vlan, id_);

    /*
     * Native vlan is added at the time of enic create. Native vlan
     * doesnt go into l2seg list. So noop here.
     */
    if (vlan != NATIVE_VLAN_ID) {
        // Check for the presence of new vlan
        it = l2seg_refs_.find(vlan);
        if (it != l2seg_refs_.end()) {
            NIC_LOG_WARN("devapi_enic already has L2seg {} with Vlan: {}",
                           it->second->l2seg->get_id(),
                           vlan);
            it->second->filter_ref_cnt++;
            goto end;
        }

        // Allocate l2seg info
        l2seg_info = (l2seg_info_t *)DEVAPI_CALLOC(DEVAPI_MEM_L2SEG_INFO,
                                                   sizeof(l2seg_info_t));

        l2seg_info->filter_ref_cnt++;
        l2seg_info->l2seg = l2seg;

        l2seg_refs_[vlan] = l2seg_info;
        l2seg->add_enic(this);

        // Sends update to Hal
        ret = trigger_halupdate();
        if (ret != SDK_RET_OK) {
            NIC_LOG_ERR("Unable to add vlan to devapi_enic. ret: {}", ret);
            // Cleaning up
            del_vlan(vlan, true /* skip_hal */);
        }
    } else {
#if 0
        // vlan is NATIVE_VLAN_ID. for SWM Lif we may have to retrigger native l2seg change
        if (lif_->is_swm()) {
            ret = trigger_halupdate();
        }
#endif
    }

end:
    return ret;
}

void
devapi_enic::del_vlan(vlan_t vlan, bool skip_hal)
{
    std::map<vlan_t, l2seg_info_t *>::iterator it;
    l2seg_info_t *l2seg_info;
    devapi_l2seg *l2seg = devapi_l2seg::lookup(lif_->get_vrf(), vlan);

    NIC_LOG_DEBUG("Deleting vlan {} on devapi_enic {}", vlan, id_);

    if (vlan != NATIVE_VLAN_ID) {
        // Check for the presence of vlan
        it = l2seg_refs_.find(vlan);
        if (it == l2seg_refs_.end()) {
            NIC_LOG_ERR("Not able to find vlan: {}", l2seg->get_id());
            return;
        }

        l2seg_info = it->second;

        // Decrement ref count
        l2seg_info->filter_ref_cnt--;

        if (!l2seg_info->filter_ref_cnt) {
            // Del vlan from the map
            l2seg_refs_.erase(vlan);

            if (!skip_hal) {
                // Sends update to Hal
                trigger_halupdate();
            }

            l2seg->del_enic(this);
            if (!l2seg->is_single_wire_mgmt() && !l2seg->num_enics()) {
                // Delete L2seg
                devapi_l2seg::destroy(l2seg);
            }

            // Free up l2seg_info
            DEVAPI_FREE(DEVAPI_MEM_L2SEG_INFO, l2seg_info);
        }
    }
}


uint32_t
devapi_enic::get_id(void)
{
    return id_;
}

devapi_l2seg *
devapi_enic::get_l2seg(vlan_t vlan)
{
    std::map<vlan_t, l2seg_info_t *>::iterator it;

    if (vlan == NATIVE_VLAN_ID) {
        return lif_->get_nativel2seg();
    }

    it = l2seg_refs_.find(vlan);
    if (it != l2seg_refs_.end()) {
        return it->second->l2seg;
    }
    return NULL;
}

}     // namespace iris
