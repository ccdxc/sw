#include <stdio.h>
#include <iostream>
#include <thread>
#include <math.h>
#include <grpc++/grpc++.h>
#include "nic/gen/proto/hal/vrf.grpc.pb.h"
#include "nic/gen/proto/hal/interface.grpc.pb.h"
#include "nic/gen/proto/hal/event.grpc.pb.h"
#include "nic/gen/proto/hal/system.grpc.pb.h"
#include "nic/gen/proto/hal/debug.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using vrf::Vrf;
using intf::Interface;
using intf::InterfaceSpec;
using intf::InterfaceRequestMsg;
using intf::InterfaceResponse;
using intf::InterfaceResponseMsg;
using intf::LifSpec;
using intf::LifGetRequestMsg;
using intf::LifGetResponseMsg;
using intf::LifRequestMsg;
using intf::LifResponseMsg;
using intf::LifQStateMapEntry;
using intf::QStateSetReq;
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
using event::Event;
using event::EventRequest;
using event::EventResponse;

std::string  hal_svc_endpoint_     = "localhost:50054";

class hal_client {
public:
    hal_client(std::shared_ptr<Channel> channel) : vrf_stub_(Vrf::NewStub(channel)),
    event_stub_(Event::NewStub(channel)), system_stub_(System::NewStub(channel)),
    debug_stub_(Debug::NewStub(channel)), intf_stub_(Interface::NewStub(channel)) {}

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

       // create few uplinks and return the handle for the 1st one
        uint64_t uplinks_create(uint64_t if_id_start, uint32_t num_uplinks) {
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

private:
    std::unique_ptr<Vrf::Stub> vrf_stub_;
    std::unique_ptr<Event::Stub> event_stub_;
    std::unique_ptr<System::Stub> system_stub_;
    std::unique_ptr<Debug::Stub> debug_stub_;
    std::unique_ptr<Interface::Stub> intf_stub_;
};

// main test driver
int
main (int argc, char** argv)
{
    uint64_t     uplink_if_handle; //, session_handle;
    uint64_t     lif_handle;
    uint64_t     if_id = 1;
    std::string  svc_endpoint = hal_svc_endpoint_;
    uint64_t num_uplinks    = 4;
    hal_client hclient(grpc::CreateChannel(svc_endpoint,
                                           grpc::InsecureChannelCredentials()));
    // create uplinks
    uplink_if_handle = hclient.uplinks_create(if_id, num_uplinks);
    assert(uplink_if_handle != 0);

    // create a lif
    lif_handle = hclient.lif_create(100, 1);
    assert(lif_handle != 0);

    hclient.lif_get_all();
    hclient.api_stats_get();
    return 0;
}
