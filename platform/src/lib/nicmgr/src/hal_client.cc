/*
* Copyright (c) 2018, Pensando Systems Inc.
*/

#include <cstdio>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <thread>
#include <bitset>
#include <cmath>
#include <map>
#include <thread>
#include <chrono>
#include <grpc++/grpc++.h>

#include "types.grpc.pb.h"
#include "vrf.grpc.pb.h"
#include "interface.grpc.pb.h"
#include "endpoint.grpc.pb.h"
#include "l2segment.grpc.pb.h"
#include "multicast.grpc.pb.h"
#include "rdma.grpc.pb.h"

#include "hal_client.hpp"

using namespace kh;
using namespace types;
using namespace intf;
using namespace endpoint;
using namespace l2segment;
using namespace multicast;

using namespace grpc;
using namespace std;

HalClient::HalClient(enum ForwardingMode fwd_mode)
{
    if (getenv("HAL_SOCK_PATH")) {
        channel = CreateChannel(std::string("unix:") + std::getenv("HAL_SOCK_PATH") + "halsock", InsecureChannelCredentials());
    } else if (getenv("HAL_GRPC_PORT")) {
        channel = CreateChannel(std::string("localhost:") + getenv("HAL_GRPC_PORT"), InsecureChannelCredentials());
    } else {
        channel = CreateChannel(std::string("localhost:50054"), InsecureChannelCredentials());
    }

    cout << "[INFO] Waiting for HAL to be ready ..." << endl;
    auto state = channel->GetState(true);
    while (state != GRPC_CHANNEL_READY) {
        // Wait for State change or deadline
        channel->WaitForStateChange(state, gpr_time_from_seconds(1, GPR_TIMESPAN));
        state = channel->GetState(true);
        // cout << "[INFO] Connecting to HAL, channel status = " << channel->GetState(true) << endl;
    }

    vrf_stub_ = Vrf::NewStub(channel);
    intf_stub_ = Interface::NewStub(channel);
    internal_stub_ = Internal::NewStub(channel);
    ep_stub_ = Endpoint::NewStub(channel);
    l2seg_stub_ = L2Segment::NewStub(channel);
    multicast_stub_ = Multicast::NewStub(channel);
    rdma_stub_ = Rdma::NewStub(channel);
    this->fwd_mode = fwd_mode;
}

/**
 * VRF APIs
 */
int
HalClient::VrfProbe()
{
    VrfGetRequest       *req __attribute__((unused));
    VrfGetResponse      rsp;
    VrfGetRequestMsg    req_msg;
    VrfGetResponseMsg   rsp_msg;
    ClientContext       context;
    Status              status;

    req = req_msg.add_request();
    status = vrf_stub_->VrfGet(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        for (int i = 0; i < rsp_msg.response().size(); i++) {
            rsp = rsp_msg.response(i);
            if (rsp.api_status() != types::API_STATUS_OK) {
                cout << "[ERROR] " << __FUNCTION__ << ": Status = " << rsp.api_status() << endl;
            } else {
                cout << "[INFO] Discovered Vrf"
                     << " handle " << rsp.status().vrf_handle()
                     << " id " << rsp.spec().key_or_handle().vrf_id()
                     << endl;
                vrf_id2handle[rsp.spec().key_or_handle().vrf_id()] = rsp.status().vrf_handle();
            }
        }
    } else {
        cout << "[ERROR] " << __FUNCTION__ << ": Status = " << status.error_code() << ":" << status.error_message() << endl;
        return -1;
    }

    return 0;
}

uint64_t
HalClient::VrfCreate(uint64_t vrf_id)
{
    VrfSpec           *spec;
    VrfResponse       rsp;
    VrfRequestMsg     req_msg;
    VrfResponseMsg    rsp_msg;
    ClientContext     context;
    Status            status;

    spec = req_msg.add_request();
    spec->mutable_key_or_handle()->set_vrf_id(vrf_id);
    spec->set_vrf_type(::types::VRF_TYPE_CUSTOMER);

    status = vrf_stub_->VrfCreate(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            cout << "[ERROR] " << __FUNCTION__ << ": Status = " << rsp.api_status() << endl;
        } else {
            cout << "[INFO] VRF create succeeded, handle = "
                 << rsp.vrf_status().vrf_handle()
                 << endl;
            vrf_id2handle[vrf_id] = rsp.vrf_status().vrf_handle();
            return rsp.vrf_status().vrf_handle();
        }
    } else {
        cout << "[ERROR] " << __FUNCTION__ << ": Status = " << status.error_code() << ":" << status.error_message() << endl;
        return -1;
    }

    return 0;
}

/**
 * Uplink APIs
 */
uint64_t
HalClient::UplinkCreate(uint64_t uplink_if_id, uint64_t port_num, uint64_t native_l2seg_id)
{
    InterfaceSpec           *spec;
    InterfaceResponse       rsp;
    InterfaceRequestMsg     req_msg;
    InterfaceResponseMsg    rsp_msg;
    ClientContext           context;
    Status                  status;

    spec = req_msg.add_request();
    spec->mutable_key_or_handle()->set_interface_id(uplink_if_id);
    spec->set_type(::intf::IfType::IF_TYPE_UPLINK);
    spec->set_admin_status(::intf::IfStatus::IF_STATUS_UP);
    spec->mutable_if_uplink_info()->set_port_num(port_num);
    spec->mutable_if_uplink_info()->set_native_l2segment_id(native_l2seg_id);

    status = intf_stub_->InterfaceCreate(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            cout << "[ERROR] " << __FUNCTION__ << ": Status = " << rsp.api_status() << endl;
        } else {
            cout << "[INFO] Uplink create succeeded, handle = "
                 << rsp.status().if_handle()
                 << endl;
            uplink_map[port_num] = rsp.status().if_handle();
            return rsp.status().if_handle();
        }
    } else {
        cout << "[ERROR] " << __FUNCTION__ << ": Status = " << status.error_code() << ":" << status.error_message() << endl;
        return -1;
    }

    return 0;
}

/**
 * L2Segment APIs
 */
int
HalClient::L2SegmentProbe()
{
    L2SegmentGetRequest         *req __attribute__((unused));
    L2SegmentGetResponse        rsp;
    L2SegmentGetRequestMsg      req_msg;
    L2SegmentGetResponseMsg     rsp_msg;
    ClientContext               context;
    Status                      status;
    uint64_t                    l2seg_handle, l2seg_id;

    req = req_msg.add_request();
    status = l2seg_stub_->L2SegmentGet(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        for (int i = 0; i < rsp_msg.response().size(); i++) {
            rsp = rsp_msg.response(i);
            if (rsp.api_status() != types::API_STATUS_OK) {
                cout << "[ERROR] " << __FUNCTION__ << ": Status = " << rsp.api_status() << endl;
            } else {
                if (rsp.spec().wire_encap().encap_type() == ENCAP_TYPE_DOT1Q) {
                    l2seg_handle = rsp.status().l2segment_handle();
                    l2seg_id = rsp.spec().key_or_handle().segment_id();
                    cout << "[INFO] Discovered L2Segment"
                         << " handle " << l2seg_handle
                         << " id " << l2seg_id
                         << " VLAN " << rsp.spec().wire_encap().encap_value() << endl;
                    l2seg_handle2id[l2seg_handle] = l2seg_id;
                    l2seg_id2handle[l2seg_id] = l2seg_handle;
                    vlan2seg_map[rsp.spec().wire_encap().encap_value()] = l2seg_handle;
                }
            }
        }
    } else {
        cout << "[ERROR] " << __FUNCTION__ << ": Status = " << status.error_code() << ":" << status.error_message() << endl;
        return -1;
    }

    return 0;
}

uint64_t
HalClient::L2SegmentCreate(uint64_t vrf_id,
                            uint64_t l2seg_id,
                           ::l2segment::BroadcastFwdPolicy bcast_policy,
                           ::l2segment::MulticastFwdPolicy mcast_policy,
                           uint16_t vlan_id)
{
    L2SegmentSpec           *spec;
    L2SegmentResponse       rsp;
    L2SegmentRequestMsg     req_msg;
    L2SegmentResponseMsg    rsp_msg;
    // NetworkKeyHandle        *nw_kh;
    ClientContext           context;
    Status                  status;

    uint64_t                l2seg_handle;

    spec = req_msg.add_request();
    spec->mutable_meta()->set_vrf_id(vrf_id);
    spec->mutable_key_or_handle()->set_segment_id(l2seg_id);
    // if (nw_handle) {
    //     nw_kh = spec->add_network_key_handle();
    //     nw_kh->set_nw_handle(nw_handle);
    // }
    spec->mutable_vrf_key_handle()->set_vrf_id(vrf_id);
    spec->set_mcast_fwd_policy(mcast_policy);
    spec->set_bcast_fwd_policy(bcast_policy);
    spec->mutable_wire_encap()->set_encap_type(::types::ENCAP_TYPE_DOT1Q);
    spec->mutable_wire_encap()->set_encap_value(vlan_id);
    status = l2seg_stub_->L2SegmentCreate(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            cout << "[ERROR] " << __FUNCTION__ << ": Status = " << rsp.api_status() << endl;
        } else {
            l2seg_handle = rsp.l2segment_status().l2segment_handle();
            cout << "[INFO] L2 segment create succeeded, handle = "
                  << l2seg_handle
                  << endl;
            l2seg_handle2id[l2seg_handle] = l2seg_id;
            l2seg_id2handle[l2seg_id] = l2seg_handle;
            vlan2seg_map[vlan_id] = l2seg_handle;
            return l2seg_handle;
        }
    } else {
        cout << "[ERROR] " << __FUNCTION__ << ": Status = " << status.error_code() << ":" << status.error_message() << endl;
        return -1;
    }

    return 0;
}

int
HalClient::AddL2SegmentOnUplink(uint64_t uplink_if_id, uint64_t l2seg_handle)
{
    InterfaceL2SegmentSpec           *spec __attribute__((unused));
    InterfaceL2SegmentResponse       rsp;
    InterfaceL2SegmentRequestMsg     req_msg;
    InterfaceL2SegmentResponseMsg    rsp_msg;
    ClientContext                    context;
    Status                           status;

    spec = req_msg.add_request();
    spec->mutable_l2segment_key_or_handle()->set_l2segment_handle(l2seg_handle);
    spec->mutable_if_key_handle()->set_interface_id(uplink_if_id);
    status = intf_stub_->AddL2SegmentOnUplink(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK &&
            rsp.api_status() != types::API_STATUS_EXISTS_ALREADY) {
            cout << "[ERROR] " << __FUNCTION__ << ": Status = " << rsp.api_status() << endl;
        } else {
            if (rsp.api_status() == types::API_STATUS_EXISTS_ALREADY) {
                cout << "[INFO] L2 Segment handle = " << l2seg_handle
                          << " already added to uplink id = "  << uplink_if_id
                          << endl;
            } else {
                cout << "[INFO] L2 Segment handle = " << l2seg_handle
                          << " add to uplink id = "  << uplink_if_id
                          << " succeeded"
                          << endl;
            }
            return 0;
        }
    } else {
        cout << "[ERROR] " << __FUNCTION__ << ": Status = " << status.error_code() << ":" << status.error_message() << endl;
        return -1;
    }

    return -1;
}

/**
 * ENIC APIs
 */
uint64_t
HalClient::EnicCreate(uint32_t enic_id, uint32_t lif_id,
                      uint64_t pinned_uplink_if_handle,
                      uint64_t native_l2seg_handle,
                      vector<uint64_t> &non_native_l2seg_handles,
                      uint64_t mac_addr)
{
    InterfaceSpec           *spec;
    InterfaceResponse       rsp;
    InterfaceRequestMsg     req_msg;
    InterfaceResponseMsg    rsp_msg;
    L2SegmentKeyHandle      *l2seg_kh;
    ClientContext           context;
    Status                  status;
    // TODO: HACK - Need an allocator for useg_vlan
    uint32_t                useg_vlan = lif_id;
    assert(lif_id < 4096);

    spec = req_msg.add_request();
    spec->mutable_key_or_handle()->set_interface_id(enic_id);
    spec->set_type(::intf::IfType::IF_TYPE_ENIC);
    spec->set_admin_status(::intf::IfStatus::IF_STATUS_UP);
    spec->mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(lif_id);
    spec->mutable_if_enic_info()->mutable_pinned_uplink_if_key_handle()->set_if_handle(pinned_uplink_if_handle);

    if (fwd_mode == FWD_MODE_CLASSIC_NIC) {
        spec->mutable_if_enic_info()->set_enic_type(::intf::IF_ENIC_TYPE_CLASSIC);
        spec->mutable_if_enic_info()->mutable_classic_enic_info()->set_native_l2segment_handle(native_l2seg_handle);
        for (auto it = non_native_l2seg_handles.cbegin(); it != non_native_l2seg_handles.cend(); it++) {
            l2seg_kh = spec->mutable_if_enic_info()->mutable_classic_enic_info()->add_l2segment_key_handle();
            l2seg_kh->set_l2segment_handle(*it);
        }
    } else if (fwd_mode == FWD_MODE_SMART_NIC) {
        spec->mutable_if_enic_info()->set_enic_type(::intf::IF_ENIC_TYPE_USEG);
        spec->mutable_if_enic_info()->mutable_enic_info()->set_mac_address(mac_addr);
        spec->mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(useg_vlan);
        spec->mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_l2segment_handle(native_l2seg_handle);
        // TODO: support non-native vlans by creating multiple enics
    } else {
        assert(0);
    }

    status = HalClient::intf_stub_->InterfaceCreate(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            cout << "[ERROR] " << __FUNCTION__ << ": Status = " << rsp.api_status() << endl;
        } else {
            cout << "[INFO] ENIC create succeeded, handle = "
                 << rsp.status().if_handle()
                 << endl;
            lif2enic_map[lif_id] = enic_id;
            enic_map[enic_id] = *spec;
            enic_handle2id[rsp.status().if_handle()] = enic_id;
            enic_id2handle[enic_id] = rsp.status().if_handle();
            return rsp.status().if_handle();
        }
    } else {
        cout << "[ERROR] " << __FUNCTION__ << ": Status = " << status.error_code() << ":" << status.error_message() << endl;
    }

    return 0;
}

/**
 * Endpoint APIs
 */
int
HalClient::EndpointProbe()
{
    uint64_t                    enic_id;
    EndpointGetResponse         rsp;
    EndpointGetRequestMsg       req_msg;
    EndpointGetResponseMsg      rsp_msg;
    ClientContext               context;
    Status                      status;

    req_msg.add_request();
    status = HalClient::ep_stub_->EndpointGet(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        for (int i = 0; i < rsp_msg.response().size(); i++) {
            rsp = rsp_msg.response(i);
            if (rsp.api_status() != types::API_STATUS_OK) {
                cout << "[ERROR] " << __FUNCTION__ << ": Status = " << rsp.api_status() << endl;
            } else {
                enic_id = rsp.spec().endpoint_attrs().interface_key_handle().interface_id();
                cout << "[INFO] Discovered Endpoint handle = " << rsp.status().endpoint_handle()
                     << " enic id " << enic_id << endl;
                enic2ep_map[enic_id].push_back(rsp.status().endpoint_handle());
            }
        }

    } else {
        cout << "[ERROR] " << __FUNCTION__ << ": Status = " << status.error_code() << ":" << status.error_message() << endl;
        return -1;
    }

    return 0;
}

uint64_t
HalClient::EndpointCreate(uint64_t vrf_id, uint64_t l2seg_handle,
                          uint64_t enic_id, uint64_t sg_id,
                          uint64_t mac_addr, uint64_t ip_addr)
{
    EndpointSpec              *spec;
    EndpointResponse          rsp;
    EndpointRequestMsg        req_msg;
    EndpointResponseMsg       rsp_msg;
    SecurityGroupKeyHandle    *sg_kh;
    IPAddress                 *ip;
    ClientContext             context;
    Status                    status;

    spec = req_msg.add_request();
    spec->mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_l2segment_handle(l2seg_handle);
    spec->mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(mac_addr);
    spec->mutable_vrf_key_handle()->set_vrf_id(vrf_id);
    spec->mutable_endpoint_attrs()->mutable_interface_key_handle()->set_interface_id(enic_id);
    if (fwd_mode == FWD_MODE_SMART_NIC && ip_addr) {
        ip = spec->mutable_endpoint_attrs()->add_ip_address();
        ip->set_ip_af(types::IPAddressFamily::IP_AF_INET);
        ip->set_v4_addr(ip_addr);
    }
    if (fwd_mode == FWD_MODE_SMART_NIC && sg_id) {
        sg_kh = spec->mutable_endpoint_attrs()->add_sg_key_handle();
        sg_kh->set_security_group_id(sg_id);
    }
    status = HalClient::ep_stub_->EndpointCreate(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            cout << "[ERROR] " << __FUNCTION__ << ": Status = " << rsp.api_status() << endl;
        } else {
            cout << "[INFO] Endpoint create succeeded, handle = "
                 << rsp.endpoint_status().endpoint_handle()
                 << endl;
            return rsp.endpoint_status().endpoint_handle();
        }
    } else {
        cout << "[ERROR] " << __FUNCTION__ << ": Status = " << status.error_code() << ":" << status.error_message() << endl;
    }

    return 0;
}

int
HalClient::EndpointDelete(uint64_t vrf_id, uint64_t handle)
{
    EndpointDeleteRequest           *req;
    EndpointDeleteResponse          rsp;
    EndpointDeleteRequestMsg        req_msg;
    EndpointDeleteResponseMsg       rsp_msg;
    ClientContext                   context;
    Status                          status;

    req = req_msg.add_request();
    req->mutable_vrf_key_handle()->set_vrf_id(vrf_id);
    req->mutable_key_or_handle()->set_endpoint_handle(handle);
    status = HalClient::ep_stub_->EndpointDelete(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            cout << "[ERROR] " << __FUNCTION__
                 << ": Handle = " << handle
                 << ", Status = " << rsp.api_status()
                 << endl;
        } else {
            cout << "[INFO] Endpoint delete succeeded, handle = " << handle << endl;
            return 0;
        }
    } else {
        cout << "[ERROR] " << __FUNCTION__
             << ": Handle = " << handle
             << ", Status = " << status.error_code() << ":" << status.error_message()
             << endl;
    }

    return -1;
}

/**
 * Interface APIs
 */
int
HalClient::InterfaceProbe()
{
    ClientContext context;
    Status status;
    InterfaceGetRequest         *req __attribute__((unused));
    InterfaceGetResponse        rsp;
    InterfaceGetRequestMsg      req_msg;
    InterfaceGetResponseMsg     rsp_msg;

    req = req_msg.add_request();
    status = intf_stub_->InterfaceGet(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        for (int i = 0; i < rsp_msg.response().size(); i++) {
            rsp = rsp_msg.response(i);
            if (rsp.api_status() != types::API_STATUS_OK) {
                cout << "[ERROR] " << __FUNCTION__ << ": Status = " << rsp.api_status() << endl;
            } else {
                if (rsp.spec().type() == IF_TYPE_UPLINK) {
                    cout << "[INFO] Discovered Uplink Interface"
                         << " handle = " << rsp.status().if_handle()
                         << " id = " << rsp.spec().key_or_handle().interface_id()
                         << " port = " << rsp.spec().if_uplink_info().port_num() << endl;
                    uplink_map[rsp.spec().if_uplink_info().port_num()] = rsp.status().if_handle();
                }
                if (rsp.spec().type() == IF_TYPE_ENIC) {
                    cout << "[INFO] Discovered Enic Interface"
                         << " handle = " << rsp.status().if_handle()
                         << " id = " << rsp.spec().key_or_handle().interface_id()
                         << " lif = " << rsp.spec().if_enic_info().lif_key_or_handle().lif_id() << endl;

                    lif2enic_map[rsp.spec().if_enic_info().lif_key_or_handle().lif_id()] = rsp.spec().key_or_handle().interface_id();
                    enic_map[rsp.spec().key_or_handle().interface_id()] = rsp.spec();
                    enic_handle2id[rsp.status().if_handle()] = rsp.spec().key_or_handle().interface_id();
                    enic_id2handle[rsp.spec().key_or_handle().interface_id()] = rsp.status().if_handle();
                }
            }
        }
    } else {
        cout << "[ERROR] " << __FUNCTION__ << ": Status = " << status.error_code() << ":" << status.error_message() << endl;
        return -1;
    }

    return 0;
}

int
HalClient::InterfaceDelete(uint64_t if_id)
{
    InterfaceDeleteRequest          *req;
    InterfaceDeleteResponse         rsp;
    InterfaceDeleteRequestMsg       req_msg;
    InterfaceDeleteResponseMsg      rsp_msg;
    ClientContext                   context;
    Status                          status;

    req = req_msg.add_request();
    req->mutable_key_or_handle()->set_interface_id(if_id);
    status = HalClient::intf_stub_->InterfaceDelete(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            cout << "[ERROR] " << __FUNCTION__
                 << ": Id = " << if_id
                 << ", Status = " << rsp.api_status()
                 << endl;
        } else {
            cout << "[INFO] Interface delete succeeded, id = " << if_id << endl;
            // Update State
            for (map<uint64_t, uint64_t>::iterator it = lif2enic_map.begin();
                    it != lif2enic_map.end(); it++) {
                if (it->second == if_id) {
                    lif2enic_map.erase(it->first);
                    break;
                }
            }
            enic_map.erase(if_id);
            enic2ep_map.erase(if_id);
            enic_handle2id.erase(enic_id2handle[if_id]);
            enic_id2handle.erase(if_id);
            return 0;
        }
    } else {
        cout << "[ERROR] " << __FUNCTION__
             << ": Id = " << if_id
             << ", Status = " << status.error_code() << ":" << status.error_message()
             << endl;
    }

    return -1;
}

/**
 * LIF APIs
 */
int
HalClient::LifProbe()
{
    LifGetResponse rsp;
    LifGetRequest *req __attribute__((unused));
    LifGetRequestMsg req_msg;
    LifGetResponseMsg rsp_msg;
    ClientContext context;
    Status status;

    req = req_msg.add_request();
    status = intf_stub_->LifGet(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        for (int i = 0; i < rsp_msg.response().size(); i++) {
            rsp = rsp_msg.response(i);
            if (rsp.api_status() != types::API_STATUS_OK) {
                cout << "[ERROR] " << __FUNCTION__ << ": Status = " << rsp.api_status() << endl;
            } else {
                cout << "[INFO] Discovered Lif id = " << rsp.spec().key_or_handle().lif_id() << endl;
                lif_map[rsp.spec().key_or_handle().lif_id()] = LifSpec(rsp.spec());
            }
        }
    } else {
        cout << "[ERROR] " << __FUNCTION__ << ": Status = " << status.error_code() << ":" << status.error_message() << endl;
        return -1;
    }

    return 0;
}

uint64_t
HalClient::LifGet(uint64_t lif_id, struct lif_info *lif_info)
{
    LifGetResponse rsp;
    LifGetRequest *req __attribute__((unused));
    LifGetRequestMsg req_msg;
    LifGetResponseMsg rsp_msg;
    ClientContext context;
    Status status;

    req = req_msg.add_request();
    req->mutable_key_or_handle()->set_lif_id(lif_id);
    status = intf_stub_->LifGet(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        for (int i = 0; i < rsp_msg.response().size(); i++) {
            rsp = rsp_msg.response(i);
            if (rsp.api_status() != types::API_STATUS_OK) {
                cout << "[ERROR] " << __FUNCTION__
                     << ": Id = " << lif_id
                     << ", Status = " << rsp.api_status()
                     << endl;
            } else {
                cout << "[INFO] Get Lif " << rsp.spec().key_or_handle().lif_id() << endl;

                lif_info->hw_lif_id = rsp.status().hw_lif_id();
                for (int i = 0; i < rsp.qstate().size(); i++) {
                    auto & qstate = rsp.qstate()[i];
                    cout << "[INFO] lif " << rsp.status().hw_lif_id()
                         << " qtype " << qstate.type_num()
                         << " qstate 0x" << hex << qstate.addr() << resetiosflags(ios::hex)
                         << endl;
                    lif_info->qstate_addr[qstate.type_num()] = qstate.addr();
                }
                lif_map[rsp.spec().key_or_handle().lif_id()] = LifSpec(rsp.spec());

                return rsp.spec().key_or_handle().lif_id();
            }
        }
    } else {
        cout << "[ERROR] " << __FUNCTION__
             << ": Id = " << lif_id
             << ", Status = " << status.error_code() << ":" << status.error_message()
             << endl;
    }

    return 0;
}

uint64_t
HalClient::LifCreate(uint32_t lif_id, struct queue_info *queue_info,
                     struct lif_info *lif_info,
                     bool enable_rdma, uint32_t max_pt_entries, uint32_t max_keys)
{
    LifSpec              *spec;
    LifResponse          rsp;
    LifRequestMsg        req_msg;
    LifResponseMsg       rsp_msg;
    LifQStateMapEntry    *lif_qstate_map_ent;
    QStateSetReq         *qstate_req;
    ClientContext        context;
    Status               status;

    spec = req_msg.add_request();
    spec->mutable_key_or_handle()->set_lif_id(lif_id);
    spec->set_admin_status(::intf::IF_STATUS_UP);
    spec->set_enable_rdma(enable_rdma);
    spec->set_rdma_max_pt_entries(max_pt_entries);
    spec->set_rdma_max_keys(max_keys);

    for (uint32_t i = 0; i < NUM_QUEUE_TYPES; i++) {
        auto & qinfo = queue_info[i];
        if (qinfo.size < 1) continue;

        lif_qstate_map_ent = spec->add_lif_qstate_map();
        lif_qstate_map_ent->set_type_num(qinfo.type_num);
        lif_qstate_map_ent->set_size(qinfo.size);
        lif_qstate_map_ent->set_entries(qinfo.entries);
        lif_qstate_map_ent->set_purpose(qinfo.purpose);
        for (uint32_t qid = 0; qid < (uint32_t)pow(2, qinfo.entries); qid++) {
            qstate_req = spec->add_lif_qstate();
            qstate_req->set_lif_handle(0);
            qstate_req->set_type_num(qinfo.type_num);
            qstate_req->set_qid(qid);
            qstate_req->set_queue_state(qinfo.qstate, (uint32_t)pow(2, qinfo.size + 5));
            qstate_req->mutable_label()->set_handle("p4plus");
            qstate_req->mutable_label()->set_prog_name(qinfo.prog);
            qstate_req->mutable_label()->set_label(qinfo.label);
        }
    }

    status = intf_stub_->LifCreate(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            cout << "[ERROR] " << __FUNCTION__
                 << ": Id = " << lif_id
                 << ", Status = " << rsp.api_status()
                 << endl;
        } else {
            cout << "[INFO] Lif create succeeded, id = "
                 << lif_id
                 << ", hw_lif_id = "
                 << rsp.status().hw_lif_id()
                 << endl;

            lif_info->hw_lif_id = rsp.status().hw_lif_id();
            for (int i = 0; i < rsp.qstate().size(); i++) {
                auto & qstate = rsp.qstate()[i];
                cout << "[INFO] lif " << rsp.status().hw_lif_id()
                     << " qtype " << qstate.type_num()
                     << " qstate 0x" << hex << qstate.addr() << resetiosflags(ios::hex)
                     << endl;
                lif_info->qstate_addr[qstate.type_num()] = qstate.addr();
            }
            lif_map[lif_id] = LifSpec(*spec);
            return rsp.status().lif_handle();
        }
    } else {
        cout << "[ERROR] " << __FUNCTION__
             << ": Id = " << lif_id
             << ", Status = " << status.error_code() << ":" << status.error_message()
             << endl;
    }

    return 0;
}

int
HalClient::LifDelete(uint32_t lif_id)
{
    LifDeleteRequest        *req;
    LifDeleteResponse       rsp;
    LifDeleteRequestMsg     req_msg;
    LifDeleteResponseMsg    rsp_msg;
    ClientContext           context;
    Status                  status;

    req = req_msg.add_request();
    req->mutable_key_or_handle()->set_lif_id(lif_id);
    status = intf_stub_->LifDelete(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            cout << "[ERROR] " << __FUNCTION__
                 << ": Id = " << lif_id
                 << ", Status = " << rsp.api_status()
                 << endl;
        } else {
            cout << "[INFO] Lif delete succeeded, id = " << lif_id << endl;
            lif_map.erase(lif_id);
            return 0;
        }
    } else {
        cout << "[ERROR] " << __FUNCTION__
             << ": Id = " << lif_id
             << ", Status = " << status.error_code() << ":" << status.error_message()
             << endl;
    }

    return -1;
}

int
HalClient::LifSetVlanStrip(uint64_t lif_id, bool enable)
{
    ClientContext context;
    Status status;
    LifSpec *lif_spec;
    LifResponse rsp;
    LifRequestMsg req_msg;
    LifResponseMsg rsp_msg;

    if (lif_map.find(lif_id) == lif_map.end()) {
        cout << "[ERROR] " << __FUNCTION__ << ": Invalid Lif id " << lif_id << endl;
        cout << *this << endl;
        return (-1);
    }
    lif_spec = req_msg.add_request();
    lif_spec->CopyFrom(lif_map[lif_id]);
    lif_spec->mutable_key_or_handle()->set_lif_id(lif_id);
    lif_spec->set_vlan_strip_en(enable);
    status = intf_stub_->LifUpdate(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            cout << "[ERROR] " << __FUNCTION__
                 << ": Id = " << lif_id
                 << ", Status = " << rsp.api_status()
                 << endl;
        } else {
            cout << "[INFO] " << __FUNCTION__ << " succeeded, id =" << lif_id << endl;
            lif_map[lif_id] = *lif_spec;
            return 0;
        }
    } else {
        cout << "[ERROR] " << __FUNCTION__
             << ": Id = " << lif_id
             << ", Status = " << status.error_code() << ":" << status.error_message()
             << endl;
    }

    return -1;
}

int
HalClient::LifSetVlanInsert(uint64_t lif_id, bool enable)
{
    ClientContext context;
    Status status;
    LifSpec *lif_spec;
    LifResponse rsp;
    LifRequestMsg req_msg;
    LifResponseMsg rsp_msg;

    if (lif_map.find(lif_id) == lif_map.end()) {
        cout << "[ERROR] " << __FUNCTION__ << ": Invalid Lif id " << lif_id << endl;
        cout << *this << endl;
        return (-1);
    }
    lif_spec = req_msg.add_request();
    lif_spec->CopyFrom(lif_map[lif_id]);
    lif_spec->mutable_key_or_handle()->set_lif_id(lif_id);
    lif_spec->set_vlan_insert_en(enable);
    status = intf_stub_->LifUpdate(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            cout << "[ERROR] " << __FUNCTION__
                 << ": Id = " << lif_id
                 << ", Status = " << rsp.api_status()
                 << endl;
        } else {
            cout << "[INFO] " << __FUNCTION__ << " succeeded, id =" << lif_id << endl;
            lif_map[lif_id] = *lif_spec;
            return 0;
        }
    } else {
        cout << "[ERROR] " << __FUNCTION__
             << ": Id = " << lif_id
             << ", Status = " << status.error_code() << ":" << status.error_message()
             << endl;
    }

    return -1;
}

int
HalClient::LifSetBroadcast(uint64_t lif_id, bool enable)
{
    ClientContext context;
    Status status;
    LifSpec *lif_spec;
    LifResponse rsp;
    LifRequestMsg req_msg;
    LifResponseMsg rsp_msg;

    if (fwd_mode == FWD_MODE_SMART_NIC) {
        return 0;
    }

    if (lif_map.find(lif_id) == lif_map.end()) {
        cout << "[ERROR] " << __FUNCTION__ << ": Invalid Lif id " << lif_id << endl;
        cout << *this << endl;
        return -1;
    }
    lif_spec = req_msg.add_request();
    lif_spec->CopyFrom(lif_map[lif_id]);
    lif_spec->mutable_key_or_handle()->set_lif_id(lif_id);
    lif_spec->mutable_packet_filter()->set_receive_broadcast(enable);
    status = intf_stub_->LifUpdate(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            cout << "[ERROR] " << __FUNCTION__
                 << ": Id = " << lif_id
                 << ", Status = " << rsp.api_status()
                 << endl;
        } else {
            cout << "[INFO] " << __FUNCTION__ << " succeeded, id = " << lif_id << endl;
            lif_map[lif_id] = *lif_spec;
            return 0;
        }
    } else {
        cout << "[ERROR] " << __FUNCTION__
             << ": Id = " << lif_id
             << ", Status = " << status.error_code() << ":" << status.error_message()
             << endl;
    }

    return -1;
}

int
HalClient::LifSetAllMulticast(uint64_t lif_id, bool enable)
{
    ClientContext context;
    Status status;
    LifSpec *lif_spec;
    LifResponse rsp;
    LifRequestMsg req_msg;
    LifResponseMsg rsp_msg;

    if (fwd_mode == FWD_MODE_SMART_NIC) {
        return 0;
    }

    if (lif_map.find(lif_id) == lif_map.end()) {
        cout << "[ERROR] " << __FUNCTION__ << ": Invalid Lif id " << lif_id << endl;
        cout << *this << endl;
        return -1;
    }
    lif_spec = req_msg.add_request();
    lif_spec->CopyFrom(lif_map[lif_id]);
    lif_spec->mutable_key_or_handle()->set_lif_id(lif_id);
    lif_spec->mutable_packet_filter()->set_receive_all_multicast(enable);
    status = intf_stub_->LifUpdate(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            cout << "[ERROR] " << __FUNCTION__
                 << ": Id = " << lif_id
                 << ", Status = " << rsp.api_status()
                 << endl;
        } else {
            cout << "[INFO] " << __FUNCTION__ << " succeeded, id = " << lif_id << endl;
            lif_map[lif_id] = *lif_spec;
            return 0;
        }
    } else {
        cout << "[ERROR] " << __FUNCTION__
             << ": Id = " << lif_id
             << ", Status = " << status.error_code() << ":" << status.error_message()
             << endl;
    }

    return -1;
}

int
HalClient::LifSetPromiscuous(uint64_t lif_id, bool enable)
{
    ClientContext context;
    Status status;
    LifSpec *lif_spec;
    LifResponse rsp;
    LifRequestMsg req_msg;
    LifResponseMsg rsp_msg;

    if (lif_map.find(lif_id) == lif_map.end()) {
        cout << "[ERROR] " << __FUNCTION__ << ": Invalid Lif id " << lif_id << endl;
        cout << *this << endl;
        return (-1);
    }
    lif_spec = req_msg.add_request();
    lif_spec->CopyFrom(lif_map[lif_id]);
    lif_spec->mutable_key_or_handle()->set_lif_id(lif_id);
    lif_spec->mutable_packet_filter()->set_receive_promiscuous(enable);
    status = intf_stub_->LifUpdate(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            cout << "[ERROR] " << __FUNCTION__
                 << ": Id = " << lif_id
                 << ", Status = " << rsp.api_status()
                 << endl;
        } else {
            cout << "[INFO] " << __FUNCTION__ << " succeeded, id = " << lif_id << endl;
            lif_map[lif_id] = *lif_spec;
            return 0;
        }
    } else {
        cout << "[ERROR] " << __FUNCTION__
             << ": Id = " << lif_id
             << ", Status = " << status.error_code() << ":" << status.error_message()
             << endl;
    }

    return -1;
}

int
HalClient::LifSetRssConfig(uint64_t lif_id, LifRssType type, string key, string table)
{
    ClientContext context;
    Status status;
    LifSpec *lif_spec;
    LifResponse rsp;
    LifRequestMsg req_msg;
    LifResponseMsg rsp_msg;

    if (lif_map.find(lif_id) == lif_map.end()) {
        cout << "[ERROR] " << __FUNCTION__ << ": Invalid Lif id " << lif_id << endl;
        cout << *this << endl;
        return -1;
    }
    lif_spec = req_msg.add_request();
    lif_spec->CopyFrom(lif_map[lif_id]);
    lif_spec->mutable_key_or_handle()->set_lif_id(lif_id);
    lif_spec->mutable_rss()->set_type(type);
    lif_spec->mutable_rss()->set_key(key);
    lif_spec->mutable_rss()->set_indir(table);
    status = intf_stub_->LifUpdate(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            cout << "[ERROR] " << __FUNCTION__
                 << ": Id = " << lif_id
                 << ", Status = " << rsp.api_status()
                 << endl;
        } else {
            cout << "[INFO] " << __FUNCTION__ << " succeeded, id = " << lif_id << endl;
            lif_map[lif_id] = *lif_spec;
            return 0;
        }
    } else {
        cout << "[ERROR] " << __FUNCTION__
             << ": Id = " << lif_id
             << ", Status = " << status.error_code() << ":" << status.error_message()
             << endl;
    }

    return -1;
}

/*
 * Multicast APIs
 */

int
HalClient::MulticastProbe()
{
    ClientContext context;
    Status status;
    MulticastEntryGetRequest *req __attribute__((unused));
    MulticastEntryGetResponse rsp;
    MulticastEntryGetRequestMsg req_msg;
    MulticastEntryGetResponseMsg rsp_msg;
    uint64_t vrf_id = 1, l2seg_id, group;

    req = req_msg.add_request();
    status = multicast_stub_->MulticastEntryGet(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        for (int i = 0; i < rsp_msg.response().size(); i++) {
            rsp = rsp_msg.response(i);
            if (rsp.api_status() != types::API_STATUS_OK) {
                cout << "[ERROR] " << __FUNCTION__
                     << " status " << rsp.api_status()
                     << endl;
            } else {
                group = rsp.spec().key_or_handle().key().mac().group();
                l2seg_id = rsp.spec().key_or_handle().key().l2segment_key_handle().segment_id();
                cout << "[INFO] Discovered"
                     << " group " << hex << group << resetiosflags(ios::hex)
                     << " vrf_id " <<  vrf_id
                     << " l2seg_id " << l2seg_id
                     << endl;
                for (auto it = rsp.spec().oif_key_handles().cbegin();
                        it != rsp.spec().oif_key_handles().cend(); it++) {
                    cout << "[INFO] Discovered"
                         << " group " << hex << group << resetiosflags(ios::hex)
                         << " vrf_id " <<  vrf_id
                         << " l2seg_id " << l2seg_id
                         << " if_id " << enic_handle2id[it->if_handle()] // TODO: HAL should return id
                         << endl;
                    tuple<uint64_t, uint64_t, uint64_t> key(vrf_id, l2seg_id, group);
                    mcast_groups[key].push_back(enic_handle2id[it->if_handle()]);
                }
            }
        }
        return 0;
    } else {
        cout << "[ERROR] " << __FUNCTION__ << ": Status = " << status.error_code() << ":" << status.error_message() << endl;
    }

    return -1;
}

int
HalClient::MulticastGroupGet(uint64_t group,
                             uint64_t vrf_id,
                             uint64_t l2seg_id)
{
    ClientContext context;
    Status status;
    MulticastEntryGetRequest *req;
    MulticastEntryGetResponse rsp;
    MulticastEntryGetRequestMsg req_msg;
    MulticastEntryGetResponseMsg rsp_msg;
    tuple<uint64_t, uint64_t, uint64_t> key;

    req = req_msg.add_request();
    req->mutable_meta()->set_vrf_id(vrf_id);
    req->mutable_key_or_handle()->mutable_key()->mutable_l2segment_key_handle()->set_l2segment_handle(l2seg_id2handle[l2seg_id]);
    req->mutable_key_or_handle()->mutable_key()->mutable_mac()->set_group(group);
    status = multicast_stub_->MulticastEntryGet(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        for (int i = 0; i < rsp_msg.response().size(); i++) {
            rsp = rsp_msg.response(i);
            if (rsp.api_status() != types::API_STATUS_OK) {
                cout << "[ERROR] " << __FUNCTION__
                     << ": group " << hex << group << resetiosflags(ios::hex)
                     << " vrf_id " << vrf_id
                     << " l2seg_id " << l2seg_id
                     << " status " << rsp.api_status()
                     << endl;
            } else {
                cout << "[INFO] Discovered"
                     << " group " << hex << group << resetiosflags(ios::hex)
                     << " vrf_id " <<  vrf_id
                     << " l2seg_id " << l2seg_id
                     << endl;

                for (auto it = rsp.spec().oif_key_handles().cbegin();
                        it != rsp.spec().oif_key_handles().cend(); it++) {
                    cout << "[INFO] Discovered"
                         << " group " << hex << group << resetiosflags(ios::hex)
                         << " vrf_id " <<  vrf_id
                         << " l2seg_id " << l2seg_id
                         << " if_id " << enic_handle2id[it->if_handle()] // TODO: HAL should return id
                         << endl;
                    tuple<uint64_t, uint64_t, uint64_t> key(vrf_id, l2seg_id, group);
                    mcast_groups[key].push_back(enic_handle2id[it->if_handle()]);
                }
                return 0;
            }
        }
    } else {
        cout << "[ERROR] " << __FUNCTION__ << ": Status = " << status.error_code() << ":" << status.error_message() << endl;
    }

    return -1;
}

int
HalClient::MulticastGroupCreate(uint64_t group,
                                uint64_t vrf_id,
                                uint64_t l2seg_id)
{
    ClientContext context;
    Status status;
    MulticastEntrySpec *spec;
    MulticastEntryResponse rsp;
    MulticastEntryRequestMsg req_msg;
    MulticastEntryResponseMsg rsp_msg;
    tuple<uint64_t, uint64_t, uint64_t> key(vrf_id, l2seg_id, group);

    if (fwd_mode == FWD_MODE_SMART_NIC) {
        return 0;
    }

    // Do we know if this group exists?
    if (mcast_groups.find(key) != mcast_groups.end()) {
        cout << "[INFO] " << __FUNCTION__
             << ": group " << hex << group << resetiosflags(ios::hex)
             << " vrf_id " << vrf_id
             << " l2seg_id " << l2seg_id
             << " already exists!"
             << endl;
        return 0;
    }

    // Create the group
    spec = req_msg.add_request();
    spec->mutable_meta()->set_vrf_id(vrf_id);
    spec->mutable_key_or_handle()->mutable_key()->mutable_l2segment_key_handle()->set_l2segment_handle(l2seg_id2handle[l2seg_id]);
    spec->mutable_key_or_handle()->mutable_key()->mutable_mac()->set_group(group);
    status = multicast_stub_->MulticastEntryCreate(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            cout << "[ERROR] " << __FUNCTION__
                 << ": group " << hex << group << resetiosflags(ios::hex)
                 << " vrf_id " << vrf_id
                 << " l2seg_id " << l2seg_id
                 << ", API Status " << rsp.api_status()
                 << endl;
        } else {
            cout << "[INFO] " << __FUNCTION__
                 << " succeeded,"
                 << " group " << hex << group << resetiosflags(ios::hex)
                 << " vrf_id " << vrf_id
                 << " l2seg_id " << l2seg_id
                 << endl;
            mcast_groups[key] = vector<uint64_t>();
            return 0;
        }
    } else {
        cout << "[ERROR] " << __FUNCTION__
             << ": group " << hex << group << resetiosflags(ios::hex)
             << " vrf_id " << vrf_id
             << " l2seg_id " << l2seg_id
             << ", Status " << status.error_code() << ":" << status.error_message()
             << endl;
    }

    return -1;
}

int
HalClient::MulticastGroupUpdate(uint64_t group,
                                uint64_t vrf_id,
                                uint64_t l2seg_id,
                                vector<uint64_t>& oifs_list)
{
    ClientContext context;
    Status status;
    MulticastEntrySpec *spec;
    MulticastEntryResponse rsp;
    MulticastEntryRequestMsg req_msg;
    MulticastEntryResponseMsg rsp_msg;
    tuple<uint64_t, uint64_t, uint64_t> key(vrf_id, l2seg_id, group);

    if (fwd_mode == FWD_MODE_SMART_NIC) {
        return 0;
    }

    spec = req_msg.add_request();
    spec->mutable_meta()->set_vrf_id(vrf_id);
    spec->mutable_key_or_handle()->mutable_key()->mutable_l2segment_key_handle()->set_l2segment_handle(l2seg_id2handle[l2seg_id]);
    spec->mutable_key_or_handle()->mutable_key()->mutable_mac()->set_group(group);
    for (auto it = oifs_list.cbegin(); it != oifs_list.cend(); it++) {
        spec->add_oif_key_handles()->set_interface_id(*it);
    }
    status = multicast_stub_->MulticastEntryUpdate(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            cout << "[ERROR] " << __FUNCTION__
                 << ": group " << hex << group << resetiosflags(ios::hex)
                 << " vrf_id " << vrf_id
                 << " l2seg_id " << l2seg_id
                 << " oifs_list ";
            for (auto it = oifs_list.cbegin(); it != oifs_list.cend(); it++)
                cout << *it << ' ';
            cout << ", API Status " << rsp.api_status()
                 << endl;
        } else {
            cout << "[INFO] " << __FUNCTION__
                 << " succeeded,"
                 << " group " << hex << group << resetiosflags(ios::hex)
                 << " vrf_id " << vrf_id
                 << " l2seg_id " << l2seg_id
                 << " oifs_list ";
            for (auto it = oifs_list.cbegin(); it != oifs_list.cend(); it++)
                cout << *it << ' ';
            cout << endl;
            return 0;
        }
    } else {
        cout << "[ERROR] " << __FUNCTION__
             << ": group " << hex << group << resetiosflags(ios::hex)
             << " vrf_id " << vrf_id
             << " l2seg_id " << l2seg_id
             << " oifs_list ";
        for (auto it = oifs_list.cbegin(); it != oifs_list.cend(); it++)
            cout << *it << ' ';
        cout << ", Status " << status.error_code() << ":" << status.error_message()
             << endl;
    }

    return -1;
}

int
HalClient::MulticastGroupJoin(uint64_t group,
                              uint64_t vrf_id,
                              uint64_t l2seg_id,
                              uint64_t enic_id)
{
    tuple<uint64_t, uint64_t, uint64_t> key(vrf_id, l2seg_id, group);

    if (fwd_mode == FWD_MODE_SMART_NIC) {
        return 0;
    }

    cout << "[INFO] " << __FUNCTION__
         << ": group " << hex << group << resetiosflags(ios::hex)
         << " vrf_id " << vrf_id
         << " l2seg_id " << l2seg_id
         << " enic_id " << enic_id
         << endl;

    // If we know about this group then add the enic_id to the oif_list
    // and update the MulticastGroup
    if (mcast_groups.find(key) != mcast_groups.end()) {
        auto it = find(mcast_groups[key].cbegin(), mcast_groups[key].cend(), enic_id);
        if (it != mcast_groups[key].cend()) {
            // enic is in the group's oif list. nop!
            cout << "[INFO] " << __FUNCTION__
                 << ": group " << hex << group << resetiosflags(ios::hex)
                 << " vrf_id " << vrf_id
                 << " l2seg_id " << l2seg_id
                 << " enic_id " << enic_id
                 << " already exists! "
                 << endl;
            return 0;
        } else {
            // enic is not in the group's oif list. add it!
            mcast_groups[key].push_back(enic_id);
        }
    } else {
        // If we don't know about this group then try to create it. If it already
        // exists in HAL then return code will be 0 and mcast_groups will be
        // populated with the oif_list.
        if (MulticastGroupCreate(group, vrf_id, l2seg_id) == 0) {
            auto it = find(mcast_groups[key].cbegin(), mcast_groups[key].cend(), enic_id);
            if (it != mcast_groups[key].cend()) {
                // enic is in the group's oif list. nop!
                cout << "[INFO] " << __FUNCTION__
                     << ": group " << hex << group << resetiosflags(ios::hex)
                     << " vrf_id " << vrf_id
                     << " l2seg_id " << l2seg_id
                     << " enic_id " << enic_id
                     << " already exists! "
                     << endl;
                return 0;
            } else {
                // enic is not in the group's oif list. add it!
                mcast_groups[key].push_back(enic_id);
            }
        } else {
            return -1;
        }
    }

    // Update the group
    return MulticastGroupUpdate(group, vrf_id, l2seg_id, mcast_groups[key]);
}

int
HalClient::MulticastGroupLeave(uint64_t group,
                               uint64_t vrf_id,
                               uint64_t l2seg_id,
                               uint64_t enic_id)
{
    tuple<uint64_t, uint64_t, uint64_t> key(vrf_id, l2seg_id, group);

    if (fwd_mode == FWD_MODE_SMART_NIC) {
        return 0;
    }

    cout << "[INFO] " << __FUNCTION__
         << ": group " << hex << group << resetiosflags(ios::hex)
         << " vrf_id " << vrf_id
         << " l2seg_id " << l2seg_id
         << " enic_id " << enic_id
         << endl;

    // If we know about this group then remove the enic_id from the oif_list
    // and update the MulticastGroup
    if (mcast_groups.find(key) != mcast_groups.end()) {
        auto it = find(mcast_groups[key].cbegin(), mcast_groups[key].cend(), enic_id);
        if (it != mcast_groups[key].cend()) {
            // enic is in the group's oif list. remove it!
            mcast_groups[key].erase(it);
        } else {
            // enic is not in the group's oif list. nop!
            cout << "[INFO] " << __FUNCTION__
                 << ": group " << hex << group << resetiosflags(ios::hex)
                 << " vrf_id " << vrf_id
                 << " l2seg_id " << l2seg_id
                 << " enic_id " << enic_id
                 << " does not exist! "
                 << endl;
            return 0;
        }
    } else {
        // If we don't know about this group then try to create it. If it already
        // exists in HAL then return code will be 0 and mcast_groups will be
        // populated with the oif_list.
        if (MulticastGroupCreate(group, vrf_id, l2seg_id) == 0) {
            auto it = find(mcast_groups[key].cbegin(), mcast_groups[key].cend(), enic_id);
            if (it != mcast_groups[key].cend()) {
                // enic is in the group's oif list. remove it!
                mcast_groups[key].erase(it);
            } else {
                // enic is not in the group's oif list. nop!
                cout << "[INFO] " << __FUNCTION__
                     << ": HAL group " << hex << group << resetiosflags(ios::hex)
                     << " vrf_id " << vrf_id
                     << " l2seg_id " << l2seg_id
                     << " enic_id " << enic_id
                     << " does not exist! "
                     << endl;
                return 0;
            }
        } else {
            return -1;
        }
    }

    // Update the group
    return MulticastGroupUpdate(group, vrf_id, l2seg_id, mcast_groups[key]);
}

int
HalClient::MulticastGroupDelete(uint64_t group,
                                uint64_t vrf_id,
                                uint64_t l2seg_id)
{
    ClientContext context;
    Status status;
    MulticastEntryDeleteRequest *req;
    MulticastEntryDeleteResponse rsp;
    MulticastEntryDeleteRequestMsg req_msg;
    MulticastEntryDeleteResponseMsg rsp_msg;
    tuple<uint64_t, uint64_t, uint64_t> key(vrf_id, l2seg_id, group);

    if (fwd_mode == FWD_MODE_SMART_NIC) {
        return 0;
    }

    req = req_msg.add_request();
    req->mutable_meta()->set_vrf_id(vrf_id);
    req->mutable_key_or_handle()->mutable_key()->mutable_l2segment_key_handle()->set_l2segment_handle(l2seg_id2handle[l2seg_id]);
    req->mutable_key_or_handle()->mutable_key()->mutable_mac()->set_group(group);
    status = multicast_stub_->MulticastEntryDelete(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            cout << "[ERROR] " << __FUNCTION__
                 << ": group " << hex << group << resetiosflags(ios::hex)
                 << " vrf_id " << vrf_id
                 << " l2seg_id " << l2seg_id
                 << ", API Status " << rsp.api_status()
                 << endl;
        } else {
            cout << "[INFO] " << __FUNCTION__
                 << " succeeded,"
                 << " group " << hex << group << resetiosflags(ios::hex)
                 << " vrf_id " << vrf_id
                 << " l2seg_id " << l2seg_id
                 << endl;
            mcast_groups.erase(key);
            return 0;
        }
    } else {
        cout << "[ERROR] " << __FUNCTION__
             << ": group " << hex << group << resetiosflags(ios::hex)
             << " vrf_id " << vrf_id
             << " l2seg_id " << l2seg_id
             << ", Status " << status.error_code() << ":" << status.error_message()
             << endl;
    }

    return -1;
}


/*
 * DEBUG
 */
ostream &operator<<(ostream& os, const HalClient& obj) {

    os << "L2SEGMENTS:" << endl;
    for (auto it = obj.vlan2seg_map.cbegin(); it != obj.vlan2seg_map.cend(); it++) {
        os << "\tvlan = " << it->first << ", handle = " << it->second << endl;
    }

    os << "UPLINKS:" << endl;
    for (auto it = obj.uplink_map.cbegin(); it != obj.uplink_map.cend(); it++) {
        os << "\tport_num = " << it->first << ", handle = " << it->second << endl;
    }

    os << "LIFS:" << endl;
    for (auto it = obj.lif_map.cbegin(); it != obj.lif_map.cend(); it++) {
        os << "\tlif_id = " << it->first << endl;
    }

    os << "ENICS:" << endl;
    for (auto it = obj.lif2enic_map.cbegin(); it != obj.lif2enic_map.cend(); it++) {
        os << "\tlif_id = " << it->first << ", enic_id = " << it->second << endl;
    }

    os << "ENDPOINTS:" << endl;
    for (auto it = obj.enic2ep_map.cbegin(); it != obj.enic2ep_map.cend(); it++) {
        for (auto ep_it = it->second.cbegin(); ep_it != it->second.cend(); ep_it++) {
            os << "\tenic_id = " << it->first << ", handle = " << *ep_it << endl;
        }
    }

    os << "MULTICAST:" << endl;
    for (auto it = obj.mcast_groups.cbegin(); it != obj.mcast_groups.cend(); it++) {
        os << "\tgroup = " << get<0>(it->first)
            << ", vrf_id = " << get<1>(it->first)
            << ", l2seg_id = " << get<2>(it->first)
            << ", oifs_list = ";
            for (auto oit = it->second.cbegin(); oit != it->second.cend(); oit++)
                os << *oit << ' ';
        os << endl;
    }

    return os;
}

int HalClient::CreateMR(uint32_t lif_id, uint32_t pd, uint64_t va, uint64_t length,
                        uint16_t access_flags, uint32_t l_key, uint32_t r_key,
                        uint32_t page_size, uint64_t *pt_table, uint32_t pt_size)
{
    ClientContext context;
    RdmaMemRegRequestMsg request;
    RdmaMemRegResponseMsg response;
    
    RdmaMemRegSpec *spec = request.add_request();
    spec->set_hw_lif_id(lif_id);
    spec->set_pd(pd);
    spec->set_va(va);
    spec->set_len(length);
    spec->set_ac_local_wr(access_flags & AC_LOCAL_WRITE);
    spec->set_ac_remote_wr(access_flags & AC_REMOTE_WRITE);
    spec->set_ac_remote_rd(access_flags & AC_REMOTE_READ);
    spec->set_ac_remote_atomic(access_flags & AC_REMOTE_ATOMIC);
    spec->set_lkey(l_key);
    spec->set_rkey(r_key);
    spec->set_hostmem_pg_size(page_size);

    cout << __FILE__ << ":" << __FUNCTION__ << " VA {} len {}" << va << length << endl;
    cout << __FILE__ << ":" << __FUNCTION__ << " PA Table:" << endl;
    
    //Set the va to pa translations.
    for (int i = 0; i < (int)pt_size; i++) {
        spec->add_va_pages_phy_addr(pt_table[i] | (1ULL << 63) | ((uint64_t)lif_id << 52));
        cout << "PT Entry: " << pt_table[i] << endl;
    }

    Status status = rdma_stub_->RdmaMemReg(&context, request, &response);
    if (status.ok()) {
            RdmaMemRegResponse rsp = response.response(0);
            if (rsp.api_status() != types::API_STATUS_OK) {
                cout << __FILE__ << ":" << __FUNCTION__ << "[Error]: Status = "  << rsp.api_status() << endl;
                return -1;
            } else {
                cout << __FILE__ << ":" << __FUNCTION__ <<  " SUCCESS\n" << endl;
            }
    } else {
        cout << __FILE__ << ":" << __FUNCTION__ << "[Error]: Status = "  << status.error_code() << ": " << status.error_message() << endl;
        return -1;
    }

    return 0;
}

int HalClient::CreateCQ(uint32_t lif_id, uint32_t cq_num, uint16_t cq_wqe_size,
                        uint16_t num_cq_wqes, uint64_t va, uint64_t length,
                        uint32_t l_key, uint32_t page_size, uint64_t *pt_table,
                        uint32_t pt_size, uint32_t eq_id)
{
    ClientContext context1;

    RdmaMemRegRequestMsg request;
    RdmaMemRegResponseMsg response;

    /*
     * Ideally we are supposed to have a single HAL command for CQ and QP
     * implementation, but its not the case right now. We need to change it
     * later. For now call MR registration first and then create_cq 
     */
    RdmaMemRegSpec *spec = request.add_request();
    spec->set_hw_lif_id(lif_id);
    //CQ does not have a PD associated. For now anyway we support only
    //one PD=1
    spec->set_pd(1);
    spec->set_va(va);
    spec->set_len(length);
    spec->set_ac_local_wr(1);
    spec->set_ac_remote_wr(0);
    spec->set_ac_remote_rd(0);
    spec->set_ac_remote_atomic(0);
    spec->set_lkey(l_key);
    spec->set_hostmem_pg_size(page_size);

    //Set the va to pa translations.
    for (int i = 0; i < (int)pt_size; i++) {
        spec->add_va_pages_phy_addr(pt_table[i] | (1ULL << 63) | ((uint64_t)lif_id << 52));
    }

    Status status = rdma_stub_->RdmaMemReg(&context1, request, &response);
    if (status.ok()) {
        RdmaMemRegResponse rsp = response.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            cout << __FILE__ << ":" << __FUNCTION__ << "[Error]: Status = "  << rsp.api_status() << endl;
            return -1;
        } else {
            cout << __FILE__ << ":" << __FUNCTION__ <<  " SUCCESS\n" << endl;
        }
    } else {
        cout << __FILE__ << ":" << __FUNCTION__ << "[Error]: Status = "  << status.error_code() << ": " << status.error_message() << endl;
        return -1;
    }

    cout << __FILE__ << ":" << __FUNCTION__ << " SUCCESS\n" << endl;
 
    /*
     * create CQ
     */
    ClientContext context2;    
    RdmaCqRequestMsg cq_request;
    RdmaCqResponseMsg cq_response;

    RdmaCqSpec *cq_spec = cq_request.add_request();

    cq_spec->set_cq_num(cq_num);
    cq_spec->set_hw_lif_id(lif_id);
    cq_spec->set_cq_wqe_size(cq_wqe_size);
    cq_spec->set_num_cq_wqes(num_cq_wqes);
    cq_spec->set_hostmem_pg_size(page_size);
    cq_spec->set_cq_lkey(l_key);

    status = rdma_stub_->RdmaCqCreate(&context2, cq_request, &cq_response);
    if (status.ok()) {
        RdmaCqResponse rsp = cq_response.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            cout << __FILE__ << ":" << __FUNCTION__ << "[Error]: Status = "  << rsp.api_status() << endl;
            return -1;
        } else {
            cout << __FILE__ << ":" << __FUNCTION__ <<  " SUCCESS\n" << endl;
        }
    } else {
        cout << __FILE__ << ":" << __FUNCTION__ << "[Error]: Status = "  << status.error_code() << ": " << status.error_message() << endl;
        return -1;
    }

    return 0;
}

int HalClient::CreateQP(uint32_t lif_id, uint32_t qp_num, uint16_t sq_wqe_size,
                        uint16_t rq_wqe_size, uint16_t num_sq_wqes,
                        uint16_t num_rq_wqes, uint16_t num_rsq_wqes,
                        uint16_t num_rrq_wqes, uint8_t pd_num,
                        uint32_t sq_cq_num, uint32_t rq_cq_num, uint32_t page_size,
                        uint32_t pmtu,
                        int service,
                        uint32_t sq_pt_size,
                        uint32_t pt_size, uint64_t *pt_table)
{
    ClientContext context;
    RdmaQpRequestMsg request;
    RdmaQpResponseMsg response;

    RdmaQpSpec *spec = request.add_request();

    spec->set_qp_num(qp_num);
    spec->set_hw_lif_id(lif_id);
    spec->set_sq_wqe_size(sq_wqe_size);
    spec->set_rq_wqe_size(rq_wqe_size);    
    spec->set_num_sq_wqes(num_sq_wqes);
    spec->set_num_rq_wqes(num_rq_wqes);
    spec->set_num_rsq_wqes(num_rsq_wqes);
    spec->set_num_rrq_wqes(num_rrq_wqes);
    spec->set_pd(pd_num);
    spec->set_pmtu(pmtu);
    spec->set_hostmem_pg_size(page_size);
    spec->set_svc((RdmaServiceType)service);
    spec->set_atomic_enabled(0);
    spec->set_sq_cq_num(sq_cq_num);
    spec->set_rq_cq_num(rq_cq_num);
    spec->set_num_sq_pages(sq_pt_size);

    for (uint32_t i = 0; i < pt_size; i++) {
        spec->add_va_pages_phy_addr(pt_table[i] | (1ULL << 63) | ((uint64_t)lif_id << 52));
    }

    Status status = rdma_stub_->RdmaQpCreate(&context, request, &response);
    if (status.ok()) {
        RdmaQpResponse rsp = response.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            cout << __FILE__ << ":" << __FUNCTION__ << "[Error]: Status = "  << rsp.api_status() << endl;
            return -1;
        } else {
            cout << __FILE__ << ":" << __FUNCTION__ <<  " SUCCESS\n" << endl;
        }
    } else {
        cout << __FILE__ << ":" << __FUNCTION__ << "[Error]: Status = "  << status.error_code() << ": " << status.error_message() << endl;
        return -1;
    }

    return 0;
    
}

int HalClient::ModifyQP(uint32_t lif_id, uint32_t qp_num, uint32_t attr_mask,
                        uint32_t dest_qp_num, uint32_t q_key, uint32_t e_psn,
                        uint32_t sq_psn, uint32_t header_template_size,
                        unsigned char *header)
{
    if (attr_mask & IB_QP_AV) {
        ClientContext context;
        RdmaQpUpdateRequestMsg request;
        RdmaQpUpdateResponseMsg response;

        RdmaQpUpdateSpec *spec = request.add_request();

        spec->set_qp_num(qp_num);
        spec->set_hw_lif_id(lif_id);
        spec->set_oper(RDMA_UPDATE_QP_OPER_SET_HEADER_TEMPLATE);
        
        spec->set_header_template(header, header_template_size);

        cout << __FILE__ << ":" << __FUNCTION__ << " set AV:" << endl;
        Status status = rdma_stub_->RdmaQpUpdate(&context, request, &response);
        if (status.ok()) {
            RdmaQpUpdateResponse rsp = response.response(0);
            if (rsp.api_status() != types::API_STATUS_OK) {
                cout << __FILE__ << ":" << __FUNCTION__ << "[Error]: Status = "  << rsp.api_status() << endl;
                return -1;
            } else {
                cout << __FILE__ << ":" << __FUNCTION__ <<  " SUCCESS\n" << endl;
            }
        } else {
            cout << __FILE__ << ":" << __FUNCTION__ << "[Error]: Status = "  << status.error_code() << ": " << status.error_message() << endl;
            return -1;
        }
    }

    if (attr_mask & IB_QP_DEST_QPN) {
        ClientContext context;
        RdmaQpUpdateRequestMsg request;
        RdmaQpUpdateResponseMsg response;

        RdmaQpUpdateSpec *spec = request.add_request();

        spec->set_qp_num(qp_num);
        spec->set_hw_lif_id(lif_id);
        spec->set_oper(RDMA_UPDATE_QP_OPER_SET_DEST_QP);
        spec->set_dst_qp_num(dest_qp_num);
        Status status = rdma_stub_->RdmaQpUpdate(&context, request, &response);
        if (status.ok()) {
            RdmaQpUpdateResponse rsp = response.response(0);
            if (rsp.api_status() != types::API_STATUS_OK) {
                cout << __FILE__ << ":" << __FUNCTION__ << "[Error]: Status = "  << rsp.api_status() << endl;
                return -1;
            } else {
                cout << __FILE__ << ":" << __FUNCTION__ <<  " SUCCESS\n" << endl;
            }
        } else {
            cout << __FILE__ << ":" << __FUNCTION__ << "[Error]: Status = "  << status.error_code() << ": " << status.error_message() << endl;
            return -1;
        }
    }

    if (attr_mask & IB_QP_RQ_PSN) {
        ClientContext context;
        RdmaQpUpdateRequestMsg request;
        RdmaQpUpdateResponseMsg response;

        RdmaQpUpdateSpec *spec = request.add_request();

        spec->set_qp_num(qp_num);
        spec->set_hw_lif_id(lif_id);
        spec->set_oper(RDMA_UPDATE_QP_OPER_SET_E_PSN);
        spec->set_e_psn(e_psn);
        Status status = rdma_stub_->RdmaQpUpdate(&context, request, &response);
        if (status.ok()) {
            RdmaQpUpdateResponse rsp = response.response(0);
            if (rsp.api_status() != types::API_STATUS_OK) {
                cout << __FILE__ << ":" << __FUNCTION__ << "[Error]: Status = "  << rsp.api_status() << endl;
                return -1;
            } else {
                cout << __FILE__ << ":" << __FUNCTION__ <<  " SUCCESS\n" << endl;
            }
        } else {
            cout << __FILE__ << ":" << __FUNCTION__ << "[Error]: Status = "  << status.error_code() << ": " << status.error_message() << endl;
            return -1;
        }
    }

    if (attr_mask & IB_QP_SQ_PSN) {
        ClientContext context;
        RdmaQpUpdateRequestMsg request;
        RdmaQpUpdateResponseMsg response;

        RdmaQpUpdateSpec *spec = request.add_request();

        spec->set_qp_num(qp_num);
        spec->set_hw_lif_id(lif_id);
        spec->set_oper(RDMA_UPDATE_QP_OPER_SET_TX_PSN);
        spec->set_tx_psn(sq_psn);
        Status status = rdma_stub_->RdmaQpUpdate(&context, request, &response);
        if (status.ok()) {
            RdmaQpUpdateResponse rsp = response.response(0);
            if (rsp.api_status() != types::API_STATUS_OK) {
                cout << __FILE__ << ":" << __FUNCTION__ << "[Error]: Status = "  << rsp.api_status() << endl;
                return -1;
            } else {
                cout << __FILE__ << ":" << __FUNCTION__ <<  " SUCCESS\n" << endl;
            }
        } else {
            cout << __FILE__ << ":" << __FUNCTION__ << "[Error]: Status = "  << status.error_code() << ": " << status.error_message() << endl;
            return -1;
        }
    }
    
    return 0;
}


int HalClient::PgmBaseAddrGet(const char *prog_name, uint64_t *base_addr)
{
    ClientContext               context;
    GetProgramAddressRequestMsg req_msg;
    ProgramAddressResponseMsg   resp_msg;

    auto req = req_msg.add_request();
    req->set_handle("p4plus");
    req->set_prog_name(prog_name);
    req->set_resolve_label(false);

    auto status = internal_stub_->GetProgramAddress(&context, req_msg, &resp_msg);
    if (!status.ok()) {
        cout << "[ERROR] " << __FUNCTION__
             << ": prog_name = " << prog_name
             << ", Status = " << status.error_code() << ":" << status.error_message()
             << endl;
        return -1;
    }

    *base_addr = resp_msg.response(0).addr();
    return 0;
}


int HalClient::AllocHbmAddress(const char *handle, uint64_t *addr, uint32_t *size)
{
    ClientContext context;
    AllocHbmAddressRequestMsg req_msg;
    AllocHbmAddressResponseMsg resp_msg;

    auto req = req_msg.add_request();
    req->set_handle(handle);

    auto status = internal_stub_->AllocHbmAddress(&context, req_msg, &resp_msg);
    if (!status.ok()) {
        cout << "[ERROR] " << __FUNCTION__
             << ": handle = " << handle
             << ", Status = " << status.error_code() << ":" << status.error_message()
             << endl;
        return -1;
    }

    *addr = resp_msg.response(0).addr();
    *size = resp_msg.response(0).size();
    return 0;
}

