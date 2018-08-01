
#include <iostream>
#include <grpc++/grpc++.h>

#include "vrf.hpp"

using namespace std;


sdk::lib::indexer *Vrf::allocator = sdk::lib::indexer::factory(Vrf::max_vrfs, false, true);


void
Vrf::Probe()
{
    grpc::ClientContext      context;
    grpc::Status             status;

    vrf::VrfGetRequest       *req __attribute__((unused));
    vrf::VrfGetResponse      rsp;
    vrf::VrfGetRequestMsg    req_msg;
    vrf::VrfGetResponseMsg   rsp_msg;

    uint64_t id, handle;

    req = req_msg.add_request();
    status = hal->vrf_stub_->VrfGet(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        for (int i = 0; i < rsp_msg.response().size(); i++) {
            rsp = rsp_msg.response(i);
            if (rsp.api_status() != types::API_STATUS_OK) {
                cout << "[ERROR] " << __FUNCTION__ << ": Status = " << rsp.api_status() << endl;
                throw ("Failed to discover Vrfs");
            } else {
                id = rsp.spec().key_or_handle().vrf_id();
                handle =  rsp.status().vrf_handle();
                cout << "[INFO] Discovered Vrf"
                     << " id = " << id << " handle = " << handle
                     << endl;
                allocator->alloc_withid(id);
            }
        }
    } else {
        cout << "[ERROR] " << __FUNCTION__ << ": Status = " << status.error_code() << ":" << status.error_message() << endl;
        throw ("Failed to discover Vrfs");
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
        throw ("Failed to allocate VRF");
    }

    req = req_msg.add_request();
    req->mutable_key_or_handle()->set_vrf_id(id);
    req->set_vrf_type(::types::VRF_TYPE_CUSTOMER);

    status = hal->vrf_stub_->VrfCreate(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            handle = rsp.vrf_status().vrf_handle();
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

    req = req_msg.add_request();
    if (id == 0) {
        req->mutable_key_or_handle()->set_vrf_id(id);
    } else {
        req->mutable_key_or_handle()->set_vrf_handle(handle);
    }

    status = hal->vrf_stub_->VrfDelete(&context, req_msg, &rsp_msg);
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
