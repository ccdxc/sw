
#include <cmath>
#include <iostream>
#include <iomanip>
#include <grpc++/grpc++.h>

#include "interface.grpc.pb.h"

#include "lif.hpp"
#include "ethlif.hpp"
#include "uplink.hpp"

using namespace std;

sdk::lib::indexer *Lif::allocator = sdk::lib::indexer::factory(Lif::max_lifs, false, true);

Lif *
Lif::Factory(EthLif *eth_lif)
{
    Lif *lif = new Lif(eth_lif);

    return lif;
}

void
Lif::Destroy(Lif *lif)
{
    if (lif) {
        lif->~Lif();
    }
}


Lif::Lif(EthLif * eth_lif)
{
    grpc::ClientContext        context;
    grpc::Status               status;

    intf::LifSpec              *req;
    intf::LifResponse          rsp;
    intf::LifRequestMsg        req_msg;
    intf::LifResponseMsg       rsp_msg;
    intf::LifQStateMapEntry    *lif_qstate_map_ent;
    // intf::QStateSetReq         *qstate_req;
    hal_lif_info_t             *lif_info = eth_lif->GetLifInfo();

    if (lif_info->id == 0) {
        if (allocator->alloc(&id_) != sdk::lib::indexer::SUCCESS) {
            HAL_TRACE_ERR("Failed to create Lif for hw_lif_id: {}. Index Exhaustion",
                          lif_info->hw_lif_id);
            return;
        }
    } else {
        id_ = lif_info->id;
    }

    // Set default number of max filters if nothing is passed
    if (!lif_info->max_vlan_filters) {
        lif_info->max_vlan_filters = LIF_DEFAULT_MAX_VLAN_FILTERS;
    }

    if (!lif_info->max_mac_filters) {
        lif_info->max_mac_filters = LIF_DEFAULT_MAX_MAC_FILTERS;
    }

    if (!lif_info->max_mac_vlan_filters) {
        lif_info->max_mac_vlan_filters = LIF_DEFAULT_MAX_MAC_VLAN_FILTERS;
    }

    eth_lif_ = eth_lif;

    HAL_TRACE_DEBUG("Creating Lif: prom: {}, oob: {}, rdma_en: {}",
                    lif_info->receive_promiscuous,
                    eth_lif_->IsOOBMnic(),
                    lif_info->enable_rdma);

    req = req_msg.add_request();
    req->mutable_key_or_handle()->set_lif_id(id_);
    req->set_hw_lif_id(lif_info->hw_lif_id);
    req->mutable_pinned_uplink_if_key_handle()->
        set_interface_id(lif_info->pinned_uplink ? lif_info->pinned_uplink->GetId() : 0);
    req->mutable_packet_filter()->set_receive_broadcast(lif_info->receive_broadcast);
    req->mutable_packet_filter()->set_receive_all_multicast(lif_info->receive_all_multicast);
    req->mutable_packet_filter()->set_receive_promiscuous(lif_info->receive_promiscuous);
    req->set_vlan_strip_en(lif_info->vlan_strip_en);
    req->set_vlan_insert_en(lif_info->vlan_insert_en);
    req->set_is_management(eth_lif_->IsOOBMnic());
    req->set_admin_status(::intf::IF_STATUS_UP);
    req->set_enable_rdma(lif_info->enable_rdma);
    
    // Populate qstate map
    for (uint32_t i = 0; i < NUM_QUEUE_TYPES; i++) {
        auto & qinfo = lif_info->queue_info[i];
        HAL_TRACE_DEBUG("Processing queue type: {}, size: {}", i, qinfo.size);
        if (qinfo.size < 1) continue;

        HAL_TRACE_DEBUG("Queue type_num: {}, entries: {}, purpose: {}, prog: {}, label: {}",
                        qinfo.type_num,
                        qinfo.entries, qinfo.purpose, qinfo.prog, qinfo.label);

        lif_qstate_map_ent = req->add_lif_qstate_map();
        lif_qstate_map_ent->set_type_num(qinfo.type_num);
        lif_qstate_map_ent->set_size(qinfo.size);
        lif_qstate_map_ent->set_entries(qinfo.entries);
        lif_qstate_map_ent->set_purpose(qinfo.purpose);
#if 0
        for (uint32_t qid = 0; qid < (uint32_t)pow(2, qinfo.entries); qid++) {
            qstate_req = req->add_lif_qstate();
            qstate_req->set_lif_handle(0);
            qstate_req->set_type_num(qinfo.type_num);
            qstate_req->set_qid(qid);
            qstate_req->set_queue_state(qinfo.qstate, (uint32_t)pow(2, qinfo.size + 5));
            qstate_req->mutable_label()->set_handle("p4plus");
            qstate_req->mutable_label()->set_prog_name(qinfo.prog);
            qstate_req->mutable_label()->set_label(qinfo.label);
        }
#endif
    }

    status = hal->lif_create(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            handle_ = rsp.status().lif_handle();
            hw_lif_id_ = rsp.status().hw_lif_id();
            HAL_TRACE_DEBUG("Created Lif id: {} hw_lif_id: {}, handle: {}",
                            id_, hw_lif_id_, handle_);
        } else {
            HAL_TRACE_ERR("Failed to create Lif for hw_lif_id: {}. err: {}",
                          hw_lif_id_, rsp.api_status());
        }
    } else {
        HAL_TRACE_ERR("Failed to create Lif for hw_lif_id: {}. err: {}:{}",
                      hw_lif_id_, status.error_code(), status.error_message());
    }

    // Store spec
    spec.CopyFrom(*req);
}

Lif::~Lif()
{
    grpc::ClientContext           context;
    grpc::Status                  status;

    intf::LifDeleteRequest        *req;
    intf::LifDeleteResponse       rsp;
    intf::LifDeleteRequestMsg     req_msg;
    intf::LifDeleteResponseMsg    rsp_msg;

    req = req_msg.add_request();
    req->mutable_key_or_handle()->set_lif_id(id_);
    status = hal->lif_delete(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            HAL_TRACE_DEBUG("Deleted Lif id: {} hw_lif_id: {}, handle: {}",
                            id_, eth_lif_->GetLifInfo()->hw_lif_id, handle_);
        } else {
            HAL_TRACE_ERR("Failed to delete Lif for id: {}. err: {}",
                          id_, rsp.api_status());
        }
    } else {
        HAL_TRACE_ERR("Failed to delete Lif for id: {}. err: {}:{}",
                      id_, status.error_code(), status.error_message());
    }
}

void
Lif::TriggerHalUpdate()
{
    grpc::ClientContext        context;
    grpc::Status               status;

    intf::LifSpec              *req;
    intf::LifResponse          rsp;
    intf::LifRequestMsg        req_msg;
    intf::LifResponseMsg       rsp_msg;
    hal_lif_info_t             *lif_info = eth_lif_->GetLifInfo();

    req = req_msg.add_request();
    req->mutable_key_or_handle()->set_lif_id(id_);
    req->set_hw_lif_id(lif_info->hw_lif_id);
    req->mutable_pinned_uplink_if_key_handle()->set_interface_id(lif_info->pinned_uplink->GetId());
    req->mutable_packet_filter()->set_receive_broadcast(lif_info->receive_broadcast);
    req->mutable_packet_filter()->set_receive_all_multicast(lif_info->receive_all_multicast);
    req->mutable_packet_filter()->set_receive_promiscuous(lif_info->receive_promiscuous);
    req->set_vlan_strip_en(lif_info->vlan_strip_en);
    req->set_vlan_insert_en(lif_info->vlan_insert_en);
    req->set_is_management(eth_lif_->IsOOBMnic());
    req->set_admin_status(::intf::IF_STATUS_UP);
    req->set_enable_rdma(lif_info->enable_rdma);
    
    status = hal->lif_update(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            HAL_TRACE_DEBUG("Created Lif id: {} hw_lif_id: {}, handle: {}",
                            id_, lif_info->hw_lif_id, handle_);
        } else {
            HAL_TRACE_ERR("Failed to create Lif for id: {}. err: {}",
                          id_, rsp.api_status());
        }
    } else {
        HAL_TRACE_ERR("Failed to create Lif for id: {}. err: {}:{}",
                      id_, status.error_code(), status.error_message());
    }

    // Store spec
    spec.CopyFrom(*req);
}

uint32_t
Lif::GetId()
{
    return id_;
}

uint32_t
Lif::GetHwLifId()
{
    return hw_lif_id_;
}
