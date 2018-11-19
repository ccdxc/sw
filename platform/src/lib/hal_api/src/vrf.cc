
#include <iostream>
#include <grpc++/grpc++.h>

#include "vrf.hpp"

using namespace std;


sdk::lib::indexer *HalVrf::allocator = sdk::lib::indexer::factory(HalVrf::max_vrfs, false, true);

HalVrf *
HalVrf::Factory(types::VrfType type, Uplink *uplink)
{
    HalVrf *vrf = new HalVrf(type, uplink);

    return vrf;
}

void
HalVrf::Destroy(HalVrf *vrf)
{
    if (vrf) {
        vrf->~HalVrf();
    }
}

HalVrf::HalVrf(types::VrfType type, Uplink *uplink)
{
    grpc::ClientContext         context;
    grpc::Status                status;

    vrf::VrfSpec                *req;
    vrf::VrfResponse            rsp;
    vrf::VrfRequestMsg          req_msg;
    vrf::VrfResponseMsg         rsp_msg;

    if (allocator->alloc(&id) != sdk::lib::indexer::SUCCESS) {
        NIC_LOG_ERR("Failed to allocate VRF");
        return;
    }

    this->type = type;
    this->uplink = uplink;

    id += VRF_ID_BASE;

    NIC_LOG_INFO("HalVrf create id: {}", id);

    req = req_msg.add_request();
    req->mutable_key_or_handle()->set_vrf_id(id);
    req->set_vrf_type(type);

    // status = hal->vrf_stub_->HalVrfCreate(&context, req_msg, &rsp_msg);
    status = hal->vrf_create(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            handle = rsp.vrf_status().vrf_handle();
            NIC_LOG_INFO("HalVrf Create: id: {}, handle: {}", id, handle);
            cout << "[INFO] VRF create succeeded,"
                 << " id = " << id << " handle = " << handle
                 << endl;
        } else if (rsp.api_status() == types::API_STATUS_EXISTS_ALREADY) {
            NIC_LOG_ERR("VRF already exists with id: {}", id);
            allocator->free(id);
            id = 0;  // TODO: HAL should return this
            handle = rsp.vrf_status().vrf_handle();
        } else {
            cout << "[ERROR] " << __FUNCTION__ << ": Status = " << rsp.api_status() << endl;
            throw ("Failed to create VRF");
        }
    } else {
        cout << "[ERROR] " << __FUNCTION__ << ": Status = " << status.error_code() << ":" << status.error_message() << endl;
        throw ("Failed to create VRF");
    }
}

HalVrf::~HalVrf()
{
    grpc::ClientContext             context;
    grpc::Status                    status;

    vrf::VrfDeleteRequest           *req;
    vrf::VrfDeleteResponse          rsp;
    vrf::VrfDeleteRequestMsg        req_msg;
    vrf::VrfDeleteResponseMsg       rsp_msg;

    NIC_LOG_INFO("HalVrf delete id: {}", id);

    req = req_msg.add_request();
    if (id == 0) {
        req->mutable_key_or_handle()->set_vrf_id(id);
    } else {
        req->mutable_key_or_handle()->set_vrf_handle(handle);
    }

    // status = hal->vrf_stub_->HalVrfDelete(&context, req_msg, &rsp_msg);
    status = hal->vrf_delete(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            cerr << "[ERROR] " << __FUNCTION__
                 << " id = " << id << " handle = " << handle
                 << ", Status = " << rsp.api_status()
                 << endl;
        } else {
            allocator->free(id);
            cout << "[INFO] HalVrf delete succeeded,"
                 << " id = " << id << " handle = " << handle
                 << endl;
        }
    } else {
        cout << "[ERROR] " << __FUNCTION__
            << " id = " << id << " handle = " << handle
            << ": Status = " << status.error_code() << ":" << status.error_message()
            << endl;
    }
}

uint64_t
HalVrf::GetId()
{
    return id;
}

uint64_t
HalVrf::GetHandle()
{
    return handle;
}
