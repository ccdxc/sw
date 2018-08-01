
#include <iostream>
#include <grpc++/grpc++.h>

#include "types.grpc.pb.h"

#include "uplink.hpp"

using namespace std;


sdk::lib::indexer *Uplink::allocator = sdk::lib::indexer::factory(Uplink::max_uplinks, false, true);


void
Uplink::Probe()
{
    grpc::ClientContext context;
    grpc::Status status;
    intf::InterfaceGetRequest         *req __attribute__((unused));
    intf::InterfaceGetResponse        rsp;
    intf::InterfaceGetRequestMsg      req_msg;
    intf::InterfaceGetResponseMsg     rsp_msg;

    req = req_msg.add_request();
    status = hal->intf_stub_->InterfaceGet(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        for (int i = 0; i < rsp_msg.response().size(); i++) {
            rsp = rsp_msg.response(i);
            if (rsp.api_status() != types::API_STATUS_OK) {
                cout << "[ERROR] " << __FUNCTION__ << ": Status = " << rsp.api_status() << endl;
                throw ("Failed to discover Uplinks");
            } else {
                if (rsp.spec().type() == intf::IF_TYPE_UPLINK) {
                    cout << "[INFO] Discovered Uplink Interface"
                         << " handle = " << rsp.status().if_handle()
                         << " id = " << rsp.spec().key_or_handle().interface_id()
                         << " port = " << rsp.spec().if_uplink_info().port_num() << endl;
                    //uplink_map[rsp.spec().if_uplink_info().port_num()] = rsp.status().if_handle();
                }
            }
        }
    } else {
        cout << "[ERROR] " << __FUNCTION__ << ": Status = " << status.error_code() << ":" << status.error_message() << endl;
        throw ("Failed to discover Uplinks");
    }
}

Uplink::Uplink(uint32_t port_num)
{
    intf::InterfaceSpec           *spec;
    intf::InterfaceResponse       rsp;
    intf::InterfaceRequestMsg     req_msg;
    intf::InterfaceResponseMsg    rsp_msg;
    grpc::ClientContext           context;
    grpc::Status                  status;

    if (allocator->alloc(&id) != sdk::lib::indexer::SUCCESS) {
        throw ("Failed to allocate Uplink interface");
    }

    spec = req_msg.add_request();
    spec->mutable_key_or_handle()->set_interface_id(id);
    spec->set_type(::intf::IfType::IF_TYPE_UPLINK);
    spec->set_admin_status(::intf::IfStatus::IF_STATUS_UP);
    spec->mutable_if_uplink_info()->set_port_num(port_num);
    //spec->mutable_if_uplink_info()->set_native_l2segment_id(native_l2seg_id);

    status = hal->intf_stub_->InterfaceCreate(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            handle = rsp.status().if_handle();
            cout << "[INFO] Uplink create succeeded,"
                 << " id = " << id << " handle = " << handle
                 << endl;
        } else if (rsp.api_status() == types::API_STATUS_EXISTS_ALREADY) {
            allocator->free(id);
            id = 0;    // TODO: HAL should return id
            handle = rsp.status().if_handle();
            cout << "[INFO] Uplink already exists,"
                 << " id = " << id << " handle = " << handle
                 << endl;
        } else {
            cout << "[ERROR] " << __FUNCTION__ << ": Status = " << rsp.api_status() << endl;
            throw ("Failed to create Uplink interface");
        } 
    } else {
        cout << "[ERROR] " << __FUNCTION__ << ": Status = " << status.error_code() << ":" << status.error_message() << endl;
        throw ("Failed to create Uplink interface");
    }
}

Uplink::~Uplink()
{
    intf::InterfaceDeleteRequest          *req;
    intf::InterfaceDeleteResponse         rsp;
    intf::InterfaceDeleteRequestMsg       req_msg;
    intf::InterfaceDeleteResponseMsg      rsp_msg;
    grpc::ClientContext                   context;
    grpc::Status                          status;

    req = req_msg.add_request();
    if (id == 0) {
        req->mutable_key_or_handle()->set_interface_id(id);
    } else {
        req->mutable_key_or_handle()->set_if_handle(handle);        
    }

    status = hal->intf_stub_->InterfaceDelete(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            cerr << "[ERROR] " << __FUNCTION__
                 << " id = " << id << " handle = " << handle
                 << ", Status = " << rsp.api_status()
                 << endl;
        } else {
            cout << "[INFO] Uplink delete succeeded, id = " << id << endl;
        }
    } else {
        cerr << "[ERROR] " << __FUNCTION__
             << " id = " << id << " handle = " << handle
             << ", Status = " << status.error_code() << ":" << status.error_message()
             << endl;
    }
}

void
Uplink::AddL2Segment(shared_ptr<L2Segment> l2seg)
{
    grpc::ClientContext                     context;
    grpc::Status                            status;

    intf::InterfaceL2SegmentSpec            *req;
    intf::InterfaceL2SegmentResponse        rsp;
    intf::InterfaceL2SegmentRequestMsg      req_msg;
    intf::InterfaceL2SegmentResponseMsg     rsp_msg;

    if (l2seg_refs.find(l2seg->GetHandle()) != l2seg_refs.cend()) {
        // WARN
        return;
    }

    req = req_msg.add_request();
    req->mutable_l2segment_key_or_handle()->set_l2segment_handle(l2seg->GetHandle());
    req->mutable_if_key_handle()->set_interface_id(id);

    status = hal->intf_stub_->AddL2SegmentOnUplink(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK){
            cout << "[INFO] L2 Segment handle = " << l2seg->GetHandle()
                  << " add to uplink id = "  << id
                  << " succeeded"
                  << endl;
        } else if (rsp.api_status() == types::API_STATUS_EXISTS_ALREADY) {
            // TODO: HAL should return already_exists on duplicate insert, it returns error.
            cout << "[INFO] L2 Segment handle = " << l2seg->GetHandle()
                  << " already added to uplink id = "  << id
                  << endl;
        } else {
            cerr << "[ERROR] " << __FUNCTION__ << ": Status = " << rsp.api_status() << endl;
            throw("Failed to add L2Segment on Uplink!");
        }
    } else {
        cerr << "[ERROR] " << __FUNCTION__ << ": Status = " << status.error_code() << ":" << status.error_message() << endl;
        throw("Failed to add L2Segment on Uplink!");
    }
}

void
Uplink::DelL2Segment(shared_ptr<L2Segment> l2seg)
{
    grpc::ClientContext                     context;
    grpc::Status                            status;

    intf::InterfaceL2SegmentSpec            *req;
    intf::InterfaceL2SegmentResponse        rsp;
    intf::InterfaceL2SegmentRequestMsg      req_msg;
    intf::InterfaceL2SegmentResponseMsg     rsp_msg;

    req = req_msg.add_request();
    req->mutable_l2segment_key_or_handle()->set_l2segment_handle(l2seg->GetHandle());
    req->mutable_if_key_handle()->set_interface_id(id);

    status = hal->intf_stub_->DelL2SegmentOnUplink(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            cerr << "[ERROR] " << __FUNCTION__ << ": Status = " << rsp.api_status() << endl;
            throw("Failed to add L2Segment on Uplink!");
        } else {
            cout << "[INFO] L2 Segment handle = " << l2seg->GetHandle()
                  << " delete from uplink id = "  << id
                  << " succeeded"
                  << endl;
        }
    } else {
        cerr << "[ERROR] " << __FUNCTION__ << ": Status = " << status.error_code() << ":" << status.error_message() << endl;
        throw("Failed to add L2Segment on Uplink!");
    }
}

uint32_t
Uplink::GetId()
{
    return id;
}

uint64_t
Uplink::GetHandle()
{
    return handle;
}