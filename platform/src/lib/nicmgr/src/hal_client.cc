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
#include <sstream>
#include <grpc++/grpc++.h>

#include "types.grpc.pb.h"
#include "vrf.grpc.pb.h"
#include "interface.grpc.pb.h"
#include "endpoint.grpc.pb.h"
#include "l2segment.grpc.pb.h"
#include "multicast.grpc.pb.h"
#include "rdma.grpc.pb.h"
#include "accel_rgroup.grpc.pb.h"

#include "logger.hpp"
#include "hal_client.hpp"

using namespace kh;
using namespace types;
using namespace intf;
using namespace endpoint;
using namespace l2segment;
using namespace multicast;
using namespace accelRGroup;
using namespace cryptokey;

using namespace grpc;
using namespace std;

HalClient::HalClient(enum ForwardingMode fwd_mode)
{
    string url = std::string("localhost:50054");
    if (getenv("HAL_GRPC_PORT")) {
        url = string("localhost:") + getenv("HAL_GRPC_PORT");
    }

    NIC_LOG_INFO("Connecting to HAL @ {}", url);
    channel = CreateChannel(url, InsecureChannelCredentials());

    NIC_LOG_INFO("Waiting for HAL to be ready ...");
    auto state = channel->GetState(true);
    while (state != GRPC_CHANNEL_READY) {
        // Wait for State change or deadline
        channel->WaitForStateChange(state, gpr_time_from_seconds(1, GPR_TIMESPAN));
        state = channel->GetState(true);
    }

    vrf_stub_ = Vrf::NewStub(channel);
    intf_stub_ = Interface::NewStub(channel);
    internal_stub_ = Internal::NewStub(channel);
    ep_stub_ = Endpoint::NewStub(channel);
    l2seg_stub_ = L2Segment::NewStub(channel);
    multicast_stub_ = Multicast::NewStub(channel);
    rdma_stub_ = Rdma::NewStub(channel);
    accel_rgroup_stub_ = AccelRGroup::NewStub(channel);
    crypto_stub_ = CryptoKey::NewStub(channel);
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
    int err = 0;

    req = req_msg.add_request();
    status = vrf_stub_->VrfGet(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        for (int i = 0; i < rsp_msg.response().size(); i++) {
            rsp = rsp_msg.response(i);
            if (rsp.api_status() == types::API_STATUS_OK) {
                NIC_FUNC_INFO("Discovered, id {} handle {}",
                    rsp.spec().key_or_handle().vrf_id(),
                    rsp.status().vrf_handle());
                vrf_id2handle[rsp.spec().key_or_handle().vrf_id()] = rsp.status().vrf_handle();
            } else {
                NIC_FUNC_ERR("API status {}", rsp.api_status());
                err = -1;
            }
        }
        return err;
    } else {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(), status.error_message());
    }

    return -1;
}

uint64_t
HalClient::VrfGet(uint64_t vrf_id)
{

    VrfGetRequest       *req __attribute__((unused));
    VrfGetResponse      rsp;
    VrfGetRequestMsg    req_msg;
    VrfGetResponseMsg   rsp_msg;
    ClientContext       context;
    Status              status;

    uint64_t            vrf_handle = 0;

    req = req_msg.add_request();
    req->mutable_key_or_handle()->set_vrf_id(vrf_id);

    status = vrf_stub_->VrfGet(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        for (int i = 0; i < rsp_msg.response().size(); i++) {
            rsp = rsp_msg.response(i);
            if (rsp.api_status() == types::API_STATUS_OK) {
                vrf_handle = rsp.status().vrf_handle();
                assert(rsp.spec().key_or_handle().vrf_id() == vrf_id);
                NIC_FUNC_INFO("API status {}, id {} handle {}",
                    rsp.api_status(), vrf_id, vrf_handle);
                vrf_id2handle[vrf_id] = vrf_handle;
                return vrf_handle;
            } else {
                NIC_FUNC_ERR("API status {}", rsp.api_status());
            }
        }
    } else {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(), status.error_message());
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

    // if vrf exists then don't try to create it
    if (vrf_id2handle.find(vrf_id) != vrf_id2handle.end()) {
        return vrf_id2handle[vrf_id];
    }

    spec = req_msg.add_request();
    spec->mutable_key_or_handle()->set_vrf_id(vrf_id);
    spec->set_vrf_type(::types::VRF_TYPE_CUSTOMER);

    status = vrf_stub_->VrfCreate(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            NIC_FUNC_INFO("API status {}, id {} handle {}", vrf_id,
                rsp.api_status(), rsp.vrf_status().vrf_handle());
            vrf_id2handle[vrf_id] = rsp.vrf_status().vrf_handle();
            return rsp.vrf_status().vrf_handle();
        } else {
            NIC_FUNC_ERR("API status {}, id {}", rsp.api_status(), vrf_id);
        }
    } else {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(), status.error_message());
    }

    return 0;
}

/**
 * Uplink APIs
 */
uint64_t
HalClient::UplinkCreate(uint64_t uplink_id, uint64_t port_num, uint64_t native_l2seg_id)
{
    InterfaceSpec           *spec;
    InterfaceResponse       rsp;
    InterfaceRequestMsg     req_msg;
    InterfaceResponseMsg    rsp_msg;
    ClientContext           context;
    Status                  status;

    if (uplink2id.find(port_num) != uplink2id.end()) {
        return uplink2id[port_num];
    }

    spec = req_msg.add_request();
    spec->mutable_key_or_handle()->set_interface_id(uplink_id);
    spec->set_type(::intf::IfType::IF_TYPE_UPLINK);
    spec->set_admin_status(::intf::IfStatus::IF_STATUS_UP);
    spec->mutable_if_uplink_info()->set_port_num(port_num);
    if (native_l2seg_id) {
        spec->mutable_if_uplink_info()->set_native_l2segment_id(native_l2seg_id);
    }

    status = intf_stub_->InterfaceCreate(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            NIC_FUNC_INFO("API status {}, id {} handle {}",
                rsp.api_status(), uplink_id, rsp.status().if_handle());
            uplink2id[port_num] = rsp.status().if_handle();
            return rsp.status().if_handle();
        } else {
            NIC_FUNC_ERR("API status {}", rsp.api_status());
        }
    } else {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(), status.error_message());
    }

    return 0;
}

int
HalClient::UplinkDelete(uint64_t uplink_id)
{
    if (fwd_mode == FWD_MODE_SMART_NIC) {
        return 0;
    }

    return InterfaceDelete(uplink_id);
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
    uint32_t                    vlan_id;
    int err = 0;

    req = req_msg.add_request();
    status = l2seg_stub_->L2SegmentGet(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        for (int i = 0; i < rsp_msg.response().size(); i++) {
            rsp = rsp_msg.response(i);
            if (rsp.api_status() == types::API_STATUS_OK) {
                if (rsp.spec().wire_encap().encap_type() == ENCAP_TYPE_DOT1Q) {
                    l2seg_handle = rsp.status().l2segment_handle();
                    l2seg_id = rsp.spec().key_or_handle().segment_id();
                    vlan_id = rsp.spec().wire_encap().encap_value();
                    NIC_FUNC_INFO("Discovered, id {} handle {} vlan {}",
                         l2seg_id, l2seg_handle, vlan_id);
                    l2seg_id2handle[l2seg_id] = l2seg_handle;
                    vlan2seg[vlan_id] = l2seg_id;
                    seg2vlan[l2seg_id] = vlan_id;
                }
            } else {
                NIC_FUNC_ERR("API status {}", rsp.api_status());
                err = -1;
            }
        }
        return err;
    } else {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(), status.error_message());
    }

    return -1;
}

uint64_t
HalClient::L2SegmentGet(uint64_t l2seg_id)
{
    L2SegmentGetRequest         *req __attribute__((unused));
    L2SegmentGetResponse        rsp;
    L2SegmentGetRequestMsg      req_msg;
    L2SegmentGetResponseMsg     rsp_msg;
    ClientContext               context;
    Status                      status;
    uint64_t                    l2seg_handle;
    uint32_t                    vlan_id;

    req = req_msg.add_request();
    status = l2seg_stub_->L2SegmentGet(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        for (int i = 0; i < rsp_msg.response().size(); i++) {
            rsp = rsp_msg.response(i);
            if (rsp.api_status() == types::API_STATUS_OK) {
                if (rsp.spec().wire_encap().encap_type() == ENCAP_TYPE_DOT1Q) {
                    l2seg_handle = rsp.status().l2segment_handle();
                    l2seg_id = rsp.spec().key_or_handle().segment_id();
                    vlan_id = rsp.spec().wire_encap().encap_value();
                    NIC_FUNC_INFO("Discovered, id {} handle {} vlan {}",
                        l2seg_id, l2seg_handle, vlan_id);
                    l2seg_id2handle[l2seg_id] = l2seg_handle;
                    vlan2seg[vlan_id] = l2seg_id;
                    seg2vlan[l2seg_id] = vlan_id;
                    return l2seg_handle;
                }
            } else {
                NIC_FUNC_ERR("API status {}", rsp.api_status());
            }
        }
    } else {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(), status.error_message());
    }

    return -1;
}

uint64_t
HalClient::L2SegmentCreate(uint64_t vrf_id, uint64_t l2seg_id, uint16_t vlan_id)
{
    L2SegmentSpec           *spec;
    L2SegmentResponse       rsp;
    L2SegmentRequestMsg     req_msg;
    L2SegmentResponseMsg    rsp_msg;
    ClientContext           context;
    Status                  status;

    uint64_t                l2seg_handle;

    if (vlan2seg.find(vlan_id) != vlan2seg.end()) {
        return l2seg_id2handle[l2seg_id];
    }

    spec = req_msg.add_request();
    spec->mutable_key_or_handle()->set_segment_id(l2seg_id);
    spec->mutable_vrf_key_handle()->set_vrf_id(vrf_id);
    spec->set_mcast_fwd_policy(::l2segment::MULTICAST_FWD_POLICY_FLOOD);
    spec->set_bcast_fwd_policy(::l2segment::BROADCAST_FWD_POLICY_FLOOD);
    spec->mutable_wire_encap()->set_encap_type(::types::ENCAP_TYPE_DOT1Q);
    spec->mutable_wire_encap()->set_encap_value(vlan_id);
    status = l2seg_stub_->L2SegmentCreate(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            l2seg_handle = rsp.l2segment_status().l2segment_handle();
            NIC_FUNC_INFO("API status {}, id {} handle {}",
                rsp.api_status(), l2seg_id, l2seg_handle);
            l2seg_id2handle[l2seg_id] = l2seg_handle;
            vlan2seg[vlan_id] = l2seg_id;
            seg2vlan[l2seg_id] = vlan_id;
            return l2seg_handle;
        } else {
            NIC_FUNC_ERR("API status {}", rsp.api_status());
        }
    } else {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(), status.error_message());
        return -1;
    }

    return 0;
}

int
HalClient::AddL2SegmentOnUplink(uint64_t uplink_id, uint64_t l2seg_id)
{
    InterfaceL2SegmentSpec           *spec __attribute__((unused));
    InterfaceL2SegmentResponse       rsp;
    InterfaceL2SegmentRequestMsg     req_msg;
    InterfaceL2SegmentResponseMsg    rsp_msg;
    ClientContext                    context;
    Status                           status;

    spec = req_msg.add_request();
    spec->mutable_l2segment_key_or_handle()->set_segment_id(l2seg_id);
    spec->mutable_if_key_handle()->set_interface_id(uplink_id);

    status = intf_stub_->AddL2SegmentOnUplink(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            NIC_FUNC_INFO("API status {}, l2seg id {} uplink id {}",
                rsp.api_status(), l2seg_id, uplink_id);
        } else if (rsp.api_status() == types::API_STATUS_EXISTS_ALREADY) {
            NIC_FUNC_WARN("API status {}, l2seg id {} uplink id {}",
                rsp.api_status(), l2seg_id, uplink_id);
        } else {
            NIC_FUNC_ERR("API status {}", rsp.api_status());
            return -1;
        }

        return 0;
    } else {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(), status.error_message());
        return -1;
    }

    return -1;
}

/**
 * ENIC APIs
 */
uint64_t
HalClient::EnicCreate(uint64_t enic_id,
                      uint64_t lif_id,
                      uint64_t native_l2seg_id,
                      vector<uint64_t> &nonnative_l2seg_ids)
{
    InterfaceSpec           *spec;
    InterfaceResponse       rsp;
    InterfaceRequestMsg     req_msg;
    InterfaceResponseMsg    rsp_msg;
    L2SegmentKeyHandle      *l2seg_kh;
    ClientContext           context;
    Status                  status;

    if (fwd_mode == FWD_MODE_SMART_NIC) {
        return -1;
    }

    spec = req_msg.add_request();
    spec->mutable_key_or_handle()->set_interface_id(enic_id);
    spec->set_type(::intf::IfType::IF_TYPE_ENIC);
    spec->set_admin_status(::intf::IfStatus::IF_STATUS_UP);
    spec->mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(lif_id);
    spec->mutable_if_enic_info()->set_enic_type(::intf::IF_ENIC_TYPE_CLASSIC);
    spec->mutable_if_enic_info()->mutable_classic_enic_info()->set_native_l2segment_handle(l2seg_id2handle[native_l2seg_id]);
    for (auto it = nonnative_l2seg_ids.cbegin(); it != nonnative_l2seg_ids.cend(); it++) {
        l2seg_kh = spec->mutable_if_enic_info()->mutable_classic_enic_info()->add_l2segment_key_handle();
        l2seg_kh->set_segment_id(*it);
    }

    status = HalClient::intf_stub_->InterfaceCreate(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            NIC_FUNC_INFO("API status {}, id {} handle {}",
                rsp.api_status(), enic_id, rsp.status().if_handle());
            lif2enic_map[lif_id] = enic_id;
            enic_map[enic_id] = *spec;
            enic_id2handle[enic_id] = rsp.status().if_handle();
            return rsp.status().if_handle();
        } else {
            NIC_FUNC_ERR("API status {}", rsp.api_status());
        }
    } else {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(), status.error_message());
    }

    return 0;
}

int
HalClient::EnicDelete(uint64_t enic_id)
{
    if (fwd_mode == FWD_MODE_SMART_NIC) {
        return 0;
    }

    return InterfaceDelete(enic_id);
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
    int err = 0;

    req_msg.add_request();
    status = HalClient::ep_stub_->EndpointGet(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        for (int i = 0; i < rsp_msg.response().size(); i++) {
            rsp = rsp_msg.response(i);
            if (rsp.api_status() == types::API_STATUS_OK) {
                enic_id = rsp.spec().endpoint_attrs().interface_key_handle().interface_id();
                NIC_FUNC_INFO("Discovered, handle {} enic id {} mac {:#x} l2seg id {}",
                    rsp.status().endpoint_handle(), enic_id,
                    rsp.spec().key_or_handle().endpoint_key().l2_key().mac_address(),
                    rsp.spec().key_or_handle().endpoint_key().l2_key().l2segment_key_handle().segment_id());
                enic2ep_map[enic_id].push_back(rsp.status().endpoint_handle());
            } else {
                NIC_FUNC_ERR("API status {}", rsp.api_status());
                err = -1;
            }
        }
        return err;
    } else {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(), status.error_message());
    }

    return -1;
}

uint64_t
HalClient::EndpointCreate(uint64_t vrf_id, uint64_t l2seg_id,
                          uint64_t enic_id, uint64_t mac_addr)
{
    EndpointSpec              *spec;
    EndpointResponse          rsp;
    EndpointRequestMsg        req_msg;
    EndpointResponseMsg       rsp_msg;
    ClientContext             context;
    Status                    status;

    if (fwd_mode == FWD_MODE_SMART_NIC) {
        return -1;
    }

    spec = req_msg.add_request();
    spec->mutable_vrf_key_handle()->set_vrf_id(vrf_id);
    spec->mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_segment_id(l2seg_id);
    spec->mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(mac_addr);
    spec->mutable_endpoint_attrs()->mutable_interface_key_handle()->set_interface_id(enic_id);

    status = HalClient::ep_stub_->EndpointCreate(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            NIC_FUNC_INFO("succeeded, enic {} mac {:#x} l2seg id {} handle {}",
                mac_addr, l2seg_id, rsp.endpoint_status().endpoint_handle());
            return rsp.endpoint_status().endpoint_handle();
        } else {
            NIC_FUNC_ERR("API status {}", rsp.api_status());
        }
    } else {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(), status.error_message());
    }

    return 0;
}

int
HalClient::EndpointDelete(uint64_t vrf_id, uint64_t l2seg_id,
                          uint64_t enic_id, uint64_t mac_addr)
{
    EndpointDeleteRequest           *req;
    EndpointDeleteResponse          rsp;
    EndpointDeleteRequestMsg        req_msg;
    EndpointDeleteResponseMsg       rsp_msg;
    ClientContext                   context;
    Status                          status;

    if (fwd_mode == FWD_MODE_SMART_NIC) {
        return 0;
    }

    req = req_msg.add_request();
    req->mutable_vrf_key_handle()->set_vrf_id(vrf_id);
    req->mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_segment_id(l2seg_id);
    req->mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(mac_addr);

    status = HalClient::ep_stub_->EndpointDelete(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            NIC_FUNC_INFO("succeeded, enic {} mac {:#x} l2seg id {}",
                enic_id, mac_addr, l2seg_id);
            return 0;
        } else {
            NIC_FUNC_ERR("API status {}", rsp.api_status());
        }
    } else {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(), status.error_message());
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
    int err = 0;

    req = req_msg.add_request();
    status = intf_stub_->InterfaceGet(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        for (int i = 0; i < rsp_msg.response().size(); i++) {
            rsp = rsp_msg.response(i);
            if (rsp.api_status() == types::API_STATUS_OK) {
                if (rsp.spec().type() == IF_TYPE_UPLINK) {
                    NIC_FUNC_INFO("Discovered, id {} handle {} port {}",
                        rsp.spec().key_or_handle().interface_id(),
                        rsp.status().if_handle(),
                        rsp.spec().if_uplink_info().port_num());
                    uplink2id[rsp.spec().if_uplink_info().port_num()] = rsp.status().if_handle();
                }
                if (rsp.spec().type() == IF_TYPE_ENIC) {
                    NIC_FUNC_INFO("Discovered, id {} handle {} lif {}",
                        rsp.spec().key_or_handle().interface_id(),
                        rsp.status().if_handle(),
                        rsp.spec().if_enic_info().lif_key_or_handle().lif_id());
                    lif2enic_map[rsp.spec().if_enic_info().lif_key_or_handle().lif_id()] = rsp.spec().key_or_handle().interface_id();
                    enic_map[rsp.spec().key_or_handle().interface_id()] = rsp.spec();
                    enic_id2handle[rsp.spec().key_or_handle().interface_id()] = rsp.status().if_handle();
                }
            } else {
                NIC_FUNC_ERR("API status {}", rsp.api_status());
                err = -1;
            }
        }
        return err;
    } else {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(), status.error_message());
    }

    return -1;
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
        if (rsp.api_status() == types::API_STATUS_OK) {
            NIC_FUNC_INFO("succeeded, id {}", if_id);
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
            enic_id2handle.erase(if_id);
            return 0;
        } else {
            NIC_FUNC_ERR("API status {}", rsp.api_status());
        }
    } else {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(), status.error_message());
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
    int err = 0;

    req = req_msg.add_request();
    status = intf_stub_->LifGet(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        for (int i = 0; i < rsp_msg.response().size(); i++) {
            rsp = rsp_msg.response(i);
            if (rsp.api_status() == types::API_STATUS_OK) {
                NIC_FUNC_INFO("Discovered, id {}", rsp.spec().key_or_handle().lif_id());
                lif_map[rsp.spec().key_or_handle().lif_id()] = LifSpec(rsp.spec());
            } else {
                NIC_FUNC_ERR("API status {}", rsp.api_status());
                err = -1;
            }
        }
        return err;
    } else {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(), status.error_message());
    }

    return -1;
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
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            NIC_FUNC_INFO("API status {}, lif {} hw_lif_id {}",
                rsp.api_status(), rsp.spec().key_or_handle().lif_id(),
                rsp.status().hw_lif_id());

            lif_info->hw_lif_id = rsp.status().hw_lif_id();
            for (int i = 0; i < rsp.qstate().size(); i++) {
                auto & qstate = rsp.qstate()[i];
                NIC_FUNC_INFO("lif {} hw_lif_id {} qtype {} qstate_base {:#x}",
                    rsp.spec().key_or_handle().lif_id(), rsp.status().hw_lif_id(),
                    qstate.type_num(), qstate.addr());
                lif_info->qstate_addr[qstate.type_num()] = qstate.addr();
            }
            lif_map[rsp.spec().key_or_handle().lif_id()] = LifSpec(rsp.spec());

            return rsp.spec().key_or_handle().lif_id();
        } else {
            NIC_FUNC_ERR("API status {}, lif {}", rsp.api_status(), lif_id);
        }
    } else {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(), status.error_message());
    }

    return 0;
}

uint64_t
HalClient::LifCreate(uint64_t lif_id,
                     struct queue_info *queue_info,
                     struct lif_info *lif_info,
                     uint64_t uplink_id,
                     bool enable_rdma,
                     uint32_t max_pt_entries,
                     uint32_t max_keys,
                     uint32_t max_ahs)
{
    LifSpec              *spec;
    LifResponse          rsp;
    LifRequestMsg        req_msg;
    LifResponseMsg       rsp_msg;
    LifQStateMapEntry    *lif_qstate_map_ent;
    QStateSetReq         *qstate_req;
    ClientContext        context;
    Status               status;

    // Get and return if LIF already exists
    if (lif_map.find(lif_id) != lif_map.end()) {
        return LifGet(lif_id, lif_info);
    }

    spec = req_msg.add_request();
    spec->mutable_key_or_handle()->set_lif_id(lif_id);
    if (uplink_id)
        spec->mutable_pinned_uplink_if_key_handle()->set_interface_id(uplink_id);
    spec->set_admin_status(::intf::IF_STATUS_UP);
    spec->set_enable_rdma(enable_rdma);
    spec->set_rdma_max_pt_entries(max_pt_entries);
    spec->set_rdma_max_keys(max_keys);
    spec->set_rdma_max_ahs(max_ahs);

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
        if (rsp.api_status() == types::API_STATUS_OK) {
            NIC_FUNC_INFO("API status {}, lif {} hw_lif_id {}",
                rsp.api_status(), lif_id, rsp.status().hw_lif_id());

            lif_info->hw_lif_id = rsp.status().hw_lif_id();
            for (int i = 0; i < rsp.qstate().size(); i++) {
                auto & qstate = rsp.qstate()[i];
                NIC_FUNC_INFO("lif {} hw_lif_id {} qtype {} qstate_base {:#x}",
                    lif_id, rsp.status().hw_lif_id(), qstate.type_num(), qstate.addr());
                lif_info->qstate_addr[qstate.type_num()] = qstate.addr();
            }

            lif_map[lif_id] = LifSpec(*spec);
            return rsp.status().lif_handle();
        } else if (rsp.api_status() == types::API_STATUS_EXISTS_ALREADY) {
            NIC_FUNC_INFO("API status {}, lif {} hw_lif_id {}",
                rsp.api_status(), lif_id, rsp.status().hw_lif_id());
            return LifGet(lif_id, lif_info);
        } else {
            NIC_FUNC_ERR("API status {}", rsp.api_status());
        }
    } else {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(), status.error_message());
    }

    return 0;
}

int
HalClient::LifDelete(uint64_t lif_id)
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
        if (rsp.api_status() == types::API_STATUS_OK) {
            NIC_FUNC_INFO("API status {}, lif {}", rsp.api_status(), lif_id);
            lif_map.erase(lif_id);
            return 0;
        } else {
            NIC_FUNC_ERR("API status {}", rsp.api_status());
        }
    } else {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(), status.error_message());
    }

    return -1;
}

int
HalClient::LifSetVlanOffload(uint64_t lif_id, bool strip, bool insert)
{
    ClientContext context;
    Status status;
    LifSpec *lif_spec;
    LifResponse rsp;
    LifRequestMsg req_msg;
    LifResponseMsg rsp_msg;

    if (lif_map.find(lif_id) == lif_map.end()) {
        NIC_FUNC_ERR("Invalid lif id {}", lif_id);
        return (-1);
    }

    lif_spec = req_msg.add_request();
    lif_spec->CopyFrom(lif_map[lif_id]);
    lif_spec->mutable_key_or_handle()->set_lif_id(lif_id);
    lif_spec->set_vlan_strip_en(strip);
    lif_spec->set_vlan_insert_en(insert);

    status = intf_stub_->LifUpdate(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            NIC_FUNC_INFO("API status {}, lif {}", rsp.api_status(), lif_id);
            lif_map[lif_id] = *lif_spec;
            return 0;
        } else {
            NIC_FUNC_ERR("API status {}", rsp.api_status());
        }
    } else {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(), status.error_message());
    }

    return -1;
}

int
HalClient::LifSetFilterMode(uint64_t lif_id, bool bcast, bool all_mc, bool promisc)
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
        NIC_FUNC_ERR("Invalid lif id {}", lif_id);
        return -1;
    }

    lif_spec = req_msg.add_request();
    lif_spec->CopyFrom(lif_map[lif_id]);
    lif_spec->mutable_key_or_handle()->set_lif_id(lif_id);
    lif_spec->mutable_packet_filter()->set_receive_broadcast(bcast);
    lif_spec->mutable_packet_filter()->set_receive_all_multicast(all_mc);
    lif_spec->mutable_packet_filter()->set_receive_promiscuous(promisc);

    status = intf_stub_->LifUpdate(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            NIC_FUNC_ERR("API status {}", rsp.api_status());
        } else {
            NIC_FUNC_INFO("API status {}, lif {}", rsp.api_status(), lif_id);
            lif_map[lif_id] = *lif_spec;
            return 0;
        }
    } else {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(), status.error_message());
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
        NIC_FUNC_ERR("Invalid lif id {}", lif_id);
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
        if (rsp.api_status() == types::API_STATUS_OK) {
            NIC_FUNC_INFO("API status {}, id {}", rsp.api_status(), lif_id);
            lif_map[lif_id] = *lif_spec;
            return 0;
        } else {
            NIC_FUNC_ERR("API status {}", rsp.api_status());
        }
    } else {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(), status.error_message());
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
    stringbuf buf;
    ostream os(&buf);
    int err = 0;

    req = req_msg.add_request();
    status = multicast_stub_->MulticastEntryGet(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        for (int i = 0; i < rsp_msg.response().size(); i++) {
            rsp = rsp_msg.response(i);
            if (rsp.api_status() == types::API_STATUS_OK) {
                group = rsp.spec().key_or_handle().key().mac().group();
                l2seg_id = rsp.spec().key_or_handle().key().l2segment_key_handle().segment_id();

                for (auto it = rsp.spec().oif_key_handles().cbegin();
                        it != rsp.spec().oif_key_handles().cend(); it++) {
                    os << it->interface_id() << " ";
                    tuple<uint64_t, uint64_t, uint64_t> key(vrf_id, l2seg_id, group);
                    mcast_groups[key].push_back(it->interface_id());
                }

                NIC_FUNC_INFO("Discovered, vrf_id {} l2seg_id {} group {:#x} oif_ids {}",
                    vrf_id, l2seg_id, group, buf.str());
            } else {
                NIC_FUNC_ERR("API status {}", rsp.api_status());
                err = -1;
            }
        }
        return err;
    } else {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(), status.error_message());
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
    stringbuf buf;
    ostream os(&buf);

    req = req_msg.add_request();
    req->mutable_key_or_handle()->mutable_key()->mutable_l2segment_key_handle()->set_segment_id(l2seg_id);
    req->mutable_key_or_handle()->mutable_key()->mutable_mac()->set_group(group);

    status = multicast_stub_->MulticastEntryGet(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        for (int i = 0; i < rsp_msg.response().size(); i++) {
            rsp = rsp_msg.response(i);
            if (rsp.api_status() == types::API_STATUS_OK) {
                for (auto it = rsp.spec().oif_key_handles().cbegin();
                        it != rsp.spec().oif_key_handles().cend(); it++) {
                    os << it->interface_id() << " ";
                    tuple<uint64_t, uint64_t, uint64_t> key(vrf_id, l2seg_id, group);
                    mcast_groups[key].push_back(it->interface_id());
                }

                NIC_FUNC_INFO("Discovered, vrf_id {} l2seg_id {} group {:#x} oif_ids {}",
                    vrf_id, l2seg_id, group, buf.str());
                return 0;
            } else {
                NIC_FUNC_ERR("API status {}, vrf_id {} l2seg_id {} group {:#x}",
                    rsp.api_status(), vrf_id, l2seg_id, group);
            }
        }
    } else {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(), status.error_message());
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
        NIC_FUNC_INFO("Already exists, vrf_id {} l2seg_id {} group {:#x}",
            vrf_id, l2seg_id, group);
        return 0;
    }

    // Create the group
    spec = req_msg.add_request();
    spec->mutable_key_or_handle()->mutable_key()->mutable_l2segment_key_handle()->set_segment_id(l2seg_id);
    spec->mutable_key_or_handle()->mutable_key()->mutable_mac()->set_group(group);
    status = multicast_stub_->MulticastEntryCreate(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            NIC_FUNC_INFO("API status {}, vrf_id {} l2seg_id {} group {:#x}",
                rsp.api_status(), vrf_id, l2seg_id, group);
            mcast_groups[key] = vector<uint64_t>();
            return 0;
        } else {
            NIC_FUNC_ERR("API status {}, vrf_id {} l2seg_id {} group {:#x}",
                rsp.api_status(), vrf_id, l2seg_id, group);
        }
    } else {
        NIC_FUNC_ERR("GRPC status {} {}, vrf_id {} l2seg_id {} group {:#x}",
            status.error_code(), status.error_message(), vrf_id, l2seg_id, group);
    }

    return -1;
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

    // Group does not exist!
    if (mcast_groups.find(key) == mcast_groups.end()) {
        NIC_FUNC_INFO("does not exist, vrf_id {} l2seg_id {} group {:#x}",
            vrf_id, l2seg_id, group);
        return 0;
    }

    req = req_msg.add_request();
    req->mutable_key_or_handle()->mutable_key()->mutable_l2segment_key_handle()->set_segment_id(l2seg_id);
    req->mutable_key_or_handle()->mutable_key()->mutable_mac()->set_group(group);
    status = multicast_stub_->MulticastEntryDelete(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            NIC_FUNC_INFO("API status {}, vrf_id {} l2seg_id {} group {:#x}",
                rsp.api_status(), vrf_id, l2seg_id, group);
            mcast_groups.erase(key);
            return 0;
        } else {
            NIC_FUNC_ERR("API status {}, vrf_id {} l2seg_id {} group {:#x}",
                rsp.api_status(), vrf_id, l2seg_id, group);
        }
    } else {
        NIC_FUNC_ERR("GRPC status {} {}, vrf_id {} l2seg_id {} group {:#x}",
            status.error_code(), status.error_message(), vrf_id, l2seg_id, group);
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
    stringbuf buf;
    ostream os(&buf);

    if (fwd_mode == FWD_MODE_SMART_NIC) {
        return 0;
    }

    spec = req_msg.add_request();
    spec->mutable_key_or_handle()->mutable_key()->mutable_l2segment_key_handle()->set_segment_id(l2seg_id);
    spec->mutable_key_or_handle()->mutable_key()->mutable_mac()->set_group(group);
    for (auto it = oifs_list.cbegin(); it != oifs_list.cend(); it++) {
        spec->add_oif_key_handles()->set_interface_id(*it);
        os << *it << " ";
    }

    status = multicast_stub_->MulticastEntryUpdate(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            NIC_FUNC_INFO("API status {}, vrf_id {} l2seg_id {} group {:#x} oif_ids {}",
                rsp.api_status(), vrf_id, l2seg_id, group, buf.str());
            return 0;
        } else {
            NIC_FUNC_ERR("API status {}, vrf_id {} l2seg_id {} group {:#x} oif_ids {}",
                rsp.api_status(), vrf_id, l2seg_id, group, buf.str());
        }
    } else {
        NIC_FUNC_ERR("GRPC status {} {}, vrf_id {} l2seg_id {} group {:#x} oif_ids {}",
            status.error_code(), status.error_message(), vrf_id, l2seg_id, group, buf.str());
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

    // If we know about this group then add the enic_id to the oif_list
    // and update the MulticastGroup
    if (mcast_groups.find(key) != mcast_groups.end()) {
        auto it = find(mcast_groups[key].cbegin(), mcast_groups[key].cend(), enic_id);
        if (it != mcast_groups[key].cend()) {
            // enic is in the group's oif list. nop!
            NIC_FUNC_WARN("Already exists, vrf_id {} l2seg_id {} group {:#x} enic_id {}",
                vrf_id, l2seg_id, group, enic_id);
            return 0;
        } else {
            NIC_FUNC_INFO("vrf_id {} l2seg_id {} group {:#x} enic_id {}",
                vrf_id, l2seg_id, group, enic_id);
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
                NIC_FUNC_WARN("Already exists, vrf_id {} l2seg_id {} group {:#x} enic_id {}",
                    vrf_id, l2seg_id, group, enic_id);
                return 0;
            } else {
                NIC_FUNC_INFO("vrf_id {} l2seg_id {} group {:#x} enic_id {}",
                    vrf_id, l2seg_id, group, enic_id);
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

    // If we know about this group then remove the enic_id from the oif_list
    // and update the MulticastGroup
    if (mcast_groups.find(key) != mcast_groups.end()) {
        auto it = find(mcast_groups[key].cbegin(), mcast_groups[key].cend(), enic_id);
        if (it != mcast_groups[key].cend()) {
            // enic is in the group's oif list. remove it!
            NIC_FUNC_INFO("vrf_id {} l2seg_id {} group {:#x} enic_id {}",
                vrf_id, l2seg_id, group, enic_id);
            mcast_groups[key].erase(it);
        } else {
            // enic is not in the group's oif list. nop!
            NIC_FUNC_WARN("Does not exist, vrf_id {} l2seg_id {} group {:#x} enic_id {}",
                vrf_id, l2seg_id, group, enic_id);
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
                NIC_FUNC_INFO("vrf_id {} l2seg_id {} group {:#x} enic_id {}",
                    vrf_id, l2seg_id, group, enic_id);
                mcast_groups[key].erase(it);
            } else {
                // enic is not in the group's oif list. nop!
                NIC_FUNC_WARN("Does not exist, vrf_id {} l2seg_id {} group {:#x} enic_id {}",
                    vrf_id, l2seg_id, group, enic_id);
                return 0;
            }
        } else {
            return -1;
        }
    }

    // Update the group
    return MulticastGroupUpdate(group, vrf_id, l2seg_id, mcast_groups[key]);
}

/*
 * DEBUG
 */
ostream &operator<<(ostream& os, const HalClient& obj) {

    os << "L2SEGMENTS:" << endl;
    for (auto it = obj.vlan2seg.cbegin(); it != obj.vlan2seg.cend(); it++) {
        os << "\tvlan = " << it->first << ", handle = " << it->second << endl;
    }

    os << "UPLINKS:" << endl;
    for (auto it = obj.uplink2id.cbegin(); it != obj.uplink2id.cend(); it++) {
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

/*
 * RDMA
 */
int HalClient::CreateMR(uint64_t lif_id, uint32_t pd, uint64_t va, uint64_t length,
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
    spec->set_host_addr(1);

    for (int i = 0; i < (int)pt_size; i++) {
        spec->add_va_pages_phy_addr(pt_table[i]);
    }

    Status status = rdma_stub_->RdmaMemReg(&context, request, &response);
    if (status.ok()) {
        RdmaMemRegResponse rsp = response.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            NIC_FUNC_ERR("API status {}", rsp.api_status());
        } else {
            NIC_FUNC_INFO("API status {}", rsp.api_status());
            return 0;
        }
    } else {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(), status.error_message());
    }

    return -1;
}

int HalClient::CreateCQ(uint32_t lif_id,
                        uint32_t cq_num, uint16_t cq_wqe_size, uint16_t num_cq_wqes,
                        uint32_t host_page_size,
                        uint64_t *pt_table, uint32_t pt_size)
{
    ClientContext context;
    RdmaCqRequestMsg request;
    RdmaCqResponseMsg response;

    RdmaCqSpec *spec = request.add_request();

    spec->set_cq_num(cq_num);
    spec->set_hw_lif_id(lif_id);
    spec->set_cq_wqe_size(cq_wqe_size);
    spec->set_num_cq_wqes(num_cq_wqes);
    spec->set_hostmem_pg_size(host_page_size);
    spec->set_host_addr(1);

    //Set the va to pa translations.
    for (int i = 0; i < (int)pt_size; i++) {
        spec->add_cq_va_pages_phy_addr(pt_table[i]);
    }
    
    Status status = rdma_stub_->RdmaCqCreate(&context, request, &response);
    if (status.ok()) {
        auto rsp = response.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            NIC_FUNC_ERR("API status {}", rsp.api_status());
        } else {
            NIC_FUNC_INFO("API status {}", rsp.api_status());
            return 0;
        }
    } else {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(), status.error_message());
    }

    return -1;
}

int HalClient::CreateQP(uint64_t lif_id, uint32_t qp_num, uint16_t sq_wqe_size,
                        uint16_t rq_wqe_size, uint16_t num_sq_wqes,
                        uint16_t num_rq_wqes, uint16_t num_rsq_wqes,
                        uint16_t num_rrq_wqes, uint32_t pd_num,
                        uint32_t sq_cq_num, uint32_t rq_cq_num, uint32_t page_size,
                        uint32_t pmtu,
                        int service, int flags,
                        uint32_t sq_pt_size,
                        uint32_t pt_size, uint64_t *pt_table,
                        uint64_t cmb_bar_base, uint64_t cmb_bar_size)
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

    uint64_t cmb_offset;
    uint32_t i = 0;

    /* send queue in controller memory */
    if (flags & (1<<13)) {
        NIC_FUNC_INFO("qp send queue in controller memory bar");

        cmb_offset = pt_table[i++];

        if (cmb_offset + sq_wqe_size * num_sq_wqes > cmb_bar_size) {
            NIC_FUNC_ERR("sqcmb out of range");
            return -1;
        }

        spec->set_num_sq_pages(0);
        spec->set_sq_base_addr(cmb_bar_base + cmb_offset);
        spec->set_sq_in_nic_memory(true);
    }

    /* receive queue in controller memory, will be the last "page" */
    if (flags & (1<<14)) {
        assert(pt_size);
        --pt_size;
    }

    /* queues in host memory */
    for (; i < pt_size; i++) {
        spec->add_va_pages_phy_addr(pt_table[i]);
    }

    /* receive queue in controller memory */
    if (flags & (1<<14)) {
        NIC_FUNC_INFO("qp receive queue in controller memory bar");

        cmb_offset = pt_table[i++];

        if (cmb_offset + rq_wqe_size * num_rq_wqes > cmb_bar_size) {
            NIC_FUNC_ERR("rqcmb out of range");
            return -1;
        }

        spec->set_rq_base_addr(cmb_bar_base + cmb_offset);
        spec->set_rq_in_nic_memory(true);
    }

    Status status = rdma_stub_->RdmaQpCreate(&context, request, &response);
    if (status.ok()) {
        RdmaQpResponse rsp = response.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            NIC_FUNC_ERR("API status {}", rsp.api_status());
        } else {
            NIC_FUNC_INFO("API status {}", rsp.api_status());
            return 0;
        }
    } else {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(), status.error_message());
    }

    return -1;
}

int HalClient::CreateAh(uint64_t lif_id, uint32_t ah_id, uint32_t pd_id,
                        uint32_t header_template_size, unsigned char *header)
{
    ClientContext context;
    RdmaAhRequestMsg request;
    RdmaAhResponseMsg response;

    RdmaAhSpec *spec = request.add_request();
    spec->set_ahid(ah_id);
    spec->set_hw_lif_id(lif_id);
    spec->set_header_template(header, header_template_size);

    Status status = rdma_stub_->RdmaAhCreate(&context, request, &response);
    if (status.ok()) {
        RdmaAhResponse rsp = response.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            NIC_FUNC_ERR("API status {}", rsp.api_status());
        } else {
            NIC_FUNC_INFO("API status {}", rsp.api_status());
            return 0;
        }
    } else {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(), status.error_message());
    }

    return -1;
}

int HalClient::ModifyQP(uint64_t lif_id, uint32_t qp_num, uint32_t attr_mask,
                        uint32_t dest_qp_num, uint32_t q_key,
                        uint32_t e_psn, uint32_t sq_psn,
                        uint32_t header_template_ah_id, uint32_t header_template_size,
                        unsigned char *header, uint32_t pmtu, uint8_t qstate)
{
    if (attr_mask & IB_QP_AV) {
        ClientContext context;
        RdmaQpUpdateRequestMsg request;
        RdmaQpUpdateResponseMsg response;

        RdmaQpUpdateSpec *spec = request.add_request();

        spec->set_qp_num(qp_num);
        spec->set_hw_lif_id(lif_id);
        spec->set_oper(RDMA_UPDATE_QP_OPER_SET_AV);

        spec->set_header_template(header, header_template_size);
        spec->set_ahid(header_template_ah_id);

        Status status = rdma_stub_->RdmaQpUpdate(&context, request, &response);
        if (status.ok()) {
            RdmaQpUpdateResponse rsp = response.response(0);
            if (rsp.api_status() != types::API_STATUS_OK) {
                NIC_FUNC_ERR("API status {}", rsp.api_status());
                return -1;
            } else {
                NIC_FUNC_INFO("API status {}", rsp.api_status());
            }
        } else {
            NIC_FUNC_ERR("GRPC status {} {}", status.error_code(), status.error_message());
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
        spec->set_oper(RDMA_UPDATE_QP_OPER_SET_DEST_QPN);
        spec->set_dst_qp_num(dest_qp_num);

        Status status = rdma_stub_->RdmaQpUpdate(&context, request, &response);
        if (status.ok()) {
            RdmaQpUpdateResponse rsp = response.response(0);
            if (rsp.api_status() != types::API_STATUS_OK) {
                NIC_FUNC_ERR("API status {}", rsp.api_status());
                return -1;
            } else {
                NIC_FUNC_INFO("API status {}", rsp.api_status());
            }
        } else {
            NIC_FUNC_ERR("GRPC status {} {}", status.error_code(), status.error_message());
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
        spec->set_oper(RDMA_UPDATE_QP_OPER_SET_RQ_PSN);
        spec->set_e_psn(e_psn);

        Status status = rdma_stub_->RdmaQpUpdate(&context, request, &response);
        if (status.ok()) {
            RdmaQpUpdateResponse rsp = response.response(0);
            if (rsp.api_status() != types::API_STATUS_OK) {
                NIC_FUNC_ERR("API status {}", rsp.api_status());
                return -1;
            } else {
                NIC_FUNC_INFO("API status {}", rsp.api_status());
            }
        } else {
            NIC_FUNC_ERR("GRPC status {} {}", status.error_code(), status.error_message());
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
        spec->set_oper(RDMA_UPDATE_QP_OPER_SET_SQ_PSN);
        spec->set_tx_psn(sq_psn);

        Status status = rdma_stub_->RdmaQpUpdate(&context, request, &response);
        if (status.ok()) {
            RdmaQpUpdateResponse rsp = response.response(0);
            if (rsp.api_status() != types::API_STATUS_OK) {
                NIC_FUNC_ERR("API status {}", rsp.api_status());
                return -1;
            } else {
                NIC_FUNC_INFO("API status {}", rsp.api_status());
            }
        } else {
            NIC_FUNC_ERR("GRPC status {} {}", status.error_code(), status.error_message());
            return -1;
        }
    }

    if (attr_mask & IB_QP_QKEY) {
        ClientContext context;
        RdmaQpUpdateRequestMsg request;
        RdmaQpUpdateResponseMsg response;

        RdmaQpUpdateSpec *spec = request.add_request();

        spec->set_qp_num(qp_num);
        spec->set_hw_lif_id(lif_id);
        spec->set_oper(RDMA_UPDATE_QP_OPER_SET_QKEY);
        spec->set_q_key(q_key);

        Status status = rdma_stub_->RdmaQpUpdate(&context, request, &response);
        if (status.ok()) {
            RdmaQpUpdateResponse rsp = response.response(0);
            if (rsp.api_status() != types::API_STATUS_OK) {
                NIC_FUNC_ERR("API status {}", rsp.api_status());
                return -1;
            } else {
                NIC_FUNC_INFO("API status {}", rsp.api_status());
            }
        } else {
            NIC_FUNC_ERR("GRPC status {} {}", status.error_code(), status.error_message());
            return -1;
        }
    }

    if (attr_mask & IB_QP_PATH_MTU) {
        ClientContext context;
        RdmaQpUpdateRequestMsg request;
        RdmaQpUpdateResponseMsg response;

        RdmaQpUpdateSpec *spec = request.add_request();

        spec->set_qp_num(qp_num);
        spec->set_hw_lif_id(lif_id);
        spec->set_oper(RDMA_UPDATE_QP_OPER_SET_PATH_MTU);
        spec->set_pmtu(pmtu);

        Status status = rdma_stub_->RdmaQpUpdate(&context, request, &response);
        if (status.ok()) {
            RdmaQpUpdateResponse rsp = response.response(0);
            if (rsp.api_status() != types::API_STATUS_OK) {
                NIC_FUNC_ERR("API status {}", rsp.api_status());
                return -1;
            } else {
                NIC_FUNC_INFO("API status {}", rsp.api_status());
            }
        } else {
            NIC_FUNC_ERR("GRPC status {} {}", status.error_code(), status.error_message());
            return -1;
        }
    }

    if (attr_mask & IB_QP_STATE) {
        ClientContext context;
        RdmaQpUpdateRequestMsg request;
        RdmaQpUpdateResponseMsg response;

        RdmaQpUpdateSpec *spec = request.add_request();

        spec->set_qp_num(qp_num);
        spec->set_hw_lif_id(lif_id);
        spec->set_oper(RDMA_UPDATE_QP_OPER_SET_STATE);
        spec->set_qstate(qstate);

        Status status = rdma_stub_->RdmaQpUpdate(&context, request, &response);
        if (status.ok()) {
            RdmaQpUpdateResponse rsp = response.response(0);
            if (rsp.api_status() != types::API_STATUS_OK) {
                NIC_FUNC_ERR("API status {}", rsp.api_status());
                return -1;
            } else {
                NIC_FUNC_INFO("API status {}", rsp.api_status());
            }
        } else {
            NIC_FUNC_ERR("GRPC status {} {}", status.error_code(), status.error_message());
            return -1;
        }
    }

    return 0;
}

int
HalClient::RDMACreateEQ(uint64_t lif_id, uint32_t eq_num,
                        uint32_t num_eq_wqes, uint32_t eq_wqe_size,
                        uint32_t base_addr, uint32_t int_num)
{
    ClientContext context;
    RdmaEqRequestMsg request;
    RdmaEqResponseMsg response;

    RdmaEqSpec *spec = request.add_request();
    spec->set_eq_id(eq_num);
    spec->set_hw_lif_id(lif_id);
    spec->set_num_eq_wqes(num_eq_wqes);
    spec->set_eq_wqe_size(eq_wqe_size);
    spec->set_eqe_base_addr_phy(base_addr);
    spec->set_int_num(int_num);

    Status status = rdma_stub_->RdmaEqCreate(&context, request, &response);
    if (status.ok()) {
        RdmaEqResponse rsp = response.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            NIC_FUNC_ERR("API status {}", rsp.api_status());
        } else {
            NIC_FUNC_INFO("API status {}", rsp.api_status());
            return 0;
        }
    } else {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(), status.error_message());
    }

    return -1;
}

int
HalClient::RDMACreateCQ(uint64_t lif_id,
                        uint32_t cq_num, uint16_t cq_wqe_size, uint16_t num_cq_wqes,
                        uint32_t host_pg_size,
                        uint64_t pa, uint32_t eq_num)
{
    Status status;
    ClientContext context;
    RdmaCqRequestMsg cq_request;
    RdmaCqResponseMsg cq_response;

    RdmaCqSpec *cq_spec = cq_request.add_request();

    cq_spec->set_cq_num(cq_num);
    cq_spec->set_hw_lif_id(lif_id);
    cq_spec->set_cq_wqe_size(cq_wqe_size);
    cq_spec->set_num_cq_wqes(num_cq_wqes);
    cq_spec->set_hostmem_pg_size(host_pg_size);
    cq_spec->set_cq_lkey(0);
    cq_spec->set_eq_id(eq_num);
    cq_spec->add_cq_va_pages_phy_addr(pa);

    status = rdma_stub_->RdmaCqCreate(&context, cq_request, &cq_response);
    if (status.ok()) {
        RdmaCqResponse rsp = cq_response.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            NIC_FUNC_ERR("API status {}", rsp.api_status());
        } else {
            NIC_FUNC_INFO("API status {}", rsp.api_status());
            return 0;
        }
    } else {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(), status.error_message());
    }

    return -1;
}

int
HalClient::RDMACreateAdminQ(uint64_t lif_id, uint32_t aq_num,
                            uint32_t log_num_wqes, uint32_t log_wqe_size,
                            uint64_t pa, uint32_t cq_num)
{
    ClientContext context;
    RdmaAqRequestMsg request;
    RdmaAqResponseMsg response;

    RdmaAqSpec *spec = request.add_request();

    spec->set_aq_num(aq_num);
    spec->set_hw_lif_id(lif_id);
    spec->set_log_num_wqes(log_num_wqes);
    spec->set_log_wqe_size(log_wqe_size);
    spec->set_phy_base_addr(pa);
    spec->set_cq_num(cq_num);

    Status status = rdma_stub_->RdmaAqCreate(&context, request, &response);
    if (status.ok()) {
        auto rsp = response.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            NIC_FUNC_ERR("API status {}", rsp.api_status());
        } else {
            NIC_FUNC_INFO("API status {}", rsp.api_status());
            return 0;
        }
    } else {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(), status.error_message());
    }

    return -1;
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
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(), status.error_message());
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
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(), status.error_message());
        return -1;
    }

    *addr = resp_msg.response(0).addr();
    *size = resp_msg.response(0).size();
    return 0;
}

int
HalClient::FilterAdd(uint64_t lif_id, uint64_t mac, uint32_t vlan)
{
    ClientContext context;
    Status status;
    FilterSpec *req;
    FilterResponse rsp;
    FilterRequestMsg req_msg;
    FilterResponseMsg rsp_msg;
    FilterType type;

    if (fwd_mode == FWD_MODE_CLASSIC_NIC) {
        return -1;  // STUB SUCCESS
    }

    req = req_msg.add_request();
    if (mac != 0 && vlan != 0) {
        type = FILTER_LIF_MAC_VLAN;
    } else if (mac != 0) {
        type = FILTER_LIF_MAC;
    } else if (vlan != 0) {
        type = FILTER_LIF_VLAN;
    } else {
        type = FILTER_NONE;
    }

    req->mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(lif_id);
    req->mutable_key_or_handle()->mutable_filter_key()->set_type(type);
    req->mutable_key_or_handle()->mutable_filter_key()->set_mac_address(mac);
    req->mutable_key_or_handle()->mutable_filter_key()->set_vlan_id(vlan);

    status = HalClient::ep_stub_->FilterCreate(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            NIC_FUNC_INFO("API status {}, lif id {} type {} mac {} vlan {} handle {}",
                rsp.api_status(), lif_id, type, mac, vlan, rsp.filter_status().filter_handle());
            return rsp.filter_status().filter_handle();
        } else if (rsp.api_status() == types::API_STATUS_EXISTS_ALREADY) {
            NIC_FUNC_WARN("API status {}, lif id {} type {} mac {} vlan {} handle {}",
                rsp.api_status(), lif_id, type, mac, vlan, rsp.filter_status().filter_handle());
            return rsp.filter_status().filter_handle();
        } else {
            NIC_FUNC_ERR("API status {}", rsp.api_status());
        }
    } else {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(), status.error_message());
    }

    return 0;
}

int
HalClient::FilterDel(uint64_t lif_id, uint64_t mac, uint32_t vlan)
{
    ClientContext context;
    Status status;
    FilterDeleteRequest *req;
    FilterDeleteResponse rsp;
    FilterDeleteRequestMsg req_msg;
    FilterDeleteResponseMsg rsp_msg;
    FilterType type;

    if (fwd_mode == FWD_MODE_CLASSIC_NIC) {
        return 0;   // STUB SUCCESS
    }

    req = req_msg.add_request();
    if (mac != 0 && vlan != 0) {
        type = FILTER_LIF_MAC_VLAN;
    } else if (mac != 0) {
        type = FILTER_LIF_MAC;
    } else if (vlan != 0) {
        type = FILTER_LIF_VLAN;
    } else {
        type = FILTER_NONE;
    }

    req->mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(lif_id);
    req->mutable_key_or_handle()->mutable_filter_key()->set_type(type);
    req->mutable_key_or_handle()->mutable_filter_key()->set_mac_address(mac);
    req->mutable_key_or_handle()->mutable_filter_key()->set_vlan_id(vlan);

    status = HalClient::ep_stub_->FilterDelete(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            NIC_FUNC_INFO("API status {}, lif id {} type {} mac {} vlan {}",
                rsp.api_status(), lif_id, type, mac, vlan);
            return 0;
        } else {
            // TODO: Handle API_STATUS_NOT_EXISTS instead of returning success here
            NIC_FUNC_ERR("API status {}, lif id {} type {} mac {} vlan {}",
                rsp.api_status(), lif_id, type, mac, vlan);
        }
    } else {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(), status.error_message());
    }

    return -1;
}

int
HalClient::AccelRGroupAdd(const std::string& rgroup_name)
{
    AccelRGroupAddRequestMsg    req_msg;
    AccelRGroupAddResponseMsg   rsp_msg;
    ClientContext               context;
    Status                      status;

    auto req = req_msg.add_request();
    req->set_rgroup_name(rgroup_name);
    status = accel_rgroup_stub_->AccelRGroupAdd(&context, req_msg, &rsp_msg);
    if (!status.ok()) {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(),
                     status.error_message());
        return -1;
    }
    auto rsp = rsp_msg.response(0);
    if (rsp.api_status() != types::API_STATUS_OK) {
        NIC_FUNC_ERR("API status {} rgroup_name {}", rsp.api_status(),
                     rgroup_name);
        return -1;
    }
    return 0;
}

int
HalClient::AccelRGroupDel(const std::string& rgroup_name)
{
    AccelRGroupDelRequestMsg    req_msg;
    AccelRGroupDelResponseMsg   rsp_msg;
    ClientContext               context;
    Status                      status;

    auto req = req_msg.add_request();
    req->set_rgroup_name(rgroup_name);
    status = accel_rgroup_stub_->AccelRGroupDel(&context, req_msg, &rsp_msg);
    if (!status.ok()) {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(),
                     status.error_message());
        return -1;
    }
    auto rsp = rsp_msg.response(0);
    if (rsp.api_status() != types::API_STATUS_OK) {
        NIC_FUNC_ERR("API status {} rgroup_name {}", rsp.api_status(),
                     rgroup_name);
        return -1;
    }

    return 0;
}

int
HalClient::AccelRGroupRingAdd(const std::string& rgroup_name,
           const std::vector<std::pair<const std::string,uint32_t>>& ring_vec)
{
    AccelRGroupRingAddRequestMsg    req_msg;
    AccelRGroupRingAddResponseMsg   rsp_msg;
    ClientContext                   context;
    Status                          status;
    int                             i;

    assert(ring_vec.size());
    for (i = 0; i < (int)ring_vec.size(); i++) {
        auto req = req_msg.add_request();
        req->set_rgroup_name(rgroup_name);
        req->set_ring_name(ring_vec[i].first);
        req->set_ring_handle(ring_vec[i].second);
    }
    status = accel_rgroup_stub_->AccelRGroupRingAdd(&context, req_msg, &rsp_msg);
    if (!status.ok()) {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(),
                     status.error_message());
        return -1;
    }

    assert(rsp_msg.response_size() == (int)ring_vec.size());
    for (i = 0; i < rsp_msg.response_size(); i++) {
        auto rsp = rsp_msg.response(i);
        if (rsp.api_status() != types::API_STATUS_OK) {
            NIC_FUNC_ERR("API status {} rgroup_name {} ring_name {}",
                         rsp.api_status(), rgroup_name, ring_vec[i].first);
            return -1;
        }
    }

    return 0;
}

int
HalClient::AccelRGroupRingDel(const std::string& rgroup_name,
           const std::vector<std::pair<const std::string,uint32_t>>& ring_vec)
{
    AccelRGroupRingDelRequestMsg    req_msg;
    AccelRGroupRingDelResponseMsg   rsp_msg;
    ClientContext                   context;
    Status                          status;
    int                             i;

    assert(ring_vec.size());
    for (i = 0; i < (int)ring_vec.size(); i++) {
        auto req = req_msg.add_request();
        req->set_rgroup_name(rgroup_name);
        req->set_ring_name(ring_vec[i].first);
    }
    status = accel_rgroup_stub_->AccelRGroupRingDel(&context, req_msg, &rsp_msg);
    if (!status.ok()) {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(),
                     status.error_message());
        return -1;
    }

    assert(rsp_msg.response_size() == (int)ring_vec.size());
    for (i = 0; i < rsp_msg.response_size(); i++) {
        auto rsp = rsp_msg.response(i);
        if (rsp.api_status() != types::API_STATUS_OK) {
            NIC_FUNC_ERR("API status {} rgroup_name {} ring_name {}",
                         rsp.api_status(), rgroup_name, ring_vec[i].first);
            return -1;
        }
    }

    return 0;
}

int
HalClient::AccelRGroupResetSet(const std::string& rgroup_name,
                               uint32_t sub_ring,
                               bool reset_sense)
{
    AccelRGroupResetSetRequestMsg   req_msg;
    AccelRGroupResetSetResponseMsg  rsp_msg;
    ClientContext                   context;
    Status                          status;

    auto req = req_msg.add_request();
    req->set_rgroup_name(rgroup_name);
    req->set_sub_ring(sub_ring);
    req->set_reset_sense(reset_sense);
    status = accel_rgroup_stub_->AccelRGroupResetSet(&context, req_msg, &rsp_msg);
    if (!status.ok()) {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(),
                     status.error_message());
        return -1;
    }
    auto rsp = rsp_msg.response(0);
    if (rsp.api_status() != types::API_STATUS_OK) {
        NIC_FUNC_ERR("API status {} rgroup_name {} last_ring_handle {} last_sub_ring {}",
                     rsp.api_status(), rgroup_name, rsp.last_ring_handle(),
                     rsp.last_sub_ring());
        return -1;
    }
    return 0;
}

int
HalClient::AccelRGroupEnableSet(const std::string& rgroup_name,
                               uint32_t sub_ring,
                               bool enable_sense)
{
    AccelRGroupEnableSetRequestMsg  req_msg;
    AccelRGroupEnableSetResponseMsg rsp_msg;
    ClientContext                   context;
    Status                          status;

    auto req = req_msg.add_request();
    req->set_rgroup_name(rgroup_name);
    req->set_sub_ring(sub_ring);
    req->set_enable_sense(enable_sense);
    status = accel_rgroup_stub_->AccelRGroupEnableSet(&context, req_msg, &rsp_msg);
    if (!status.ok()) {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(),
                     status.error_message());
        return -1;
    }
    auto rsp = rsp_msg.response(0);
    if (rsp.api_status() != types::API_STATUS_OK) {
        NIC_FUNC_ERR("API status {} rgroup_name {} last_ring_handle {} last_sub_ring {}",
                     rsp.api_status(), rgroup_name, rsp.last_ring_handle(),
                     rsp.last_sub_ring());
        return -1;
    }
    return 0;
}

int
HalClient::AccelRGroupPndxSet(const std::string& rgroup_name,
                              uint32_t sub_ring,
                              uint32_t val,
                              bool conditional)
{
    AccelRGroupPndxSetRequestMsg    req_msg;
    AccelRGroupPndxSetResponseMsg   rsp_msg;
    ClientContext                   context;
    Status                          status;

    auto req = req_msg.add_request();
    req->set_rgroup_name(rgroup_name);
    req->set_sub_ring(sub_ring);
    req->set_val(val);
    req->set_conditional(conditional);
    status = accel_rgroup_stub_->AccelRGroupPndxSet(&context, req_msg, &rsp_msg);
    if (!status.ok()) {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(),
                     status.error_message());
        return -1;
    }
    auto rsp = rsp_msg.response(0);
    if (rsp.api_status() != types::API_STATUS_OK) {
        NIC_FUNC_ERR("API status {} rgroup_name {} last_ring_handle {} last_sub_ring {}",
                     rsp.api_status(), rgroup_name, rsp.last_ring_handle(),
                     rsp.last_sub_ring());
        return -1;
    }
    return 0;
}

int
HalClient::AccelRGroupInfoGet(const std::string& rgroup_name,
                              uint32_t sub_ring,
                              accel_rgroup_rinfo_rsp_cb_t rsp_cb_func,
                              void *user_ctx,
                              uint32_t& ret_num_entries)
{
    AccelRGroupInfoGetRequestMsg    req_msg;
    AccelRGroupInfoGetResponseMsg   rsp_msg;
    ClientContext                   context;
    Status                          status;
    int                             i;

    ret_num_entries = 0;
    auto req = req_msg.add_request();
    req->set_rgroup_name(rgroup_name);
    req->set_sub_ring(sub_ring);
    status = accel_rgroup_stub_->AccelRGroupInfoGet(&context, req_msg, &rsp_msg);
    if (!status.ok()) {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(),
                     status.error_message());
        return -1;
    }
    auto rsp = rsp_msg.response(0);
    if (rsp.api_status() != types::API_STATUS_OK) {
        NIC_FUNC_ERR("API status {} rgroup_name {}",
                     rsp.api_status(), rgroup_name);
        return -1;
    }

    ret_num_entries = (uint32_t)rsp.ring_info_spec_size();
    for (i = 0; i < (int)ret_num_entries; i++) {
        accel_rgroup_rinfo_rsp_t rinfo = {0};
        auto spec = rsp.ring_info_spec(i);

        rinfo.ring_handle = spec.ring_handle();
        rinfo.sub_ring = spec.sub_ring();
        rinfo.base_pa = spec.base_pa();
        rinfo.pndx_pa = spec.pndx_pa();
        rinfo.shadow_pndx_pa = spec.shadow_pndx_pa();
        rinfo.opaque_tag_pa = spec.opaque_tag_pa();
        rinfo.opaque_tag_size = spec.opaque_tag_size();
        rinfo.ring_size = spec.ring_size();
        rinfo.desc_size = spec.desc_size();
        rinfo.pndx_size = spec.pndx_size();
        rinfo.sw_reset_capable = spec.sw_reset_capable();
        rinfo.sw_enable_capable = spec.sw_enable_capable();
        (*rsp_cb_func)(user_ctx, rinfo);
    }
    return 0;
}

int
HalClient::AccelRGroupIndicesGet(const std::string& rgroup_name,
                                 uint32_t sub_ring,
                                 accel_rgroup_rindices_rsp_cb_t rsp_cb_func,
                                 void *user_ctx,
                                 uint32_t& ret_num_entries)
{
    AccelRGroupIndicesGetRequestMsg     req_msg;
    AccelRGroupIndicesGetResponseMsg    rsp_msg;
    ClientContext                       context;
    Status                              status;
    int                                 i;

    auto req = req_msg.add_request();
    req->set_rgroup_name(rgroup_name);
    req->set_sub_ring(sub_ring);
    status = accel_rgroup_stub_->AccelRGroupIndicesGet(&context, req_msg, &rsp_msg);
    if (!status.ok()) {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(),
                     status.error_message());
        return -1;
    }
    auto rsp = rsp_msg.response(0);
    if (rsp.api_status() != types::API_STATUS_OK) {
        NIC_FUNC_ERR("API status {} rgroup_name {}",
                     rsp.api_status(), rgroup_name);
        return -1;
    }

    ret_num_entries = (uint32_t)rsp.ring_indices_spec_size();
    for (i = 0; i < (int)ret_num_entries; i++) {
        accel_rgroup_rindices_rsp_t rindices = {0};
        auto spec = rsp.ring_indices_spec(i);

        rindices.ring_handle = spec.ring_handle();
        rindices.sub_ring = spec.sub_ring();
        rindices.pndx = spec.pndx();
        rindices.cndx = spec.cndx();
        (*rsp_cb_func)(user_ctx, rindices);
    }
    return 0;
}

int
HalClient::crypto_key_index_update(uint32_t key_index,
                                   types::CryptoKeyType key_type,
                                   void *key,
                                   uint32_t key_size)
{
    CryptoKeyCreateWithIdRequestMsg     create_req_msg;
    CryptoKeyCreateWithIdResponseMsg    create_rsp_msg;
    CryptoKeyUpdateRequestMsg           update_req_msg;
    CryptoKeyUpdateResponseMsg          update_rsp_msg;
    ClientContext                       context;
    ClientContext                       context2;
    Status                              status;

    auto create_req = create_req_msg.add_request();
    create_req->set_keyindex(key_index);
    create_req->set_allow_dup_alloc(true);
    status = crypto_stub_->CryptoKeyCreateWithId(&context, create_req_msg,
                                                 &create_rsp_msg);
    if (!status.ok()) {
        NIC_FUNC_ERR("GRPC create status {} {}", status.error_code(),
                     status.error_message());
        return -1;
    }

    auto create_rsp = create_rsp_msg.response(0);
    if (create_rsp.keyindex() != key_index) {
        NIC_FUNC_ERR("GRPC create expected key_index {} actual {}",
                     key_index, create_rsp.keyindex());
        return -1;
    }

    auto update_req = update_req_msg.add_request();
    auto update_spec = update_req->mutable_key();
    update_spec->set_key(key, key_size);
    update_spec->set_keyindex(key_index);
    update_spec->set_key_size(key_size);
    update_spec->set_key_type(key_type);

    status = crypto_stub_->CryptoKeyUpdate(&context2, update_req_msg,
                                           &update_rsp_msg);
    if (!status.ok()) {
        NIC_FUNC_ERR("GRPC update status {} {}", status.error_code(),
                     status.error_message());
        return -1;
    }
    auto update_rsp = update_rsp_msg.response(0);
    if (update_rsp.api_status() != types::API_STATUS_OK) {
        NIC_FUNC_ERR("API update status {} key_index {}",
                     update_rsp.api_status(), key_index);
        return -1;
    }
    return 0;
}
