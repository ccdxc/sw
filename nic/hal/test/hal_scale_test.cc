#include <stdio.h>
#include <iostream>
#include <thread>
#include <math.h>
#include <random>
#include <getopt.h>
#include <grpc++/grpc++.h>
#include "gen/proto/types.grpc.pb.h"
#include "gen/proto/vrf.grpc.pb.h"
#include "gen/proto/l2segment.grpc.pb.h"
#include "gen/proto/interface.grpc.pb.h"
#include "gen/proto/nw.grpc.pb.h"
#include "gen/proto/nwsec.grpc.pb.h"
#include "gen/proto/port.grpc.pb.h"
#include "gen/proto/event.grpc.pb.h"
#include "gen/proto/system.grpc.pb.h"
#include "gen/proto/debug.grpc.pb.h"
#include "gen/proto/endpoint.grpc.pb.h"
#include "gen/proto/session.grpc.pb.h"
#include "nic/sdk/include/sdk/timestamp.hpp"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using vrf::Vrf;
using vrf::VrfSpec;
using vrf::VrfRequestMsg;
using vrf::VrfResponse;
using vrf::VrfResponseMsg;
using vrf::VrfGetRequestMsg;
using vrf::VrfGetRequest;
using vrf::VrfGetResponseMsg;
using vrf::VrfDeleteRequestMsg;
using vrf::VrfDeleteRequest;
using vrf::VrfDeleteResponseMsg;
using l2segment::L2Segment;
using l2segment::L2SegmentSpec;
using l2segment::L2SegmentRequestMsg;
using l2segment::L2SegmentResponse;
using l2segment::L2SegmentResponseMsg;
using l2segment::L2SegmentDeleteRequest;
using l2segment::L2SegmentDeleteRequestMsg;
using l2segment::L2SegmentDeleteResponseMsg;
using kh::L2SegmentKeyHandle;
using nw::Network;
using nw::NetworkSpec;
using nw::NetworkRequestMsg;
using nw::NetworkResponseMsg;
using kh::NetworkKeyHandle;
using endpoint::Endpoint;
using endpoint::EndpointSpec;
using endpoint::EndpointRequestMsg;
using endpoint::EndpointResponse;
using endpoint::EndpointResponseMsg;
using endpoint::EndpointDeleteRequest;
using endpoint::EndpointDeleteRequestMsg;
using endpoint::EndpointDeleteResponseMsg;
using nwsec::NwSecurity;
using nwsec::SecurityGroupSpec;
using nwsec::SecurityGroupRequestMsg;
using nwsec::SecurityGroupResponseMsg;
using kh::SecurityGroupKeyHandle;
using session::Session;
using session::SessionSpec;
using session::SessionRequestMsg;
using session::SessionResponseMsg;
using session::FlowSpec;
using session::FlowKey;
using session::FlowInfo;
using session::ConnTrackInfo;
using session::FlowData;
using intf::Interface;
using intf::InterfaceSpec;
using intf::InterfaceRequestMsg;
using intf::InterfaceResponse;
using intf::InterfaceResponseMsg;
using intf::InterfaceL2SegmentSpec;
using intf::InterfaceL2SegmentRequestMsg;
using intf::InterfaceL2SegmentResponseMsg;
using intf::InterfaceL2SegmentResponse;
using types::Empty;
using types::EncapInfo;
using types::IPAddress;

std::string  hal_svc_endpoint_     = "localhost:50054";
std::string  linkmgr_svc_endpoint_ = "localhost:50053";

bool         g_no_cleanup = false;
bool         g_random = true;
bool         g_dup_session = false;
uint64_t     g_num_l2segments = 10;
uint64_t     g_encap_value = 100;
uint64_t     g_num_uplinks = 8;
uint32_t     g_num_sessions = 10;
uint32_t     g_batch_size = 1;

enum {
    SIM = 0,
    HW  = 1,
};

class hal_client {
public:
    hal_client(std::shared_ptr<Channel> channel) : vrf_stub_(Vrf::NewStub(channel)),
    l2seg_stub_(L2Segment::NewStub(channel)), intf_stub_(Interface::NewStub(channel)),
    sg_stub_(NwSecurity::NewStub(channel)), nw_stub_(Network::NewStub(channel)),
    ep_stub_(Endpoint::NewStub(channel)), session_stub_(Session::NewStub(channel)) {}

    bool port_handle_api_status(types::ApiStatus api_status,
                                uint32_t port_id) {
        switch(api_status) {
            case types::API_STATUS_OK:
                return true;

            case types::API_STATUS_NOT_FOUND:
                std::cout << "Port "
                          << port_id
                          << " not found"
                          << std::endl;
                return false;

            case types::API_STATUS_EXISTS_ALREADY:
                std::cout << "Port "
                          << port_id
                          << " exists already"
                          << std::endl;
                return false;

            default:
                assert(0);
        }

        return true;
    }

    uint64_t vrf_create(uint64_t vrf_id) {
        VrfSpec           *spec;
        VrfRequestMsg     req_msg;
        VrfResponseMsg    rsp_msg;
        ClientContext     context;
        Status            status;

        spec = req_msg.add_request();
        spec->mutable_key_or_handle()->set_vrf_id(vrf_id);
        spec->set_vrf_type(::types::VRF_TYPE_CUSTOMER);

        status = vrf_stub_->VrfCreate(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            assert((rsp_msg.response(0).api_status() == types::API_STATUS_OK) ||
                   (rsp_msg.response(0).api_status() == types::API_STATUS_EXISTS_ALREADY));
            std::cout << "Vrf create succeeded, handle = "
                      << rsp_msg.response(0).vrf_status().vrf_handle()
                      << std::endl;
            return rsp_msg.response(0).vrf_status().vrf_handle();
        }
        std::cout << "Vrf create failed, error = "
                  << rsp_msg.response(0).api_status()
                  << std::endl;
        return 0;
    }

    uint64_t sg_create(uint64_t sg_id) {
        SecurityGroupSpec           *spec;
        SecurityGroupRequestMsg     req_msg;
        SecurityGroupResponseMsg    rsp_msg;
        ClientContext               context;
        Status                      status;

        spec = req_msg.add_request();
        spec->mutable_key_or_handle()->set_security_group_id(sg_id);
        status = sg_stub_->SecurityGroupCreate(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            assert(rsp_msg.response(0).api_status() == types::API_STATUS_OK);
            std::cout << "SG create succeeded, handle = "
                      << rsp_msg.response(0).status().sg_handle()
                      << std::endl;
            return rsp_msg.response(0).status().sg_handle();
        }
        std::cout << "SG create failed, error = "
                  << rsp_msg.response(0).api_status()
                  << std::endl;
        return 0;
    }

    uint64_t enic_if_create(uint32_t enic_if_id, uint32_t lif_id,
                            uint64_t pinned_uplink_if_handle,
                            uint64_t native_l2seg_handle,
                            uint64_t non_native_l2seg_id) {
        InterfaceSpec           *spec;
        InterfaceRequestMsg     req_msg;
        InterfaceResponseMsg    rsp_msg;
        L2SegmentKeyHandle      *l2seg_kh;
        ClientContext           context;
        Status                  status;

        spec = req_msg.add_request();
        spec->mutable_key_or_handle()->set_interface_id(enic_if_id);
        spec->set_type(::intf::IfType::IF_TYPE_ENIC);
        spec->set_admin_status(::intf::IfStatus::IF_STATUS_UP);
        spec->mutable_if_enic_info()->set_enic_type(::intf::IF_ENIC_TYPE_CLASSIC);
        spec->mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(lif_id);
        spec->mutable_if_enic_info()->mutable_pinned_uplink_if_key_handle()->set_if_handle(pinned_uplink_if_handle);
        spec->mutable_if_enic_info()->mutable_classic_enic_info()->set_native_l2segment_handle(native_l2seg_handle);
        l2seg_kh = spec->mutable_if_enic_info()->mutable_classic_enic_info()->add_l2segment_key_handle();
        l2seg_kh->set_segment_id(non_native_l2seg_id);
        status = intf_stub_->InterfaceCreate(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            assert(rsp_msg.response(0).api_status() == types::API_STATUS_OK);
            std::cout << "ENIC if create succeeded, handle = "
                      << rsp_msg.response(0).status().if_handle()
                      << std::endl;
            return rsp_msg.response(0).status().if_handle();
        }
        std::cout << "ENIC if create failed, error = "
                  << rsp_msg.response(0).api_status() << std::endl;
        return 0;
    }

    uint64_t ep_create(uint64_t vrf_id, uint64_t l2seg_id,
                       uint64_t if_id, uint64_t sg_id,
                       uint64_t mac_addr, uint32_t ip_addr) {
        EndpointSpec              *spec;
        EndpointRequestMsg        req_msg;
        EndpointResponseMsg       rsp_msg;
        SecurityGroupKeyHandle    *sg_kh;
        IPAddress                 *ip;
        ClientContext             context;
        Status                    status;

        spec = req_msg.add_request();
        spec->mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_segment_id(l2seg_id);
        spec->mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(mac_addr);
        spec->mutable_vrf_key_handle()->set_vrf_id(vrf_id);
        spec->mutable_endpoint_attrs()->mutable_interface_key_handle()->set_interface_id(if_id);
        if (ip_addr) {
            ip = spec->mutable_endpoint_attrs()->add_ip_address();
            ip->set_ip_af(types::IPAddressFamily::IP_AF_INET);
            ip->set_v4_addr(ip_addr);
        }
        sg_kh = spec->mutable_endpoint_attrs()->add_sg_key_handle();
        sg_kh->set_security_group_id(sg_id);
        status = ep_stub_->EndpointCreate(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            assert(rsp_msg.response(0).api_status() == types::API_STATUS_OK);
            std::cout << "Endpoint create succeeded, handle = "
                      << rsp_msg.response(0).endpoint_status().endpoint_handle()
                      << std::endl;
            return rsp_msg.response(0).endpoint_status().endpoint_handle();
        }
        std::cout << "Endpoint create failed, error = "
                  << rsp_msg.response(0).api_status()
                  << std::endl;
        return 0;
    }

    void ep_delete(uint64_t vrf_id, uint64_t l2seg_id, uint64_t mac_addr) {
        EndpointDeleteRequest     *spec;
        EndpointDeleteRequestMsg  req_msg;
        EndpointDeleteResponseMsg rsp_msg;
        ClientContext             context;
        Status                    status;

        spec = req_msg.add_request();
        spec->mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_segment_id(l2seg_id);
        spec->mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(mac_addr);
        spec->mutable_vrf_key_handle()->set_vrf_id(vrf_id);

        status = ep_stub_->EndpointDelete(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            assert(rsp_msg.response(0).api_status() == types::API_STATUS_OK);
            std::cout << "Endpoint delete succeeded"
                      << std::endl;
            return;
        }
        std::cout << "Endpoint delete failed"
                  << std::endl;
        return;
    }

    uint64_t session_create(SessionRequestMsg &req_msg,
                            uint64_t session_id, uint64_t vrf_id, uint32_t sip, uint32_t dip,
                            ::types::IPProtocol proto, uint16_t sport, uint16_t dport,
                            ::session::FlowAction action, bool& send) {
        SessionSpec               *spec;
        FlowSpec                  *flow;
        SessionResponseMsg        rsp_msg;
        ClientContext             context;
        Status                    status;

        spec = req_msg.add_request();
        spec->mutable_vrf_key_handle()->set_vrf_id(vrf_id);
        spec->set_session_id(session_id);
        spec->set_conn_track_en(false);
        spec->set_tcp_ts_option(false);
        spec->set_tcp_sack_perm_option(false);
        flow = spec->mutable_initiator_flow();
        flow->mutable_flow_key()->mutable_v4_key()->set_sip(sip);
        flow->mutable_flow_key()->mutable_v4_key()->set_dip(dip);
        flow->mutable_flow_key()->mutable_v4_key()->set_ip_proto(proto);
        flow->mutable_flow_key()->mutable_v4_key()->mutable_tcp_udp()->set_sport(sport);
        flow->mutable_flow_key()->mutable_v4_key()->mutable_tcp_udp()->set_dport(dport);
        flow->mutable_flow_data()->mutable_flow_info()->set_flow_action(action);
        flow->mutable_flow_data()->mutable_conn_track_info()->set_iflow_syn_ack_delta(0);

        flow = spec->mutable_responder_flow();
        flow->mutable_flow_key()->mutable_v4_key()->set_sip(dip);
        flow->mutable_flow_key()->mutable_v4_key()->set_dip(sip);
        flow->mutable_flow_key()->mutable_v4_key()->set_ip_proto(proto);
        flow->mutable_flow_key()->mutable_v4_key()->mutable_tcp_udp()->set_sport(dport);
        flow->mutable_flow_key()->mutable_v4_key()->mutable_tcp_udp()->set_dport(sport);
        flow->mutable_flow_data()->mutable_flow_info()->set_flow_action(action);

        if (send) {
            std::cout << "Batch Size " << req_msg.request_size() << std::endl;
            status = session_stub_->SessionCreate(&context, req_msg, &rsp_msg);
            req_msg.Clear();
            if (status.ok()) {
#if 0
                for (int i = 0; i < rsp_msg.response_size(); i ++) {
                    if (rsp_msg.response(i).api_status() == types::API_STATUS_OK) {
                        std::cout << "Session create succeeded" << std::endl;
                        return true;
                    }
                }
#endif
            } else {
                std::cout << "Session create failed" << std::endl;
            }
            send = false;
        } else {
            return true;
        }
        return false;
    }

    uint64_t nw_create(uint64_t nw_id, uint64_t vrf_id, uint32_t ip_pfx,
                       uint8_t pfx_len, uint64_t rmac, uint64_t sg_id) {
        NetworkSpec               *spec;
        NetworkRequestMsg         req_msg;
        NetworkResponseMsg        rsp_msg;
        SecurityGroupKeyHandle    *sg_kh;
        ClientContext             context;
        Status                    status;

        // now create network
        spec = req_msg.add_request();
        spec->mutable_key_or_handle()->mutable_nw_key()->mutable_vrf_key_handle()->set_vrf_id(vrf_id);
        spec->mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
        spec->mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_v4_addr(ip_pfx);
        spec->mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->set_prefix_len(pfx_len);
        spec->set_rmac(rmac);
        //spec->set_gateway_ep_handle(gw_ep_handle);
        sg_kh = spec->add_sg_key_handle();
        sg_kh->set_security_group_id(sg_id);
        status = nw_stub_->NetworkCreate(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            assert(rsp_msg.response(0).api_status() == types::API_STATUS_OK);
            std::cout << "Network create succeeded, handle = "
                      << rsp_msg.response(0).status().nw_handle()
                      << std::endl;
            return rsp_msg.response(0).status().nw_handle();
        }
        std::cout << "Network create failed, error = "
                  << rsp_msg.response(0).api_status()
                  << std::endl;
        return 0;
    }

    uint64_t vrf_get_by_id(uint32_t id) {
        VrfGetRequestMsg     req_msg;
        VrfGetRequest        *req;
        VrfGetResponseMsg    rsp_msg;
        ClientContext        context;
        Status               status;

        req = req_msg.add_request();
        req->mutable_key_or_handle()->set_vrf_id(id);
        status = vrf_stub_->VrfGet(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            std::cout << "Vrf get succeeded, handle = "
                      << rsp_msg.response(0).status().vrf_handle()
                      << std::endl;
            return rsp_msg.response(0).status().vrf_handle();
        }
        std::cout << "Vrf get failed, error = "
                  << rsp_msg.response(0).api_status()
                  << std::endl;
        return 0;
    }

    uint64_t vrf_get_by_handle(uint64_t hal_handle) {
        VrfGetRequestMsg     req_msg;
        VrfGetRequest        *req;
        VrfGetResponseMsg    rsp_msg;
        ClientContext           context;
        Status                  status;

        req = req_msg.add_request();
        req->mutable_key_or_handle()->set_vrf_handle(hal_handle);
        status = vrf_stub_->VrfGet(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            std::cout << "Vrf get succeeded, handle = "
                      << rsp_msg.response(0).status().vrf_handle()
                      << std::endl;
            return rsp_msg.response(0).status().vrf_handle();
        }
        std::cout << "Vrf get failed, error = "
                  << rsp_msg.response(0).api_status()
                  << std::endl;
        return 0;
    }

    void vrf_delete_by_id(uint32_t id) {
        VrfDeleteRequestMsg     req_msg;
        VrfDeleteRequest        *req;
        VrfDeleteResponseMsg    rsp_msg;
        ClientContext              context;
        Status                     status;

        req = req_msg.add_request();
        req->mutable_key_or_handle()->set_vrf_id(id);

        status = vrf_stub_->VrfDelete(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            std::cout << "Vrf delete succeeded" << std::endl;
            return;
        } else {
            std::cout << "Vrf delete failed" << std::endl;
        }
        return;
    }

    void vrf_delete_by_handle(uint64_t hal_handle) {
        VrfDeleteRequestMsg     req_msg;
        VrfDeleteRequest        *req;
        VrfDeleteResponseMsg    rsp_msg;
        ClientContext              context;
        Status                     status;

        req = req_msg.add_request();
        req->mutable_key_or_handle()->set_vrf_handle(hal_handle);

        status = vrf_stub_->VrfDelete(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            std::cout << "Vrf delete succeeded" << std::endl;
            return;
        }
        std::cout << "Vrf delete failed" << std::endl;
        return;
    }

    uint64_t l2segment_create(uint64_t vrf_id,
                              uint64_t l2segment_id,
                              uint64_t nw_handle,
                              ::l2segment::BroadcastFwdPolicy bcast_policy,
                              ::l2segment::MulticastFwdPolicy mcast_policy,
                              EncapInfo   l2seg_encap) {
        L2SegmentSpec           *spec;
        L2SegmentRequestMsg     req_msg;
        L2SegmentResponseMsg    rsp_msg;
        NetworkKeyHandle        *nw_kh;
        ClientContext           context;
        Status                  status;

        spec = req_msg.add_request();
        spec->mutable_key_or_handle()->set_segment_id(l2segment_id);
        nw_kh = spec->add_network_key_handle();
        nw_kh->set_nw_handle(nw_handle);
        spec->mutable_vrf_key_handle()->set_vrf_id(vrf_id);
        spec->set_mcast_fwd_policy(mcast_policy);
        spec->set_bcast_fwd_policy(bcast_policy);
        spec->mutable_wire_encap()->set_encap_type(l2seg_encap.encap_type());
        spec->mutable_wire_encap()->set_encap_value(l2seg_encap.encap_value());
        status = l2seg_stub_->L2SegmentCreate(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            assert(rsp_msg.response(0).api_status() == types::API_STATUS_OK);
            std::cout << "L2 segment create succeeded, handle = "
                      << rsp_msg.response(0).l2segment_status().l2segment_handle()
                      << std::endl;
            return rsp_msg.response(0).l2segment_status().l2segment_handle();
        }
        std::cout << "L2 segment create failed, error = "
                  << rsp_msg.response(0).api_status()
                  << std::endl;
        return 0;
    }

    void l2segment_delete(uint64_t vrf_id,
                              uint64_t l2segment_id) {
        L2SegmentDeleteRequest          *spec;
        L2SegmentDeleteRequestMsg       req_msg;
        L2SegmentDeleteResponseMsg      rsp_msg;
        ClientContext                   context;
        Status                          status;

        spec = req_msg.add_request();
        spec->mutable_key_or_handle()->set_segment_id(l2segment_id);
        status = l2seg_stub_->L2SegmentDelete(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            assert(rsp_msg.response(0).api_status() == types::API_STATUS_OK);
            std::cout << "L2 segment delete succeeded"
                      << std::endl;
            return;
        }
        std::cout << "L2 segment delete failed"
                  << std::endl;
        return;
    }

    // create few uplinks and return the handle for the 1st one
    uint64_t uplinks_create(uint64_t if_id_start, uint32_t num_uplinks,
                            uint64_t native_l2seg_id) {
        InterfaceSpec           *spec;
        InterfaceRequestMsg     req_msg;
        InterfaceResponseMsg    rsp_msg;
        ClientContext           context;
        Status                  status;
        static uint64_t         port_num = 1;

        for (uint32_t i = 0; i < num_uplinks; i++) {
            spec = req_msg.add_request();
            spec->mutable_key_or_handle()->set_interface_id(if_id_start++);
            spec->set_type(::intf::IfType::IF_TYPE_UPLINK);
            spec->set_admin_status(::intf::IfStatus::IF_STATUS_UP);
            spec->mutable_if_uplink_info()->set_port_num(port_num++);
            spec->mutable_if_uplink_info()->set_native_l2segment_id(native_l2seg_id);
        }
        status = intf_stub_->InterfaceCreate(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            for (uint32_t i = 0; i < num_uplinks; i++) {
                assert(rsp_msg.response(i).api_status() == types::API_STATUS_OK);
                std::cout << "Uplink interface create succeeded, handle = "
                          << rsp_msg.response(i).status().if_handle()
                          << std::endl;
            }
            return rsp_msg.response(0).status().if_handle();
        } else {
            for (uint32_t i = 0; i < num_uplinks; i++) {
                std::cout << "Uplink interface create failed, error = "
                          << rsp_msg.response(i).api_status()
                          << std::endl;
            }
        }

        return 0;
    }

    void add_l2seg_on_uplinks(uint64_t if_id_start, uint32_t num_uplinks, uint64_t l2seg_id) {
        InterfaceL2SegmentSpec           *spec;
        InterfaceL2SegmentRequestMsg     req_msg;
        InterfaceL2SegmentResponseMsg    rsp_msg;
        ClientContext                    context;
        Status                           status;

        for (uint32_t i = 0; i < num_uplinks; i++) {
            spec = req_msg.add_request();
            spec->mutable_l2segment_key_or_handle()->set_segment_id(l2seg_id);
            spec->mutable_if_key_handle()->set_interface_id(if_id_start++);
        }
        status = intf_stub_->AddL2SegmentOnUplink(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            for (uint32_t i = 0; i < num_uplinks; i++) {
                assert(rsp_msg.response(i).api_status() == types::API_STATUS_OK);
                std::cout << "L2 Segment " << l2seg_id
                          << " add on uplink " << i + 1
                          << " succeeded" << std::endl;
            }
        } else {
            for (uint32_t i = 0; i < num_uplinks; i++) {
                std::cout << "L2 Segment " << l2seg_id
                          << " add on uplink " << i + 1
                          << " failed, err " << rsp_msg.response(i).api_status()
                          << std::endl;
            }
        }

        return;
    }

    void delete_l2seg_on_uplinks(uint64_t if_id_start, uint32_t num_uplinks, uint64_t l2seg_id) {
        InterfaceL2SegmentSpec           *spec;
        InterfaceL2SegmentRequestMsg     req_msg;
        InterfaceL2SegmentResponseMsg    rsp_msg;
        ClientContext                    context;
        Status                           status;

        for (uint32_t i = 0; i < num_uplinks; i++) {
            spec = req_msg.add_request();
            spec->mutable_l2segment_key_or_handle()->set_segment_id(l2seg_id);
            spec->mutable_if_key_handle()->set_interface_id(if_id_start++);
        }
        status = intf_stub_->DelL2SegmentOnUplink(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            for (uint32_t i = 0; i < num_uplinks; i++) {
                assert(rsp_msg.response(i).api_status() == types::API_STATUS_OK);
                std::cout << "L2 Segment " << l2seg_id
                          << " delete on uplink " << i + 1
                          << " succeeded" << std::endl;
            }
        } else {
            for (uint32_t i = 0; i < num_uplinks; i++) {
                std::cout << "L2 Segment " << l2seg_id
                          << " delete on uplink " << i + 1
                          << " failed, err " << rsp_msg.response(i).api_status()
                          << std::endl;
            }
        }

        return;
    }

private:
    std::unique_ptr<Vrf::Stub> vrf_stub_;
    std::unique_ptr<L2Segment::Stub> l2seg_stub_;
    std::unique_ptr<Interface::Stub> intf_stub_;
    std::unique_ptr<NwSecurity::Stub> sg_stub_;
    std::unique_ptr<Network::Stub> nw_stub_;
    std::unique_ptr<Endpoint::Stub> ep_stub_;
    std::unique_ptr<Session::Stub> session_stub_;
};

static int
create_l2segments (uint64_t   l2seg_id_start,
                   uint64_t   encap_start,
                   uint64_t   if_id_start,
                   uint64_t   num_l2segments,
                   uint64_t   nw_handle,
                   uint64_t   vrf_id,
                   uint64_t   num_uplinks,
                   hal_client &hclient,
                   uint64_t   *l2seg_handle_out)
{
    EncapInfo  l2seg_encap;
    uint64_t   l2seg_handle = 0;
    uint64_t   encap_value  = encap_start;

    for (uint64_t l2seg_id = l2seg_id_start;
                  l2seg_id < l2seg_id_start + num_l2segments;
                  ++l2seg_id) {

        // create L2 segments
        l2seg_encap.set_encap_type(::types::ENCAP_TYPE_DOT1Q);
        l2seg_encap.set_encap_value(encap_value);

        l2seg_handle =
            hclient.l2segment_create(vrf_id,
                                     l2seg_id,
                                     nw_handle,
                                     ::l2segment::BROADCAST_FWD_POLICY_FLOOD,
                                     ::l2segment::MULTICAST_FWD_POLICY_FLOOD,
                                     l2seg_encap);
        assert(l2seg_handle != 0);

        if (encap_value == 100) {
            *l2seg_handle_out = l2seg_handle;
        }

        // bringup this L2seg on all uplinks
        hclient.add_l2seg_on_uplinks(if_id_start,
                                     num_uplinks,
                                     l2seg_id);
        encap_value++;
    }

    return 0;
}

static int
delete_l2segments (uint64_t   l2seg_id_start,
                   uint64_t   if_id_start,
                   uint64_t   num_l2segments,
                   uint64_t   vrf_id,
                   uint64_t   num_uplinks,
                   hal_client &hclient)
{
    for (uint64_t l2seg_id = l2seg_id_start;
                  l2seg_id < l2seg_id_start + num_l2segments;
                  ++l2seg_id) {

        // Delete this L2seg from all uplinks
        hclient.delete_l2seg_on_uplinks(if_id_start,
                                     num_uplinks,
                                     l2seg_id);

        hclient.l2segment_delete(vrf_id, l2seg_id);
    }

    return 0;
}

// setup all the config for "sim" mode
// TODO: make use of sip, dip, sport_lo, sport_hi, dport etc. provided in the
// CLI
static int
setup_sim_config (hal_client& hclient, uint64_t vrf_id, uint64_t l2seg_id,
                  uint64_t sg_id, uint64_t if_id, uint64_t nw_id)
{
    uint64_t     vrf_handle, l2seg_handle, sg_handle;
    uint64_t     nw1_handle, nw2_handle, uplink_if_handle;
    uint32_t     src_ip[15], dst_ip[15];
    EncapInfo    l2seg_encap;
    uint32_t     session_count = 0, batch_count = 0;
    bool         send = false;
    timespec_t   start_ts, end_ts;
    uint64_t     start_ns, end_ns;

    // create the vrf
    vrf_handle = hclient.vrf_create(vrf_id);
    assert(vrf_handle != 0);

    // create a security group
    sg_handle = hclient.sg_create(sg_id);
    assert(sg_handle != 0);

    // create network objects
    nw1_handle = hclient.nw_create(nw_id, vrf_id, 0x0a0a0100, 24,
                                   0x020a0a0101, sg_id);
    assert(nw1_handle != 0);
    nw_id++;
    nw2_handle = hclient.nw_create(nw_id, vrf_id, 0x0a0a0200, 24,
                                   0x020a0a0201, sg_id);
    assert(nw2_handle != 0);

    // create uplinks with this L2 seg as native L2 seg
    uplink_if_handle = hclient.uplinks_create(if_id, g_num_uplinks, l2seg_id);
    assert(uplink_if_handle != 0);

    create_l2segments(l2seg_id, g_encap_value, if_id, g_num_l2segments,
                      nw1_handle, vrf_id, g_num_uplinks, hclient,
                      &l2seg_handle);

    uint64_t dest_encap_value = g_encap_value + g_num_l2segments;
    uint64_t dest_l2seg_id = l2seg_id + g_num_l2segments;
    uint64_t dest_if_id = if_id + 1;

    l2seg_encap.set_encap_type(::types::ENCAP_TYPE_DOT1Q);
    l2seg_encap.set_encap_value(dest_encap_value);
    l2seg_handle =
        hclient.l2segment_create(vrf_id, dest_l2seg_id,
                                 nw2_handle,
                                 ::l2segment::BROADCAST_FWD_POLICY_FLOOD,
                                 ::l2segment::MULTICAST_FWD_POLICY_FLOOD,
                                 l2seg_encap);
    assert(l2seg_handle != 0);

    // bringup this l2seg on all uplinks
    hclient.add_l2seg_on_uplinks(if_id, g_num_uplinks, dest_l2seg_id);

    if (g_random) {
        // create random remote endpoints
        std::random_device rd;

        // random number generator
        std::default_random_engine generator(rd());

        // distribution on which to apply the generator
        std::uniform_int_distribution<> distribution(0x0a0a0a0a, 0x0f0f0f0f);

        for (int i = 0; i < 14; i++) {
            src_ip[i] = distribution(generator);
            dst_ip[i] = distribution(generator);
        }
    } else {
        src_ip[0] = 0x0a0a0102;
        dst_ip[0] = 0x0a0a0202;

        for (int i = 1; i < 14; i++) {
            src_ip[i] = src_ip[i-1] + 1;
            dst_ip[i] = dst_ip[i-1] + 1;
        }
    }

    hclient.ep_create(vrf_id, l2seg_id, if_id, sg_id, 0x020a0a0102, src_ip[0]);
    hclient.ep_create(vrf_id, l2seg_id, if_id, sg_id, 0x020a0a0103, src_ip[1]);
    hclient.ep_create(vrf_id, l2seg_id, if_id, sg_id, 0x020a0a0104, src_ip[2]);
    hclient.ep_create(vrf_id, l2seg_id, if_id, sg_id, 0x020a0a0105, src_ip[3]);
    hclient.ep_create(vrf_id, l2seg_id, if_id, sg_id, 0x020a0a0106, src_ip[4]);
    hclient.ep_create(vrf_id, l2seg_id, if_id, sg_id, 0x020a0a0107, src_ip[5]);
    hclient.ep_create(vrf_id, l2seg_id, if_id, sg_id, 0x020a0a0108, src_ip[6]);
    hclient.ep_create(vrf_id, l2seg_id, if_id, sg_id, 0x020a0a0109, src_ip[7]);
    hclient.ep_create(vrf_id, l2seg_id, if_id, sg_id, 0x020a0a010a, src_ip[8]);
    hclient.ep_create(vrf_id, l2seg_id, if_id, sg_id, 0x020a0a010b, src_ip[9]);
    hclient.ep_create(vrf_id, l2seg_id, if_id, sg_id, 0x020a0a010c, src_ip[10]);
    hclient.ep_create(vrf_id, l2seg_id, if_id, sg_id, 0x020a0a010d, src_ip[11]);
    hclient.ep_create(vrf_id, l2seg_id, if_id, sg_id, 0x020a0a010e, src_ip[12]);
    hclient.ep_create(vrf_id, l2seg_id, if_id, sg_id, 0x020a0a010f, src_ip[13]);
    hclient.ep_create(vrf_id, dest_l2seg_id, dest_if_id, sg_id, 0x020a0a0202, dst_ip[0]);
    hclient.ep_create(vrf_id, dest_l2seg_id, dest_if_id, sg_id, 0x020a0a0203, dst_ip[1]);
    hclient.ep_create(vrf_id, dest_l2seg_id, dest_if_id, sg_id, 0x020a0a0204, dst_ip[2]);
    hclient.ep_create(vrf_id, dest_l2seg_id, dest_if_id, sg_id, 0x020a0a0205, dst_ip[3]);
    hclient.ep_create(vrf_id, dest_l2seg_id, dest_if_id, sg_id, 0x020a0a0206, dst_ip[4]);
    hclient.ep_create(vrf_id, dest_l2seg_id, dest_if_id, sg_id, 0x020a0a0207, dst_ip[5]);
    hclient.ep_create(vrf_id, dest_l2seg_id, dest_if_id, sg_id, 0x020a0a0208, dst_ip[6]);
    hclient.ep_create(vrf_id, dest_l2seg_id, dest_if_id, sg_id, 0x020a0a0209, dst_ip[7]);
    hclient.ep_create(vrf_id, dest_l2seg_id, dest_if_id, sg_id, 0x020a0a020a, dst_ip[8]);
    hclient.ep_create(vrf_id, dest_l2seg_id, dest_if_id, sg_id, 0x020a0a020b, dst_ip[9]);
    hclient.ep_create(vrf_id, dest_l2seg_id, dest_if_id, sg_id, 0x020a0a020c, dst_ip[10]);
    hclient.ep_create(vrf_id, dest_l2seg_id, dest_if_id, sg_id, 0x020a0a020d, dst_ip[11]);
    hclient.ep_create(vrf_id, dest_l2seg_id, dest_if_id, sg_id, 0x020a0a020e, dst_ip[12]);
    hclient.ep_create(vrf_id, dest_l2seg_id, dest_if_id, sg_id, 0x020a0a020f, dst_ip[13]);

    // NOTE: uncomment this in smart nic mode
    // create sessions
    SessionRequestMsg req_msg;
    clock_gettime(CLOCK_MONOTONIC, &start_ts);
    sdk::timestamp_to_nsecs(&start_ts, &start_ns);
    if (g_dup_session) {
        for (int i = 0; i < 2; i ++) {
            send = true;
            hclient.session_create(req_msg, session_count, vrf_id,
                                   src_ip[1], dst_ip[1],
                                   ::types::IPProtocol::IPPROTO_UDP,
                                   5024, 7050,
                                   ::session::FlowAction::FLOW_ACTION_ALLOW,
                                   send);
        }
    } else {
        for (int src_port = 5024; src_port < 7035; src_port ++) {
            for (int dst_port = 7050; dst_port < 9051; dst_port ++) {
                for (int i = 0; i < 14; i ++) {        // src EPs
                    for (int j = 0; j < 14; j ++) {    // dst EPs
                        batch_count++;
                        if (batch_count == g_batch_size) {
                            send = true;
                            batch_count = 0;
                        }
                        session_count++;
                        hclient.session_create(req_msg, session_count, vrf_id,
                                               src_ip[i], dst_ip[j],
                                               ::types::IPProtocol::IPPROTO_UDP,
                                               src_port, dst_port,
                                               ::session::FlowAction::FLOW_ACTION_ALLOW,
                                               send);
                        if (session_count == g_num_sessions) {
                            goto done;
                        }
                    }
                }
            }
        }
    }

done:

    clock_gettime(CLOCK_MONOTONIC, &end_ts);
    sdk::timestamp_to_nsecs(&end_ts, &end_ns);
    float time = (float(end_ns - start_ns)) /1000000000;

    std::cout << "Time to create " << g_num_sessions << " sessions is "
              << time << " secs" << std::endl;
    std::cout << "Session/sec is " << g_num_sessions/time << std::endl;

    return 0;
}

// cleanup all the config in "sim" mode
static int
cleanup_sim_config (hal_client& hclient, uint64_t vrf_id, uint64_t l2seg_id,
                    uint64_t sg_id, uint64_t if_id, uint64_t nw_id)
{
    uint64_t dest_l2seg_id = l2seg_id + g_num_l2segments;

    hclient.ep_delete(vrf_id, l2seg_id, 0x020a0a0102);
    hclient.ep_delete(vrf_id, l2seg_id, 0x020a0a0103);
    hclient.ep_delete(vrf_id, l2seg_id, 0x020a0a0104);
    hclient.ep_delete(vrf_id, l2seg_id, 0x020a0a0105);
    hclient.ep_delete(vrf_id, l2seg_id, 0x020a0a0106);
    hclient.ep_delete(vrf_id, l2seg_id, 0x020a0a0107);
    hclient.ep_delete(vrf_id, l2seg_id, 0x020a0a0108);
    hclient.ep_delete(vrf_id, l2seg_id, 0x020a0a0109);
    hclient.ep_delete(vrf_id, l2seg_id, 0x020a0a010a);
    hclient.ep_delete(vrf_id, l2seg_id, 0x020a0a010b);
    hclient.ep_delete(vrf_id, l2seg_id, 0x020a0a010c);
    hclient.ep_delete(vrf_id, l2seg_id, 0x020a0a010d);
    hclient.ep_delete(vrf_id, l2seg_id, 0x020a0a010e);
    hclient.ep_delete(vrf_id, l2seg_id, 0x020a0a010f);
    hclient.ep_delete(vrf_id, dest_l2seg_id, 0x020a0a0202);
    hclient.ep_delete(vrf_id, dest_l2seg_id, 0x020a0a0203);
    hclient.ep_delete(vrf_id, dest_l2seg_id, 0x020a0a0204);
    hclient.ep_delete(vrf_id, dest_l2seg_id, 0x020a0a0205);
    hclient.ep_delete(vrf_id, dest_l2seg_id, 0x020a0a0206);
    hclient.ep_delete(vrf_id, dest_l2seg_id, 0x020a0a0207);
    hclient.ep_delete(vrf_id, dest_l2seg_id, 0x020a0a0208);
    hclient.ep_delete(vrf_id, dest_l2seg_id, 0x020a0a0209);
    hclient.ep_delete(vrf_id, dest_l2seg_id, 0x020a0a020a);
    hclient.ep_delete(vrf_id, dest_l2seg_id, 0x020a0a020b);
    hclient.ep_delete(vrf_id, dest_l2seg_id, 0x020a0a020c);
    hclient.ep_delete(vrf_id, dest_l2seg_id, 0x020a0a020d);
    hclient.ep_delete(vrf_id, dest_l2seg_id, 0x020a0a020e);
    hclient.ep_delete(vrf_id, dest_l2seg_id, 0x020a0a020f);

    // Delete L2 segments
    hclient.delete_l2seg_on_uplinks(if_id, g_num_uplinks, dest_l2seg_id);
    hclient.l2segment_delete(vrf_id, dest_l2seg_id);
    delete_l2segments(l2seg_id, if_id, g_num_l2segments,
                      vrf_id, g_num_uplinks, hclient);

    return 0;
}

static int
setup_hw_config (hal_client& hclient, uint64_t vrf_id, uint32_t sip,
                 uint32_t dip, uint16_t sport_lo, uint16_t sport_hi,
                 uint16_t dport)
{
    SessionRequestMsg req_msg;
    timespec_t   start_ts, end_ts;
    uint64_t     start_ns, end_ns;
    uint32_t     session_count = 0, batch_count = 0;
    bool         send = false;

    std::cout << "sip : " << sip << ", dip : " << dip
              << "sport_lo : " << sport_lo << ", sport_hi : " << sport_hi
              << ", dport : " << dport << std::endl;

    clock_gettime(CLOCK_MONOTONIC, &start_ts);
    sdk::timestamp_to_nsecs(&start_ts, &start_ns);
    for (int src_port = sport_lo; src_port < sport_hi; src_port ++) {
        for (int dst_port = dport; dst_port <= dport; dst_port ++) {
            batch_count++;
            if (batch_count == g_batch_size) {
                send = true;
                batch_count = 0;
            }
            session_count++;
            //std::cout <<"Creating session id: " << session_count << ", sip : "
                      //<< sip << ", dip : " << dip << ", sport:" << src_port
                      //<< ", dport :" << dst_port << std::endl;
            hclient.session_create(req_msg, session_count, vrf_id,
                                   sip, dip,
                                   ::types::IPProtocol::IPPROTO_UDP,
                                   src_port, dst_port,
                                   ::session::FlowAction::FLOW_ACTION_ALLOW,
                                   send);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end_ts);
    sdk::timestamp_to_nsecs(&end_ts, &end_ns);
    float time = (float(end_ns - start_ns)) /1000000000;

    std::cout << "Time to create " << g_num_sessions << " sessions is "
              << time << " secs" << std::endl;
    std::cout << "Session/sec is " << g_num_sessions/time << std::endl;

    return 0;
}

//------------------------------------------------------------------------------
// Supported arguments
// --m seq | random or --mode seq | random: Source and Destination IP addresses 
//                                          of session are sequential | random
//                                          (default they are chosen at random)
// --n xx or --num-sessions xx: Specify number of sessions to be created
// --b xx or --batch-size xx: Specify batch size (Needs to be the last argument always)
//------------------------------------------------------------------------------
int
main (int argc, char** argv)
{
    uint64_t     vrf_id = 1, l2seg_id = 1, sg_id = 1, if_id = 2, nw_id = 1;
    std::string  svc_endpoint = hal_svc_endpoint_;
    int          oc;
    uint32_t     target, sip = 0, dip = 0;
    uint16_t     sport_lo = 0, sport_hi = 0, dport = 0;

    struct option longopts[] = {
       { "mode",                required_argument,  NULL, 'm' },
       { "num-sessions",        required_argument,  NULL, 'n' },
       { "batch-size",          required_argument,  NULL, 'b' },
       { "target",              required_argument,  NULL, 't' },
       { "no-cleanup",          no_argument,        NULL, 'c' },
       { "duplicate-sessions",  no_argument,        NULL, 'e' },
       { "sip",                 no_argument,        NULL, 's' },
       { "dip",                 no_argument,        NULL, 'd' },
       { "sport-low",           no_argument,        NULL, 'l' },
       { "sport-high",          no_argument,        NULL, 'h' },
       { "dport",               no_argument,        NULL, 'p' },
       { 0,                     0,                  0,     0  }
    };

    // parse CLI options
    while ((oc = getopt_long(argc, argv, ":cem:n:b:s:d:l:h:p:t:", longopts, NULL)) != -1) {
        switch (oc) {
        case 'm':
            if (!strcmp(optarg, "seq")) {
                g_random = false;
            } else if (!strcmp(optarg, "random")) {
                g_random = true;
            }
            break;
        case 'n':
            g_num_sessions = atoi(optarg);
            break;
        case 'b':
            g_batch_size = atoi(optarg);
            break;
        case 'c':
            g_no_cleanup = true;
            break;
        case 'e':
            std::cout << "setting dup session to true" << std::endl;
            g_dup_session = true;
            break;
        case 's':
            sip = atoi(optarg);
            break;
        case 'd':
            dip = atoi(optarg);
            break;
        case 'l':
            sport_lo = atoi(optarg);
            break;
        case 'h':
            sport_hi = atoi(optarg);
            break;
        case 'p':
            dport = atoi(optarg);
            break;
        case 't':
            if (!strcmp(optarg, "sim")) {
                target = SIM;
            } else {
                target = HW;
            }
            break;
        case ':':
            break;
        case '?':
        default:
            std::cout << "Invalid Argument" << std::endl;
            exit(0);
            break;
        }
    }

    hal_client hclient(grpc::CreateChannel(svc_endpoint,
                                           grpc::InsecureChannelCredentials()));

    std::cout << "mode is " << (g_random ? "random" : "sequential") << std::endl
              << "number of Sessions to be created is " << g_num_sessions << std::endl
              << "batch size is " << g_batch_size << std::endl;

    if (target == SIM) {
        setup_sim_config(hclient, vrf_id, l2seg_id, sg_id, if_id, nw_id);
    } else {
        // heimdall would have pushed the config already, just setup sessions
        vrf_id = 4;    // TODO: fix this to match heimdall !!
        setup_hw_config(hclient, vrf_id, sip, dip, sport_lo, sport_hi, dport);
    }

    if (g_no_cleanup) {
        return 0;
    } else if (target == SIM) {
        cleanup_sim_config(hclient, vrf_id, l2seg_id, sg_id, if_id, nw_id);
                           
    }
 
    return 0;
}
