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
#include "nic/gen/proto/hal/gft.grpc.pb.h"
#include "nic/gen/proto/hal/telemetry.grpc.pb.h"

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
using telemetry::MirrorSessionConfigMsg;
using telemetry::MirrorSessionResponseMsg;

std::string  hal_svc_endpoint_     = "localhost:50054";
std::string  linkmgr_svc_endpoint_ = "localhost:50053";

port::PortOperStatus port_oper_status = port::PORT_OPER_STATUS_NONE;
port::PortType       port_type        = port::PORT_TYPE_NONE;
port::PortAdminState port_admin_state = port::PORT_ADMIN_STATE_NONE;
port::PortSpeed      port_speed       = port::PORT_SPEED_NONE;

class hal_client {
public:
    hal_client(std::shared_ptr<Channel> channel) : vrf_stub_(Vrf::NewStub(channel)),
    l2seg_stub_(L2Segment::NewStub(channel)), port_stub_(Port::NewStub(channel)),
    event_stub_(Event::NewStub(channel)), system_stub_(System::NewStub(channel)),
    debug_stub_(Debug::NewStub(channel)), intf_stub_(Interface::NewStub(channel)),
    sg_stub_(NwSecurity::NewStub(channel)), nw_stub_(Network::NewStub(channel)),
    ep_stub_(Endpoint::NewStub(channel)), session_stub_(Session::NewStub(channel)),
    telemetry_stub_(Telemetry::NewStub(channel)) {}

    int mpu_trace_enable(int stage_id,
                         int mpu,
                         bool enable,
                         char *pipeline_type)
    {
        ClientContext              context;
        Status                     status;
        debug::MpuTraceRequestMsg  req_msg;
        debug::MpuTraceResponseMsg rsp_msg;

        debug::MpuTraceRequest *req = req_msg.add_request();

        if (!strcmp(pipeline_type, "p4_ingress")) {
            req->set_pipeline_type(debug::MPU_TRACE_PIPELINE_P4_INGRESS);
        } else if (!strcmp(pipeline_type, "p4_egress")) {
            req->set_pipeline_type(debug::MPU_TRACE_PIPELINE_P4_EGRESS);
        } else {
            req->set_pipeline_type(debug::MPU_TRACE_PIPELINE_NONE);
        }

        if (stage_id != -1) {
            req->set_stage_id(stage_id);
        }

        if (mpu != -1) {
            req->set_mpu(mpu);
        }

        req->set_wrap(1);
        req->set_table_key(true);
        req->set_instructions(false);
        req->set_enable(enable);

        status = debug_stub_->MpuTraceOpn(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            std::cout << "MPU trace "
                      << (enable ? "enable" : "disable")
                      << " succeeded"
                      << std::endl;
            return 0;
        }

        std::cout << "MPU trace "
                  << (enable ? "enable" : "disable")
                  << " failed"
                  << std::endl;

        return -1;
    }

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

    uint64_t session_create(uint64_t session_id, uint64_t vrf_id, uint32_t sip, uint32_t dip,
                            ::types::IPProtocol proto, uint16_t sport, uint16_t dport,
                            ::session::NatType nat_type,
                            uint32_t nat_sip, uint32_t nat_dip,
                            uint16_t nat_sport, uint16_t nat_dport,
                            ::session::FlowAction action,
                            uint32_t ing_mirror_session_id) {
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

        // create initiator flow
        flow = spec->mutable_initiator_flow();
        flow->mutable_flow_key()->mutable_v4_key()->set_sip(sip);
        flow->mutable_flow_key()->mutable_v4_key()->set_dip(dip);
        flow->mutable_flow_key()->mutable_v4_key()->set_ip_proto(proto);
        flow->mutable_flow_key()->mutable_v4_key()->mutable_tcp_udp()->set_sport(sport);
        flow->mutable_flow_key()->mutable_v4_key()->mutable_tcp_udp()->set_dport(dport);
        flow->mutable_flow_data()->mutable_flow_info()->set_flow_action(action);
        if (ing_mirror_session_id) {
            auto msess = flow->mutable_flow_data()->mutable_flow_info()->add_ing_mirror_sessions();
            msess->set_session_id(ing_mirror_session_id);
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
            msess->set_session_id(ing_mirror_session_id);
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

    uint32_t mirror_session_create(uint32_t vrf_id, uint32_t session_id,
                                   uint32_t sip, uint32_t dip) {
        MirrorSessionConfigMsg      req_msg;
        MirrorSessionSpec           *spec;
        MirrorSessionResponseMsg    rsp_msg;
        Status                      status;
        ClientContext               context;

        spec = req_msg.add_request();
        spec->mutable_meta()->set_vrf_id(vrf_id);
        spec->mutable_id()->set_session_id(session_id);
        spec->mutable_erspan_spec()->mutable_dest_ip()->set_ip_af(::types::IP_AF_INET);
        spec->mutable_erspan_spec()->mutable_dest_ip()->set_v4_addr(sip);
        spec->mutable_erspan_spec()->mutable_src_ip()->set_ip_af(::types::IP_AF_INET);
        spec->mutable_erspan_spec()->mutable_dest_ip()->set_v4_addr(dip);
        spec->mutable_erspan_spec()->set_span_id(session_id);

        status = telemetry_stub_->MirrorSessionCreate(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            assert(rsp_msg.status() == types::API_STATUS_OK);
            assert(rsp_msg.response(0).api_status() == types::API_STATUS_OK);
            assert(rsp_msg.response(0).status().code() ==
                       ::telemetry::MirrorSessionStatus_MirrorSessionStatusCode_SUCCESS);
            std::cout << "Mirror session ssucceeded, id = " << session_id << std::endl;
        }

        return 0;
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
    return std::numeric_limits<unsigned long>::max();
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

// main test driver
int
main (int argc, char** argv)
{
    uint64_t     vrf_handle, l2seg_handle, native_l2seg_handle, sg_handle;
    uint64_t     nw1_handle, nw2_handle, uplink_if_handle, session_handle;
    uint64_t     lif_handle, enic_if_handle;
    uint64_t     vrf_id = 1, l2seg_id = 1, sg_id = 1, if_id = 1, nw_id = 1;
    uint64_t     lif_id = 100;
    uint64_t     enic_if_id = 200;
    EncapInfo    l2seg_encap;
    bool         test_port = false;
    bool         test_port_get = false;
    std::string  svc_endpoint = hal_svc_endpoint_;

    bool         mpu_trace = false;
    bool         enable = false;
    bool         size_check = false;
    bool         ep_delete_test = false;
    bool         session_delete_test = false;
    bool         session_create = false;
    bool         system_get = false;
    bool         ep_create = false;
    bool         config = false;
    int          stage_id = -1;
    int          mpu = -1;
    char         pipeline_type[32] = {0};
    int          count = 1;

    uint64_t num_l2segments = 1;
    uint64_t encap_value    = 100;
    uint64_t num_uplinks    = 4;

    uint64_t dest_encap_value = encap_value + num_l2segments;
    uint64_t dest_l2seg_id    = l2seg_id    + num_l2segments;
    uint64_t dest_if_id       = if_id       + 1;

    uint32_t ip_address = 0;

    if (argc > 1) {
        if (!strcmp(argv[1], "port_test")) {
            test_port = true;
            svc_endpoint = linkmgr_svc_endpoint_;
        } else if (!strcmp(argv[1], "port_get")) {
            test_port_get = true;
            svc_endpoint = linkmgr_svc_endpoint_;
        } else if (!strcmp(argv[1], "mpu_trace_enable")) {
            mpu_trace = true;
            enable = true;
        } else if (!strcmp(argv[1], "mpu_trace_disable")) {
            mpu_trace = true;
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
        } else if (!strcmp(argv[1], "config")) {
            config = true;
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
    } else if (mpu_trace == true) {
        if (argc != 5) {
            std::cout << "Usage: <pgm> <mpu_trace_enable/mpu_trace_disable>"
                         " <p4_ingress/p4_egress/none> stage_id mpu"
                      << std::endl;
            return 0;
        }

        strcpy(pipeline_type, argv[2]);
        stage_id = atoi(argv[3]);
        mpu = atoi(argv[4]);

        hclient.mpu_trace_enable(stage_id, mpu, enable, pipeline_type);
        return 0;
    } else if (size_check == true) {
        gft_proto_size_check();
        return 0;
    } else if (system_get == true) {
        hclient.system_get();
        return 0;
    } else if (session_delete_test == true) {

        std::cout << "session_delete_test" << std::endl;
        hclient.session_delete_all(0);

        return 0;

    } else if (session_create == true) {
        std::cout << "session_create" << std::endl;

        // create a session
        session_handle = hclient.session_create(1, vrf_id, 0x0a0a0102, 0x0a0a0104,
                                                ::types::IPProtocol::IPPROTO_UDP,
                                                10000, 10001,
                                                ::session::NAT_TYPE_NONE, 0, 0, 0, 0,
                                                ::session::FlowAction::FLOW_ACTION_ALLOW,
                                                1);
        assert(session_handle != 0);

        return 0;

    } else if (ep_create == true) {
        ip_address = 0x0a0a0102;
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
    } else if (config == false) {
        std::cout << "Usage: <pgm> config" << std::endl;
        return 0;
    }

    // delete a non-existent vrf
    hclient.vrf_delete_by_id(1);

    // create a vrf and perform GETs
    vrf_handle = hclient.vrf_create(vrf_id);
    assert(vrf_handle != 0);
    assert(hclient.vrf_get_by_handle(vrf_handle) != 0);
    assert(hclient.vrf_get_by_id(vrf_id) != 0);

    // recreate the vrf
    vrf_handle = hclient.vrf_create(vrf_id);
    assert(vrf_handle != 0);

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

    uint32_t ip_addr[2] = { 0x0a0a0102, 0x0a0a01FE };

    // endpoint for PEER00
    hclient.ep_create(vrf_id, l2seg_id, if_id, sg_id, 0x0cc47a2a7b61, ip_addr, 2);

    ip_addr[0] = 0x0a0a0104;
    ip_addr[1] = 0x0a0a0105;

    // endpoint for HOST03
    hclient.ep_create(vrf_id, l2seg_id, dest_if_id, sg_id, 0x70695a480273, ip_addr, 2);

    hclient.gre_tunnel_if_create(vrf_id, 100, 0x0a0a01FD, 0x0a0a01FE);
    hclient.mirror_session_create(1, vrf_id,
                                  0x0a0a01FD, 0x0a0a01FE);  // 10.10.1.253 is our IP

    // create a session for NAT case
    session_handle = hclient.session_create(1, vrf_id, 0x0a0a0102, 0x0a0a01FD,
                                            ::types::IPProtocol::IPPROTO_TCP, 10000, 11000,
                                            ::session::NAT_TYPE_TWICE_NAT,
                                            0x0a0a01FD, 0x0a0a0105, 20000, 22000,
                                            ::session::FlowAction::FLOW_ACTION_ALLOW,
                                            0);   // no mirroring
    assert(session_handle != 0);

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
