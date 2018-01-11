#include <stdio.h>
#include <iostream>
#include <thread>
#include <math.h>
#include <grpc++/grpc++.h>
#include "nic/gen/proto/hal/types.grpc.pb.h"
#include "nic/gen/proto/hal/vrf.grpc.pb.h"
#include "nic/gen/proto/hal/l2segment.grpc.pb.h"
#include "nic/gen/proto/hal/interface.grpc.pb.h"
#include "nic/gen/proto/hal/nw.grpc.pb.h"
#include "nic/gen/proto/hal/nwsec.grpc.pb.h"
#include "nic/gen/proto/hal/port.grpc.pb.h"
#include "nic/gen/proto/hal/event.grpc.pb.h"
#include "nic/gen/proto/hal/system.grpc.pb.h"
#include "nic/gen/proto/hal/debug.grpc.pb.h"
#include "nic/gen/proto/hal/endpoint.grpc.pb.h"
#include "nic/gen/proto/hal/session.grpc.pb.h"

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
using intf::LifSpec;
using intf::LifRequestMsg;
using intf::LifResponseMsg;
using intf::LifQStateMapEntry;
using intf::QStateSetReq;

using port::Port;
using port::PortSpec;
using port::PortRequestMsg;
using port::PortResponse;
using port::PortResponseMsg;
using port::PortGetRequest;
using port::PortGetRequestMsg;
using port::PortGetResponse;
using port::PortGetResponseMsg;
using port::PortDeleteRequest;
using port::PortDeleteRequestMsg;
using port::PortDeleteResponseMsg;
using port::PortDeleteResponse;

using sys::System;
using sys::SystemResponse;
using sys::Stats;
using sys::DropStats;
using sys::DropStatsEntry;
using sys::TableStats;
using sys::TableStatsEntry;
using types::Empty;

using debug::Debug;
using debug::SlabGetRequest;
using debug::SlabGetRequestMsg;
using debug::SlabGetResponse;
using debug::SlabGetResponseMsg;
using debug::SlabSpec;
using debug::SlabStats;

using types::EncapInfo;
using types::IPAddress;

using event::Event;
using event::EventRequest;
using event::EventResponse;

const std::string&    hal_svc_endpoint_("localhost:50054");

class hal_client {
public:
    hal_client(std::shared_ptr<Channel> channel) : vrf_stub_(Vrf::NewStub(channel)),
    l2seg_stub_(L2Segment::NewStub(channel)), port_stub_(Port::NewStub(channel)),
    event_stub_(Event::NewStub(channel)), system_stub_(System::NewStub(channel)),
    debug_stub_(Debug::NewStub(channel)), intf_stub_(Interface::NewStub(channel)),
    sg_stub_(NwSecurity::NewStub(channel)), nw_stub_(Network::NewStub(channel)),
    ep_stub_(Endpoint::NewStub(channel)), session_stub_(Session::NewStub(channel)) {}

    bool port_handle_api_status(types::ApiStatus api_status,
                                uint32_t port_id) {
        switch(api_status) {
            case types::API_STATUS_OK:
                return true;

            case types::API_STATUS_PORT_NOT_FOUND:
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

    int port_create(uint32_t vrf_id, uint32_t port_id) {
        PortSpec            *spec;
        PortRequestMsg      req_msg;
        PortResponseMsg     rsp_msg;
        ClientContext       context;
        Status               status;

        spec = req_msg.add_request();
        spec->mutable_key_or_handle()->set_port_id(port_id);
        spec->mutable_meta()->set_vrf_id(vrf_id);
        spec->set_port_speed(::port::PORT_SPEED_25G);
        spec->set_num_lanes(1);
        spec->set_port_type(::port::PORT_TYPE_ETH);
        spec->set_admin_state(::port::PORT_ADMIN_STATE_UP);

        status = port_stub_->PortCreate(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            if (port_handle_api_status(
                    rsp_msg.response(0).api_status(), port_id) == true) {
                std::cout << "Port create succeeded for port "
                          << port_id
                          << std::endl;
            }

            return 0;
        }

        std::cout << "Port create failed for port "
                  << port_id
                  << " , error = " << rsp_msg.response(0).api_status()
                  << std::endl;
        return -1;
    }

    int port_update(uint32_t vrf_id,
                    uint32_t port_id,
                    ::port::PortSpeed speed,
                    ::port::PortAdminState admin_state) {
        PortSpec            *spec;
        PortRequestMsg      req_msg;
        PortResponseMsg     rsp_msg;
        ClientContext       context;
        Status               status;

        spec = req_msg.add_request();
        spec->mutable_key_or_handle()->set_port_id(port_id);
        spec->mutable_meta()->set_vrf_id(vrf_id);
        spec->set_port_speed(speed);
        spec->set_admin_state(admin_state);

        status = port_stub_->PortUpdate(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            if (port_handle_api_status(
                    rsp_msg.response(0).api_status(), port_id) == true) {
                std::cout << "Port update succeeded for port "
                          << port_id
                          << std::endl;
            }

            return 0;
        }

        std::cout << "Port update failed for port "
                  << port_id
                  << " , error = "
                  << rsp_msg.response(0).api_status()
                  << std::endl;
        return -1;
    }

    int port_get(uint32_t vrf_id, uint32_t port_id) {
        PortGetRequest      *req;
        PortGetRequestMsg   req_msg;
        PortGetResponseMsg  rsp_msg;
        ClientContext       context;
        Status              status;

        req = req_msg.add_request();
        req->mutable_key_or_handle()->set_port_id(port_id);
        req->mutable_meta()->set_vrf_id(vrf_id);

        // port get
        status = port_stub_->PortGet(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            if (port_handle_api_status(
                    rsp_msg.response(0).api_status(), port_id) == true) {
                std::cout << "Port Get succeeded for port "
                          << port_id << std::endl
                          << " Port oper status: "
                          << rsp_msg.response(0).status() << std::endl
                          << " Port type: "
                          << rsp_msg.response(0).spec().port_type() << std::endl
                          << " Admin state: "
                          << rsp_msg.response(0).spec().admin_state() << std::endl
                          << " Port speed: "
                          << rsp_msg.response(0).spec().port_speed() << std::endl
                          << " MAC ID: "
                          << rsp_msg.response(0).spec().mac_id() << std::endl
                          << " MAC channel: "
                          << rsp_msg.response(0).spec().mac_ch() << std::endl
                          << " Num lanes: "
                          << rsp_msg.response(0).spec().num_lanes() << std::endl;
            }

            return 0;
        }

        std::cout << "Port Get failed for port "
                  << port_id
                  << " , error = "
                  << rsp_msg.response(0).api_status()
                  << std::endl;
        return -1;
    }

    int port_delete(uint32_t vrf_id, uint32_t port_id) {
        PortDeleteRequest      *req;
        PortDeleteRequestMsg   req_msg;
        PortDeleteResponseMsg     rsp_msg;
        ClientContext          context;
        Status                 status;

        req = req_msg.add_request();
        req->mutable_key_or_handle()->set_port_id(port_id);
        req->mutable_meta()->set_vrf_id(vrf_id);

        // port get
        status = port_stub_->PortDelete(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            if (port_handle_api_status(
                    rsp_msg.response(0).api_status(), port_id) == true) {
                std::cout << "Port Delete succeeded for port "
                          << port_id << std::endl;
            }

            return 0;
        }

        std::cout << "Port Delete failed for port"
                  << port_id
                  << " , error = "
                  << rsp_msg.response(0).api_status()
                  << std::endl;
        return -1;
    }

    int slab_get() {
        ClientContext           context;
        SlabGetRequest          *req;
        SlabGetRequestMsg       req_msg;
        SlabGetResponse         rsp;
        SlabGetResponseMsg      rsp_msg;
        SlabSpec                spec;
        SlabStats               stats;
        Status                  status;
        int                     rsp_count, i;

        req = req_msg.add_request();
        req->set_id(5);

        std::cout << "Slab Get\n\n";
        status = debug_stub_->SlabGet(&context, req_msg, &rsp_msg);

        if (status.ok()) {
            rsp_count = rsp_msg.response_size();
            for (i = 0; i < rsp_count; i ++) {
                rsp = rsp_msg.response(i);
                if ((rsp.api_status() != types::API_STATUS_OK) || !rsp.has_spec() || !rsp.has_stats()) {
                    continue;
                }
                spec = rsp.spec();
                std::cout << "Slab Name" << spec.name() << "\n";

                stats = rsp.stats();
                std::cout << "Num elements in use: " << stats.num_elements_in_use() << "\n";
                std::cout << "Num allocs: " << stats.num_allocs() << "\n";
                std::cout << "Num frees: " << stats.num_frees() << "\n";
                std::cout << "Num alloc errors: " << stats.num_alloc_errors() << "\n";
                std::cout << "Num blocks: " << stats.num_blocks() << "\n\n";
            }
        }

        return 0;
    }

    int system_get() {
        ClientContext       context;
        Empty               request;
        SystemResponse      response;
        Status              status;
        Stats               stats;
        DropStats           drop_stats;
        DropStatsEntry      drop_entry;
        TableStats          table_stats;
        TableStatsEntry     table_entry;
        int                 count;

        std::cout << "System Get\n";
        status = system_stub_->SystemGet(&context, request, &response);
        if (status.ok() && (response.api_status() == types::API_STATUS_OK)) {
            // Get Statistics
            Stats stats = response.stats();

            // First print drop stats

            // Get DropStats
            DropStats drop_stats = stats.drop_stats();

            // Get count of DropEntries
            count = drop_stats.drop_entries_size();

            std::cout << "\nDrop Statistics:\n";

            for (int i = 0; i < count; i ++) {
                DropStatsEntry drop_entry = drop_stats.drop_entries(i);
                std::cout << "Stats " << i
                << ": " << drop_entry.drop_count() << "\n";
            }

            // Print table stats

            // Get TableStats
            TableStats table_stats = stats.table_stats();

            // Get count of TableStats
            count = table_stats.table_stats_size();

            std::cout << "\nTable Statistics:\n";

            for (int i = 0; i < count; i ++) {
                TableStatsEntry table_entry = table_stats.table_stats(i);
                std::cout << "Table type: " << table_entry.table_type() << "\n"
                    << "Table name: " << table_entry.table_name() << "\n"
                    << "Table size: " << table_entry.table_size() << "\n"
                    << "Overflow size: " << table_entry.overflow_table_size() << "\n"
                    << "Entries in use: " << table_entry.entries_in_use() << "\n"
                    << "Overflow entries in use: " << table_entry.overflow_entries_in_use() << "\n"
                    << "Num inserts: " << table_entry.num_inserts() << "\n"
                    << "Num insert errors: " << table_entry.num_insert_errors() << "\n"
                    << "Num deletes: " << table_entry.num_deletes() << "\n"
                    << "Num delete errors: " << table_entry.num_delete_errors() << "\n\n";
            }
        }

        return 0;
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
            assert(rsp_msg.response(0).api_status() == types::API_STATUS_OK);
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

    uint64_t lif_create(uint32_t lif_id, uint32_t type_num) {
        LifSpec              *spec;
        LifRequestMsg        req_msg;
        LifResponseMsg       rsp_msg;
        LifQStateMapEntry    *lif_qstate_map;
        QStateSetReq         *lif_qstate;
        ClientContext        context;
        Status               status;
        uint32_t             entries = 1;
        uint32_t             size = 64;

        spec = req_msg.add_request();
        spec->mutable_key_or_handle()->set_lif_id(lif_id);
        spec->set_admin_status(::intf::IF_STATUS_UP);
        spec->mutable_packet_filter()->set_receive_broadcast(true);
        lif_qstate_map = spec->add_lif_qstate_map();
        lif_qstate_map->set_type_num(type_num);
        lif_qstate_map->set_size(log2(size));
        lif_qstate_map->set_entries(log2(entries));
        lif_qstate_map->set_purpose(::intf::LIF_QUEUE_PURPOSE_TX);
        lif_qstate = spec->add_lif_qstate();
        lif_qstate->set_lif_handle(0);
        lif_qstate->set_type_num(type_num);
        lif_qstate->set_qid(0);
        /*
        lif_qstate->mutable_label()->set_handle("p4plus");
        req_spec.queue_state = bytes(EthTxQstate(host=1, total=1, enable=1));
        req_spec.label.prog_name = "txdma_stage0.bin";
        req_spec.label.label = "eth_tx_stage0"
        */
        status = intf_stub_->LifCreate(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            assert(rsp_msg.response(0).api_status() == types::API_STATUS_OK);
            std::cout << "Lif create succeeded, handle = "
                      << rsp_msg.response(0).status().lif_handle()
                      << std::endl;
            return rsp_msg.response(0).status().lif_handle();
        }
        std::cout << "Lif create failed, error = "
                  << rsp_msg.response(0).api_status() << std::endl;
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
        spec->mutable_if_enic_info()->set_pinned_uplink_if_handle(pinned_uplink_if_handle);
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

    uint64_t session_create(uint64_t session_id, uint64_t vrf_id, uint32_t sip, uint32_t dip,
                            ::types::IPProtocol proto, uint16_t sport, uint16_t dport,
                            ::session::FlowAction action) {
        SessionSpec               *spec;
        SessionRequestMsg         req_msg;
        SessionResponseMsg        rsp_msg;
        FlowSpec                  *flow;
        ClientContext             context;
        Status                    status;

        spec = req_msg.add_request();
        spec->mutable_meta()->set_vrf_id(vrf_id);
        spec->set_session_id(session_id);
        spec->set_conn_track_en(false);
        spec->set_tcp_ts_option(false);
        spec->set_tcp_sack_perm_option(false);
        spec->set_iflow_syn_ack_delta(0);
        flow = spec->mutable_initiator_flow();
        flow->mutable_flow_key()->mutable_v4_key()->set_sip(sip);
        flow->mutable_flow_key()->mutable_v4_key()->set_dip(dip);
        flow->mutable_flow_key()->mutable_v4_key()->set_ip_proto(proto);
        flow->mutable_flow_key()->mutable_v4_key()->mutable_tcp_udp()->set_sport(sport);
        flow->mutable_flow_key()->mutable_v4_key()->mutable_tcp_udp()->set_dport(dport);
        flow->mutable_flow_data()->mutable_flow_info()->set_flow_action(action);

        flow = spec->mutable_responder_flow();
        flow->mutable_flow_key()->mutable_v4_key()->set_sip(dip);
        flow->mutable_flow_key()->mutable_v4_key()->set_dip(sip);
        flow->mutable_flow_key()->mutable_v4_key()->set_ip_proto(proto);
        flow->mutable_flow_key()->mutable_v4_key()->mutable_tcp_udp()->set_sport(dport);
        flow->mutable_flow_key()->mutable_v4_key()->mutable_tcp_udp()->set_dport(sport);
        flow->mutable_flow_data()->mutable_flow_info()->set_flow_action(action);

        status = session_stub_->SessionCreate(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            assert(rsp_msg.response(0).api_status() == types::API_STATUS_OK);
            std::cout << "Session create succeeded, handle = "
                      << rsp_msg.response(0).status().session_handle()
                      << std::endl;
            return rsp_msg.response(0).status().session_handle();
        }
        std::cout << "Session create failed, error = "
                  << rsp_msg.response(0).api_status()
                  << std::endl;
        return 0;
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
        spec->mutable_vrf_key_handle()->set_vrf_id(vrf_id);
        spec->mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
        spec->mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_v4_addr(ip_pfx);
        spec->mutable_key_or_handle()->mutable_ip_prefix()->set_prefix_len(pfx_len);
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
                              ::types::L2SegmentType l2seg_type,
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
        spec->mutable_meta()->set_vrf_id(vrf_id);
        spec->mutable_key_or_handle()->set_segment_id(l2segment_id);
        nw_kh = spec->add_network_key_handle();
        nw_kh->set_nw_handle(nw_handle);
        spec->mutable_vrf_key_handle()->set_vrf_id(vrf_id);
        spec->set_segment_type(l2seg_type);
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

    void event_test(void) {
        ClientContext context;
        std::shared_ptr<grpc::ClientReaderWriter<EventRequest, EventResponse> > stream(
            event_stub_->EventListen(&context));

        std::thread writer([stream]() {
            std::vector<EventRequest>    events(6);

            events[0].set_event_id(event::EVENT_ID_PORT);
            events[0].set_event_operation(event::EVENT_OP_UNSUBSCRIBE);
            events[1].set_event_id(event::EVENT_ID_ENDPOINT);
            events[1].set_event_operation(event::EVENT_OP_UNSUBSCRIBE);
            events[2].set_event_id(event::EVENT_ID_ENDPOINT);
            events[2].set_event_operation(event::EVENT_OP_SUBSCRIBE);
            events[3].set_event_id(event::EVENT_ID_PORT);
            events[3].set_event_operation(event::EVENT_OP_SUBSCRIBE);
            events[4].set_event_id(event::EVENT_ID_PORT);
            events[4].set_event_operation(event::EVENT_OP_UNSUBSCRIBE);
            events[5].set_event_id(event::EVENT_ID_ENDPOINT);
            events[5].set_event_operation(event::EVENT_OP_UNSUBSCRIBE);
            for (const EventRequest& event : events) {
                std::cout << "Subscribing to event " << event.event_id() << std::endl;
                stream->Write(event);
            }
            stream->WritesDone();
        });

        EventResponse event_response;
        while (stream->Read(&event_response)) {
            std::cout << "Got event " << event_response.event_id() << std::endl;
        }
        writer.join();
        Status status = stream->Finish();
        if (!status.ok()) {
            std::cout << "Event test failed" << std::endl;
        }
    }

private:
    std::unique_ptr<Vrf::Stub> vrf_stub_;
    std::unique_ptr<L2Segment::Stub> l2seg_stub_;
    std::unique_ptr<Port::Stub> port_stub_;
    std::unique_ptr<Event::Stub> event_stub_;
    std::unique_ptr<System::Stub> system_stub_;
    std::unique_ptr<Debug::Stub> debug_stub_;
    std::unique_ptr<Interface::Stub> intf_stub_;
    std::unique_ptr<NwSecurity::Stub> sg_stub_;
    std::unique_ptr<Network::Stub> nw_stub_;
    std::unique_ptr<Endpoint::Stub> ep_stub_;
    std::unique_ptr<Session::Stub> session_stub_;
};

int port_enable(hal_client *hclient, int vrf_id, int port)
{
    std::cout <<  "*********** Port "
              << port
              << " enable and get"
              << " **********"
              << std::endl;

    hclient->port_update(
                     vrf_id, port,
                     ::port::PORT_SPEED_NONE, ::port::PORT_ADMIN_STATE_UP);

    hclient->port_get(vrf_id, port);

    return 0;
}

int ports_enable(hal_client *hclient, int vrf_id)
{
    int port = 0;

    for (port = 1; port <= 8; ++port) {
        port_enable (hclient, vrf_id, port);
    }

    return 0;
}

int port_get(hal_client *hclient, int vrf_id, int port)
{
    std::cout <<  "*********** Port "
              << port
              << " get"
              << " **********"
              << std::endl;

    hclient->port_get(vrf_id, port);

    return 0;
}

int ports_get(hal_client *hclient, int vrf_id)
{
    int port = 0;

    for (port = 1; port <= 8; ++port) {
        port_get (hclient, vrf_id, port);
    }

    return 0;
}

int port_test(hal_client *hclient, int vrf_id)
{
    int port = 1;

    // port 1: create and get
    std::cout <<  "*********** Port "
              << port
              << " create, enable and get"
              << " **********"
              << std::endl;
    hclient->port_create(vrf_id, port);
    hclient->port_get(vrf_id, port);

    // port 1: update speed and get
    std::cout <<  "*********** Port "
              << port
              << " update speed and get"
              << " **********"
              << std::endl;
    hclient->port_update(vrf_id, port,
                     ::port::PORT_SPEED_10G, ::port::PORT_ADMIN_STATE_NONE);
    hclient->port_get(vrf_id, port);

    // port 1: delete
    std::cout <<  "*********** Port "
              << port
              << " delete"
              << " **********"
              << std::endl;
    hclient->port_delete(vrf_id, port);

    port = 2;

    // port 2: create and get
    std::cout <<  "*********** Port "
              << port
              << " create, enable and get"
              << " **********"
              << std::endl;
    hclient->port_create(vrf_id, port);
    hclient->port_get(vrf_id, port);

    // port 2: update speed and get
    std::cout <<  "*********** Port "
              << port
              << " update speed and get"
              << " **********"
              << std::endl;
    hclient->port_update(vrf_id, port,
                     ::port::PORT_SPEED_10G, ::port::PORT_ADMIN_STATE_NONE);
    hclient->port_get(vrf_id, port);

    port = 1;

    // port 1: create and get
    std::cout <<  "*********** Port "
              << port
              << " create, enable and get"
              << " **********"
              << std::endl;
    hclient->port_create(vrf_id, port);
    hclient->port_get(vrf_id, port);

    // port 1: update speed and get
    std::cout <<  "*********** Port "
              << port
              << " update speed and get"
              << " **********"
              << std::endl;
    hclient->port_update(vrf_id, port,
                     ::port::PORT_SPEED_10G, ::port::PORT_ADMIN_STATE_NONE);
    hclient->port_get(vrf_id, port);

    // port 1: delete and get
    std::cout <<  "*********** Port "
              << port
              << " delete and get"
              << " **********"
              << std::endl;
    hclient->port_delete(vrf_id, port);
    hclient->port_get(vrf_id, port);

    port = 2;

    // port 2: disable and get
    std::cout <<  "*********** Port "
              << port
              << " disable and get"
              << " **********"
              << std::endl;
    hclient->port_update(vrf_id, port,
                     ::port::PORT_SPEED_NONE, ::port::PORT_ADMIN_STATE_DOWN);
    hclient->port_get(vrf_id, port);

    // port 2: create and get
    std::cout <<  "*********** Port "
              << port
              << " create, enable and get"
              << " **********"
              << std::endl;
    hclient->port_create(vrf_id, port);
    hclient->port_get(vrf_id, port);

    // port 2: delete and get
    std::cout <<  "*********** Port "
              << port
              << " delete and get"
              << " **********"
              << std::endl;
    hclient->port_delete(vrf_id, port);
    hclient->port_get(vrf_id, port);

    return 0;
}

// main test driver
int
main (int argc, char** argv)
{
    hal_client hclient(grpc::CreateChannel(hal_svc_endpoint_,
                                           grpc::InsecureChannelCredentials()));

    uint64_t    vrf_handle, l2seg_handle, native_l2seg_handle, sg_handle;
    uint64_t    nw1_handle, nw2_handle, uplink_if_handle; //, session_handle;
    uint64_t    lif_handle, enic_if_handle;
    uint64_t    vrf_id = 1, l2seg_id = 1, sg_id = 1, if_id = 1, nw_id = 1;
    uint64_t    lif_id = 100;
    uint64_t    enic_if_id = 200;
    EncapInfo   l2seg_encap;

#if 0
    //port_test(&hclient, vrf_id);
    //ports_enable(&hclient, vrf_id);
    ports_get(&hclient, vrf_id);
    return 0;

    // delete a non-existent vrf
    hclient.vrf_delete_by_id(1);
#endif

    // create a vrf and perform GETs
    vrf_handle = hclient.vrf_create(vrf_id);
    assert(vrf_handle != 0);
    assert(hclient.vrf_get_by_handle(vrf_handle) != 0);
    assert(hclient.vrf_get_by_id(vrf_id) != 0);
    //hclient.vrf_delete_by_handle(hal_handle);
    //assert(hclient.vrf_get_by_id(1) == 0);   // should fail

    // recreate the vrf
    //vrf_handle = hclient.vrf_create(vrf_id);
    //assert(vrf_handle != 0);

    // Get slab statistics
    //hclient.slab_get();
  
    // create a security group
    sg_handle = hclient.sg_create(sg_id);
    assert(sg_handle != 0);

    // create network objects
    nw1_handle = hclient.nw_create(nw_id, vrf_id, 0x0a0a0100, 24, 0x020a0a0101, sg_id);
    assert(nw1_handle != 0);
    nw_id++;
    nw2_handle = hclient.nw_create(nw_id, vrf_id, 0x0a0a0200, 24, 0x020a0a0201, sg_id);
    assert(nw2_handle != 0);

    // create L2 segments
    l2seg_encap.set_encap_type(::types::ENCAP_TYPE_DOT1Q);
    l2seg_encap.set_encap_value(100);
    native_l2seg_handle = l2seg_handle =
        hclient.l2segment_create(vrf_id, l2seg_id, nw1_handle,
                                 ::types::L2_SEGMENT_TYPE_TENANT,
                                 ::l2segment::BROADCAST_FWD_POLICY_FLOOD,
                                 ::l2segment::MULTICAST_FWD_POLICY_FLOOD,
                                 l2seg_encap);
    assert(l2seg_handle != 0);
    // create uplinks with this L2 seg as native L2 seg
    uplink_if_handle = hclient.uplinks_create(if_id, 4, l2seg_id);
    // bringup this L2seg on all uplinks
    hclient.add_l2seg_on_uplinks(if_id, 4, l2seg_id);

    l2seg_encap.set_encap_type(::types::ENCAP_TYPE_DOT1Q);
    l2seg_encap.set_encap_value(101);
    l2seg_handle =
        hclient.l2segment_create(vrf_id, l2seg_id + 1, nw2_handle,
                                 ::types::L2_SEGMENT_TYPE_TENANT,
                                 ::l2segment::BROADCAST_FWD_POLICY_FLOOD,
                                 ::l2segment::MULTICAST_FWD_POLICY_FLOOD,
                                 l2seg_encap);
    assert(l2seg_handle != 0);
    // bringup this L2seg on all uplinks
    hclient.add_l2seg_on_uplinks(if_id, 4, l2seg_id + 1);

    // create remote endpoints
    hclient.ep_create(vrf_id, l2seg_id, if_id, sg_id, 0x020a0a0102, 0x0a0a0102);
    hclient.ep_create(vrf_id, l2seg_id + 1, if_id + 1, sg_id, 0x020a0a0202, 0x0a0a0202);

#if 0
    // NOTE: uncomment this in smart nic mode
    // create a session
    session_handle = hclient.session_create(1, vrf_id, 0x0a0a0102, 0x0a0a0202,
                                            ::types::IPProtocol::IPPROTO_UDP,
                                            10000, 10001,
                                            ::session::FlowAction::FLOW_ACTION_ALLOW);
    assert(session_handle != 0);
#endif

    // create a lif
    lif_handle = hclient.lif_create(lif_id, 1);
    assert(lif_handle != 0);

    // create a ENIC interface
    enic_if_handle = hclient.enic_if_create(enic_if_id, lif_id,
                                            uplink_if_handle,  // pinned uplink
                                            native_l2seg_handle,
                                            l2seg_id + 1);
    assert(enic_if_handle != 0);

    // create EP with this ENIC
    hclient.ep_create(vrf_id, l2seg_id, enic_if_id, sg_id, 0x020a0a0103, 0x0a0a0103);

    //hclient.vrf_delete_by_id(1);

    // Get system statistics
    //hclient.system_get();

    // subscribe and listen to HAL events
    //hclient.event_test();
    return 0;
}
