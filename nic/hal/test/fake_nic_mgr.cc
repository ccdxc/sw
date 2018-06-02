#include <stdio.h>
#include <unistd.h>
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
using types::Empty;

const char tx_qstate[] = {
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x11,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    (char)0xc0,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
};

const char rx_qstate[] = {
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x11,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    (char)0xc0,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x3f,
    0x00,
};

const char admin_qstate[] = {
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x11,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    (char)0xc0,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
};

class hal_client {
public:
    hal_client(std::shared_ptr<Channel> channel) :
        intf_stub_(Interface::NewStub(channel)) {
        channel_ = channel;
    }

    void wait_until_ready(void) {
        while (channel_->GetState(true) != GRPC_CHANNEL_READY) {
            std::cout << "Waiting for HAL to be ready ..." << std::endl;
            sleep(5);
        }
    }

    void lif_get_all (void) {
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

    uint64_t lif_create(uint32_t lif_id, uint32_t type_num, uint32_t num_lifs) {
        LifSpec              *spec;
        LifRequestMsg        req_msg;
        LifResponseMsg       rsp_msg;
        LifQStateMapEntry    *lif_qstate_map;
        QStateSetReq         *lif_qstate;
        ClientContext        context;
        Status               status;

        for (uint32_t i = 0; i < num_lifs; i++) {
            spec = req_msg.add_request();
            spec->mutable_key_or_handle()->set_lif_id(lif_id + i);
            spec->set_admin_status(::intf::IF_STATUS_UP);
            spec->mutable_packet_filter()->set_receive_broadcast(true);
            lif_qstate_map = spec->add_lif_qstate_map();
            lif_qstate_map->set_type_num(1);
            lif_qstate_map->set_size(1);
            lif_qstate_map->set_purpose(::intf::LIF_QUEUE_PURPOSE_TX);

            lif_qstate_map = spec->add_lif_qstate_map();
            lif_qstate_map->set_size(1);
            lif_qstate_map->set_purpose(::intf::LIF_QUEUE_PURPOSE_RX);

            lif_qstate_map = spec->add_lif_qstate_map();
            lif_qstate_map->set_type_num(2);
            lif_qstate_map->set_size(1);
            lif_qstate_map->set_purpose(::intf::LIF_QUEUE_PURPOSE_ADMIN);

            lif_qstate = spec->add_lif_qstate();
            lif_qstate->set_type_num(1);
            auto label = lif_qstate->mutable_label();
            label->set_handle("p4plus");
            label->set_prog_name("txdma_stage0.bin");
            label->set_label("eth_tx_stage0");
            lif_qstate->set_queue_state(tx_qstate, sizeof(tx_qstate));

            lif_qstate = spec->add_lif_qstate();
            label = lif_qstate->mutable_label();
            label->set_handle("p4plus");
            label->set_prog_name("rxdma_stage0.bin");
            label->set_label("eth_rx_stage0");
            lif_qstate->set_queue_state(rx_qstate, sizeof(rx_qstate));

            lif_qstate = spec->add_lif_qstate();
            lif_qstate->set_type_num(2);
            label = lif_qstate->mutable_label();
            label->set_handle("p4plus");
            label->set_prog_name("txdma_stage0.bin");
            label->set_label("adminq_stage0");
            lif_qstate->set_queue_state(admin_qstate, sizeof(admin_qstate));
        }
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
    std::unique_ptr<Interface::Stub> intf_stub_;
    std::shared_ptr<Channel> channel_;
};

int
main (int argc, char** argv)
{
    uint64_t       uplink_if_handle;
    uint64_t       lif_id = 100;
    uint64_t       if_id = 1;
    uint64_t       num_lifs = 16, num_uplinks = 2;
    std::string    svc_endpoint;

    grpc_init();
    if (getenv("HAL_GRPC_PORT")) {
        svc_endpoint = "localhost:" + std::string(getenv("HAL_GRPC_PORT"));
    } else {
        svc_endpoint = "localhost:50054";
    }

    // create gRPC channel to connect to HAL and wait until it is ready
    hal_client hclient(grpc::CreateChannel(svc_endpoint,
                                           grpc::InsecureChannelCredentials()));
    //hclient.wait_until_ready();

    // create uplinks
    uplink_if_handle = hclient.uplinks_create(if_id, num_uplinks);
    if (uplink_if_handle == 0) {
        exit(1);
    }

    // create lif(s)
    hclient.lif_create(lif_id, 1, num_lifs);
    hclient.lif_get_all();

    return 0;
}
