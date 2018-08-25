
#include <cmath>
#include <iostream>
#include <iomanip>
#include <grpc++/grpc++.h>

#include "interface.grpc.pb.h"

#include "lif.hpp"
#include "uplink.hpp"

using namespace std;

sdk::lib::indexer *Lif::allocator = sdk::lib::indexer::factory(Lif::max_lifs, false, true);

Lif *
Lif::Factory(uint32_t hw_lif_id, Uplink *pinned_uplink)
{
    Lif *lif = new Lif(hw_lif_id, pinned_uplink);

    return lif;
}

void
Lif::Destroy(Lif *lif)
{
    if (lif) {
        lif->~Lif();
    }
}


Lif::Lif(uint32_t hw_lif_id, Uplink *pinned_uplink)
{
    grpc::ClientContext        context;
    grpc::Status               status;

    intf::LifSpec              *req;
    intf::LifResponse          rsp;
    intf::LifRequestMsg        req_msg;
    intf::LifResponseMsg       rsp_msg;

    if (allocator->alloc(&id) != sdk::lib::indexer::SUCCESS) {
        HAL_TRACE_ERR("Failed to create Lif for hw_lif_id: {}. Index Exhaustion",
                      hw_lif_id);
        return;
    }

    req = req_msg.add_request();
    req->mutable_key_or_handle()->set_lif_id(id);
    req->set_hw_lif_id(hw_lif_id);
    req->mutable_pinned_uplink_if_key_handle()->set_interface_id(pinned_uplink->GetId());
    req->set_admin_status(::intf::IF_STATUS_UP);

    status = hal->lif_create(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            handle = rsp.status().lif_handle();
            HAL_TRACE_DEBUG("Created Lif id: {} hw_lif_id: {}, handle: {}",
                            id, hw_lif_id, handle);
        } else {
            HAL_TRACE_ERR("Failed to create Lif for hw_lif_id: {}. err: {}",
                          hw_lif_id, rsp.api_status());
        }
    } else {
        HAL_TRACE_ERR("Failed to create Lif for hw_lif_id: {}. err: {}:{}",
                      hw_lif_id, status.error_code(), status.error_message());
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
    req->mutable_key_or_handle()->set_lif_id(id);
    status = hal->lif_delete(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            HAL_TRACE_DEBUG("Deleted Lif id: {} hw_lif_id: {}, handle: {}",
                            id, hw_lif_id, handle);
        } else {
            HAL_TRACE_ERR("Failed to delete Lif for id: {}. err: {}",
                          id, rsp.api_status());
        }
    } else {
        HAL_TRACE_ERR("Failed to delete Lif for id: {}. err: {}:{}",
                      id, status.error_code(), status.error_message());
    }
}

uint32_t
Lif::GetId()
{
    return id;
}
