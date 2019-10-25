//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include <grpc++/grpc++.h>
#include "gen/proto/types.grpc.pb.h"
#include "platform/src/lib/nicmgr/include/logger.hpp"
#include "uplink.hpp"
#include "vrf.hpp"
#include "devapi_mem.hpp"
#include "l2seg.hpp"
#include "print.hpp"
#include "utils.hpp"

namespace iris {

std::map<uint64_t, devapi_uplink*> devapi_uplink::uplink_db_;

devapi_uplink *
devapi_uplink::factory(uplink_id_t id, uint32_t port_num, bool is_oob)
{
    sdk_ret_t ret = SDK_RET_OK;
    void *mem = NULL;
    devapi_uplink *up = NULL;

    api_trace("uplink create");

    if (uplink_db_.find(port_num) != uplink_db_.end()) {
        NIC_LOG_WARN("Duplicate Create of uplink with port: {}",
                     port_num);
        return NULL;
    }

    NIC_LOG_DEBUG("Id: {}, port: {}, is_oob: {}",
                  id, port_num, is_oob);

    mem = (devapi_uplink *)DEVAPI_CALLOC(DEVAPI_MEM_ALLOC_UPLINK,
                                  sizeof(devapi_uplink));
    if (mem) {
        up = new (mem) devapi_uplink();
        ret = up->init_(id, port_num, is_oob);
        if (ret != SDK_RET_OK) {
            up->~devapi_uplink();
            DEVAPI_FREE(DEVAPI_MEM_ALLOC_UPLINK, mem);
            up = NULL;
            goto end;
        }
    }

    // Store in DB
    uplink_db_[up->get_port_num()] = up;

end:
    return up;
}

sdk_ret_t
devapi_uplink::init_(uplink_id_t id, uint32_t port_num, bool is_oob)
{
    id_           = id;
    port_num_     = port_num;
    num_lifs_     = 0;
    is_oob_       = is_oob;
    vrf_          = NULL;
    native_l2seg_ = NULL;

    if (is_oob) {
        // Get lif for oob to be used in SWM
        populate_lif_(id);
    }

    return SDK_RET_OK;
}

void
devapi_uplink::destroy(devapi_uplink *uplink)
{
    if (!uplink) {
        return;
    }
    api_trace("uplink delete");

    // Remove from DB
    uplink_db_.erase(uplink->get_port_num());

    uplink->~devapi_uplink();
    DEVAPI_FREE(DEVAPI_MEM_ALLOC_UPLINK, uplink);
}

sdk_ret_t
devapi_uplink::populate_lif_(uplink_id_t id)
{
    sdk_ret_t                 ret = SDK_RET_OK;
    grpc::Status              status;
    InterfaceGetRequest       *req __attribute__((unused));
    InterfaceGetResponse      rsp;
    InterfaceGetRequestMsg    req_msg;
    InterfaceGetResponseMsg   rsp_msg;

    req = req_msg.add_request();
    req->mutable_key_or_handle()->set_interface_id(id_);
    VERIFY_HAL();
    status = hal->interface_get(req_msg, rsp_msg);
    if (status.ok()) {
        SDK_ASSERT(rsp_msg.response().size() == 1);
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            if (rsp.spec().type() == intf::IF_TYPE_UPLINK) {
                lif_id_ = rsp.status().uplink_info().hw_lif_id();
            }
        }
    }

end:
    NIC_LOG_DEBUG("oob uplink's lif: {}", lif_id_);
    return ret;
}

devapi_uplink *
devapi_uplink::get_uplink(uint32_t port_num)
{
    if (uplink_db_.find(port_num) == uplink_db_.end()) {
        NIC_LOG_WARN("No uplink with port: {}",
                     port_num);
        return NULL;
    }

    return uplink_db_[port_num];
}

devapi_uplink *
devapi_uplink::get_oob_uplink(void)
{
    devapi_uplink *uplink = NULL;

    for (auto it = uplink_db_.cbegin(); it != uplink_db_.cend(); it++) {
        uplink = (devapi_uplink *)(it->second);
        if (uplink->is_oob()) {
            return uplink;
        }
    }
    return NULL;
}

sdk_ret_t
devapi_uplink::create_vrf(void)
{
    // In both Classic and hostpin modes, every uplink will get a VRF.
    vrf_ = devapi_vrf::factory(is_oob() ?
                               types::VRF_TYPE_OOB_MANAGEMENT :
                               types::VRF_TYPE_INBAND_MANAGEMENT,
                               this);
    if (vrf_ == NULL) {
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
devapi_uplink::delete_vrf(void)
{
    if (vrf_) {
        // Delete Vrf
        devapi_vrf::destroy(vrf_);
        vrf_ = NULL;
    }
    return SDK_RET_OK;
}

sdk_ret_t
devapi_uplink::create_vrfs(void)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi_uplink *uplink = NULL;

    NIC_LOG_DEBUG("Creating VRFs for uplinks");

    for (auto it = uplink_db_.cbegin(); it != uplink_db_.cend(); it++) {
        uplink = (devapi_uplink *)(it->second);
        ret = uplink->create_vrf();
        if (ret != SDK_RET_OK) {
            NIC_LOG_ERR("Failed creating VRF for uplink: {}", uplink->get_id());
            goto end;
        }
        NIC_LOG_DEBUG("Created VRF for uplink: {}", uplink->get_id());
    }
end:
    return ret;
}

sdk_ret_t
devapi_uplink::update_hal_native_l2seg(uint32_t native_l2seg_id)
{
    sdk_ret_t                 ret = SDK_RET_OK;
    grpc::Status              status;
    InterfaceGetRequest       *req __attribute__((unused));
    InterfaceGetResponse      rsp;
    InterfaceGetRequestMsg    req_msg;
    InterfaceGetResponseMsg   rsp_msg;
    InterfaceSpec             *if_spec;
    InterfaceResponse         if_rsp;
    InterfaceRequestMsg       if_req_msg;
    InterfaceResponseMsg      if_rsp_msg;

    req = req_msg.add_request();
    req->mutable_key_or_handle()->set_interface_id(id_);
    VERIFY_HAL();
    status = hal->interface_get(req_msg, rsp_msg);
    if (status.ok()) {
        NIC_LOG_DEBUG("Updated uplink:{} with native l2seg:{}", id_, native_l2seg_id);
        for (int i = 0; i < rsp_msg.response().size(); i++) {
            rsp = rsp_msg.response(i);
            if (rsp.api_status() == types::API_STATUS_OK) {
                if (rsp.spec().type() == intf::IF_TYPE_UPLINK) {

                    if_spec = if_req_msg.add_request();
                    if_spec->CopyFrom(rsp.spec());
                    if_spec->mutable_if_uplink_info()->
                        set_native_l2segment_id(native_l2seg_id);
                    if_spec->mutable_if_uplink_info()->set_is_oob_management(is_oob_);

                    status = hal->interface_update(if_req_msg, if_rsp_msg);
                    if (status.ok()) {
                        rsp = rsp_msg.response(0);
                        if (rsp.api_status() == types::API_STATUS_OK) {
                            NIC_LOG_DEBUG("uplink {} updated with "
                                          "native_l2seg_id: {} succeeded",
                                          id_, native_l2seg_id);
                        } else {
                            NIC_LOG_ERR("Failed to update uplink's:{} "
                                        "native_l2seg_id:{}: err: {}",
                                        id_, native_l2seg_id,
                                        rsp.api_status());
                        }
                    } else {
                        NIC_LOG_ERR("Failed to update uplink's:{} "
                                    "native_l2seg_id:{}: err: {}, err_msg: {}",
                                    status.error_code(),
                                    status.error_message());
                    }
                }
            }
        }
    }

end:
    return ret;
}

uint32_t
devapi_uplink::get_id(void)
{
    return id_;
}

uint32_t
devapi_uplink::get_port_num(void)
{
    return port_num_;
}

uint32_t
devapi_uplink::get_num_lifs(void)
{
    return num_lifs_;
}

void
devapi_uplink::inc_num_lifs(void)
{
    num_lifs_++;
}

void
devapi_uplink::dec_num_lifs(void)
{
    num_lifs_--;
}

devapi_vrf *
devapi_uplink::get_vrf(void)
{
    return vrf_;
}

devapi_l2seg *
devapi_uplink::get_native_l2seg(void)
{
    return native_l2seg_;
}

uint32_t 
devapi_uplink::get_lif_id(void)
{
    return lif_id_;
}

bool
devapi_uplink::is_oob(void)
{
    return is_oob_;
}

void
devapi_uplink::set_native_l2seg(devapi_l2seg *l2seg)
{
    native_l2seg_ = l2seg;
}

void
devapi_uplink::set_port_num(uint32_t port_num)
{
    port_num_ = port_num;
}

}    // namespace iris
