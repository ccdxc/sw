
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
    lif_info_t                 *lif_info = eth_lif->GetLifInfo();

    if (allocator->alloc(&id_) != sdk::lib::indexer::SUCCESS) {
        HAL_TRACE_ERR("Failed to create Lif for hw_lif_id: {}. Index Exhaustion",
                      lif_info->hw_lif_id);
        return;
    }

    eth_lif_ = eth_lif;

    HAL_TRACE_DEBUG("Creating Lif: prom: {}", lif_info->receive_promiscuous);

    req = req_msg.add_request();
    req->mutable_key_or_handle()->set_lif_id(id_);
    req->set_hw_lif_id(lif_info->hw_lif_id);
    req->mutable_pinned_uplink_if_key_handle()->set_interface_id(lif_info->pinned_uplink->GetId());
    req->mutable_packet_filter()->set_receive_broadcast(lif_info->receive_broadcast);
    req->mutable_packet_filter()->set_receive_all_multicast(lif_info->receive_all_multicast);
    req->mutable_packet_filter()->set_receive_promiscuous(lif_info->receive_promiscuous);
    req->set_vlan_strip_en(lif_info->vlan_strip_en);
    req->set_vlan_insert_en(lif_info->vlan_insert_en);
    req->set_is_management(lif_info->is_management);

    req->set_admin_status(::intf::IF_STATUS_UP);

    status = hal->lif_create(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            handle_ = rsp.status().lif_handle();
            HAL_TRACE_DEBUG("Created Lif id: {} hw_lif_id: {}, handle: {}",
                            id_, lif_info->hw_lif_id, handle_);
        } else {
            HAL_TRACE_ERR("Failed to create Lif for hw_lif_id: {}. err: {}",
                          lif_info->hw_lif_id, rsp.api_status());
        }
    } else {
        HAL_TRACE_ERR("Failed to create Lif for hw_lif_id: {}. err: {}:{}",
                      lif_info->hw_lif_id, status.error_code(), status.error_message());
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
    lif_info_t                 *lif_info = eth_lif_->GetLifInfo();

    req = req_msg.add_request();
    req->mutable_key_or_handle()->set_lif_id(id_);
    req->set_hw_lif_id(lif_info->hw_lif_id);
    req->mutable_pinned_uplink_if_key_handle()->set_interface_id(lif_info->pinned_uplink->GetId());
    req->mutable_packet_filter()->set_receive_broadcast(lif_info->receive_broadcast);
    req->mutable_packet_filter()->set_receive_all_multicast(lif_info->receive_all_multicast);
    req->mutable_packet_filter()->set_receive_promiscuous(lif_info->receive_promiscuous);
    req->set_vlan_strip_en(lif_info->vlan_strip_en);
    req->set_vlan_insert_en(lif_info->vlan_insert_en);
    req->set_is_management(lif_info->is_management);

    req->set_admin_status(::intf::IF_STATUS_UP);

    status = hal->lif_update(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            HAL_TRACE_DEBUG("Created Lif id: {} hw_lif_id: {}, handle: {}",
                            id_, lif_info->hw_lif_id, handle_);
        } else {
            HAL_TRACE_ERR("Failed to create Lif for hw_lif_id: {}. err: {}",
                          lif_info->hw_lif_id, rsp.api_status());
        }
    } else {
        HAL_TRACE_ERR("Failed to create Lif for hw_lif_id: {}. err: {}:{}",
                      lif_info->hw_lif_id, status.error_code(), status.error_message());
    }

    // Store spec
    spec.CopyFrom(*req);
}

uint32_t
Lif::GetId()
{
    return id_;
}
