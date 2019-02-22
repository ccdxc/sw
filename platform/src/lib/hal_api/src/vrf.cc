
#include <iostream>
#include <grpc++/grpc++.h>

#include "nic/include/globals.hpp"
#include "vrf.hpp"
#include "uplink.hpp"
#include "utils.hpp"

using namespace std;


sdk::lib::indexer *HalVrf::allocator = sdk::lib::indexer::factory(HalVrf::max_vrfs, false, true);

HalVrf *
HalVrf::Factory(types::VrfType type, Uplink *uplink)
{
    hal_irisc_ret_t ret = HAL_IRISC_RET_SUCCESS;
    HalVrf          *vrf = new HalVrf(type, uplink);

    ret = vrf->HalVrfCreate();
    if (ret != HAL_IRISC_RET_SUCCESS) {
        NIC_LOG_DEBUG("VrfCreate failed. ret: {}", ret);
        goto end;
    }

end:
    if (ret != HAL_IRISC_RET_SUCCESS) {
        if (vrf) {
            delete vrf;
            vrf = NULL;
        }
    }
    return vrf;
}

hal_irisc_ret_t
HalVrf::Destroy(HalVrf *vrf)
{
    hal_irisc_ret_t ret = HAL_IRISC_RET_SUCCESS;
    ret = vrf->HalVrfDelete();
    if (ret != HAL_IRISC_RET_SUCCESS) {
        NIC_LOG_CRIT("FATAL:Vrf Delete failed. ret: {}", ret);
        goto end;
    }
    delete vrf;

end:
    return ret;
}

HalVrf::HalVrf(types::VrfType type, Uplink *uplink)
{
    this->id = 0;
    this->type = type;
    this->uplink = uplink;
}

hal_irisc_ret_t
HalVrf::HalVrfCreate()
{
    hal_irisc_ret_t             ret = HAL_IRISC_RET_SUCCESS;
    grpc::ClientContext         context;
    grpc::Status                status;

    vrf::VrfSpec                *req;
    vrf::VrfResponse            rsp;
    vrf::VrfRequestMsg          req_msg;
    vrf::VrfResponseMsg         rsp_msg;

    if (allocator->alloc(&id) != sdk::lib::indexer::SUCCESS) {
        NIC_LOG_ERR("Failed to allocate VRF. Resource exhaustion");
        return HAL_IRISC_RET_FAIL;
    }

    id += NICMGR_VRF_ID_MIN;

    NIC_LOG_DEBUG("HalVrf create id: {}", id);

    req = req_msg.add_request();
    req->mutable_key_or_handle()->set_vrf_id(id);
    req->set_vrf_type(type);

    VERIFY_HAL();
    status = hal->vrf_create(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            NIC_LOG_DEBUG("HalVrf Create: id: {}", id);
        } else if (rsp.api_status() == types::API_STATUS_EXISTS_ALREADY) {
            NIC_LOG_ERR("VRF already exists with id: {}", id);
            ret = HAL_IRISC_RET_FAIL;
            goto end;
        } else {
            NIC_LOG_ERR("Failed to create Vrf for id:{}. err: {}",
                        id, rsp.api_status());
            ret = HAL_IRISC_RET_FAIL;
            goto end;
        }
    } else {
        NIC_LOG_ERR("Failed to create Vrf for id:{}. err: {}:{}",
                    id, status.error_code(), status.error_message());
        ret = HAL_IRISC_RET_FAIL;
        goto end;
    }

end:
    if (ret != HAL_IRISC_RET_SUCCESS) {
        allocator->free(id);
    }
    return ret;
}

hal_irisc_ret_t
HalVrf::HalVrfDelete()
{
    hal_irisc_ret_t                 ret = HAL_IRISC_RET_SUCCESS;
    grpc::ClientContext             context;
    grpc::Status                    status;

    vrf::VrfDeleteRequest           *req;
    vrf::VrfDeleteResponse          rsp;
    vrf::VrfDeleteRequestMsg        req_msg;
    vrf::VrfDeleteResponseMsg       rsp_msg;

    NIC_LOG_DEBUG("HalVrf delete id: {}", id);

    req = req_msg.add_request();
    req->mutable_key_or_handle()->set_vrf_id(id);

    VERIFY_HAL();
    status = hal->vrf_delete(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            NIC_LOG_ERR("Failed to delete Vrf for id: {}. err: {}",
                          id, rsp.api_status());
            ret = HAL_IRISC_RET_FAIL;
            goto end;
        } else {
            NIC_LOG_DEBUG("Deleted Vrf id: {}", id);
        }
    } else {
        NIC_LOG_ERR("Failed to delete Vrf for id: {}. err: {}:{}",
                      id, status.error_code(), status.error_message());
        ret = HAL_IRISC_RET_FAIL;
        goto end;
    }

    allocator->free(id);
end:
    return ret;
}

uint64_t
HalVrf::GetId()
{
    return id;
}
