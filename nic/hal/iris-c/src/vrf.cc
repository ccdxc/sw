
#include <iostream>
#include <grpc++/grpc++.h>

#include "vrf.hpp"

using namespace std;


sdk::lib::indexer *Vrf::allocator = sdk::lib::indexer::factory(Vrf::max_vrfs, false, true);

Vrf *
Vrf::Factory()
{
    Vrf *vrf = new Vrf();

    return vrf;
}

void
Vrf::Destroy(Vrf *vrf)
{
    if (vrf) {
        vrf->~Vrf();
    }
}

Vrf::Vrf()
{
    grpc::ClientContext         context;
    grpc::Status                status;

    vrf::VrfSpec                *req;
    vrf::VrfResponse            rsp;
    vrf::VrfRequestMsg          req_msg;
    vrf::VrfResponseMsg         rsp_msg;

    if (allocator->alloc(&id) != sdk::lib::indexer::SUCCESS) {
        HAL_TRACE_ERR("Failed to allocate VRF");
        return;
    }

    HAL_TRACE_DEBUG("Vrf create id: {}", id);

    req = req_msg.add_request();
    req->mutable_key_or_handle()->set_vrf_id(id);
    req->set_vrf_type(::types::VRF_TYPE_CUSTOMER);

    // status = hal->vrf_stub_->VrfCreate(&context, req_msg, &rsp_msg);
    status = hal->vrf_create(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            handle = rsp.vrf_status().vrf_handle();
            HAL_TRACE_DEBUG("Vrf Create: id: {}, handle: {}", id, handle);
            cout << "[INFO] VRF create succeeded,"
                 << " id = " << id << " handle = " << handle
                 << endl;
        } else if (rsp.api_status() == types::API_STATUS_EXISTS_ALREADY) {
            allocator->free(id);
            id = 0;  // TODO: HAL should return this
            handle = rsp.vrf_status().vrf_handle();
            cout << "[INFO] VRF already exists,"
                 << " id = " << id << " handle = " << handle
                 << endl;
        } else {
            cout << "[ERROR] " << __FUNCTION__ << ": Status = " << rsp.api_status() << endl;
            throw ("Failed to create VRF");
        }
    } else {
        cout << "[ERROR] " << __FUNCTION__ << ": Status = " << status.error_code() << ":" << status.error_message() << endl;
        throw ("Failed to create VRF");
    }
}

Vrf::~Vrf()
{
    grpc::ClientContext             context;
    grpc::Status                    status;

    vrf::VrfDeleteRequest           *req;
    vrf::VrfDeleteResponse          rsp;
    vrf::VrfDeleteRequestMsg        req_msg;
    vrf::VrfDeleteResponseMsg       rsp_msg;

    HAL_TRACE_DEBUG("Vrf delete id: {}", id);

    req = req_msg.add_request();
    if (id == 0) {
        req->mutable_key_or_handle()->set_vrf_id(id);
    } else {
        req->mutable_key_or_handle()->set_vrf_handle(handle);
    }

    // status = hal->vrf_stub_->VrfDelete(&context, req_msg, &rsp_msg);
    status = hal->vrf_delete(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            cerr << "[ERROR] " << __FUNCTION__
                 << " id = " << id << " handle = " << handle
                 << ", Status = " << rsp.api_status()
                 << endl;
        } else {
            cout << "[INFO] Vrf delete succeeded,"
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
Vrf::GetId()
{
    return id;
}

uint64_t
Vrf::GetHandle()
{
    return handle;
}
