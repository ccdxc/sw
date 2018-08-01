
#include <iostream>
#include <grpc++/grpc++.h>

#include "l2seg.hpp"

using namespace std;


std::map<l2seg_key_t, std::shared_ptr<L2Segment>> L2Segment::registry;
sdk::lib::indexer *L2Segment::allocator = sdk::lib::indexer::factory(L2Segment::max_l2segs, false, true);


void
L2Segment::Probe()
{
    grpc::ClientContext                     context;
    grpc::Status                            status;

    l2segment::L2SegmentGetRequest         *req __attribute__((unused));
    l2segment::L2SegmentGetResponse        rsp;
    l2segment::L2SegmentGetRequestMsg      req_msg;
    l2segment::L2SegmentGetResponseMsg     rsp_msg;

    uint16_t                               vlan_id;
    uint64_t                               l2seg_handle, l2seg_id;

    req = req_msg.add_request();
    status = hal->l2seg_stub_->L2SegmentGet(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        for (int i = 0; i < rsp_msg.response().size(); i++) {
            rsp = rsp_msg.response(i);
            if (rsp.api_status() != types::API_STATUS_OK) {
                cerr << "[ERROR] " << __FUNCTION__ << ": Status = " << rsp.api_status() << endl;
            } else {
                if (rsp.spec().wire_encap().encap_type() == types::ENCAP_TYPE_DOT1Q) {
                    l2seg_handle = rsp.status().l2segment_handle();
                    l2seg_id = rsp.spec().key_or_handle().segment_id();
                    vlan_id = rsp.spec().wire_encap().encap_value();
                    cout << "[INFO] Discovered L2Segment"
                         << " handle " << l2seg_handle
                         << " id " << l2seg_id
                         << " VLAN " << vlan_id << endl;
                }
            }
        }
    } else {
        cerr << "[ERROR] " << __FUNCTION__ << ": Status = " << status.error_code() << ":" << status.error_message() << endl;
        throw ("Failed to delete L2Segments!");
    }
}

L2Segment::L2Segment(shared_ptr<Vrf> vrf, uint16_t vlan)
{
    grpc::ClientContext                 context;
    grpc::Status                        status;

    l2segment::L2SegmentSpec            *req;
    l2segment::L2SegmentResponse        rsp;
    l2segment::L2SegmentRequestMsg      req_msg;
    l2segment::L2SegmentResponseMsg     rsp_msg;

    if (allocator->alloc(&id) != sdk::lib::indexer::SUCCESS) {
        throw ("Failed to allocate L2Segment");
    }

    req = req_msg.add_request();
    req->mutable_meta()->set_vrf_id(vrf->GetId());
    req->mutable_key_or_handle()->set_segment_id(id);
    //req->mutable_vrf_key_handle()->set_vrf_handle(vrf->GetHandle());
    req->mutable_vrf_key_handle()->set_vrf_id(vrf->GetId());
    req->set_mcast_fwd_policy(l2segment::MulticastFwdPolicy::MULTICAST_FWD_POLICY_FLOOD);
    req->set_bcast_fwd_policy(l2segment::BroadcastFwdPolicy::BROADCAST_FWD_POLICY_FLOOD);
    req->mutable_wire_encap()->set_encap_type(::types::ENCAP_TYPE_DOT1Q);
    req->mutable_wire_encap()->set_encap_value(vlan);

    status = hal->l2seg_stub_->L2SegmentCreate(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            handle = rsp.l2segment_status().l2segment_handle();
            cout << "[INFO] L2 segment create succeeded,"
                 << " id = " << id << " handle = " << handle
                 << endl;
            _vlan = vlan;
            vrf_ref = vrf;
        } else {
            cerr << "[ERROR] " << __FUNCTION__ << ": Status = " << rsp.api_status() << endl;
            throw ("Failed to create L2Segment");
        }
    } else {
        cerr << "[ERROR] " << __FUNCTION__ << ": Status = " << status.error_code() << ":" << status.error_message() << endl;
        throw ("Failed to create L2Segment");
    }
}

shared_ptr<L2Segment>
L2Segment::GetInstance(shared_ptr<Vrf> vrf, uint16_t vlan)
{
    l2seg_key_t key(vrf->GetId(), vlan);

    if (registry.find(key) == registry.cend()) {
        registry[key] = make_shared<L2Segment>(vrf, vlan);
    }

    return registry[key];
}

L2Segment::~L2Segment()
{
    grpc::ClientContext                     context;
    grpc::Status                            status;

    l2segment::L2SegmentDeleteRequest       *req;
    l2segment::L2SegmentDeleteResponse      rsp;
    l2segment::L2SegmentDeleteRequestMsg    req_msg;
    l2segment::L2SegmentDeleteResponseMsg   rsp_msg;

    req = req_msg.add_request();
    req->mutable_key_or_handle()->set_segment_id(id);
    req->mutable_vrf_key_handle()->set_vrf_id(vrf_ref->GetId());

    status = hal->l2seg_stub_->L2SegmentDelete(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            cout << "[INFO] L2 segment delete succeeded,"
                 << " id = " << id << " handle = " << handle
                 << endl;
            vrf_ref.reset();
        } else {
            cerr << "[ERROR] " << __FUNCTION__ << ": Status = " << rsp.api_status() << endl;
        }
    } else {
        cerr << "[ERROR] " << __FUNCTION__ << ": Status = " << status.error_code() << ":" << status.error_message() << endl;
    }
}

uint64_t
L2Segment::GetId()
{
    return id;
}


uint64_t
L2Segment::GetHandle()
{
    return handle;
}
