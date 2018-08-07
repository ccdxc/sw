#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <math.h>
#include <grpc++/grpc++.h>
#include "nic/gen/proto/hal/vrf.grpc.pb.h"
#include "nic/gen/proto/hal/nic.grpc.pb.h"
#include "nic/gen/proto/hal/interface.grpc.pb.h"
#include "nic/gen/proto/hal/event.grpc.pb.h"
#include "nic/gen/proto/hal/system.grpc.pb.h"
#include "nic/gen/proto/hal/debug.grpc.pb.h"
#include "nic/gen/proto/hal/nic.pb.h"
#include "nic/include/eth_common.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using intf::Interface;
using intf::InterfaceSpec;
using intf::InterfaceRequestMsg;
using intf::InterfaceResponse;
using intf::InterfaceResponseMsg;
using device::DeviceRequestMsg;
using device::DeviceResponseMsg;
using device::Nic;
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
    (char)0xe0,
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
    (char)0xe0,
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
    (char)0xe0,
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

void
print_tx_qstate (const char *qstate)
{
   eth_tx_qstate  *tx  = (eth_tx_qstate *)qstate;
   std::cout  << "tx_qstate: " 
              << "pc offset: " << tx->pc_offset << "\n"
              << "rsvd0:" << tx->rsvd0 << "\n"
              << "cosA:" << tx->cosA << "\n"
              << "cosB:" << tx->cosB << "\n"
              << "cos_sel: " << tx->cos_sel << "\n"
              << "eval_last: " << tx->eval_last << "\n"
              << "host: " << tx->host << "\n"
              << "total: " << tx->total << "\n"
              << "pid: " << tx->pid  << "\n"
              << "p_index0: " << tx->p_index0 << "\n"
              << "c_index0: " << tx->c_index0  << "\n"
              << "comp_index: " << tx->comp_index << "\n"
              << "ci_fetch: " << tx->ci_fetch << "\n"
              << "enable: " << tx->enable << "\n"
              << "color: " << tx->color << "\n"
              << "host_queue: " << tx->host_queue << "\n"
              << "rsvd1: " << tx->rsvd1 << "\n"
              << "ring_base: " << tx->ring_base << "\n"
              << "ring_size: " << tx->ring_size << "\n"
              << "cq_ring_base: " << tx->cq_ring_base << "\n"
              << "intr_assert_addr: " << tx->intr_assert_addr << "\n"
              << "spurious_db_cnt: " << tx->spurious_db_cnt << "\n"
              << "sg_ring_base: " << tx->sg_ring_base << "\n"
              << std::endl;
}


void
print_rx_qstate (const char *qstate)
{
   eth_rx_qstate  *rx  = (eth_rx_qstate *)qstate;
   std::cout  << "rx_qstate: " 
              << "pc offset " << rx->pc_offset << "\n"
              << "rsvd0" << rx->rsvd0 << "\n"
              << "cosA" << rx->cosA << "\n"
              << "cosB" << rx->cosB << "\n"
              << "cos_sel" << rx->cos_sel << "\n"
              << "eval_last" << rx->eval_last << "\n"
              << "host" << rx->host << "\n"
              << "total" << rx->total << "\n"
              << "pid" << rx->pid << "\n"
              << "p_index0" << rx->p_index0 << "\n"
              << "c_index0" << rx->c_index0 << "\n"
              << "comp_index" << rx->comp_index << "\n"
              << "c_index1" << rx->c_index1 << "\n"
              << "enable" << rx->enable << "\n"
              << "color" << rx->color << "\n"
              << "host_queue: " << rx->host_queue << "\n"
              << "rsvd1" << rx->rsvd1 << "\n"
              << "ring_base" << rx->ring_base << "\n"
              << "ring_size" << rx->ring_size << "\n"
              << "cq_ring_base" << rx->cq_ring_base << "\n"
              << "intr_assert_addr" << rx->intr_assert_addr << "\n"
              << "rss_type" << rx->rss_type << "\n"
              << std::endl;
}


void
print_admin_qstate (const char *qstate)
{
   eth_admin_qstate *ad  = (eth_admin_qstate *)qstate;
   std::cout  << "admin_qstate: " 
              << "pc offset " << ad->pc_offset << "\n"
              << "rsvd0" << ad->rsvd0 << "\n"
              << "cosA" << ad->cosA << "\n" 
              << "cosB" << ad->cosB << "\n"
              << "cos_sel" << ad->cos_sel << "\n"
              << "eval_last" << ad->eval_last << "\n"
              << "host" << ad->host << "\n"
              << "total" << ad->total << "\n"
              << "pid" << ad->pid << "\n"
              << "p_index0" << ad->p_index0 << "\n"
              << "c_index0" << ad->c_index0 << "\n"
              << "comp_index" << ad->comp_index << "\n"
              << "ci_fetch" << ad->ci_fetch << "\n"
              << "enable" << ad->enable << "\n"
              << "color" << ad->color << "\n"
              << "host_queue: " << ad->host_queue << "\n"
              << "rsvd1" << ad->rsvd1 << "\n"
              << "ring_base" << ad->ring_base << "\n"
              << "ring_size" << ad->ring_size << "\n"
              << "cq_ring_base" << ad->cq_ring_base << "\n"
              << std::endl;
}

class hal_client {
public:
    hal_client(std::shared_ptr<Channel> channel) :
        intf_stub_(Interface::NewStub(channel)),
        nic_stub_(Nic::NewStub(channel)) {
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
                          << "HW_LIF_ID: "
                          << rsp_msg.response(i).status().hw_lif_id()
                          << std::endl;
            }
        } else {
            std::cout << "Lif Get Failed" << std::endl;
        }
    }

    void device_set_smart_nic_mode(void) {
        DeviceRequestMsg            nic_req;
        DeviceResponseMsg           nic_rsp;
        Status                      status;
        ClientContext               context;

        // Set device mode as Smart switch
        nic_req.mutable_request()->mutable_device()->set_device_mode(device::DEVICE_MODE_MANAGED_SWITCH);

        status = nic_stub_->DeviceCreate(&context, nic_req, &nic_rsp);
        if (status.ok()) {
            assert(nic_rsp.response().api_status() == types::API_STATUS_OK);
            std::cout << "Device set to Smart NIC Mode"
                      << std::endl;
        } else {
        std::cout << "Device not set to Smart NIC Mode"
                  << std::endl;
        }

        return;
    }

    uint64_t lif_create(uint32_t lif_id, uint32_t type_num, uint32_t num_lifs) {
        LifSpec              *spec;
        LifRequestMsg        req_msg;
        LifResponseMsg       rsp_msg;
        LifQStateMapEntry    *lif_qstate_map;
        QStateSetReq         *lif_qstate;
        ClientContext        context;
        Status               status;
        
        print_tx_qstate(tx_qstate); 
        print_rx_qstate(rx_qstate);
        print_admin_qstate(admin_qstate);

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

            for (int32_t idx = 0; idx < rsp_msg.response_size(); idx++) {
                assert(rsp_msg.response(idx).api_status() == types::API_STATUS_OK);
                std::cout << "Lif create succeeded, handle = "
                    << rsp_msg.response(idx).status().lif_handle()
                    << " DPS_DBG: qstate().size = "
                    << rsp_msg.response(idx).qstate().size()
                    << std::endl;

                auto & rsp = rsp_msg.response(idx);
                for (int i = 0; i < rsp.qstate().size(); i++) {
                    auto & qstate = rsp.qstate()[i];
                    std::cout << "[INFO] lif " << rsp.status().hw_lif_id()
                        << " qtype " << qstate.type_num()
                        << " qstate 0x" << std::hex << qstate.addr() 
                        << std::endl;
                }
            }//end of for

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
    std::unique_ptr<Nic::Stub> nic_stub_;
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
    std::string    max_lifs;

    grpc_init();
    if (getenv("HAL_GRPC_PORT")) {
        svc_endpoint = "localhost:" + std::string(getenv("HAL_GRPC_PORT"));
    } else {
        svc_endpoint = "localhost:50054";
    }
    if (getenv("MAX_LIFS")) {
        max_lifs = std::string(getenv("MAX_LIFS"));
	num_lifs = strtoul(max_lifs.c_str(), NULL, 0);
    }

    // create gRPC channel to connect to HAL and wait until it is ready
    hal_client hclient(grpc::CreateChannel(svc_endpoint,
                                           grpc::InsecureChannelCredentials()));
    //hclient.wait_until_ready();

    hclient.device_set_smart_nic_mode();

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
