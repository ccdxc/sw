#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <thread>
#include <iostream>
#include <fstream>
#include <math.h>
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
#include "gen/proto/gft.grpc.pb.h"
#include "gen/proto/telemetry.grpc.pb.h"
#include "gen/proto/qos.grpc.pb.h"
#include "nic/sdk/include/sdk/pal.hpp"
#include "nic/sdk/include/sdk/types.hpp"
#include "gen/proto/proxy.grpc.pb.h"
#include "gen/proto/tcp_proxy_cb.grpc.pb.h"
#include "gen/proto/crypto_apis.grpc.pb.h"
#include "gen/proto/tcp_proxy.grpc.pb.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


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
using endpoint::EndpointDeleteRequestMsg;
using endpoint::EndpointDeleteRequest;
using endpoint::EndpointDeleteResponseMsg;

using nwsec::NwSecurity;
using nwsec::SecurityGroupSpec;
using nwsec::SecurityGroupRequestMsg;
using nwsec::SecurityGroupResponseMsg;
using nwsec::SecurityProfileSpec;
using nwsec::SecurityProfileRequestMsg;
using nwsec::SecurityProfileResponseMsg;
using nwsec::SecurityRule;
using nwsec::SecurityPolicySpec;
using nwsec::SecurityPolicyRequestMsg;
using nwsec::SecurityPolicyResponseMsg;
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
using session::SessionDeleteRequestMsg;
using session::SessionDeleteRequest;
using session::SessionDeleteResponseMsg;
using session::SessionGetRequestMsg;
using session::SessionGetRequest;
using session::SessionGetResponseMsg;

using intf::Interface;
using intf::InterfaceSpec;
using intf::InterfaceRequestMsg;
using intf::InterfaceResponse;
using intf::InterfaceResponseMsg;
using intf::InterfaceL2SegmentSpec;
using intf::InterfaceL2SegmentRequestMsg;
using intf::InterfaceL2SegmentResponseMsg;
using intf::InterfaceL2SegmentResponse;
using intf::InterfaceDeleteRequest;
using intf::InterfaceDeleteRequestMsg;
using intf::InterfaceDeleteResponseMsg;
using intf::LifSpec;
using intf::LifGetRequestMsg;
using intf::LifGetResponseMsg;
using intf::LifRequestMsg;
using intf::LifResponseMsg;
using intf::LifQStateMapEntry;
using intf::QStateSetReq;

using qos::QOS;
using qos::QosClassSpec;
using qos::QosClassGetRequestMsg;
using qos::QosClassGetResponseMsg;
using qos::QosClassRequestMsg;
using qos::QosClassResponseMsg;
using kh::QosGroup;

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

using sys::ApiStatsResponse;
using sys::ApiStatsEntry;
using sys::System;
using sys::SystemResponse;
using sys::Stats;
using sys::DropStats;
using sys::DropStatsEntry;
using sys::EgressDropStats;
using sys::EgressDropStatsEntry;
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

using gft::GftExactMatchFlowEntryRequestMsg;
using gft::GftExactMatchFlowEntrySpec;
using gft::GftHeaderGroupExactMatch;
using gft::GftHeaderGroupTransposition;

using telemetry::Telemetry;
using telemetry::MirrorSessionSpec;
using telemetry::MirrorSessionStatus;
using telemetry::MirrorSessionResponse;
using telemetry::MirrorSessionRequestMsg;
using telemetry::MirrorSessionResponseMsg;
using telemetry::MirrorSessionDeleteRequest;
using telemetry::MirrorSessionDeleteResponse;
using telemetry::MirrorSessionDeleteRequestMsg;
using telemetry::MirrorSessionDeleteResponseMsg;
using telemetry::MirrorSessionGetRequest;
using telemetry::MirrorSessionGetRequestMsg;
using telemetry::MirrorSessionGetResponse;
using telemetry::MirrorSessionGetResponseMsg;

using telemetry::CollectorSpec;
using telemetry::CollectorStatus;
using telemetry::CollectorResponse;
using telemetry::CollectorRequestMsg;
using telemetry::CollectorResponseMsg;
using telemetry::CollectorDeleteRequest;
using telemetry::CollectorDeleteResponse;
using telemetry::CollectorDeleteRequestMsg;
using telemetry::CollectorDeleteResponseMsg;
using telemetry::CollectorGetRequest;
using telemetry::CollectorGetRequestMsg;
using telemetry::CollectorGetResponse;
using telemetry::CollectorGetResponseMsg;

using telemetry::DropMonitorRuleSpec;
using telemetry::DropMonitorRuleStatus;
using telemetry::DropMonitorRuleResponse;
using telemetry::DropMonitorRuleRequestMsg;
using telemetry::DropMonitorRuleResponseMsg;
using telemetry::DropMonitorRuleDeleteRequest;
using telemetry::DropMonitorRuleDeleteResponse;
using telemetry::DropMonitorRuleDeleteRequestMsg;
using telemetry::DropMonitorRuleDeleteResponseMsg;
using telemetry::DropMonitorRuleGetRequest;
using telemetry::DropMonitorRuleGetRequestMsg;
using telemetry::DropMonitorRuleGetResponse;
using telemetry::DropMonitorRuleGetResponseMsg;

using proxy::Proxy;
using proxy::ProxySpec;
using proxy::ProxyRequestMsg;
using proxy::ProxyResponseMsg;
using proxy::ProxyFlowConfigRequestMsg;
using proxy::ProxyFlowConfigRequest;
using proxy::ProxyGlobalCfgRequest;
using proxy::ProxyGlobalCfgRequestMsg;
using proxy::ProxyGlobalCfgResponseMsg;

using tcpcb::TcpCb;
using tcpcb::TcpCbGetRequest;
using tcpcb::TcpCbGetRequestMsg;
using tcpcb::TcpCbGetResponseMsg;
using tcpcb::TcpCbGetResponse;
using tcpcb::TcpCbStats;
using tcpcb::TcpCbSpec;

using cryptoapis::CryptoApiType;
using cryptoapis::CryptoApis;
using cryptoapis::CryptoApiRequestMsg;
using cryptoapis::CryptoApiRequest;
using cryptoapis::CryptoApiResponseMsg;
using cryptoapis::CryptoApiResponse;
using grpc::ClientContext;


using tcp_proxy::TcpProxy;
using tcp_proxy::TcpProxyRuleRequestMsg;
using tcp_proxy::TcpProxyRuleSpec;
using tcp_proxy::TcpProxyRuleResponseMsg;
using tcp_proxy::TcpProxyRuleMatchSpec;
using types::IPAddressObj;

std::string  hal_svc_endpoint_     = "localhost:50054";
std::string  linkmgr_svc_endpoint_ = "localhost:50053";

port::PortOperStatus port_oper_status = port::PORT_OPER_STATUS_NONE;
port::PortType       port_type        = port::PORT_TYPE_NONE;
port::PortAdminState port_admin_state = port::PORT_ADMIN_STATE_NONE;
port::PortSpeed      port_speed       = port::PORT_SPEED_NONE;

static inline uint64_t
min(uint64_t a, uint64_t b) {
    return a < b ? a : b;
}

#define MIRROR_SESSION_ID       1
#define DROP_MONITOR_RULE_ID    1

class hal_client {
public:
    hal_client(std::shared_ptr<Channel> channel) : vrf_stub_(Vrf::NewStub(channel)),
    l2seg_stub_(L2Segment::NewStub(channel)), port_stub_(Port::NewStub(channel)),
    event_stub_(Event::NewStub(channel)), system_stub_(System::NewStub(channel)),
    debug_stub_(Debug::NewStub(channel)), intf_stub_(Interface::NewStub(channel)),
    sg_stub_(NwSecurity::NewStub(channel)), nw_stub_(Network::NewStub(channel)),
    ep_stub_(Endpoint::NewStub(channel)), session_stub_(Session::NewStub(channel)),
    telemetry_stub_(Telemetry::NewStub(channel)),
    proxy_stub_(Proxy::NewStub(channel)),
    tcpcb_stub_(TcpCb::NewStub(channel)),
    crypto_apis_stub_(CryptoApis::NewStub(channel)),
    tcp_proxy_stub_(TcpProxy::NewStub(channel)),
    qos_stub_(QOS::NewStub(channel)) {}

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
            } else {
                return -1;
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
            } else {
                return -1;
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

    int port_get(uint32_t vrf_id, uint32_t port_id, bool compare=false) {
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
                          << rsp_msg.response(0).status().oper_status() << std::endl
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
            } else {
                return -1;
            }

            if (compare == true) {
                if (port_oper_status != port::PORT_OPER_STATUS_NONE) {
                    assert(rsp_msg.response(0).status().oper_status()
                                                == port_oper_status);
                }
                if (port_type != port::PORT_TYPE_NONE) {
                    assert(rsp_msg.response(0).spec().port_type() == port_type);
                }
                if (port_admin_state != port::PORT_ADMIN_STATE_NONE) {
                    assert(rsp_msg.response(0).spec().admin_state() == port_admin_state);
                }
                if (port_speed != port::PORT_SPEED_NONE) {
                    assert(rsp_msg.response(0).spec().port_speed() == port_speed);
                }
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
            } else {
                return -1;
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

    int api_stats_get() {
        ClientContext       context;
        Empty               request;
        ApiStatsResponse    response;
        int                 count;
        Status              status;

        std::cout << "API Stats Get" << std::endl;

        status = system_stub_->ApiStatsGet(&context, request, &response);
        if (status.ok()) {
            count = response.api_entries_size();

            std::cout << "\nAPI Statistics:\n";

            for (int i = 0; i < count; i ++) {
                ApiStatsEntry entry = response.api_entries(i);
                std::cout << "Stats " << entry.api_type() << ": "
                << entry.num_api_call() << "\t"
                << entry.num_api_success() << "\t"
                << entry.num_api_fail() << "\n";
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

            // Get EgressDropStats
            EgressDropStats egress_drop_stats = stats.egress_drop_stats();

            // Get count of EgressDropEntries
            count = egress_drop_stats.drop_entries_size();

            std::cout << "\nEgress Drop Statistics:\n";

            for (int i = 0; i < count; i ++) {
                EgressDropStatsEntry drop_entry = egress_drop_stats.drop_entries(i);
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

    uint64_t security_policy_create(uint64_t policy_id, uint64_t vrf_id) {
        SecurityPolicySpec              *spec;
        SecurityPolicyRequestMsg        req_msg;
        SecurityPolicyResponseMsg       rsp_msg;
        ClientContext                   context;
        Status                          status;
        SecurityRule                    *rule_spec;

        spec = req_msg.add_request();

        spec->mutable_policy_key_or_handle()->mutable_security_policy_key()->set_security_policy_id(policy_id);
        spec->mutable_policy_key_or_handle()->mutable_security_policy_key()->mutable_vrf_id_or_handle()->set_vrf_id(vrf_id);

        rule_spec = spec->add_rule();

        // Create nwsec
        rule_spec->set_rule_id(1);
        rule_spec->mutable_action()->set_sec_action(nwsec::SecurityAction::SECURITY_RULE_ACTION_ALLOW);

        types::RuleMatch *match = rule_spec->mutable_match();
        types::IPAddressObj *dst_addr = match->add_dst_address();
        dst_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
        dst_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_v4_addr(0xAABBCC00);

        types::IPAddressObj *src_addr = match->add_src_address();
        src_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
        src_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_v4_addr(0x11223300);

        types::RuleMatch_AppMatch *app = match->mutable_app_match();
        types::L4PortRange *port_range = app->mutable_port_info()->add_dst_port_range();
        port_range->set_port_low(1000);
        port_range->set_port_high(2000);
        
        types::L4PortRange *src_port_range = app->mutable_port_info()->add_src_port_range();
        src_port_range->set_port_low(100);
        src_port_range->set_port_high(200);

        status = sg_stub_->SecurityPolicyCreate(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            assert((rsp_msg.response(0).api_status() == types::API_STATUS_OK) ||
                   (rsp_msg.response(0).api_status() == types::API_STATUS_EXISTS_ALREADY));
            std::cout << "Security Policy create succeeded, handle = "
                      << rsp_msg.response(0).policy_status().security_policy_handle()
                      << std::endl;
            return rsp_msg.response(0).policy_status().security_policy_handle();
        }
        std::cout << "Security Policy create failed, error = "
                  << rsp_msg.response(0).api_status()
                  << std::endl;
        return 0;
    }

    uint64_t security_profile_create(uint64_t prof_id) {
        SecurityProfileSpec           *spec;
        SecurityProfileRequestMsg     req_msg;
        SecurityProfileResponseMsg    rsp_msg;
        ClientContext     context;
        Status            status;

        spec = req_msg.add_request();
        spec->mutable_key_or_handle()->set_profile_id(prof_id);

        status = sg_stub_->SecurityProfileCreate(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            assert((rsp_msg.response(0).api_status() == types::API_STATUS_OK) ||
                   (rsp_msg.response(0).api_status() == types::API_STATUS_EXISTS_ALREADY));
            std::cout << "Security Profile create succeeded, handle = "
                      << rsp_msg.response(0).profile_status().profile_handle()
                      << std::endl;
            return rsp_msg.response(0).profile_status().profile_handle();
        }
        std::cout << "Security Profile create failed, error = "
                  << rsp_msg.response(0).api_status()
                  << std::endl;
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

    void lif_get_all() {
        LifGetRequestMsg    req_msg;
        LifGetResponseMsg   rsp_msg;
        ClientContext       context;
        Status              status;
        uint32_t            num_rsp;

        req_msg.add_request();
        status = intf_stub_->LifGet(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            num_rsp = rsp_msg.response_size();
            std::cout << "Num of Rsps: " << num_rsp << std::endl;
            for (uint32_t i = 0; i < num_rsp; i ++) {
                std::cout << "Lif ID = "
                          << rsp_msg.response(i).spec().key_or_handle().lif_id()
                          << std::endl;
            }
        } else {
            std::cout << "Lif Get Failed" << std::endl;
        }
    }

    void lif_policer_update(uint32_t lif_id, bool is_rx, bool pps, uint64_t rate, uint64_t burst)
    {
        LifSpec           *spec;
        LifGetRequestMsg  get_req_msg;
        LifGetResponseMsg get_rsp_msg;
        LifRequestMsg     req_msg;
        LifResponseMsg    rsp_msg;
        ClientContext     get_context;
        ClientContext     context;
        Status            status;

        std::cout << "Policer update lif " << lif_id
                                    << "is_rx" << is_rx
                                    << "pps " << pps
                                    << "rate " << rate
                                    << "burst " << burst
                                    << std::endl;

        get_req_msg.add_request()->mutable_key_or_handle()->set_lif_id(lif_id);
        status = intf_stub_->LifGet(&get_context, get_req_msg, &get_rsp_msg);
        if (!status.ok() || (get_rsp_msg.response_size() != 1) || (get_rsp_msg.response(0).api_status() != types::API_STATUS_OK)) {
            std::cout << "Lif Get Failed" << std::endl;
            return;
        }

        std::cout << "Lif Get Succeeded " << std::endl;

        auto get_rsp = get_rsp_msg.response(0);

        std::cout << "Lif Rx policer stats" << std::endl;
        std::cout << "permitted_packets " << 
            get_rsp.stats().rx_stats().policer_stats().permitted_packets() << std::endl;
        std::cout << "permitted_bytes " << 
            get_rsp.stats().rx_stats().policer_stats().permitted_bytes() << std::endl;
        std::cout << "dropped_packets " << 
            get_rsp.stats().rx_stats().policer_stats().dropped_packets() << std::endl;
        std::cout << "dropped_bytes " << 
            get_rsp.stats().rx_stats().policer_stats().dropped_bytes() << std::endl;

        spec = req_msg.add_request();
        *spec = get_rsp.spec();

        if (pps) {
            if (is_rx) {
                spec->mutable_rx_policer()->mutable_pps_policer()->set_packets_per_sec(rate);
                spec->mutable_rx_policer()->mutable_pps_policer()->set_burst_packets(burst);
            } else {
                spec->mutable_tx_policer()->mutable_pps_policer()->set_packets_per_sec(rate);
                spec->mutable_tx_policer()->mutable_pps_policer()->set_burst_packets(burst);
            }        
        } else {
            if (is_rx) {
                spec->mutable_rx_policer()->mutable_bps_policer()->set_bytes_per_sec(rate);
                spec->mutable_rx_policer()->mutable_bps_policer()->set_burst_bytes(burst);
            } else {
                spec->mutable_tx_policer()->mutable_bps_policer()->set_bytes_per_sec(rate);
                spec->mutable_tx_policer()->mutable_bps_policer()->set_burst_bytes(burst);
            }
        }

        std::cout << "Calling lif update" << std::endl;
        status = intf_stub_->LifUpdate(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            assert(rsp_msg.response(0).api_status() == types::API_STATUS_OK);
            std::cout << "Lif update succeeded, handle = "
                      << rsp_msg.response(0).status().lif_handle()
                      << std::endl;

            std::cout << "hw_lif_id: " << rsp_msg.response(0).status().hw_lif_id() << std::endl;
            for (int i = 0; i < rsp_msg.response(0).qstate().size(); i++) {
                auto & qstate = rsp_msg.response(0).qstate()[i];
                std::cout << "type_num: " << qstate.type_num()
                          << " qstate: " << std::hex << qstate.addr()
                          << std::endl;
            }
            return;
        }
        std::cout << "Lif update failed " << std::endl;
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

    uint64_t cpu_if_create(uint32_t lif_id) {
        InterfaceSpec           *spec;
        InterfaceRequestMsg     req_msg;
        InterfaceResponseMsg    rsp_msg;
        ClientContext           context;
        Status                  status;

        spec = req_msg.add_request();
        spec->mutable_key_or_handle()->set_interface_id(lif_id);
        spec->set_type(::intf::IfType::IF_TYPE_CPU);
        spec->set_admin_status(::intf::IfStatus::IF_STATUS_UP);
        spec->mutable_if_cpu_info()->mutable_lif_key_or_handle()->set_lif_id(lif_id);
        status = intf_stub_->InterfaceCreate(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            assert(rsp_msg.response(0).api_status() == types::API_STATUS_OK);
            std::cout << "CPU if create succeeded, handle = "
                      << rsp_msg.response(0).status().if_handle()
                      << std::endl;
            return rsp_msg.response(0).status().if_handle();
        }
        std::cout << "CPU if create failed, error = "
                  << rsp_msg.response(0).api_status() << std::endl;
        return 0;
    }

    uint64_t enic_if_delete(uint32_t enic_if_id) {
        InterfaceDeleteRequest     *req;
        InterfaceDeleteRequestMsg  req_msg;
        InterfaceDeleteResponseMsg rsp_msg;
        ClientContext           context;
        Status                  status;

        req = req_msg.add_request();
        req->mutable_key_or_handle()->set_interface_id(enic_if_id);

        status = intf_stub_->InterfaceDelete(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            assert(rsp_msg.response(0).api_status() == types::API_STATUS_OK);
            std::cout << "ENIC if delete succeeded, id = "
                      << enic_if_id
                      << std::endl;
            return 0;
        }
        std::cout << "ENIC if delete failed, error = "
                  << rsp_msg.response(0).api_status() << std::endl;
        return -1;
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

    int ep_delete(uint64_t vrf_id, uint64_t l2seg_id, uint64_t mac_addr) {

        EndpointDeleteRequestMsg  req_msg;
        EndpointDeleteResponseMsg rsp_msg;
        EndpointDeleteRequest     *req;
        ClientContext             context;
        Status                    status;

        req = req_msg.add_request();

        req->mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_segment_id(l2seg_id);
        req->mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(mac_addr);
        req->mutable_vrf_key_handle()->set_vrf_id(vrf_id);

        status = ep_stub_->EndpointDelete(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            assert(rsp_msg.response(0).api_status() == types::API_STATUS_OK);
            std::cout << "Endpoint delete succeeded."
                      << " vrf: "       << vrf_id
                      << ", l2seg_id: " << l2seg_id
                      << ", mac_addr: " << mac_addr
                      << std::endl;
            return 0;
        }

        std::cout << "Endpoint delete failed for: "
                  << " vrf: "       << vrf_id
                  << ", l2seg_id: " << l2seg_id
                  << ", mac_addr: " << mac_addr
                  << ", error = "
                  << rsp_msg.response(0).api_status()
                  << std::endl;
        return -1;
    }

    uint64_t ep_create(uint64_t vrf_id, uint64_t l2seg_id,
                       uint64_t if_id, uint64_t sg_id,
                       uint64_t mac_addr, uint32_t *ip_addr, int num_ips) {
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
            for (int i = 0; i < num_ips; ++i) {
                std::cout << "ep_create with ip: "
                          << std::hex << ip_addr[i] << std::dec
                          << std::endl;
                ip = spec->mutable_endpoint_attrs()->add_ip_address();
                ip->set_ip_af(types::IPAddressFamily::IP_AF_INET);
                ip->set_v4_addr(ip_addr[i]);
            }
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

    int session_delete_all (uint64_t session_handle) {
        SessionDeleteRequestMsg  req_msg;
        SessionDeleteResponseMsg rsp_msg;
        ClientContext            context;
        Status                   status;

        status = session_stub_->SessionDelete(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            assert(rsp_msg.response(0).api_status() == types::API_STATUS_OK);
            std::cout << "Session delete succeeded, handle = "
                      << session_handle
                      << std::endl;
            return 0;
        }
        std::cout << "Session delete failed, error = "
                  << rsp_msg.response(0).api_status()
                  << std::endl;
        return -1;
    }

    int session_delete (uint64_t session_handle) {
        SessionDeleteRequestMsg  req_msg;
        SessionDeleteRequest     *req;
        SessionDeleteResponseMsg rsp_msg;
        ClientContext            context;
        Status                   status;

        req = req_msg.add_request();

        if (session_handle != 0) {
            req->set_session_handle(session_handle);
        }

        status = session_stub_->SessionDelete(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            assert(rsp_msg.response(0).api_status() == types::API_STATUS_OK);
            std::cout << "Session delete succeeded, handle = "
                      << session_handle
                      << std::endl;
            return 0;
        }
        std::cout << "Session delete failed, error = "
                  << rsp_msg.response(0).api_status()
                  << std::endl;
        return -1;
    }

    int session_get (uint64_t session_handle,
                     SessionGetResponseMsg *rsp_msg) {
        SessionGetRequestMsg req_msg;
        // SessionGetRequest    *req;
        ClientContext        context;
        Status               status;

        // empty request
        req_msg.add_request();

        status = session_stub_->SessionGet(&context, req_msg, rsp_msg);
        if (status.ok()) {
            assert(rsp_msg->response(0).api_status() == types::API_STATUS_OK);
            std::cout << "Session get succeeded, handle = "
                      << session_handle
                      << std::endl;

            return 0;
        }
        std::cout << "Session get failed, error = "
                  << rsp_msg->response(0).api_status()
                  << std::endl;
        return -1;
    }

    int session_get_and_delete (uint64_t session_handle) {
        int ret = 0;
        SessionGetResponseMsg rsp_msg;

        ret = session_get (session_handle, &rsp_msg);

        if (ret == 0) {
            for (int i = 0; i < rsp_msg.response_size(); ++i) {
                session_delete(rsp_msg.response(i).status().session_handle());
            }
        }

        return ret;
    }

    int flow_key_populate(uint64_t vrf_id, uint32_t sip, uint32_t dip,
                         ::types::IPProtocol proto, uint16_t sport, uint16_t dport,
                         session::FlowKey *flow_key) {
        flow_key->mutable_v4_key()->set_sip(sip);
        flow_key->mutable_v4_key()->set_dip(dip);
        flow_key->mutable_v4_key()->set_ip_proto(proto);
        flow_key->mutable_v4_key()->mutable_tcp_udp()->set_sport(sport);
        flow_key->mutable_v4_key()->mutable_tcp_udp()->set_dport(dport);

        return 0;
    }

    int session_populate(uint64_t session_id, uint64_t vrf_id, uint32_t sip, uint32_t dip,
                         ::types::IPProtocol proto, uint16_t sport, uint16_t dport,
                         ::session::NatType nat_type,
                         uint32_t nat_sip, uint32_t nat_dip,
                         uint16_t nat_sport, uint16_t nat_dport,
                         ::session::FlowAction action,
                         uint32_t ing_mirror_session_id,
                         SessionRequestMsg &req_msg) {
        SessionSpec  *spec;
        FlowSpec     *flow;

        spec = req_msg.add_request();
        spec->mutable_meta()->set_vrf_id(vrf_id);
        spec->set_session_id(session_id);
        spec->set_conn_track_en(false);
        spec->set_tcp_ts_option(false);
        spec->set_tcp_sack_perm_option(false);

        // create initiator flow
        flow = spec->mutable_initiator_flow();
        flow->mutable_flow_key()->mutable_v4_key()->set_sip(sip);
        flow->mutable_flow_key()->mutable_v4_key()->set_dip(dip);
        flow->mutable_flow_key()->mutable_v4_key()->set_ip_proto(proto);
        flow->mutable_flow_key()->mutable_v4_key()->mutable_tcp_udp()->set_sport(sport);
        flow->mutable_flow_key()->mutable_v4_key()->mutable_tcp_udp()->set_dport(dport);
        flow->mutable_flow_data()->mutable_flow_info()->set_flow_action(action);
        flow->mutable_flow_data()->mutable_conn_track_info()->set_iflow_syn_ack_delta(0);
        if (ing_mirror_session_id) {
            auto msess = flow->mutable_flow_data()->mutable_flow_info()->add_ing_mirror_sessions();
            msess->set_mirrorsession_id(ing_mirror_session_id);
        }
        switch (nat_type) {
        case ::session::NAT_TYPE_NONE:
            break;
        case ::session::NAT_TYPE_TWICE_NAT:
            flow->mutable_flow_data()->mutable_flow_info()->set_nat_type(nat_type);
            flow->mutable_flow_data()->mutable_flow_info()->mutable_nat_sip()->set_ip_af(::types::IP_AF_INET);
            flow->mutable_flow_data()->mutable_flow_info()->mutable_nat_sip()->set_v4_addr(nat_sip);
            flow->mutable_flow_data()->mutable_flow_info()->mutable_nat_dip()->set_ip_af(::types::IP_AF_INET);
            flow->mutable_flow_data()->mutable_flow_info()->mutable_nat_dip()->set_v4_addr(nat_dip);
            flow->mutable_flow_data()->mutable_flow_info()->set_nat_sport(nat_sport);
            flow->mutable_flow_data()->mutable_flow_info()->set_nat_dport(nat_dport);
            break;
        default:
            std::cout << "Unsupported NAT Type" << std::endl;
            return 0;
            break;
        }

        // create responder flow
        flow = spec->mutable_responder_flow();
        flow->mutable_flow_key()->mutable_v4_key()->set_ip_proto(proto);
        flow->mutable_flow_data()->mutable_flow_info()->set_flow_action(action);
        if (ing_mirror_session_id) {
            auto msess = flow->mutable_flow_data()->mutable_flow_info()->add_ing_mirror_sessions();
            msess->set_mirrorsession_id(ing_mirror_session_id);
        }
        switch (nat_type) {
        case ::session::NAT_TYPE_NONE:
            flow->mutable_flow_key()->mutable_v4_key()->set_sip(dip);
            flow->mutable_flow_key()->mutable_v4_key()->set_dip(sip);
            flow->mutable_flow_key()->mutable_v4_key()->mutable_tcp_udp()->set_sport(dport);
            flow->mutable_flow_key()->mutable_v4_key()->mutable_tcp_udp()->set_dport(sport);
            break;
        case ::session::NAT_TYPE_TWICE_NAT:
            // fix the key
            flow->mutable_flow_data()->mutable_flow_info()->set_nat_type(nat_type);
            flow->mutable_flow_key()->mutable_v4_key()->set_sip(nat_dip);
            flow->mutable_flow_key()->mutable_v4_key()->set_dip(nat_sip);
            flow->mutable_flow_key()->mutable_v4_key()->mutable_tcp_udp()->set_sport(nat_dport);
            flow->mutable_flow_key()->mutable_v4_key()->mutable_tcp_udp()->set_dport(nat_sport);

            flow->mutable_flow_data()->mutable_flow_info()->mutable_nat_sip()->set_ip_af(::types::IP_AF_INET);
            flow->mutable_flow_data()->mutable_flow_info()->mutable_nat_sip()->set_v4_addr(dip);

            flow->mutable_flow_data()->mutable_flow_info()->mutable_nat_dip()->set_ip_af(::types::IP_AF_INET);
            flow->mutable_flow_data()->mutable_flow_info()->mutable_nat_dip()->set_v4_addr(sip);

            flow->mutable_flow_data()->mutable_flow_info()->set_nat_sport(dport);
            flow->mutable_flow_data()->mutable_flow_info()->set_nat_dport(sport);
            break;
        default:
            std::cout << "Unsupported NAT Type" << std::endl;
            return 0;
            break;
        }
        return 0;
    }

    int session_create(uint64_t session_id, uint64_t vrf_id,
                       uint32_t sip, uint32_t dip,
                       ::types::IPProtocol proto,
                       uint16_t sport,
                       uint16_t dport_start, uint64_t num_dports,
                       ::session::NatType nat_type,
                       uint32_t nat_sip, uint32_t nat_dip,
                       uint16_t nat_sport, uint16_t nat_dport,
                       ::session::FlowAction action,
                       uint32_t ing_mirror_session_id) {
        uint64_t max_batch_req = 500;
        if (num_dports < max_batch_req) {
            max_batch_req = num_dports;
        }

        for (uint64_t dport_batch_start = dport_start;
             dport_batch_start < dport_start + num_dports;
             dport_batch_start += max_batch_req) {

            SessionRequestMsg   req_msg;
            SessionResponseMsg  rsp_msg;
            ClientContext       context;
            Status              status;

            std::cout << __func__ << ": Start batching: " << dport_batch_start << std::endl;

            for (uint64_t dport = dport_batch_start;
                 dport < min(dport_batch_start + max_batch_req, dport_start + num_dports);
                 ++dport) {

                std::cout << __func__ << ": dport: " << dport << std::endl;

                session_populate(session_id, vrf_id,
                                 sip, dip,
                                 proto,
                                 sport, dport,
                                 nat_type,
                                 nat_sip, nat_dip,
                                 nat_sport, nat_dport,
                                 action,
                                 ing_mirror_session_id,
                                 req_msg);
            }

            status = session_stub_->SessionCreate(&context, req_msg, &rsp_msg);
            if (status.ok()) {
                for (int i = 0; i < rsp_msg.response_size(); ++i) {
                    assert(rsp_msg.response(i).api_status() == types::API_STATUS_OK);
                    std::cout << "Session create succeeded, handle = "
                              << rsp_msg.response(i).status().session_handle()
                              << std::endl;
                }
            } else {
                std::cout << "Session create failed, error = "
                          << rsp_msg.response(0).api_status()
                          << std::endl;
                return -1;
            }
        }

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
        spec->mutable_meta()->set_vrf_id(vrf_id);
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

    void del_l2seg_on_uplinks(uint64_t if_id_start, uint32_t num_uplinks, uint64_t l2seg_id) {
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
                          << " Del on uplink " << i + 1
                          << " succeeded" << std::endl;
            }
        } else {
            for (uint32_t i = 0; i < num_uplinks; i++) {
                std::cout << "L2 Segment " << l2seg_id
                          << " Del on uplink " << i + 1
                          << " failed, err " << rsp_msg.response(i).api_status()
                          << std::endl;
            }
        }

        return;
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

    // create gre tunnel interface
    uint64_t gre_tunnel_if_create(uint32_t vrf_id, uint64_t tunnel_if_id,
                                  uint32_t sip, uint32_t dip) {
        InterfaceSpec           *spec;
        InterfaceRequestMsg     req_msg;
        InterfaceResponseMsg    rsp_msg;
        ClientContext           context;
        Status                  status;

        spec = req_msg.add_request();
        spec->mutable_meta()->set_vrf_id(vrf_id);
        spec->mutable_key_or_handle()->set_interface_id(tunnel_if_id);
        spec->set_type(::intf::IfType::IF_TYPE_TUNNEL);
        spec->set_admin_status(::intf::IfStatus::IF_STATUS_UP);
        spec->mutable_if_tunnel_info()->set_encap_type(::intf::IF_TUNNEL_ENCAP_TYPE_GRE);
        spec->mutable_if_tunnel_info()->mutable_gre_info()->mutable_source()->set_ip_af(::types::IP_AF_INET);
        spec->mutable_if_tunnel_info()->mutable_gre_info()->mutable_source()->set_v4_addr(sip);
        spec->mutable_if_tunnel_info()->mutable_gre_info()->mutable_destination()->set_ip_af(::types::IP_AF_INET);
        spec->mutable_if_tunnel_info()->mutable_gre_info()->mutable_destination()->set_v4_addr(dip);
        spec->mutable_if_tunnel_info()->mutable_gre_info()->set_ttl(100);
        spec->mutable_if_tunnel_info()->mutable_vrf_key_handle()->set_vrf_id(vrf_id);
        status = intf_stub_->InterfaceCreate(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            assert(rsp_msg.response(0).api_status() == types::API_STATUS_OK);
            std::cout << "GRE tunnel if create succeeded, handle = "
                      << rsp_msg.response(0).status().if_handle()
                      << std::endl;
            return rsp_msg.response(0).status().if_handle();
        } else {
            std::cout << "GRE tunnel if create failed, error = "
                      << rsp_msg.response(0).api_status()
                      << std::endl;
        }
        return 0;
    }

    #define DROP_MONITOR_RULE_CREATE_(arg1) \
    { \
        DropMonitorRuleRequestMsg     req_msg; \
        DropMonitorRuleSpec           *spec; \
        DropMonitorRuleResponseMsg    rsp_msg; \
        Status                        status; \
        ClientContext                 context; \
        spec = req_msg.add_request(); \
        spec->mutable_meta()->set_vrf_id(vrf_id); \
        spec->mutable_key_or_handle()->set_dropmonitorrule_id(dropmon_rule_id); \
        auto ms_kh = spec->add_ms_key_handle(); \
        ms_kh->set_mirrorsession_id(mirror_session_id); \
        spec->mutable_reasons()->set_##arg1 (true); \
        status = telemetry_stub_->DropMonitorRuleCreate(&context, req_msg, &rsp_msg); \
        if (status.ok()) { \
            assert(rsp_msg.response(0).api_status() == types::API_STATUS_OK); \
            std::cout << "drop monitor rule succeeded, mirr_sess_id = " << mirror_session_id << std::endl; \
        } \
    } \

    uint32_t drop_monitor_rule_create (uint32_t vrf_id, uint32_t mirror_session_id,
                                       uint32_t dropmon_rule_id) {
        /* Need to program mirror session for every drop reason */
        DROP_MONITOR_RULE_CREATE_(drop_malformed_pkt);
        DROP_MONITOR_RULE_CREATE_(drop_input_mapping);
        DROP_MONITOR_RULE_CREATE_(drop_input_mapping_dejavu);
        DROP_MONITOR_RULE_CREATE_(drop_flow_hit);
        DROP_MONITOR_RULE_CREATE_(drop_flow_miss);
        DROP_MONITOR_RULE_CREATE_(drop_nacl);
        DROP_MONITOR_RULE_CREATE_(drop_ipsg);
        DROP_MONITOR_RULE_CREATE_(drop_ip_normalization);
        DROP_MONITOR_RULE_CREATE_(drop_tcp_normalization);
        DROP_MONITOR_RULE_CREATE_(drop_tcp_rst_with_invalid_ack_num);
        DROP_MONITOR_RULE_CREATE_(drop_tcp_non_syn_first_pkt);
        DROP_MONITOR_RULE_CREATE_(drop_icmp_normalization);
        DROP_MONITOR_RULE_CREATE_(drop_input_properties_miss);
        DROP_MONITOR_RULE_CREATE_(drop_tcp_out_of_window);
        DROP_MONITOR_RULE_CREATE_(drop_tcp_split_handshake);
        DROP_MONITOR_RULE_CREATE_(drop_tcp_win_zero_drop);
        DROP_MONITOR_RULE_CREATE_(drop_tcp_data_after_fin);
        DROP_MONITOR_RULE_CREATE_(drop_tcp_non_rst_pkt_after_rst);
        DROP_MONITOR_RULE_CREATE_(drop_tcp_invalid_responder_first_pkt);
        DROP_MONITOR_RULE_CREATE_(drop_tcp_unexpected_pkt);
        DROP_MONITOR_RULE_CREATE_(drop_src_lif_mismatch);
        DROP_MONITOR_RULE_CREATE_(drop_parser_icrc_error);
        DROP_MONITOR_RULE_CREATE_(drop_parse_len_error);
        DROP_MONITOR_RULE_CREATE_(drop_hardware_error);

        return 0;
    }

    uint32_t mirror_session_create(uint32_t vrf_id, uint32_t session_id,
                                   uint32_t sip, uint32_t dip) {
        MirrorSessionRequestMsg     req_msg;
        MirrorSessionSpec           *spec;
        MirrorSessionResponseMsg    rsp_msg;
        Status                      status;
        ClientContext               context;

        spec = req_msg.add_request();
        spec->mutable_vrf_key_handle()->set_vrf_id(vrf_id);
        spec->mutable_key_or_handle()->set_mirrorsession_id(session_id);
        spec->mutable_erspan_spec()->mutable_dest_ip()->set_ip_af(::types::IP_AF_INET);
        spec->mutable_erspan_spec()->mutable_dest_ip()->set_v4_addr(sip);
        spec->mutable_erspan_spec()->mutable_src_ip()->set_ip_af(::types::IP_AF_INET);
        spec->mutable_erspan_spec()->mutable_dest_ip()->set_v4_addr(dip);
        spec->mutable_erspan_spec()->set_span_id(session_id);

        status = telemetry_stub_->MirrorSessionCreate(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            assert(rsp_msg.response(0).api_status() == types::API_STATUS_OK);
            std::cout << "Mirror session succeeded, id = " << session_id << std::endl;
        }

        return 0;
    }

    uint32_t netflow_collector_create(uint32_t vrf_id, uint32_t export_ctrl_id,
                                      uint64_t l2seg_handle, uint16_t vlan_encap,
                                      uint32_t sip, uint32_t dip, uint16_t dport,
                                      uint32_t template_id) {
        CollectorRequestMsg     req_msg;
        CollectorSpec           *spec;
        CollectorResponseMsg    rsp_msg;
        Status                  status;
        ClientContext           context;

        spec = req_msg.add_request();
        spec->mutable_vrf_key_handle()->set_vrf_id(vrf_id);
        spec->mutable_key_or_handle()->set_collector_id(export_ctrl_id);
        if (vlan_encap) {
            spec->mutable_encap()->set_encap_type(::types::ENCAP_TYPE_DOT1Q);
            spec->mutable_encap()->set_encap_value(vlan_encap);
        } else {
            spec->mutable_encap()->set_encap_type(::types::ENCAP_TYPE_DOT1Q);
            spec->mutable_encap()->set_encap_value(0);
        }
        spec->mutable_l2seg_key_handle()->set_l2segment_handle(l2seg_handle);
        spec->mutable_src_ip()->set_ip_af(::types::IP_AF_INET);
        spec->mutable_src_ip()->set_v4_addr(sip);
        spec->mutable_dest_ip()->set_ip_af(::types::IP_AF_INET);
        spec->mutable_dest_ip()->set_v4_addr(dip);
        spec->set_protocol(::types::IPPROTO_UDP);
        spec->set_dest_port(dport);
        spec->set_format(::telemetry::IPFIX);
        spec->set_template_id(template_id);

        status = telemetry_stub_->CollectorCreate(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            assert(rsp_msg.response(0).api_status() == types::API_STATUS_OK);
            std::cout << "Collector create succeeded" << std::endl;
        } else {
            std::cout << "Collector create failed" << std::endl;
        }
        return 0;
    }

    int bypass_tls() {
        ProxyGlobalCfgRequest       *spec;
        ProxyGlobalCfgRequestMsg    req_msg;
        ProxyGlobalCfgResponseMsg   rsp_msg;
        ClientContext               context;
        Status                      status;

        spec = req_msg.add_request();
        spec->set_proxy_type(types::PROXY_TYPE_TLS);
        spec->set_bypass_mode(1);

        status = proxy_stub_->ProxyGlobalCfg(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            std::cout << "Proxy bypass succeeded"
                      << std::endl;
            return 0;

        } else {
            std::cout << "Proxy bypass failed"
                      << std::endl;
            return -1;
        }
    }

    int proxy_enable(types::ProxyType type) {
        ProxySpec           *spec;
        ProxyRequestMsg     req_msg;
        ProxyResponseMsg    rsp_msg;
        ClientContext       context;
        Status              status;

        spec = req_msg.add_request();
        spec->set_proxy_type(type);

        status = proxy_stub_->ProxyEnable(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            std::cout << "Proxy create succeeded for "
                      << type
                      << std::endl;
            return 0;

        } else {
            std::cout << "Proxy create failed for TCP"
                      << type
                      << std::endl;
            return -1;
        }
    }

    int proxy_flow_config(bool proxy_enable, int vrf_id,
            uint32_t sip, uint32_t dip, uint16_t sport,
            uint16_t dport, int num_entries) {
        ProxyFlowConfigRequest      *spec;
        ProxyFlowConfigRequestMsg   req_msg;
        ProxyResponseMsg            rsp_msg;
        ClientContext       context;
        Status              status;

        for (int i = 0; i < num_entries; i++) {
            spec = req_msg.add_request();
            spec->mutable_meta()->set_vrf_id(vrf_id);
            spec->mutable_spec()->set_proxy_type(types::PROXY_TYPE_TCP);
            spec->set_proxy_en(proxy_enable);
            spec->set_alloc_qid(true);

            flow_key_populate(
                    vrf_id,
                    sip, dip, ::types::IPProtocol::IPPROTO_TCP,
                    sport + i, dport + i,
                    spec->mutable_flow_key());
        }

        status = proxy_stub_->ProxyFlowConfig(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            std::cout << "Proxy config succeeded for "
                      << dip << "/" << sip << " " << dport << "/" << sport
                      << std::endl;
            return 0;

        } else {
            std::cout << "Proxy config failed for TCP"
                      << dip << "/" << sip << " " << dport << "/" << sport
                      << std::endl;
            return -1;
        }
    }

    int tcpcb_get(int qid) {
        TcpCbGetRequest             *req1;
        TcpCbGetRequest             *req2;
        TcpCbGetRequestMsg          req_msg;
        TcpCbGetResponseMsg         rsp_msg;
        ClientContext               context;
        Status                      status;

        req1 = req_msg.add_request();
        req1->mutable_key_or_handle()->set_tcpcb_id(qid);

        req2 = req_msg.add_request();
        req2->mutable_key_or_handle()->set_tcpcb_id(qid + 1);

        status = tcpcb_stub_->TcpCbGet(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            for (int i = 0; i < rsp_msg.response_size(); i++) {
                const TcpCbSpec &spec = rsp_msg.response(i).spec();

                if (rsp_msg.response(i).api_status() != types::API_STATUS_OK) {
                    std::cout << "TcpCbGet failed! response = " <<
                        rsp_msg.response(i).api_status() << std::endl;
                    continue;
                }

                std::cout << "TcpCb for qid " << rsp_msg.response(i).spec().key_or_handle().tcpcb_id() << std::endl;
                std::cout << "================\n";
                std::cout << "rcv_nxt: " << std::dec << spec.rcv_nxt() << std::endl;
                std::cout << "snd_nxt: " << std::dec << spec.snd_nxt() << std::endl;
                std::cout << "snd_una: " << std::dec << spec.snd_una() << std::endl;
                std::cout << "rcv_tsval: " << std::dec << spec.rcv_tsval() << std::endl;
                std::cout << "ts_recent: " << std::dec << spec.ts_recent() << std::endl;
                std::cout << "serq_base: " << std::hex << "0x" << spec.serq_base() << std::endl;
                std::cout << "debug_dol: " << std::hex << "0x" << spec.debug_dol() << std::endl;
                std::cout << "sesq_base: " << std::hex << "0x" << spec.sesq_base() << std::endl;
                std::cout << "snd_wnd: " << std::dec << spec.snd_wnd() << std::endl;
                std::cout << "snd_cwnd: " << std::dec << spec.snd_cwnd() << std::endl;
                std::cout << "rcv_mss: " << std::dec << spec.rcv_mss() << std::endl;
                std::cout << "source_port: " << std::dec << spec.source_port() << std::endl;
                std::cout << "dest_port: " << std::dec << spec.dest_port() << std::endl;
                std::cout << "state: " << std::dec << spec.state() << std::endl;
                std::cout << "source_lif: " << std::dec << spec.source_lif() << std::endl;
                std::cout << "debug_dol_tx: " << std::hex << "0x" << spec.debug_dol_tx() << std::endl;
                std::cout << "header_len: " << std::dec << spec.header_len() << std::endl;
                std::cout << "pending_ack_send: " << std::dec << spec.pending_ack_send() << std::endl;
                std::cout << "retx_snd_una: " << std::dec << spec.retx_snd_una() << std::endl;
                std::cout << "other_qid: " << std::dec << spec.other_qid() << std::endl;
                std::cout << "rto_backoff: " << std::dec << spec.rto_backoff() << std::endl;
            }
            return 0;

        } else {
            std::cout << "TcpCb get failed for qid " << qid;
            return -1;
        }
    }
    int tcpcb_stats_get(int qid) {
        TcpCbGetRequest             *req1;
        TcpCbGetRequest             *req2;
        TcpCbGetRequestMsg          req_msg;
        TcpCbGetResponseMsg         rsp_msg;
        ClientContext               context;
        Status                      status;

        req1 = req_msg.add_request();
        req1->mutable_key_or_handle()->set_tcpcb_id(qid);

        req2 = req_msg.add_request();
        req2->mutable_key_or_handle()->set_tcpcb_id(qid + 1);

        status = tcpcb_stub_->TcpCbGet(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            for (int i = 0; i < rsp_msg.response_size(); i++) {
                const TcpCbStats &stats = rsp_msg.response(i).stats();

                if (rsp_msg.response(i).api_status() != types::API_STATUS_OK) {
                    std::cout << "TcpCbGet failed! response = " <<
                        rsp_msg.response(i).api_status() << std::endl;
                    continue;
                }

                std::cout << "Stats for qid " << rsp_msg.response(i).spec().key_or_handle().tcpcb_id() << std::endl;
                std::cout << "================\n";
                std::cout << "pkts_rcvd: " << stats.pkts_rcvd() << std::endl;
                std::cout << "bytes_rcvd: " << stats.bytes_rcvd() << std::endl;
                std::cout << "bytes_acked: " << stats.bytes_acked() << std::endl;
                std::cout << "pkts_sent: " << stats.pkts_sent() << std::endl;
                std::cout << "bytes_sent: " << stats.bytes_sent() << std::endl;
                std::cout << "slow_path_cnt: " << stats.slow_path_cnt() << std::endl;
                std::cout << "serq_full_cnt: " << stats.serq_full_cnt() << std::endl;
                std::cout << "ooo_cnt: " << stats.ooo_cnt() << std::endl;
                std::cout << "sesq_pi: " << stats.sesq_pi() << std::endl;
                std::cout << "sesq_ci: " << stats.sesq_ci() << std::endl;
                std::cout << "sesq_retx_ci: " << stats.sesq_retx_ci() << std::endl;
                std::cout << "send_ack_pi: " << stats.send_ack_pi() << std::endl;
                std::cout << "send_ack_ci: " << stats.send_ack_ci() << std::endl;
                std::cout << "del_ack_pi: " << stats.del_ack_pi() << std::endl;
                std::cout << "del_ack_ci: " << stats.del_ack_ci() << std::endl;
                std::cout << "retx_timer_pi: " << stats.retx_timer_pi() << std::endl;
                std::cout << "retx_timer_ci: " << stats.retx_timer_ci() << std::endl;
                std::cout << "asesq_pi: " << stats.asesq_pi() << std::endl;
                std::cout << "asesq_ci: " << stats.asesq_ci() << std::endl;
                std::cout << "asesq_retx_ci: " << stats.asesq_retx_ci() << std::endl;
                std::cout << "pending_tx_pi: " << stats.pending_tx_pi() << std::endl;
                std::cout << "pending_tx_ci: " << stats.pending_tx_ci() << std::endl;
                std::cout << "fast_retrans_pi: " << stats.fast_retrans_pi() << std::endl;
                std::cout << "fast_retrans_ci: " << stats.fast_retrans_ci() << std::endl;
                std::cout << "clean_retx_pi: " << stats.clean_retx_pi() << std::endl;
                std::cout << "clean_retx_ci: " << stats.clean_retx_ci() << std::endl;
                std::cout << "packets_out: " << stats.packets_out() << std::endl;
                std::cout << "rto_pi: " << stats.rto_pi() << std::endl;
                std::cout << "tx_ring_pi: " << stats.tx_ring_pi() << std::endl;
                std::cout << "partial_ack_cnt: " << stats.partial_ack_cnt() << std::endl;
                std::cout << std::endl;
            }
            return 0;

        } else {
            std::cout << "TcpCb get failed for qid " << qid;
            return -1;
        }
    }

    std::string read_file_contents(const char *filename)
    {
        std::ifstream in((const char *)filename, std::ios::in | std::ios::binary);
        if (in)
        {
            std::string contents;
            in.seekg(0, std::ios::end);
            contents.resize(in.tellg());
            in.seekg(0, std::ios::beg);
            in.read(&contents[0], contents.size());
            in.close();
            return(contents);
        }
        throw(errno);
    }

    int tcp_tls_proxy_ecdsa_key_add(const char * filename, uint32_t *sign_key_idx) {
        CryptoApiRequestMsg     req_msg;
        CryptoApiRequest        *req;
        CryptoApiResponseMsg    rsp_msg;
        ClientContext           context;
        Status                  status;

        req = req_msg.add_request();
        req->set_api_type(cryptoapis::ASYMAPI_SETUP_PRIV_KEY);
        req->mutable_setup_priv_key()->set_key(read_file_contents(filename));
        status = crypto_apis_stub_->CryptoApiInvoke(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            assert(rsp_msg.response(0).setup_priv_key().key_type() ==
                    types::CRYPTO_ASYM_KEY_TYPE_ECDSA);
            *sign_key_idx =
                rsp_msg.response(0).setup_priv_key().ecdsa_key_info().sign_key_idx();
            std::cout << "Setup key(" << filename << "): ID:" << *sign_key_idx << std::endl;
        }
        else {
            std::cout << "Setup key(" << filename << "): Failed" << std::endl;
            return -1;
        }
        return 0;
    }

    int tcp_tls_proxy_rsa_key_add(const char * filename,
            uint32_t *sign_key_idx, uint32_t *decrypt_key_idx) {
        CryptoApiRequestMsg     req_msg;
        CryptoApiRequest        *req;
        CryptoApiResponseMsg    rsp_msg;
        ClientContext           context;
        Status                  status;

        req = req_msg.add_request();
        req->set_api_type(cryptoapis::ASYMAPI_SETUP_PRIV_KEY);
        req->mutable_setup_priv_key()->set_key(read_file_contents(filename));
        status = crypto_apis_stub_->CryptoApiInvoke(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            assert(rsp_msg.response(0).setup_priv_key().key_type() ==
                    types::CRYPTO_ASYM_KEY_TYPE_RSA);
            *sign_key_idx =
                rsp_msg.response(0).setup_priv_key().rsa_key_info().sign_key_idx();
            *decrypt_key_idx = 
            rsp_msg.response(0).setup_priv_key().rsa_key_info().decrypt_key_idx();
        }
        else {
            return -1;
        }
        return 0;
    }

    int tcp_tls_proxy_cert_add(const char * filename, uint32_t cert_id,
            uint32_t next_cert_id)
    {
        CryptoApiRequestMsg     req_msg;
        CryptoApiRequest        *req;
        CryptoApiResponseMsg    rsp_msg;
        ClientContext           context;
        Status                  status;

        req = req_msg.add_request();
        req->set_api_type(cryptoapis::ASYMAPI_SETUP_CERT);
        req->mutable_setup_cert()->set_update_type(cryptoapis::ADD_UPDATE);
        req->mutable_setup_cert()->set_cert_id(cert_id);
        req->mutable_setup_cert()->set_body(read_file_contents(filename));
        status = crypto_apis_stub_->CryptoApiInvoke(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            std::cout << "Setup cert(" << filename << "): ID:" << cert_id << std::endl;
        }
        else {
            std::cout << "Setup cert(" << filename << "): ID:" << cert_id << " Failed" << std::endl;
            return -1;
        }
        return 0;
    }



    int tcp_proxy_policy_rule_match_setup(
            TcpProxyRuleSpec *req,
            TcpProxyRuleMatchSpec **rule_match_spec,
            uint64_t vrf_id,
            in_addr_t src_address_start,
            in_addr_t src_address_end,
            in_addr_t dst_address_start,
            in_addr_t dst_address_end)
    {

        TcpProxyRuleMatchSpec       *rm_spec;

        /* TcpProxyRuleSpec setup */
        req->mutable_key_or_handle()->mutable_rule_key()->set_tcp_proxy_rule_id(1);
        req->mutable_key_or_handle()->mutable_rule_key()->mutable_vrf_key_or_handle()->set_vrf_id(vrf_id);
        req->mutable_vrf_key_handle()->set_vrf_id(vrf_id);

        /* TcpProxyRuleSpec->TcpProxyRuleMatchSpec Setup */
        rm_spec = req->add_rules();
        rm_spec->set_rule_id(1); /* TBD define */


        /* TcpProxyRuleMatchSpec: RuleMatch */
        IPAddressObj                *src_addr;
#ifdef USE_DST_ADDRESS_MATCH
        IPAddressObj                *dst_addr;
#endif

        /* SRC Address Range */
        src_addr = rm_spec->mutable_match()->add_src_address();
        src_addr->set_type(types::IP_ADDRESS_IPV4_ANY);
        src_addr->mutable_address()->mutable_range()->mutable_ipv4_range()->mutable_low_ipaddr()->set_ip_af(types::IP_AF_INET);
        src_addr->mutable_address()->mutable_range()->mutable_ipv4_range()->mutable_low_ipaddr()->set_v4_addr(ntohl(src_address_start));
        src_addr->mutable_address()->mutable_range()->mutable_ipv4_range()->mutable_high_ipaddr()->set_ip_af(types::IP_AF_INET);
        src_addr->mutable_address()->mutable_range()->mutable_ipv4_range()->mutable_high_ipaddr()->set_v4_addr(ntohl(src_address_end));

#ifdef USE_DST_ADDRESS_MATCH
        /* DST Address Range */
        dst_addr = rm_spec->mutable_match()->add_dst_address();
        dst_addr->set_type(types::IP_ADDRESS_IPV4_ANY);
        dst_addr->mutable_address()->mutable_range()->mutable_ipv4_range()->mutable_low_ipaddr()->set_ip_af(types::IP_AF_INET);
        dst_addr->mutable_address()->mutable_range()->mutable_ipv4_range()->mutable_low_ipaddr()->set_v4_addr(dst_address_start);
        dst_addr->mutable_address()->mutable_range()->mutable_ipv4_range()->mutable_high_ipaddr()->set_ip_af(types::IP_AF_INET);
        dst_addr->mutable_address()->mutable_range()->mutable_ipv4_range()->mutable_high_ipaddr()->set_v4_addr(dst_address_end);
#else
        (void)dst_address_start;
        (void)dst_address_end;
#endif

        rm_spec->mutable_match()->set_protocol(types::IPPROTO_TCP);

        *rule_match_spec = rm_spec;
        return 0;
    }



    int tcp_proxy_policy_add(
            uint64_t vrf_id,
            in_addr_t src_address_start,
            in_addr_t src_address_end,
            in_addr_t dst_address_start,
            in_addr_t dst_address_end)
    {
        TcpProxyRuleRequestMsg      req_msg;
        TcpProxyRuleSpec            *req;
        TcpProxyRuleMatchSpec       *rule_match_spec;
        TcpProxyRuleResponseMsg     rsp_msg;
        ClientContext               context;
        Status                      status;

        req = req_msg.add_request();

        if (tcp_proxy_policy_rule_match_setup(req, &rule_match_spec, vrf_id, src_address_start, src_address_end, dst_address_start, dst_address_end)) {
            std::cout << "Policy rule match setup failed" << std::endl;
            return -1;
        }

        /* TcpProxyRuleMatchSpec: TcpProxyAction - TCP only proxy */
        rule_match_spec->mutable_tcp_proxy_action()->set_tcp_proxy_action_type(tcp_proxy::TCP_PROXY_ACTION_TYPE_ENABLE);
        rule_match_spec->mutable_tcp_proxy_action()->set_proxy_type(types::PROXY_TYPE_TCP);

        status = tcp_proxy_stub_->TcpProxyRuleCreate(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            std::cout << "Setup Proxy Policy Succeeded" << std::endl;
        }
        else {
            std::cout << "Setup Proxy Policy Failed" << std::endl;
            return -1;
        }
        return 0;
    }

    int tcp_proxy_tls_policy_add(
            types::CryptoAsymKeyType    key_type,
            uint64_t                    vrf_id,
            in_addr_t                   src_address_start,
            in_addr_t                   src_address_end,
            in_addr_t                   dst_address_start,
            in_addr_t                   dst_address_end,
            uint32_t                    sign_key_idx,
            uint32_t                    decrypt_key_idx,
            uint32_t                    cert_idx)
    {
        TcpProxyRuleRequestMsg      req_msg;
        TcpProxyRuleSpec            *req;
        TcpProxyRuleMatchSpec       *rule_match_spec;
        TcpProxyRuleResponseMsg     rsp_msg;
        ClientContext               context;
        Status                      status;

        req = req_msg.add_request();

        if (tcp_proxy_policy_rule_match_setup(req, &rule_match_spec, vrf_id, src_address_start, src_address_end, dst_address_start, dst_address_end)) {
            std::cout << "Policy rule match setup failed" << std::endl;
            return -1;
        }

        /* TcpProxyRuleMatchSpec: TcpProxyAction - TCP only proxy */
        rule_match_spec->mutable_tcp_proxy_action()->set_tcp_proxy_action_type(tcp_proxy::TCP_PROXY_ACTION_TYPE_ENABLE);
        rule_match_spec->mutable_tcp_proxy_action()->set_proxy_type(types::PROXY_TYPE_TLS);

        if (key_type == types::CRYPTO_ASYM_KEY_TYPE_ECDSA) {
            /* TLS ECDSA specific policy configuration */
            rule_match_spec->mutable_tcp_proxy_action()->mutable_tls()->set_asym_key_type(key_type);
            rule_match_spec->mutable_tcp_proxy_action()->mutable_tls()->set_cert_id(cert_idx);
            rule_match_spec->mutable_tcp_proxy_action()->mutable_tls()->mutable_ecdsa_key()->set_sign_key_idx(sign_key_idx);
        }
        else if (key_type == types::CRYPTO_ASYM_KEY_TYPE_RSA) {
            /* TLS RSA specific policy configuration */
            rule_match_spec->mutable_tcp_proxy_action()->mutable_tls()->set_asym_key_type(key_type);
            rule_match_spec->mutable_tcp_proxy_action()->mutable_tls()->set_cert_id(cert_idx);
            rule_match_spec->mutable_tcp_proxy_action()->mutable_tls()->mutable_rsa_key()->set_sign_key_idx(sign_key_idx);
            rule_match_spec->mutable_tcp_proxy_action()->mutable_tls()->mutable_rsa_key()->set_decrypt_key_idx(decrypt_key_idx);
        }

        status = tcp_proxy_stub_->TcpProxyRuleCreate(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            std::cout << "Setup Proxy Policy Succeeded" << std::endl;
        }
        else {
            std::cout << "Setup Proxy Policy Failed" << std::endl;
            return -1;
        }
        return 0;
    }

    int tcp_proxy_flow_setup(
            uint64_t vrf_id, 
            in_addr_t src_range_start,
            in_addr_t src_range_end,
            in_addr_t dst_range_start,
            in_addr_t dst_range_end)
    {
        if (tcp_proxy_policy_add(
                vrf_id,
                src_range_start,
                src_range_end,
                dst_range_start,
                dst_range_end)) {
            std::cout << "Failed to setup Proxy Policy";
            return -1;
        }
        return 0;
    }
                


    int tcp_tls_proxy_client_ecdsa_flow_setup(
            uint64_t vrf_id, 
            in_addr_t src_range_start,
            in_addr_t src_range_end,
            in_addr_t dst_range_start,
            in_addr_t dst_range_end)
    {
        uint32_t        sign_key_idx;
        uint32_t        cert_idx = 1;

        if (tcp_tls_proxy_ecdsa_key_add(
                    "/nic/conf/openssl/certs/ecdsa/client.key",
                    &sign_key_idx))
        {
            std::cout << "Failed to setup Client ECDSA Key";
            return -1;
        }

        if (tcp_tls_proxy_cert_add(
                    "/nic/conf/openssl/certs/ecdsa/client.crt",
                    cert_idx,
                    0))
        {
            std::cout << "Failed to setup Client ECDSA Cert";
            return -1;
        }

        if (tcp_proxy_tls_policy_add(
                types::CRYPTO_ASYM_KEY_TYPE_ECDSA,
                vrf_id,
                src_range_start,
                src_range_end,
                dst_range_start,
                dst_range_end,
                sign_key_idx,
                0,
                cert_idx)) {
            std::cout << "Failed to setup Proxy Policy";
            return -1;
        }
        return 0;
    }

    int tcp_tls_proxy_server_ecdsa_flow_setup(
            uint64_t vrf_id, 
            in_addr_t src_range_start,
            in_addr_t src_range_end,
            in_addr_t dst_range_start,
            in_addr_t dst_range_end)
    {
        uint32_t        sign_key_idx;
        uint32_t        cert_idx = 1;

        if (tcp_tls_proxy_ecdsa_key_add(
                    "/nic/conf/openssl/certs/ecdsa/server.key",
                    &sign_key_idx))
        {
            std::cout << "Failed to setup Server ECDSA Key";
            return -1;
        }
        if (tcp_tls_proxy_cert_add(
                    "/nic/conf/openssl/certs/ecdsa/server.crt",
                    cert_idx,
                    0))
        {
            std::cout << "Failed to setup Server ECDSA Cert";
            return -1;
        }

        if (tcp_proxy_tls_policy_add(
                types::CRYPTO_ASYM_KEY_TYPE_ECDSA,
                vrf_id,
                src_range_start,
                src_range_end,
                dst_range_start,
                dst_range_end,
                sign_key_idx,
                0,
                cert_idx)) {
            std::cout << "Failed to setup Proxy Policy";
            return -1;
        }
        return 0;
    }

    int tcp_tls_proxy_client_rsa_flow_setup(
            uint64_t vrf_id, 
            in_addr_t src_range_start,
            in_addr_t src_range_end,
            in_addr_t dst_range_start,
            in_addr_t dst_range_end)
    {
        uint32_t        sign_key_idx;
        uint32_t        decrypt_key_idx;
        uint32_t        cert_idx = 1;

        if (tcp_tls_proxy_rsa_key_add(
                    "/nic/conf/openssl/certs/rsa/client.key",
                    &sign_key_idx, &decrypt_key_idx))
        {
            std::cout << "Failed to setup Client RSA Key";
            return -1;
        }

        if (tcp_tls_proxy_cert_add(
                    "/nic/conf/openssl/certs/rsa/client.crt",
                    cert_idx,
                    0))
        {
            std::cout << "Failed to setup Client RSA Cert";
            return -1;
        }
        if (tcp_proxy_tls_policy_add(
                types::CRYPTO_ASYM_KEY_TYPE_RSA,
                vrf_id,
                src_range_start,
                src_range_end,
                dst_range_start,
                dst_range_end,
                sign_key_idx,
                decrypt_key_idx,
                cert_idx)) {
            std::cout << "Failed to setup Proxy Policy";
            return -1;
        }
        return 0;
    }

    int tcp_tls_proxy_server_rsa_flow_setup(
            uint64_t vrf_id, 
            in_addr_t src_range_start,
            in_addr_t src_range_end,
            in_addr_t dst_range_start,
            in_addr_t dst_range_end)
    {
        uint32_t        sign_key_idx;
        uint32_t        decrypt_key_idx;
        uint32_t        cert_idx = 1;

        if (tcp_tls_proxy_rsa_key_add("/nic/conf/openssl/certs/rsa/server.key",
                    &sign_key_idx, &decrypt_key_idx))
        {
            std::cout << "Failed to setup Server RSA Key";
            return -1;
        }

        if (tcp_tls_proxy_cert_add(
                    "/nic/conf/openssl/certs/rsa/server.crt",
                    cert_idx,
                    0))
        {
            std::cout << "Failed to setup Server RSA Cert";
            return -1;
        }
        if (tcp_proxy_tls_policy_add(
                types::CRYPTO_ASYM_KEY_TYPE_RSA,
                vrf_id,
                src_range_start,
                src_range_end,
                dst_range_start,
                dst_range_end,
                sign_key_idx,
                decrypt_key_idx,
                cert_idx)) {
            std::cout << "Failed to setup Proxy Policy";
            return -1;
        }
        return 0;
    }

    void qos_class_create(uint32_t qos_group, uint32_t pcp, 
                          uint32_t dscp, uint32_t strict, uint32_t rate_or_dwrr) {
        QosClassSpec        *spec;
        QosClassRequestMsg  req_msg;
        QosClassResponseMsg rsp_msg;
        ClientContext       context;
        Status              status;
        
        std::cout << "Qos class create with " 
                  << qos_group << " "
                  << pcp << " "
                  << dscp << " "
                  << strict << " "
                  << rate_or_dwrr
                  << std::endl;
        spec = req_msg.add_request();
        spec->mutable_key_or_handle()->set_qos_group(static_cast<kh::QosGroup>(qos_group));
        spec->set_mtu(9216);
        spec->mutable_uplink_class_map()->set_dot1q_pcp(pcp);
        spec->mutable_uplink_class_map()->add_ip_dscp(dscp);
        if (strict) {
            spec->mutable_sched()->mutable_strict()->set_bps(rate_or_dwrr);
        } else {
            spec->mutable_sched()->mutable_dwrr()->set_bw_percentage(rate_or_dwrr);
        }
        std::cout << "6" << std::endl;

        status = qos_stub_->QosClassCreate(&context, req_msg, &rsp_msg);
        std::cout << "7" << std::endl;
        if (status.ok()) {
            std::cout << "QosClass create succeeded"
                      << std::endl;
        } else {
            std::cout << "QosClass create failed"
                << std::endl;
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
    std::unique_ptr<Telemetry::Stub> telemetry_stub_;
    std::unique_ptr<Proxy::Stub> proxy_stub_;
    std::unique_ptr<TcpCb::Stub> tcpcb_stub_;
    std::unique_ptr<CryptoApis::Stub> crypto_apis_stub_;
    std::unique_ptr<TcpProxy::Stub> tcp_proxy_stub_;
    std::unique_ptr<QOS::Stub> qos_stub_;
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
    int ret  = 0;

    // port 1: create and get
    std::cout <<  "*********** Port "
              << port
              << " create, enable and get"
              << " **********"
              << std::endl;
    ret = hclient->port_create(vrf_id, port);
    assert(ret == -1);

    ret = hclient->port_get(vrf_id, port);
    assert(ret != -1);

    // port 1: update speed and get
    std::cout <<  "*********** Port "
              << port
              << " update speed and get"
              << " **********"
              << std::endl;
    ret = hclient->port_update(vrf_id, port,
                     ::port::PORT_SPEED_10G, ::port::PORT_ADMIN_STATE_NONE);
    assert(ret != -1);

    port_speed = ::port::PORT_SPEED_10G;
    ret = hclient->port_get(vrf_id, port, true);
    assert(ret != -1);
    port_speed = ::port::PORT_SPEED_NONE;

    // port 1: delete
    std::cout <<  "*********** Port "
              << port
              << " delete"
              << " **********"
              << std::endl;
    ret = hclient->port_delete(vrf_id, port);
    assert(ret != -1);

    port = 2;

    // port 2: create and get
    std::cout <<  "*********** Port "
              << port
              << " create, enable and get"
              << " **********"
              << std::endl;
    ret = hclient->port_create(vrf_id, port);
    assert(ret == -1);

    ret = hclient->port_get(vrf_id, port);
    assert(ret != -1);

    // port 2: update speed and get
    std::cout <<  "*********** Port "
              << port
              << " update speed and get"
              << " **********"
              << std::endl;
    ret = hclient->port_update(vrf_id, port,
                     ::port::PORT_SPEED_10G, ::port::PORT_ADMIN_STATE_NONE);
    assert(ret != -1);

    port_speed = ::port::PORT_SPEED_10G;
    ret = hclient->port_get(vrf_id, port, true);
    assert(ret != -1);
    port_speed = ::port::PORT_SPEED_NONE;

    port = 1;

    // port 1: create and get
    std::cout <<  "*********** Port "
              << port
              << " create, enable and get"
              << " **********"
              << std::endl;

    ret = hclient->port_create(vrf_id, port);
    assert(ret != -1);

    ret = hclient->port_get(vrf_id, port);
    assert(ret != -1);

    // port 1: update speed and get
    std::cout <<  "*********** Port "
              << port
              << " update speed and get"
              << " **********"
              << std::endl;
    ret = hclient->port_update(vrf_id, port,
                     ::port::PORT_SPEED_10G, ::port::PORT_ADMIN_STATE_NONE);
    assert(ret != -1);

    port_speed = ::port::PORT_SPEED_10G;
    ret = hclient->port_get(vrf_id, port, true);
    assert(ret != -1);
    port_speed = ::port::PORT_SPEED_NONE;

    // port 1: delete and get
    std::cout <<  "*********** Port "
              << port
              << " delete and get"
              << " **********"
              << std::endl;
    ret = hclient->port_delete(vrf_id, port);
    assert(ret != -1);

    ret = hclient->port_get(vrf_id, port);
    assert(ret == -1);

    port = 2;

    // port 2: disable and get
    std::cout <<  "*********** Port "
              << port
              << " disable and get"
              << " **********"
              << std::endl;
    ret = hclient->port_update(vrf_id, port,
                     ::port::PORT_SPEED_NONE, ::port::PORT_ADMIN_STATE_DOWN);
    assert(ret != -1);

    port_admin_state = port::PORT_ADMIN_STATE_DOWN;
    ret = hclient->port_get(vrf_id, port);
    assert(ret != -1);
    port_admin_state = port::PORT_ADMIN_STATE_NONE;

    // port 2: create and get
    std::cout <<  "*********** Port "
              << port
              << " create, enable and get"
              << " **********"
              << std::endl;
    ret = hclient->port_create(vrf_id, port);
    assert(ret == -1);

    ret = hclient->port_get(vrf_id, port);
    assert(ret != -1);

    // port 2: delete and get
    std::cout <<  "*********** Port "
              << port
              << " delete and get"
              << " **********"
              << std::endl;
    ret = hclient->port_delete(vrf_id, port);
    assert(ret != -1);

    ret = hclient->port_get(vrf_id, port);
    assert(ret == -1);

    return 0;
}

static int
create_l2segments(uint64_t   l2seg_id_start,
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

static uint32_t
max_uint32 (void)
{
    return std::numeric_limits<unsigned int>::max();
}

static uint64_t
max_uint64 (void)
{
    return std::numeric_limits<unsigned long long>::max();
}

static void
gft_proto_size_gftheaders_check (gft::GftHeaders *headers)
{
    headers->set_ethernet_header(true);
    headers->set_ipv4_header(true);
    headers->set_ipv6_header(true);
    headers->set_tcp_header(true);
    headers->set_udp_header(true);
    headers->set_icmp_header(true);
    headers->set_no_encap(true);
    headers->set_ip_in_ip_encap(true);
    headers->set_ip_in_gre_encap(true);
    headers->set_nvgre_encap(true);
    headers->set_vxlan_encap(true);
}

static void
gft_proto_size_gftheaderfields_check (gft::GftHeaderFields *header_fields)
{
    header_fields->set_dst_mac_addr(true);
    header_fields->set_src_mac_addr(true);
    header_fields->set_eth_type(true);
    header_fields->set_customer_vlan_id(true);
    header_fields->set_provider_vlan_id(true);
    header_fields->set_dot1p_priority(true);
    header_fields->set_src_ip_addr(true);
    header_fields->set_dst_ip_addr(true);
    header_fields->set_ip_ttl(true);
    header_fields->set_ip_protocol(true);
    header_fields->set_ip_dscp(true);
    header_fields->set_src_port(true);
    header_fields->set_dst_port(true);
    header_fields->set_tcp_flags(true);
    header_fields->set_tenant_id(true);
    header_fields->set_icmp_type(true);
    header_fields->set_icmp_code(true);
    header_fields->set_oob_vlan(true);
    header_fields->set_oob_tenant_id(true);
    header_fields->set_gre_protocol(true);
}

static void
gft_proto_size_ethfields_check (gft::GftEthFields *eth_fields)
{
    eth_fields->set_dst_mac_addr(max_uint64());
    eth_fields->set_src_mac_addr(max_uint64());
    eth_fields->set_eth_type(max_uint32());
    eth_fields->set_customer_vlan_id(max_uint32());
    eth_fields->set_provider_vlan_id(max_uint32());
    eth_fields->set_priority(max_uint32());
}

static void
gft_proto_size_gftheader_group_exact_match_check (
                            GftExactMatchFlowEntrySpec *match_flow_spec)
{
    std::string ipv6_ip = "00010001000100010001000100010001";
    GftHeaderGroupExactMatch *hdr_grp_match
                                = match_flow_spec->add_exact_matches();

    gft_proto_size_gftheaders_check(hdr_grp_match->mutable_headers());
    gft_proto_size_gftheaderfields_check(hdr_grp_match->mutable_match_fields());
    gft_proto_size_ethfields_check(hdr_grp_match->mutable_eth_fields());

    types::IPAddress *src_ip = hdr_grp_match->mutable_src_ip_addr();
    src_ip->set_ip_af(types::IP_AF_INET6);
    src_ip->set_v6_addr(ipv6_ip);

    types::IPAddress *dst_ip = hdr_grp_match->mutable_dst_ip_addr();
    dst_ip->set_ip_af(types::IP_AF_INET6);
    dst_ip->set_v6_addr(ipv6_ip);

    hdr_grp_match->set_ip_ttl(max_uint32());
    hdr_grp_match->set_ip_dscp(max_uint32());
    hdr_grp_match->set_ip_protocol(max_uint32());

    gft::TcpMatchFields *encap_tcp_fields =
            hdr_grp_match->mutable_encap_or_transport()->mutable_tcp_fields();
    encap_tcp_fields->set_sport(max_uint32());
    encap_tcp_fields->set_dport(max_uint32());
    encap_tcp_fields->set_tcp_flags(max_uint32());
}

static void
gft_proto_size_gftheader_group_trans_check (
                    GftExactMatchFlowEntrySpec *match_flow_spec)
{
    std::string ipv6_ip = "00010001000100010001000100010001";
    GftHeaderGroupTransposition *hdr_grp_trans
                              = match_flow_spec->add_transpositions();
    hdr_grp_trans->set_action(gft::TRANSPOSITION_ACTION_MODIFY);

    gft_proto_size_gftheaders_check(hdr_grp_trans->mutable_headers());
    gft_proto_size_gftheaderfields_check(hdr_grp_trans->mutable_header_fields());
    gft_proto_size_ethfields_check(hdr_grp_trans->mutable_eth_fields());

    types::IPAddress *src_ip = hdr_grp_trans->mutable_src_ip_addr();
    src_ip->set_ip_af(types::IP_AF_INET6);
    src_ip->set_v6_addr(ipv6_ip);

    types::IPAddress *dst_ip = hdr_grp_trans->mutable_dst_ip_addr();
    dst_ip->set_ip_af(types::IP_AF_INET6);
    dst_ip->set_v6_addr(ipv6_ip);

    hdr_grp_trans->set_ip_ttl(max_uint32());
    hdr_grp_trans->set_ip_dscp(max_uint32());
    hdr_grp_trans->set_ip_protocol(max_uint32());

    gft::TcpTranspositionFields *encap_fields =
            hdr_grp_trans->mutable_encap_or_transport()->mutable_tcp_fields();
    encap_fields->set_sport(max_uint32());
    encap_fields->set_dport(max_uint32());
}

static void
gft_proto_size_check (void)
{
    GftExactMatchFlowEntryRequestMsg req_msg;
    GftExactMatchFlowEntrySpec  *match_flow_spec = req_msg.add_request();

    std::cout << "GftExactMatchFlowEntrySpec Init size: "
              << match_flow_spec->ByteSizeLong()
              << std::endl;

    match_flow_spec->mutable_meta()->set_vrf_id(max_uint64());
    match_flow_spec->set_table_type(gft::GFT_TABLE_TYPE_EXACT_MATCH_EGRESS);
    match_flow_spec->mutable_key_or_handle()->set_flow_entry_id(max_uint64());
    match_flow_spec->mutable_exact_match_profile()->set_profile_id(max_uint64());
    match_flow_spec->mutable_transposition_profile()->set_profile_id(max_uint64());

    match_flow_spec->set_add_in_activated_state(true);
    match_flow_spec->set_rdma_flow(true);
    match_flow_spec->set_redirect_to_vport_ingress_queue(true);
    match_flow_spec->set_redirect_to_vport_egress_queue(true);
    match_flow_spec->set_redirect_to_vport_ingress_queue_if_ttl_is_one(true);
    match_flow_spec->set_redirect_to_vport_egress_queue_if_ttl_is_one(true);
    match_flow_spec->set_copy_all_packets(true);
    match_flow_spec->set_copy_first_packet(true);
    match_flow_spec->set_copy_when_tcp_flag_set(true);
    match_flow_spec->set_custom_action_present(true);
    match_flow_spec->set_meta_action_before_transposition(true);
    match_flow_spec->set_copy_after_tcp_fin_flag_set(true);
    match_flow_spec->set_copy_after_tcp_rst_flag_set(true);
    match_flow_spec->set_vport_id(max_uint32());
    match_flow_spec->set_redirect_vport_id(max_uint32());
    match_flow_spec->set_ttl_one_redirect_vport_id(max_uint32());

    std::cout << "GftExactMatchFlowEntrySpec before GftHeaderGroupExactMatch size: "
              << match_flow_spec->ByteSizeLong()
              << std::endl;

    int count = 3;

    for (int i = 0; i < count; ++i) {
        gft_proto_size_gftheader_group_exact_match_check(match_flow_spec);
    }

    std::cout << "GftExactMatchFlowEntrySpec after GftHeaderGroupExactMatch size: "
              << match_flow_spec->ByteSizeLong()
              << std::endl;

    for (int i = 0; i < count; ++i) {
        gft_proto_size_gftheader_group_trans_check(match_flow_spec);
    }

    std::cout << "GftExactMatchFlowEntrySpec total size: "
              << match_flow_spec->ByteSizeLong()
              << std::endl;
}

int proxy_parse_args(int argc, char** argv, 
        uint64_t *vrf_id, 
        in_addr_t *src_range_start,
        in_addr_t *src_range_end,
        in_addr_t *dst_range_start,
        in_addr_t *dst_range_end)
{
    if (argc!= 7) {
        std::cout << "Usage:" <<  argv[0]  << " <VRF id> <SRC IP Address Range Start> <SRC IP Address Range End> <DST IP Address Range Start> <DST IP Address Range End>" << std::endl;
        return -1;
    }

    *vrf_id = strtoll(argv[2], NULL, 10);
    errno = 0;
    if (errno) {
        std::cout << "Failed to extract VRF ID from the parameters, Err:" << errno << std::endl;
        return -1;
    }

    *src_range_start = inet_addr(argv[3]);
    if (*src_range_start == INADDR_NONE) {
        std::cout << "Failed to extract SRC Range Start from the parameters" << std::endl;
        return -1;
    }

    *src_range_end = inet_addr(argv[4]);
    if (*src_range_start == INADDR_NONE) {
        std::cout << "Failed to extract SRC Range End from the parameters" << std::endl;
        return -1;
    }

    *dst_range_start = inet_addr(argv[5]);
    if (*dst_range_start == INADDR_NONE) {
        std::cout << "Failed to extract DST Range Start from the parameters" << std::endl;
        return -1;
    }

    *dst_range_end = inet_addr(argv[6]);
    if (*dst_range_end == INADDR_NONE) {
        std::cout << "Failed to extract DST Range End from the parameters" << std::endl;
        return -1;
    }
    return 0;
}

// main test driver
int
main (int argc, char** argv)
{
    uint64_t     vrf_handle = 0, l2seg_handle = 0, native_l2seg_handle = 0, sg_handle = 0;
    uint64_t     nw1_handle = 0, nw2_handle = 0, uplink_if_handle = 0;
    uint64_t     lif_handle = 0, enic_if_handle = 0, sec_prof_handle = 0, sec_policy_handle = 0;
    uint64_t     vrf_id = 1, l2seg_id = 1, sg_id = 1, if_id = 1, nw_id = 1;
    uint64_t     lif_id = 100;
    uint64_t     enic_if_id = 200;
    EncapInfo    l2seg_encap;
    bool         test_port = false;
    bool         test_port_get = false;
    std::string  svc_endpoint = hal_svc_endpoint_;

    bool         size_check = false;
    bool         ep_delete_test = false;
    bool         session_delete_test = false;
    bool         session_create = false;
    bool         session_create_cache_test = false;
    bool         system_get = false;
    bool         ep_create = false;
    bool         config = false;
    int          count = 1;
    bool         proxy_create = false;
    bool         tcpcb_stats_get = false;
    bool         tcpcb_get = false;
    int          tcpcb_id = 0;
    bool         bypass_tls = false;
    bool         proxy_agent_if = false;
    bool         proxy_srv = true;
    types::ProxyType            proxy_type = types::PROXY_TYPE_TCP;
    types::CryptoAsymKeyType    key_type = types::CRYPTO_ASYM_KEY_TYPE_ECDSA;

    uint64_t num_l2segments = 1;
    uint64_t encap_value    = 100;
    uint64_t num_uplinks    = 4;

    uint64_t dest_encap_value = encap_value + num_l2segments;
    uint64_t dest_l2seg_id    = l2seg_id    + num_l2segments;
    uint64_t dest_if_id       = if_id       + 1;

    uint32_t ip_address = 0;

    bool policer_update = false;
    bool pps = false;
    bool is_rx = false;
    uint32_t policer_lif_id = 0;
    uint64_t policer_rate = 0;
    uint64_t policer_burst = 0;
    uint64_t  proxy_vrf_id;
    in_addr_t src_range_start;
    in_addr_t src_range_end;
    in_addr_t dst_range_start;
    in_addr_t dst_range_end;

    bool qos_class_create = false;
    uint32_t qos_group = 1;
    uint32_t pcp = 1;
    uint32_t dscp = 0;
    uint32_t strict = 0;
    uint32_t rate_or_dwrr = 0;

    sdk::lib::pal_init(sdk::types::platform_type_t::PLATFORM_TYPE_MOCK);

    if (argc > 1) {
        if (!strcmp(argv[1], "port_test")) {
            test_port = true;
            svc_endpoint = linkmgr_svc_endpoint_;
        } else if (!strcmp(argv[1], "port_get")) {
            test_port_get = true;
            svc_endpoint = linkmgr_svc_endpoint_;
        } else if (!strcmp(argv[1], "size_check")) {
            size_check = true;
        } else if (!strcmp(argv[1], "system_get")) {
            system_get = true;
        } else if (!strcmp(argv[1], "ep_create")) {
            ep_create = true;
        } else if (!strcmp(argv[1], "ep_delete_test")) {
            if (argc != 5) {
                std::cout << "Usage: <pgm> ep_delete_test <uplink_if_handle> <l2seg_handle> <count>"
                          << std::endl;
                return 0;
            }
            uplink_if_handle = atoi(argv[2]);
            native_l2seg_handle = atoi(argv[3]);
            count = atoi(argv[5]);
            std::cout << "uplink_if_handle: " << uplink_if_handle
                      << "native_l2seg_handle: " << native_l2seg_handle
                      << std::endl;
            ep_delete_test = true;
        } else if (!strcmp(argv[1], "session_delete_test")) {
            if (argc != 2) {
                std::cout << "Usage: <pgm> session_delete_test"
                          << std::endl;
                return 0;
            }
            session_delete_test = true;
        } else if (!strcmp(argv[1], "session_create")) {
            session_create = true;
        } else if (!strcmp(argv[1], "session_create_cache_test")) {
            session_create_cache_test = true;
        } else if (!strcmp(argv[1], "policer_update")) {
            policer_update = true;
            is_rx = atoi(argv[2]); // 0 = TX, 1 = RX
            policer_lif_id = atoi(argv[3]);
            pps = atoi(argv[4]);
            policer_rate = atoi(argv[5]);
            policer_burst = atoi(argv[6]);
        } else if (!strcmp(argv[1], "qos_class_create")) {
            qos_class_create = true;
            qos_group = atoi(argv[2]);
            pcp = atoi(argv[3]);
            dscp = atoi(argv[4]);
            strict = atoi(argv[5]);
            rate_or_dwrr = atoi(argv[6]);
        } else if (!strcmp(argv[1], "proxy")) {
            proxy_create = true;
        } else if (!strcmp(argv[1], "tcpcb_get")) {
            if (argc != 3) {
                std::cout << "Usage: hal_test tcpcb_get <qid>"
                          << std::endl;
                return 0;
            }
            tcpcb_get = true;
            tcpcb_id = atoi(argv[2]);
        } else if (!strcmp(argv[1], "tcpcb_stats_get")) {
            if (argc != 3) {
                std::cout << "Usage: hal_test tcpcb_stats_get <qid>"
                          << std::endl;
                return 0;
            }
            tcpcb_stats_get = true;
            tcpcb_id = atoi(argv[2]);
        } else if (!strcmp(argv[1], "bypass_tls")) {
            bypass_tls = true;
        } else if (!strcmp(argv[1], "config")) {
            config = true;
        } else if (!strcmp(argv[1], "tcp-proxy")) {
            proxy_agent_if = true;
            proxy_type = types::PROXY_TYPE_TCP;
            if (proxy_parse_args(argc, argv, 
                        &proxy_vrf_id, 
                        &src_range_start,
                        &src_range_end,
                        &dst_range_start,
                        &dst_range_end))
            {
                return -1;
            }
        } else if (!strcmp(argv[1], "tcp-tls-proxy-server-ecdsa")) {
            proxy_agent_if = true;
            proxy_type = types::PROXY_TYPE_TLS;
            proxy_srv = true;
            key_type = types::CRYPTO_ASYM_KEY_TYPE_ECDSA;
            if (proxy_parse_args(argc, argv, 
                        &proxy_vrf_id, 
                        &src_range_start,
                        &src_range_end,
                        &dst_range_start,
                        &dst_range_end))
            {
                return -1;
            }
        } else if (!strcmp(argv[1], "tcp-tls-proxy-client-ecdsa")) {
            proxy_agent_if = true;
            proxy_type = types::PROXY_TYPE_TLS;
            proxy_srv = false;
            key_type = types::CRYPTO_ASYM_KEY_TYPE_ECDSA;
            if (proxy_parse_args(argc, argv, 
                        &proxy_vrf_id, 
                        &src_range_start,
                        &src_range_end,
                        &dst_range_start,
                        &dst_range_end))
            {
                return -1;
            }
        } else if (!strcmp(argv[1], "tcp-tls-proxy-client-rsa")) {
            proxy_agent_if = true;
            proxy_type = types::PROXY_TYPE_TLS;
            proxy_srv = false;
            key_type = types::CRYPTO_ASYM_KEY_TYPE_RSA;
            if (proxy_parse_args(argc, argv, 
                        &proxy_vrf_id, 
                        &src_range_start,
                        &src_range_end,
                        &dst_range_start,
                        &dst_range_end))
            {
                return -1;
            }
        }
    } else {
        std::cout << "Usage: <pgm> config" << std::endl;
        return 0;
    }

    hal_client hclient(grpc::CreateChannel(svc_endpoint,
                                           grpc::InsecureChannelCredentials()));
    if (test_port == true) {
        port_test(&hclient, vrf_id);
        return 0;
    } else if (test_port_get == true) {
        // ports_enable(&hclient, vrf_id);
        ports_get(&hclient, vrf_id);
        return 0;
    } else if (size_check == true) {
        gft_proto_size_check();
        return 0;
    } else if (system_get == true) {
        hclient.system_get();
        return 0;
    } else if (proxy_create) {
        hclient.proxy_enable(types::PROXY_TYPE_TCP);
        hclient.proxy_enable(types::PROXY_TYPE_TLS);
        hclient.bypass_tls();

        // n2n
        hclient.proxy_flow_config(true, vrf_id,
            0x0a0a0102,     // sip
            0x0a0a0104,     // dip
            0xbaba, // sport
            80,     // dport
            100);   // num_entries

        // h2n
        hclient.proxy_flow_config(true, vrf_id,
            0x0a0a010c,     // sip
            0x0a0a0103,     // dip
            0xbaba, // sport
            80,     // dport
            100);   // num_entries

        return 0;
    } else if (bypass_tls) {
        hclient.bypass_tls();
    } else if (tcpcb_get) {
        hclient.tcpcb_get(tcpcb_id);
        return 0;
    } else if (tcpcb_stats_get) {
        hclient.tcpcb_stats_get(tcpcb_id);
        return 0;
    } else if (session_delete_test == true) {

        std::cout << "session_delete_test" << std::endl;
        hclient.session_delete_all(0);

        return 0;

    } else if (session_create == true) {
        std::cout << "session_create" << std::endl;

        // create a session
        hclient.session_create(1, vrf_id, 0x0a0a01FD, 0x0a0a0105,
                               ::types::IPProtocol::IPPROTO_UDP,
                               32007, 2055, 1,
                               ::session::NAT_TYPE_NONE, 0, 0, 0, 0,
                               ::session::FlowAction::FLOW_ACTION_ALLOW,
                               0);
        return 0;
    } else if (ep_create == true) {
        ip_address = 0x0a0a01FD;
        hclient.ep_create(vrf_id, l2seg_id, if_id, sg_id, 0x0cc47a2a7b61, &ip_address, 1);

        return 0;
    } else if (ep_delete_test == true) {
        for (int i = 0; i < count; ++i) {
            hclient.ep_delete(vrf_id, l2seg_id, 0x0cc47a2a7b61);
            hclient.ep_delete(vrf_id, l2seg_id, 0x70695a480273);
            hclient.enic_if_delete(enic_if_id);

            for (uint64_t seg_id = l2seg_id;
                 seg_id < l2seg_id + num_l2segments;
                 ++seg_id) {
                hclient.del_l2seg_on_uplinks(if_id, num_uplinks, seg_id);
            }
            hclient.del_l2seg_on_uplinks(if_id, num_uplinks, dest_l2seg_id);

            for (uint64_t seg_id = l2seg_id;
                 seg_id < l2seg_id + num_l2segments;
                 ++seg_id) {
                hclient.add_l2seg_on_uplinks(if_id, num_uplinks, seg_id);
            }
            hclient.add_l2seg_on_uplinks(if_id, num_uplinks, dest_l2seg_id);

            // endpoint for PEER00
            ip_address = 0x0a0a0102;
            hclient.ep_create(vrf_id, l2seg_id, if_id, sg_id, 0x0cc47a2a7b61, &ip_address, 1);

            // endpoint for HOST03
            ip_address = 0x0a0a0104;
            hclient.ep_create(vrf_id, l2seg_id, dest_if_id, sg_id, 0x70695a480273, &ip_address, 1);

            // create a ENIC interface
            enic_if_handle = hclient.enic_if_create(enic_if_id, lif_id,
                                                    uplink_if_handle,  // pinned uplink
                                                    native_l2seg_handle,
                                                    dest_l2seg_id);
            ip_address = 0x0a0a0103;
            hclient.ep_create(vrf_id, l2seg_id, enic_if_id, sg_id,
                              0x020a0a0103, &ip_address, 1);
            assert(enic_if_handle != 0);
        }

        return 0;
    } else if (session_create_cache_test == true) {
        // create a session
        hclient.session_create(1, vrf_id, 0x0a0a0102, 0x0a0a0104,
                               ::types::IPProtocol::IPPROTO_UDP,
                               10000, 10002, 2500,
                               ::session::NAT_TYPE_NONE, 0, 0, 0, 0,
                               ::session::FlowAction::FLOW_ACTION_ALLOW,
                               0);
    } else if (policer_update) {
        hclient.lif_policer_update(policer_lif_id, is_rx, pps, policer_rate, policer_burst);
        return 0;
    } else if (proxy_agent_if == true) {

        if (proxy_type == types::PROXY_TYPE_TCP) {
            if (hclient.tcp_proxy_flow_setup(
                    vrf_id, 
                    src_range_start,
                    src_range_end,
                    dst_range_start,
                    dst_range_end)) {
                return -1;
            }
            else {
                return 0;
            }
        }
        else if (proxy_type == types::PROXY_TYPE_TLS) {
            if (proxy_srv == true) {
            }
            else {
                if (key_type == types::CRYPTO_ASYM_KEY_TYPE_ECDSA) {
                    if (hclient.tcp_tls_proxy_client_ecdsa_flow_setup(proxy_vrf_id,
                            src_range_start,
                            src_range_end,
                            dst_range_start,
                            dst_range_end
                            )) {
                        return -1;
                    }
                    else {
                        return 0;
                    }
                }
                else if (key_type == types::CRYPTO_ASYM_KEY_TYPE_RSA) {
                    if (hclient.tcp_tls_proxy_client_rsa_flow_setup(proxy_vrf_id,
                            src_range_start,
                            src_range_end,
                            dst_range_start,
                            dst_range_end
                            )) {
                        return -1;
                    }
                    else {
                        return 0;
                    }
                }
            }
        }
    } else if (qos_class_create) {
        hclient.qos_class_create(qos_group, pcp, dscp, strict, rate_or_dwrr);
        return 0;
    } else if (config == false) {
        std::cout << "Usage: <pgm> config" << std::endl;
        return 0;
    }

    // delete a non-existent vrf
    hclient.vrf_delete_by_id(1);

    // create a security profile
    sec_prof_handle = hclient.security_profile_create(1);
    assert(sec_prof_handle != 0);

    // create a vrf and perform GETs
    vrf_handle = hclient.vrf_create(vrf_id);
    assert(vrf_handle != 0);
    assert(hclient.vrf_get_by_handle(vrf_handle) != 0);
    assert(hclient.vrf_get_by_id(vrf_id) != 0);

    // recreate the vrf
    vrf_handle = hclient.vrf_create(vrf_id);
    assert(vrf_handle != 0);

    // create a security policy
    sec_policy_handle = hclient.security_policy_create(1, vrf_id);
    assert(sec_policy_handle != 0);

    // create a security group
    sg_handle = hclient.sg_create(sg_id);
    assert(sg_handle != 0);

    // create network objects
    nw1_handle = hclient.nw_create(nw_id, vrf_id, 0x0a0a0100, 24, 0x020a0a0101, sg_id);
    assert(nw1_handle != 0);
    nw_id++;
    nw2_handle = hclient.nw_create(nw_id, vrf_id, 0x0a0a0200, 24, 0x020a0a0201, sg_id);
    assert(nw2_handle != 0);

    // create uplinks with this L2 seg as native L2 seg
    uplink_if_handle = hclient.uplinks_create(if_id,
                                              num_uplinks,
                                              l2seg_id);
    assert(uplink_if_handle != 0);

    create_l2segments(l2seg_id, encap_value, if_id, num_l2segments,
                      nw1_handle, vrf_id, num_uplinks, hclient,
                      &l2seg_handle);

    native_l2seg_handle = l2seg_handle;

    l2seg_encap.set_encap_type(::types::ENCAP_TYPE_DOT1Q);
    l2seg_encap.set_encap_value(dest_encap_value);
    l2seg_handle =
        hclient.l2segment_create(vrf_id,
                                 dest_l2seg_id,
                                 nw2_handle,
                                 ::l2segment::BROADCAST_FWD_POLICY_FLOOD,
                                 ::l2segment::MULTICAST_FWD_POLICY_FLOOD,
                                 l2seg_encap);
    assert(l2seg_handle != 0);

    // bringup this L2seg on all uplinks
    hclient.add_l2seg_on_uplinks(if_id, 4, dest_l2seg_id);

    uint32_t ip_addr[3] = { 0x0a0a0102, 0x0a0a01FD, 0x0a0a01FE };

    // endpoint for PEER00
    hclient.ep_create(vrf_id, l2seg_id, if_id, sg_id, 0x0cc47a2a7b61, ip_addr, 3);

    ip_addr[0] = 0x0a0a0104;
    ip_addr[1] = 0x0a0a0105;

    // endpoint for HOST03
    hclient.ep_create(vrf_id, l2seg_id, dest_if_id, sg_id, 0x70695a480273, ip_addr, 2);

    hclient.gre_tunnel_if_create(vrf_id, 100, 0x0a0a01FD, 0x0a0a01FE);
    hclient.mirror_session_create(vrf_id, MIRROR_SESSION_ID,
                                  0x0a0a01FD, 0x0a0a01FE);  // 10.10.1.253 is our IP
    hclient.drop_monitor_rule_create(vrf_id, MIRROR_SESSION_ID,
                                     DROP_MONITOR_RULE_ID);

    // create a session for NAT case
    hclient.session_create(1, vrf_id, 0x0a0a0102, 0x0a0a01FD,
                           ::types::IPProtocol::IPPROTO_TCP, 10000, 11000, 1,
                           ::session::NAT_TYPE_TWICE_NAT,
                           0x0a0a01FD, 0x0a0a0105, 20000, 22000,
                           ::session::FlowAction::FLOW_ACTION_ALLOW,
                           0);   // no mirroring

    // create a netflow collector
    hclient.netflow_collector_create(vrf_id, 1, native_l2seg_handle,
                                     encap_value, 0x0a0a01FD,
                                     0x0a0a0105, 2055, 1);

    // create a lif
    lif_handle = hclient.lif_create(100, 1);
    assert(lif_handle != 0);

    hclient.lif_get_all();

    std::cout << "ENIC CREATE: uplink_if_handle: " << uplink_if_handle
              << ", native_l2seg_handle: " << native_l2seg_handle
              << std::endl;

    // create a ENIC interface
    enic_if_handle = hclient.enic_if_create(enic_if_id, lif_id,
                                            uplink_if_handle,  // pinned uplink
                                            native_l2seg_handle,
                                            dest_l2seg_id);
    assert(enic_if_handle != 0);

    // create EP with this ENIC
    ip_address = 0x0a0a0103;
    hclient.ep_create(vrf_id, l2seg_id, enic_if_id, sg_id, 0x020a0a0103, &ip_address, 1);

    // Get API stats
    hclient.api_stats_get();

    return 0;
}
