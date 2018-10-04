

#include <iostream>
#include <iomanip>
#include <grpc++/grpc++.h>

#include "multicast.hpp"

using namespace std;

std::map<mcast_key_t, Multicast*> Multicast::mcast_db;

Multicast *
Multicast::Factory(L2Segment *l2seg, mac_t mac)
{
    mcast_key_t mcast_key(l2seg, mac);

    Multicast *mcast = new Multicast(l2seg, mac);

    // Store in DB
    mcast_db[mcast_key] = mcast;
    return mcast;
}

void
Multicast::Destroy(Multicast *mcast)
{
    mcast_key_t mcast_key(mcast->GetL2Seg(), mcast->GetMac());

    if (mcast) {
        mcast->~Multicast();
    }

    // Remove from DB
    mcast_db.erase(mcast_key);
}

Multicast::Multicast(L2Segment *l2seg, mac_t mac)
{
    grpc::ClientContext context;
    grpc::Status status;

    multicast::MulticastEntrySpec *spec;
    multicast::MulticastEntryResponse rsp;
    multicast::MulticastEntryRequestMsg req_msg;
    multicast::MulticastEntryResponseMsg rsp_msg;

    this->mac = mac;
    this->l2seg = l2seg;

    spec = req_msg.add_request();
    spec->mutable_key_or_handle()->mutable_key()->mutable_l2segment_key_handle()->set_l2segment_handle(l2seg->GetHandle());
    spec->mutable_key_or_handle()->mutable_key()->mutable_mac()->set_group(mac);

    status = hal->multicast_create(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            HAL_TRACE_DEBUG("Created Mcast L2seg: {}, Mac: {}", l2seg->GetId(), mac);
        } else {
            HAL_TRACE_ERR("Failed to create Mcast L2seg: {}, Mac: {} err: {}", l2seg->GetId(), mac,
                          rsp.api_status());
        }
    } else {
        HAL_TRACE_ERR("Failed to create Mcast L2seg: {}, Mac: {}. err: {}, msg: {}", l2seg->GetId(), mac,
                      status.error_code(), status.error_message());
    }
}

Multicast::~Multicast()
{
    grpc::ClientContext context;
    grpc::Status status;

    multicast::MulticastEntryDeleteRequest *req;
    multicast::MulticastEntryDeleteResponse rsp;
    multicast::MulticastEntryDeleteRequestMsg req_msg;
    multicast::MulticastEntryDeleteResponseMsg rsp_msg;

    req = req_msg.add_request();
    req->mutable_key_or_handle()->mutable_key()->mutable_l2segment_key_handle()->set_l2segment_handle(l2seg->GetHandle());
    req->mutable_key_or_handle()->mutable_key()->mutable_mac()->set_group(mac);

    // status = hal->multicast_stub_->MulticastEntryDelete(&context, req_msg, &rsp_msg);
    status = hal->multicast_delete(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            HAL_TRACE_ERR("Failed to delete Mcast L2seg: {}, Mac: {}. err: {}", l2seg->GetId(), mac,
                          rsp.api_status());
        } else {
            HAL_TRACE_DEBUG("Delete Mcast L2seg: {}, Mac: {}", l2seg->GetId(), mac);
        }
    } else {
        HAL_TRACE_ERR("Failed to delete Mcast L2seg: {}, Mac: {}. err: {}, msg: {}", l2seg->GetId(),
                      mac, status.error_code(), status.error_message());
    }
}

Multicast *
Multicast::GetInstance(L2Segment *l2seg, mac_t mac)
{
    std::map<mcast_key_t, Multicast*>::iterator it;
    mcast_key_t key(l2seg, mac);
    Multicast *mcast;

    it = mcast_db.find(key);
    if (it == mcast_db.end()) {
        // Create the multicast
        mcast = Multicast::Factory(l2seg, mac);
        return mcast;
    } else {
        return it->second;
    }
}

void
Multicast::TriggerHal()
{
    grpc::ClientContext context;
    grpc::Status status;

    multicast::MulticastEntrySpec *spec;
    multicast::MulticastEntryResponse rsp;
    multicast::MulticastEntryRequestMsg req_msg;
    multicast::MulticastEntryResponseMsg rsp_msg;

    spec = req_msg.add_request();
    spec->mutable_key_or_handle()->mutable_key()->mutable_l2segment_key_handle()->set_l2segment_handle(l2seg->GetHandle());
    spec->mutable_key_or_handle()->mutable_key()->mutable_mac()->set_group(mac);
    for (auto it = enic_refs.cbegin(); it != enic_refs.cend(); it++) {
        spec->add_oif_key_handles()->set_interface_id(it->second->GetId());
    }

    status = hal->multicast_update(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            HAL_TRACE_DEBUG("Updated Mcast L2seg: {}, Mac: {}", l2seg->GetId(), mac);
        } else {
            HAL_TRACE_ERR("Failed to update Mcast L2seg: {}, Mac: {}. err: {}", l2seg->GetId(), mac,
                          rsp.api_status());
        }
    } else {
        HAL_TRACE_ERR("Failed to update Mcast L2seg: {}, Mac: {}. err: {}, msg: {}", l2seg->GetId(), mac,
                      status.error_code(), status.error_message());
    }
}

void
Multicast::AddEnic(Enic *enic)
{
    enic_refs[enic->GetId()] = enic;
    return TriggerHal();
}

void
Multicast::DelEnic(Enic *enic)
{
    enic_refs.erase(enic->GetId());
    return TriggerHal();
}

L2Segment *
Multicast::GetL2Seg()
{
    return l2seg;
}
mac_t
Multicast::GetMac()
{
    return mac;
}

uint32_t
Multicast::GetNumEnics()
{
    return enic_refs.size();
}
