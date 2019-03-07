//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include <grpc++/grpc++.h>
#include "platform/src/lib/nicmgr/include/logger.hpp"
#include "multicast.hpp"
#include "print.hpp"
#include "utils.hpp"
#include "l2seg.hpp"
#include "enic.hpp"
#include "devapi_mem.hpp"

namespace iris {

std::map<mcast_key_t, devapi_mcast*> devapi_mcast::mcast_db_;

devapi_mcast *
devapi_mcast::factory(devapi_l2seg *l2seg, mac_t mac)
{
    sdk_ret_t ret = SDK_RET_OK;
    mcast_key_t key(l2seg, mac);
    void *mem = NULL;
    devapi_mcast *mcast = NULL;

    if (mcast_db_.find(key) != mcast_db_.end()) {
        NIC_LOG_WARN("Duplicate Create of mcast with l2seg: {}, mac: {}",
                     l2seg->get_id(), macaddr2str(mac));
        return NULL;
    }

    NIC_LOG_DEBUG("mcast create: l2seg: {}, mac: {}",
                  l2seg->get_id(), macaddr2str(mac));

    mem = (devapi_mcast *)DEVAPI_CALLOC(DEVAPI_MEM_ALLOC_MCAST,
                                  sizeof(devapi_mcast));
    if (mem) {
        mcast = new (mem) devapi_mcast();
        ret = mcast->init_(l2seg, mac);
        if (ret != SDK_RET_OK) {
            goto end;
        }
        ret = mcast->mcast_halcreate();
        if (ret != SDK_RET_OK) {
            goto end;
        }
    }

    // Store in DB
    mcast_db_[key] = mcast;

end:
    if (ret != SDK_RET_OK) {
        mcast->~devapi_mcast();
        DEVAPI_FREE(DEVAPI_MEM_ALLOC_MCAST, mem);
        mcast = NULL;
        goto end;
    }
    return mcast;
}

sdk_ret_t
devapi_mcast::init_(devapi_l2seg *l2seg, mac_t mac)
{
    NIC_LOG_DEBUG("Mcast create: l2seg: {}, mac: {}",
                  l2seg->get_id(), macaddr2str(mac));
    mac_ = mac;
    l2seg_ = l2seg;

    return SDK_RET_OK;
}

void
devapi_mcast::destroy(devapi_mcast *mcast)
{
    mcast_key_t key(mcast->get_l2seg(), mcast->get_mac());

    // remove from hal
    mcast->mcast_haldelete();
    // remove from db
    mcast_db_.erase(key);
    // destructor
    mcast->~devapi_mcast();
    // free mem
    DEVAPI_FREE(DEVAPI_MEM_ALLOC_MCAST, mcast);
}

sdk_ret_t
devapi_mcast::mcast_halcreate(void)
{
    sdk_ret_t ret = SDK_RET_OK;
    grpc::ClientContext context;
    grpc::Status status;
    MulticastEntrySpec *spec;
    MulticastEntryResponse rsp;
    MulticastEntryRequestMsg req_msg;
    MulticastEntryResponseMsg rsp_msg;

    spec = req_msg.add_request();
    // spec->mutable_meta()->set_vrf_id(l2seg->GetVrf()->get_id());
    spec->mutable_key_or_handle()->mutable_key()->mutable_l2segment_key_handle()->
        set_segment_id(l2seg_->get_id());
    spec->mutable_key_or_handle()->mutable_key()->mutable_mac()->set_group(mac_);

    VERIFY_HAL();
    status = hal->multicast_create(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            NIC_LOG_DEBUG("Created Mcast L2seg: {}, Mac: {}", l2seg_->get_id(),
                          macaddr2str(mac_));
        } else {
            NIC_LOG_ERR("Failed to create Mcast L2seg: {}, Mac: {} err: {}",
                        l2seg_->get_id(),
                        macaddr2str(mac_), rsp.api_status());
            ret = SDK_RET_ERR;
        }
    } else {
        NIC_LOG_ERR("Failed to create Mcast L2seg: {}, Mac: {}. err: {}, msg: {}",
                    l2seg_->get_id(),
                    macaddr2str(mac_), status.error_code(), status.error_message());
        ret = SDK_RET_ERR;
    }

end:
    return ret;
}

sdk_ret_t
devapi_mcast::mcast_haldelete(void)
{
    sdk_ret_t ret = SDK_RET_OK;
    grpc::ClientContext context;
    grpc::Status status;
    MulticastEntryDeleteRequest *req;
    MulticastEntryDeleteResponse rsp;
    MulticastEntryDeleteRequestMsg req_msg;
    MulticastEntryDeleteResponseMsg rsp_msg;

    req = req_msg.add_request();
    req->mutable_key_or_handle()->mutable_key()->mutable_l2segment_key_handle()->
        set_segment_id(l2seg_->get_id());
    req->mutable_key_or_handle()->mutable_key()->mutable_mac()->set_group(mac_);

    VERIFY_HAL();
    status = hal->multicast_delete(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            NIC_LOG_ERR("Failed to delete Mcast L2seg: {}, Mac: {}. err: {}",
                        l2seg_->get_id(),
                        macaddr2str(mac_), rsp.api_status());
            ret = SDK_RET_ERR;
        } else {
            NIC_LOG_DEBUG("Delete Mcast L2seg: {}, Mac: {}", l2seg_->get_id(),
                          macaddr2str(mac_));
        }
    } else {
        NIC_LOG_ERR("Failed to delete Mcast L2seg: {}, Mac: {}. err: {}, msg: {}",
                    l2seg_->get_id(),
                    macaddr2str(mac_), status.error_code(), status.error_message());
        ret = SDK_RET_ERR;
    }

end:
    return ret;
}

devapi_mcast *
devapi_mcast::find_or_create(devapi_l2seg *l2seg, mac_t mac, bool create)
{
    std::map<mcast_key_t, devapi_mcast*>::iterator it;
    mcast_key_t key(l2seg, mac);
    devapi_mcast *mcast = NULL;

    it = mcast_db_.find(key);
    if (it == mcast_db_.end()) {
        if (create) {
            // Create the multicast
            mcast = devapi_mcast::factory(l2seg, mac);
        }
        return mcast;
    } else {
        return it->second;
    }
}

sdk_ret_t
devapi_mcast::trigger_hal(void)
{
    sdk_ret_t ret = SDK_RET_OK;
    grpc::ClientContext context;
    grpc::Status status;
    MulticastEntrySpec *spec;
    MulticastEntryResponse rsp;
    MulticastEntryRequestMsg req_msg;
    MulticastEntryResponseMsg rsp_msg;

    spec = req_msg.add_request();
    spec->mutable_key_or_handle()->mutable_key()->mutable_l2segment_key_handle()->
        set_segment_id(l2seg_->get_id());
    spec->mutable_key_or_handle()->mutable_key()->mutable_mac()->set_group(mac_);
    for (auto it = enic_refs_.cbegin(); it != enic_refs_.cend(); it++) {
        spec->add_oif_key_handles()->set_interface_id(it->second->get_id());
    }

    VERIFY_HAL();
    status = hal->multicast_update(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            NIC_LOG_DEBUG("Updated Mcast L2seg: {}, Mac: {}", l2seg_->get_id(),
                macaddr2str(mac_));
        } else {
            NIC_LOG_ERR("Failed to update Mcast L2seg: {}, Mac: {}. err: {}",
                        l2seg_->get_id(),
                        macaddr2str(mac_), rsp.api_status());
        }
    } else {
        NIC_LOG_ERR("Failed to update Mcast L2seg: {}, Mac: {}. err: {}, msg: {}",
                    l2seg_->get_id(),
                    macaddr2str(mac_), status.error_code(), status.error_message());
    }

end:
    return ret;
}

sdk_ret_t
devapi_mcast::add_enic(devapi_enic *enic)
{
    enic_refs_[enic->get_id()] = enic;
    return trigger_hal();
}

sdk_ret_t
devapi_mcast::del_enic(devapi_enic *enic)
{
    enic_refs_.erase(enic->get_id());
    return trigger_hal();
}

devapi_l2seg *
devapi_mcast::get_l2seg(void)
{
    return l2seg_;
}
mac_t
devapi_mcast::get_mac(void)
{
    return mac_;
}

uint32_t
devapi_mcast::get_numenics(void)
{
    return enic_refs_.size();
}

} // namespace iris
