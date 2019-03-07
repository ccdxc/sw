//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include "gen/proto/kh.grpc.pb.h"
#include "gen/proto/types.grpc.pb.h"
#include "platform/src/lib/nicmgr/include/logger.hpp"
#include "endpoint.hpp"
#include "print.hpp"
#include "utils.hpp"
#include "devapi_mem.hpp"
#include "hal_grpc.hpp"
#include "l2seg.hpp"
#include "enic.hpp"
#include "vrf.hpp"

namespace iris {

std::map<ep_key_t, devapi_ep*> devapi_ep::ep_db_;

devapi_ep *
devapi_ep::factory(devapi_l2seg *l2seg, mac_t mac, devapi_enic *enic)
{
    sdk_ret_t ret = SDK_RET_OK;
    ep_key_t key(l2seg, mac);
    void *mem = NULL;
    devapi_ep *ep = NULL;

    if (ep_db_.find(key) != ep_db_.end()) {
        NIC_LOG_WARN("Duplicate Create of EP with l2seg: {}, mac: {}, enic: {}",
                     l2seg->get_id(), macaddr2str(mac), enic->get_id());
        return NULL;
    }

    NIC_LOG_DEBUG("EP Create: l2seg: {}, mac: {}, enic: {}",
                  l2seg->get_id(), macaddr2str(mac), enic->get_id());

    mem = (devapi_ep *)DEVAPI_CALLOC(DEVAPI_MEM_ALLOC_EP,
                                  sizeof(devapi_ep));
    if (mem) {
        ep = new (mem) devapi_ep();
        ret = ep->init_(l2seg, mac, enic);
        if (ret != SDK_RET_OK) {
            goto end;
        }
        ret = ep->ep_halcreate();
        if (ret != SDK_RET_OK) {
            goto end;
        }
    }

    // Store in DB
    ep_db_[key] = ep;

end:
    if (ret != SDK_RET_OK) {
        ep->~devapi_ep();
        DEVAPI_FREE(DEVAPI_MEM_ALLOC_EP, mem);
        ep = NULL;
        goto end;
    }
    return ep;
}

sdk_ret_t
devapi_ep::init_(devapi_l2seg *l2seg, mac_t mac, devapi_enic *enic)
{
    mac_   = mac;
    l2seg_ = l2seg;
    enic_  = enic;

    return SDK_RET_OK;
}

void
devapi_ep::destroy(devapi_ep *ep)
{
    ep_key_t key(ep->get_l2seg(), ep->get_mac());

    // remove from hal
    ep->ep_haldelete();
    // remove from db
    ep_db_.erase(key);
    // destructor
    ep->~devapi_ep();
    // free mem
    DEVAPI_FREE(DEVAPI_MEM_ALLOC_EP, ep);
}

devapi_ep *
devapi_ep::lookup(devapi_l2seg *l2seg, mac_t mac)
{
    ep_key_t key(l2seg, mac);

    if (ep_db_.find(key) != ep_db_.cend()) {
        return ep_db_[key];
    } else {
        return NULL;
    }
}

sdk_ret_t
devapi_ep::ep_halcreate(void)
{
    sdk_ret_t             ret = SDK_RET_OK;
    grpc::ClientContext   context;
    grpc::Status          status;
    EndpointSpec          *req;
    EndpointResponse      rsp;
    EndpointRequestMsg    req_msg;
    EndpointResponseMsg   rsp_msg;

    req = req_msg.add_request();
    req->mutable_vrf_key_handle()->set_vrf_id(l2seg_->get_vrf()->get_id());
    req->mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->
        set_mac_address(mac_);
    req->mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->
        mutable_l2segment_key_handle()->set_segment_id(l2seg_->get_id());
    req->mutable_endpoint_attrs()->mutable_interface_key_handle()->
        set_interface_id(enic_->get_id());
    VERIFY_HAL();
    status = hal->endpoint_create(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            NIC_LOG_ERR("Failed to create EP L2seg: {}, Mac: {}. err: {}",
                        l2seg_->get_id(), macaddr2str(mac_),
                        rsp.api_status());
            ret = SDK_RET_ERR;
        } else {
            NIC_LOG_DEBUG("Created EP L2seg: {}, Mac: {}", l2seg_->get_id(),
                          macaddr2str(mac_));
        }
    } else {
        NIC_LOG_ERR("Failed to create EP L2seg: {}, Mac: {}. err: {}, msg: {}",
                    l2seg_->get_id(), macaddr2str(mac_),
                    status.error_code(), status.error_message());
        ret = SDK_RET_ERR;
    }

end:
    return ret;
}

sdk_ret_t
devapi_ep::ep_haldelete(void)
{
    sdk_ret_t                   ret = SDK_RET_OK;
    grpc::ClientContext         context;
    grpc::Status                status;
    EndpointDeleteRequest       *req;
    EndpointDeleteResponse      rsp;
    EndpointDeleteRequestMsg    req_msg;
    EndpointDeleteResponseMsg   rsp_msg;

    NIC_LOG_DEBUG("EP delete: l2seg: {}, mac: {}, enic: {}",
                  l2seg_->get_id(), macaddr2str(mac_), enic_->get_id());

    req = req_msg.add_request();
    req->mutable_vrf_key_handle()->set_vrf_id(l2seg_->get_vrf()->get_id());
    req->mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->
        set_mac_address(mac_);
    req->mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->
        mutable_l2segment_key_handle()->set_segment_id(l2seg_->get_id());

    VERIFY_HAL();
    status = hal->endpoint_delete(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            NIC_LOG_ERR("Failed to delete EP L2seg: {}, Mac: {}. err: {}",
                        l2seg_->get_id(), macaddr2str(mac_),
                        rsp.api_status());
            ret = SDK_RET_ERR;
        } else {
            NIC_LOG_DEBUG("Delete EP L2seg: {}, Mac: {}",
                          l2seg_->get_id(), macaddr2str(mac_));
        }
    } else {
        NIC_LOG_ERR("Failed to delete EP L2seg: {}, Mac: {}. err: {}, msg: {}",
                    l2seg_->get_id(), macaddr2str(mac_),
                    status.error_code(), status.error_message());
        ret = SDK_RET_ERR;
    }

end:
    return ret;
}

devapi_l2seg *
devapi_ep::get_l2seg(void)
{
    return l2seg_;
}
mac_t
devapi_ep::get_mac(void)
{
    return mac_;
}

}    // namespace iris
