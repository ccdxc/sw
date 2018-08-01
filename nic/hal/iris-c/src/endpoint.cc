
#include <iostream>
#include <iomanip>

#include "kh.grpc.pb.h"
#include "types.grpc.pb.h"

#include "endpoint.hpp"

using namespace std;


void
Endpoint::Probe()
{
    grpc::ClientContext                 context;
    grpc::Status                        status;

    endpoint::EndpointGetResponse       rsp;
    endpoint::EndpointGetRequestMsg     req_msg;
    endpoint::EndpointGetResponseMsg    rsp_msg;

    uint64_t enic_id;
    uint64_t handle;

    req_msg.add_request();
    status = hal->ep_stub_->EndpointGet(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        for (int i = 0; i < rsp_msg.response().size(); i++) {
            rsp = rsp_msg.response(i);
            if (rsp.api_status() != types::API_STATUS_OK) {
                cerr << "[ERROR] " << __FUNCTION__ << ": Status = " << rsp.api_status() << endl;
                throw ("Failed to discover Endpoints");
            } else {
                enic_id = rsp.spec().endpoint_attrs().interface_key_handle().interface_id();
                handle = rsp.status().endpoint_handle();
                cout << "[INFO] Discovered Endpoint handle = " << handle
                     << " enic id " << enic_id << endl;
                return;
            }
        }
    } else {
        cerr << "[ERROR] " << __FUNCTION__ << ": Status = " << status.error_code() << ":" << status.error_message() << endl;
        throw ("Failed to discover Endpoints");
    }
}

Endpoint::Endpoint(shared_ptr<Enic> enic,
                   shared_ptr<Vrf> vrf,
                   mac_t mac, vlan_t vlan)
{
    grpc::ClientContext             context;
    grpc::Status                    status;

    endpoint::EndpointSpec          *req;
    endpoint::EndpointResponse      rsp;
    endpoint::EndpointRequestMsg    req_msg;
    endpoint::EndpointResponseMsg   rsp_msg;
    // kh::SecurityGroupKeyHandle      *sg_kh;
    // types::IPAddress                *ip;

    shared_ptr<L2Segment> l2seg = L2Segment::GetInstance(vrf, vlan);

    _mac = mac;
    vrf_ref = vrf;
    l2seg_ref = l2seg;

    req = req_msg.add_request();
    req->mutable_vrf_key_handle()->set_vrf_id(vrf->GetId());
    req->mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(mac);
    req->mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_l2segment_handle(l2seg->GetHandle());
    req->mutable_endpoint_attrs()->mutable_interface_key_handle()->set_interface_id(enic->GetId());
    // if (fwd_mode == FWD_MODE_SMART_NIC && ip_addr) {
    //     ip = req->mutable_endpoint_attrs()->add_ip_address();
    //     ip->set_ip_af(types::IPAddressFamily::IP_AF_INET);
    //     ip->set_v4_addr(ip_addr);
    // }
    // if (fwd_mode == FWD_MODE_SMART_NIC && sg_id) {
    //     sg_kh = req->mutable_endpoint_attrs()->add_sg_key_handle();
    //     sg_kh->set_security_group_id(sg_id);
    // }
    status = hal->ep_stub_->EndpointCreate(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            cerr << "[ERROR] " << __FUNCTION__ << ": Status = " << rsp.api_status() << endl;
            throw ("Failed to create Endpoint");
        } else {
            handle = rsp.endpoint_status().endpoint_handle();
            cout << "[INFO] Endpoint create succeeded,"
                 << " handle = " << handle
                 << endl;
        }
    } else {
        cerr << "[ERROR] " << __FUNCTION__ << ": Status = " << status.error_code() << ":" << status.error_message() << endl;
        throw ("Failed to create Endpoint");
    }
}

Endpoint::~Endpoint()
{
    grpc::ClientContext                   context;
    grpc::Status                          status;

    endpoint::EndpointDeleteRequest       *req;
    endpoint::EndpointDeleteResponse      rsp;
    endpoint::EndpointDeleteRequestMsg    req_msg;
    endpoint::EndpointDeleteResponseMsg   rsp_msg;

    req = req_msg.add_request();
    req->mutable_vrf_key_handle()->set_vrf_id(vrf_ref->GetId());
    req->mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(_mac);
    req->mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_l2segment_handle(l2seg_ref->GetHandle());

    status = hal->ep_stub_->EndpointDelete(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            cerr << "[ERROR] " << __FUNCTION__
                 << ": Handle = " << handle
                 << ", Status = " << rsp.api_status()
                 << endl;
        } else {
            cout << "[INFO] Endpoint delete succeeded,"
                 << " handle = " << handle
                 << endl;
            return;
        }
    } else {
        cerr << "[ERROR] " << __FUNCTION__
             << ": Handle = " << handle
             << ", Status = " << status.error_code() << ":" << status.error_message()
             << endl;
    }

    l2seg_ref.reset();
    vrf_ref.reset();
}
