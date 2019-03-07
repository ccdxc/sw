//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include <iostream>
#include <grpc++/grpc++.h>
#include "nic/include/globals.hpp"
#include "platform/src/lib/nicmgr/include/logger.hpp"
#include "vrf.hpp"
#include "uplink.hpp"
#include "print.hpp"
#include "devapi_mem.hpp"
#include "hal_grpc.hpp"
#include "utils.hpp"
#include "print.hpp"

namespace iris {

indexer *devapi_vrf::allocator_ = indexer::factory(devapi_vrf::max_vrfs, false, true);

devapi_vrf *
devapi_vrf::factory (types::VrfType type, devapi_uplink *up)
{
    sdk_ret_t ret = SDK_RET_OK;
    void *mem = NULL;
    devapi_vrf *v = NULL;

    api_trace("vrf create");

    mem = (devapi_vrf *)DEVAPI_CALLOC(DEVAPI_MEM_ALLOC_VRF,
                                      sizeof(devapi_vrf));
    if (mem) {
        v = new (mem) devapi_vrf();
        ret = v->init_(type, up);
        if (ret != SDK_RET_OK) {
            goto end;
        }
        ret = v->vrf_halcreate();
        if (ret != SDK_RET_OK) {
            goto end;
        }
    }

end:
    if (ret != SDK_RET_OK) {
        v->deallocate_id();
        v->~devapi_vrf();
        DEVAPI_FREE(DEVAPI_MEM_ALLOC_VRF, mem);
        v = NULL;
    }
    return v;
}

sdk_ret_t
devapi_vrf::init_(types::VrfType type, devapi_uplink *uplink)
{
    if (allocator_->alloc(&id_) != indexer::SUCCESS) {
        NIC_LOG_ERR("Failed to allocate vrf. Resource exhaustion");
        id_ = indexer::INVALID_INDEXER;
        return SDK_RET_ERR;
    }

    id_ += NICMGR_VRF_ID_MIN;
    type_ = type;
    uplink_ = uplink;
    return SDK_RET_OK;
}

void
devapi_vrf::destroy(devapi_vrf *v)
{
    api_trace("vrf delete");

    v->vrf_haldelete();
    v->deallocate_id();
    v->~devapi_vrf();
    DEVAPI_FREE(DEVAPI_MEM_ALLOC_VRF, v);
}

void
devapi_vrf::deallocate_id(void)
{
    if (id_ != indexer::INVALID_INDEXER) {
        allocator_->free(id_);
    }
}

sdk_ret_t
devapi_vrf::vrf_halcreate(void)
{
    sdk_ret_t             ret = SDK_RET_OK;
    grpc::ClientContext   context;
    grpc::Status          status;
    vrf::VrfSpec          *req;
    vrf::VrfResponse      rsp;
    vrf::VrfRequestMsg    req_msg;
    vrf::VrfResponseMsg   rsp_msg;

    NIC_LOG_DEBUG("vrf create id: {}", id_);

    req = req_msg.add_request();
    req->mutable_key_or_handle()->set_vrf_id(id_);
    req->set_vrf_type(type_);

    VERIFY_HAL();
    status = hal->vrf_create(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
        } else if (rsp.api_status() == types::API_STATUS_EXISTS_ALREADY) {
            NIC_LOG_ERR("vrf already exists with id: {}", id_);
            ret = SDK_RET_ERR;
            goto end;
        } else {
            NIC_LOG_ERR("Failed to create vrf for id:{}. err: {}",
                        id_, rsp.api_status());
            ret = SDK_RET_ERR;
            goto end;
        }
    } else {
        NIC_LOG_ERR("Failed to create vrf for id:{}. err: {}:{}",
                    id_, status.error_code(), status.error_message());
        ret = SDK_RET_ERR;
        goto end;
    }

end:
    return ret;
}

sdk_ret_t
devapi_vrf::vrf_haldelete(void)
{
    sdk_ret_t                 ret = SDK_RET_OK;
    grpc::ClientContext       context;
    grpc::Status              status;

    vrf::VrfDeleteRequest     *req;
    vrf::VrfDeleteResponse    rsp;
    vrf::VrfDeleteRequestMsg  req_msg;
    vrf::VrfDeleteResponseMsg rsp_msg;

    NIC_LOG_DEBUG("vrf delete id: {}", id_);

    req = req_msg.add_request();
    req->mutable_key_or_handle()->set_vrf_id(id_);

    VERIFY_HAL();
    status = hal->vrf_delete(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            NIC_LOG_ERR("Failed to delete Vrf for id: {}. err: {}",
                          id_, rsp.api_status());
            ret = SDK_RET_ERR;
            goto end;
        } else {
            NIC_LOG_DEBUG("Deleted Vrf id: {}", id_);
        }
    } else {
        NIC_LOG_ERR("Failed to delete Vrf for id: {}. err: {}:{}",
                      id_, status.error_code(), status.error_message());
        ret = SDK_RET_ERR;
        goto end;
    }

end:
    return ret;
}

uint64_t
devapi_vrf::get_id(void)
{
    return id_;
}

}    // namespace iris
