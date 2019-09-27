#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <math.h>
#include <grpc++/grpc++.h>
#include "gen/proto/vrf.grpc.pb.h"
#include "gen/proto/nic.grpc.pb.h"
#include "gen/proto/interface.grpc.pb.h"
#include "gen/proto/event.grpc.pb.h"
#include "gen/proto/system.grpc.pb.h"
#include "gen/proto/debug.grpc.pb.h"
#include "gen/proto/nic.pb.h"
#include "nic/include/eth_common.h"
#include "nic/include/adminq.h"


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

static bool is_smart_nic_mode() {
    return (getenv("SMART_NIC_MODE") && atoi(getenv("SMART_NIC_MODE")));
}

void
print_eth_qstate_intr(const eth_qstate_intr_t *intr)
{
    std::cout
        << "pc_offset: " << intr->pc_offset << std::endl
        << "rsvd:" << intr->rsvd << std::endl
        << "cosA:" << intr->cosA << std::endl
        << "cosB:" << intr->cosB << std::endl
        << "cos_sel: " << intr->cos_sel << std::endl
        << "eval_last: " << intr->eval_last << std::endl
        << "host: " << intr->host << std::endl
        << "total: " << intr->total << std::endl
        << "pid: " << intr->pid  << std::endl;
}

void
print_eth_qstate_ring(const eth_qstate_ring_t *ring, int i)
{
    std::cout
        << "p_index" << i << ": " << ring->p_index << std::endl
        << "c_index" << i << ": " << ring->c_index << std::endl;
}

void
print_eth_qstate_cfg(const eth_qstate_cfg_t *cfg)
{
    std::cout
        << "enable: " << cfg->enable << std::endl
        << "debug: " << cfg->debug << std::endl
        << "host_queue: " << cfg->host_queue << std::endl
        << "cpu_queue: " << cfg->cpu_queue << std::endl
        << "eq_enable: " << cfg->eq_enable << std::endl
        << "intr_enable: " << cfg->intr_enable << std::endl
        << "rsvd_cfg: " << cfg->rsvd_cfg << std::endl;
}

void
print_eth_qstate_common(const eth_qstate_common_t *q)
{
    print_eth_qstate_intr(&q->intr);
    print_eth_qstate_ring(&q->ring[0], 0);
    print_eth_qstate_ring(&q->ring[1], 1);
    print_eth_qstate_ring(&q->ring[2], 2);
    print_eth_qstate_cfg(&q->cfg);

    std::cout
        << "rsvd_db_cnt: " << q->rsvd_db_cnt << std::endl
        << "ring_size: " << q->ring_size << std::endl
        << "lif_index: " << q->lif_index << std::endl;
}

void
print_tx_qstate (const char *qstate)
{
    auto tx = reinterpret_cast<const eth_tx_co_qstate_t *>(qstate);

    std::cout
        << "tx_qstate: " << std::endl;

    print_eth_qstate_common(&tx->tx.q);

    std::cout
        << "comp_index: " << tx->tx.comp_index << std::endl
        << "color: " << tx->tx.sta.color << std::endl
        << "armed: " << tx->tx.sta.armed << std::endl
        << "rsvd_sta: " << tx->tx.sta.rsvd << std::endl
        << "lg2_desc_sz: " << tx->tx.lg2_desc_sz << std::endl
        << "lg2_cq_desc_sz: " << tx->tx.lg2_cq_desc_sz << std::endl
        << "lg2_sg_desc_sz: " << tx->tx.lg2_sg_desc_sz << std::endl
        << "ring_base: " << tx->tx.ring_base << std::endl
        << "cq_ring_base: " << tx->tx.cq_ring_base << std::endl
        << "sg_ring_base: " << tx->tx.sg_ring_base << std::endl
        << "intr_index_or_eq_addr: " << tx->tx.intr_index_or_eq_addr << std::endl
        << std::endl;
}


void
print_rx_qstate (const char *qstate)
{
    auto rx = reinterpret_cast<const eth_rx_qstate_t *>(qstate);

    std::cout
        << "rx_qstate: " << std::endl;

    print_eth_qstate_common(&rx->q);

    std::cout
        << "comp_index: " << rx->comp_index << std::endl
        << "color: " << rx->sta.color << std::endl
        << "armed: " << rx->sta.armed << std::endl
        << "rsvd_sta: " << rx->sta.rsvd << std::endl
        << "lg2_desc_sz: " << rx->lg2_desc_sz << std::endl
        << "lg2_cq_desc_sz: " << rx->lg2_cq_desc_sz << std::endl
        << "lg2_sg_desc_sz: " << rx->lg2_sg_desc_sz << std::endl
        << "sg_max_elems: " << rx->sg_max_elems << std::endl
        << "ring_base: " << rx->ring_base << std::endl
        << "cq_ring_base: " << rx->cq_ring_base << std::endl
        << "sg_ring_base: " << rx->sg_ring_base << std::endl
        << "intr_index_or_eq_addr: " << rx->intr_index_or_eq_addr << std::endl
        << std::endl;
}


void
print_admin_qstate (const char *qstate)
{
   admin_qstate_t *ad  = (admin_qstate_t *)qstate;
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
              << "color" << ad->sta.color << "\n"
              << "enable" << ad->cfg.enable << "\n"
              << "host_queue: " << ad->cfg.host_queue << "\n"
              << "ring_base" << ad->ring_base << "\n"
              << "ring_size" << ad->ring_size << "\n"
              << "cq_ring_base" << ad->cq_ring_base << "\n"
              << std::endl;
}

#define NUM_UPLINKS 2
class hal_client {
public:
    uint64_t       uplink_hdls[NUM_UPLINKS];
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

    uint64_t lif_create(uint32_t lif_id, uint32_t type_num, uint32_t num_lifs, uint32_t pinned_uplink_if_id) {
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
            if (is_smart_nic_mode()) {
                spec->mutable_pinned_uplink_if_key_handle()->set_interface_id(uplink_hdls[i%NUM_UPLINKS]);
            }
            spec->set_admin_status(::intf::IF_STATUS_UP);
            spec->mutable_packet_filter()->set_receive_broadcast(true);
            lif_qstate_map = spec->add_lif_qstate_map();
            lif_qstate_map->set_type_num(1);
            lif_qstate_map->set_size(2);
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
    int uplinks_create(uint64_t if_id_start, uint32_t num_uplinks) {
//        InterfaceSpec           *spec;
//        InterfaceRequestMsg     req_msg;
//        InterfaceResponseMsg    rsp_msg;
//        ClientContext           context;
//        Status                  status;
//        static uint64_t         port_num = 1;
//
//        for (uint32_t i = 0; i < num_uplinks; i++) {
//            spec = req_msg.add_request();
//            spec->mutable_key_or_handle()->set_interface_id(if_id_start++);
//            spec->set_type(::intf::IfType::IF_TYPE_UPLINK);
//            spec->set_admin_status(::intf::IfStatus::IF_STATUS_UP);
//            spec->mutable_if_uplink_info()->set_port_num(port_num++);
//        }
//        status = intf_stub_->InterfaceCreate(&context, req_msg, &rsp_msg);
//        if (status.ok()) {
//            for (uint32_t i = 0; i < num_uplinks; i++) {
//                assert(rsp_msg.response(i).api_status() == types::API_STATUS_OK);
//                std::cout << "Uplink interface create succeeded, handle = "
//                          << rsp_msg.response(i).status().if_handle()
//                          << std::endl;
//                if (rsp_msg.response(i).status().if_handle() == 0) {
//                    return -1;
//                }
//                uplink_hdls[i] = rsp_msg.response(i).status().if_handle();
//            }
//        } else {
//            for (uint32_t i = 0; i < num_uplinks; i++) {
//                std::cout << "Uplink interface create failed, error = "
//                          << rsp_msg.response(i).api_status()
//                          << std::endl;
//            }
//        }

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
    uint64_t       lif_id = 100;
    uint64_t       if_id = 128;
    uint64_t       num_lifs = 16;
    std::string    svc_endpoint;
    std::string    max_lifs;
    int            ret;

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

    if (is_smart_nic_mode()) {
        hclient.device_set_smart_nic_mode();
    }

    // create uplinks
    ret = hclient.uplinks_create(if_id, NUM_UPLINKS);
    if (ret < 0) {
        exit(1);
    }

    // create lif(s)
    hclient.lif_create(lif_id, 1, num_lifs, if_id);
    hclient.lif_get_all();

    return 0;
}
