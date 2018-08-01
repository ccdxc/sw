
#include <cmath>
#include <iostream>
#include <iomanip>
#include <grpc++/grpc++.h>

#include "interface.grpc.pb.h"

#include "lif.hpp"

using namespace std;


sdk::lib::indexer *Lif::allocator = sdk::lib::indexer::factory(Lif::max_lifs, false, true);


void
Lif::Probe()
{
    grpc::ClientContext context;
    grpc::Status status;

    intf::LifGetResponse rsp;
    intf::LifGetRequest *req __attribute__((unused));
    intf::LifGetRequestMsg req_msg;
    intf::LifGetResponseMsg rsp_msg;

    uint64_t id;

    req = req_msg.add_request();
    status = hal->intf_stub_->LifGet(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        for (int i = 0; i < rsp_msg.response().size(); i++) {
            rsp = rsp_msg.response(i);
            if (rsp.api_status() == types::API_STATUS_OK) {
                id = rsp.spec().key_or_handle().lif_id();
                cout << "[INFO] Discovered Lif id = " << id << endl;
                allocator->alloc_withid(id);
            } else {
                cout << "[ERROR] " << __FUNCTION__ << ": Status = " << rsp.api_status() << endl;
                throw ("Failed to discover LIFs");
            } 
        }
        return;
    } else {
        cout << "[ERROR] " << __FUNCTION__ << ": Status = " << status.error_code() << ":" << status.error_message() << endl;
        throw ("Failed to discover LIFs");
    }
}

Lif::Lif(struct lif_spec &lif_spec)
{
    grpc::ClientContext        context;
    grpc::Status               status;

    intf::LifSpec              *req;
    intf::LifResponse          rsp;
    intf::LifRequestMsg        req_msg;
    intf::LifResponseMsg       rsp_msg;
    intf::LifQStateMapEntry    *lif_qstate_map_ent;
    intf::QStateSetReq         *qstate_req;

    if (allocator->alloc(&id) != sdk::lib::indexer::SUCCESS) {
        throw ("Failed to allocate LIF");
    }

    req = req_msg.add_request();
    req->mutable_key_or_handle()->set_lif_id(id);
    req->set_admin_status(::intf::IF_STATUS_UP);
    // req->set_enable_rdma(lif_spec.enable_rdma);
    // req->set_rdma_max_pt_entries(lif_spec.max_pt_entries);
    // req->set_rdma_max_keys(lif_spec.max_keys);

    for (uint32_t i = 0; i < NUM_QUEUE_TYPES; i++) {
        auto & qinfo = lif_spec.queue_spec[i];
        if (qinfo.size < 1) continue;

        lif_qstate_map_ent = req->add_lif_qstate_map();
        lif_qstate_map_ent->set_type_num(qinfo.type_num);
        lif_qstate_map_ent->set_size(qinfo.size);
        lif_qstate_map_ent->set_entries(qinfo.entries);
        lif_qstate_map_ent->set_purpose((::intf::LifQPurpose)qinfo.purpose);
        for (uint32_t qid = 0; qid < (uint32_t)pow(2, qinfo.entries); qid++) {
            qstate_req = req->add_lif_qstate();
            qstate_req->set_lif_handle(0);
            qstate_req->set_type_num(qinfo.type_num);
            qstate_req->set_qid(qid);
            qstate_req->set_queue_state(qinfo.qstate, (uint32_t)pow(2, qinfo.size + 5));
            qstate_req->mutable_label()->set_handle("p4plus");
            qstate_req->mutable_label()->set_prog_name(qinfo.prog);
            qstate_req->mutable_label()->set_label(qinfo.label);
        }
    }

    status = hal->intf_stub_->LifCreate(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            cout << "[INFO] Lif create succeeded, id = "
                 << id
                 << ", hw_lif_id = "
                 << rsp.status().hw_lif_id()
                 << endl;

            hw_lif_id = rsp.status().hw_lif_id();
            handle = rsp.status().lif_handle();
            spec = intf::LifSpec(*req);

            hw_lif_id = rsp.status().hw_lif_id();
            for (int i = 0; i < rsp.qstate().size(); i++) {
                auto & qstate = rsp.qstate()[i];
                cout << "[INFO] lif " << rsp.status().hw_lif_id()
                     << " qtype " << qstate.type_num()
                     << " qstate 0x" << hex << qstate.addr() << resetiosflags(ios::hex)
                     << endl;
                qstate_addr[qstate.type_num()] = qstate.addr();
            }
        } else {
            cerr << "[ERROR] " << __FUNCTION__
                 << ": Id = " << id
                 << ", Status = " << rsp.api_status()
                 << endl;
            throw ("Failed to create LIF");
        } 
    } else {
        cerr << "[ERROR] " << __FUNCTION__
             << ": Id = " << id
             << ", Status = " << status.error_code() << ":" << status.error_message()
             << endl;
        throw ("Failed to create LIF");
    }
}

Lif::Lif(uint64_t lif_id)
{
    grpc::ClientContext context;
    grpc::Status status;

    intf::LifGetResponse rsp;
    intf::LifGetRequest *req __attribute__((unused));
    intf::LifGetRequestMsg req_msg;
    intf::LifGetResponseMsg rsp_msg;

    if (allocator->alloc_withid(lif_id) != sdk::lib::indexer::SUCCESS) {
        throw ("Failed to allocate LIF");
    }
    id = lif_id;

    req = req_msg.add_request();
    req->mutable_key_or_handle()->set_lif_id(lif_id);
    status = hal->intf_stub_->LifGet(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        for (int i = 0; i < rsp_msg.response().size(); i++) {
            rsp = rsp_msg.response(i);
            if (rsp.api_status() != types::API_STATUS_OK) {
                cerr << "[ERROR] " << __FUNCTION__
                     << ": Id = " << lif_id
                     << ", Status = " << rsp.api_status()
                     << endl;
                throw("Failed to create LIF");
            } else {
                cout << "[INFO] Get Lif " << rsp.spec().key_or_handle().lif_id() << endl;

                hw_lif_id = rsp.status().hw_lif_id();
                for (int i = 0; i < rsp.qstate().size(); i++) {
                    auto & qstate = rsp.qstate()[i];
                    cout << "[INFO] lif " << rsp.status().hw_lif_id()
                         << " qtype " << qstate.type_num()
                         << " qstate 0x" << hex << qstate.addr() << resetiosflags(ios::hex)
                         << endl;
                    qstate_addr[qstate.type_num()] = qstate.addr();
                }                
            }
        }
    } else {
        cerr << "[ERROR] " << __FUNCTION__
             << ": Id = " << lif_id
             << ", Status = " << status.error_code() << ":" << status.error_message()
             << endl;
        throw("Failed to create LIF");
    }
}

Lif::~Lif()
{
    grpc::ClientContext           context;
    grpc::Status                  status;

    intf::LifDeleteRequest        *req;
    intf::LifDeleteResponse       rsp;
    intf::LifDeleteRequestMsg     req_msg;
    intf::LifDeleteResponseMsg    rsp_msg;

    

    req = req_msg.add_request();
    req->mutable_key_or_handle()->set_lif_id(id);
    status = hal->intf_stub_->LifDelete(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            cerr << "[ERROR] " << __FUNCTION__
                 << ": Id = " << id
                 << ", Status = " << rsp.api_status()
                 << endl;
        } else {
            cout << "[INFO] Lif delete succeeded, id = " << id << endl;
        }
    } else {
        cerr << "[ERROR] " << __FUNCTION__
             << ": Id = " << id
             << ", Status = " << status.error_code() << ":" << status.error_message()
             << endl;
    }
}

uint32_t
Lif::GetId()
{
    return id;
}
