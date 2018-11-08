

#include <iostream>
#include <iomanip>
#include <grpc++/grpc++.h>

#include "multicast.hpp"

using namespace std;

std::map<mcast_key_t, HalMulticast*> HalMulticast::mcast_db;

HalMulticast *
HalMulticast::Factory(HalL2Segment *l2seg, mac_t mac)
{
    mcast_key_t mcast_key(l2seg, mac);

    HalMulticast *mcast = new HalMulticast(l2seg, mac);

    // Store in DB
    mcast_db[mcast_key] = mcast;
    return mcast;
}

void
HalMulticast::Destroy(HalMulticast *mcast)
{
    mcast_key_t mcast_key(mcast->GetL2Seg(), mcast->GetMac());

    if (mcast) {
        mcast->~HalMulticast();
    }

    // Remove from DB
    mcast_db.erase(mcast_key);
}

HalMulticast::HalMulticast(HalL2Segment *l2seg, mac_t mac)
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
    // spec->mutable_meta()->set_vrf_id(l2seg->GetVrf()->GetId());
    spec->mutable_key_or_handle()->mutable_key()->mutable_l2segment_key_handle()->set_l2segment_handle(l2seg->GetHandle());
    spec->mutable_key_or_handle()->mutable_key()->mutable_mac()->set_group(mac);

    status = hal->multicast_create(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            NIC_LOG_DEBUG("Created Mcast L2seg: {}, Mac: {}", l2seg->GetId(), mac);
        } else {
            NIC_LOG_ERR("Failed to create Mcast L2seg: {}, Mac: {} err: {}", l2seg->GetId(), mac,
                          rsp.api_status());
        }
    } else {
        NIC_LOG_ERR("Failed to create Mcast L2seg: {}, Mac: {}. err: {}, msg: {}", l2seg->GetId(), mac,
                      status.error_code(), status.error_message());
    }
}

HalMulticast::~HalMulticast()
{
    grpc::ClientContext context;
    grpc::Status status;

    multicast::MulticastEntryDeleteRequest *req;
    multicast::MulticastEntryDeleteResponse rsp;
    multicast::MulticastEntryDeleteRequestMsg req_msg;
    multicast::MulticastEntryDeleteResponseMsg rsp_msg;

    req = req_msg.add_request();
    // req->mutable_meta()->set_vrf_id(l2seg->GetVrf()->GetId());
    req->mutable_key_or_handle()->mutable_key()->mutable_l2segment_key_handle()->set_l2segment_handle(l2seg->GetHandle());
    req->mutable_key_or_handle()->mutable_key()->mutable_mac()->set_group(mac);

    // status = hal->multicast_stub_->HalMulticastEntryDelete(&context, req_msg, &rsp_msg);
    status = hal->multicast_delete(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            NIC_LOG_ERR("Failed to delete Mcast L2seg: {}, Mac: {}. err: {}", l2seg->GetId(), mac,
                          rsp.api_status());
        } else {
            NIC_LOG_DEBUG("Delete Mcast L2seg: {}, Mac: {}", l2seg->GetId(), mac);
        }
    } else {
        NIC_LOG_ERR("Failed to delete Mcast L2seg: {}, Mac: {}. err: {}, msg: {}", l2seg->GetId(),
                      mac, status.error_code(), status.error_message());
    }
}

HalMulticast *
HalMulticast::GetInstance(HalL2Segment *l2seg, mac_t mac)
{
    std::map<mcast_key_t, HalMulticast*>::iterator it;
    mcast_key_t key(l2seg, mac);
    HalMulticast *mcast;

    it = mcast_db.find(key);
    if (it == mcast_db.end()) {
        // Create the multicast
        mcast = HalMulticast::Factory(l2seg, mac);
        return mcast;
    } else {
        return it->second;
    }
}

void
HalMulticast::TriggerHal()
{
    grpc::ClientContext context;
    grpc::Status status;

    multicast::MulticastEntrySpec *spec;
    multicast::MulticastEntryResponse rsp;
    multicast::MulticastEntryRequestMsg req_msg;
    multicast::MulticastEntryResponseMsg rsp_msg;

    spec = req_msg.add_request();
    // spec->mutable_meta()->set_vrf_id(l2seg->GetVrf()->GetId());
    spec->mutable_key_or_handle()->mutable_key()->mutable_l2segment_key_handle()->set_l2segment_handle(l2seg->GetHandle());
    spec->mutable_key_or_handle()->mutable_key()->mutable_mac()->set_group(mac);
    for (auto it = enic_refs.cbegin(); it != enic_refs.cend(); it++) {
        spec->add_oif_key_handles()->set_interface_id(it->second->GetId());
    }

    status = hal->multicast_update(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            NIC_LOG_DEBUG("Updated Mcast L2seg: {}, Mac: {}", l2seg->GetId(), mac);
        } else {
            NIC_LOG_ERR("Failed to update Mcast L2seg: {}, Mac: {}. err: {}", l2seg->GetId(), mac,
                          rsp.api_status());
        }
    } else {
        NIC_LOG_ERR("Failed to update Mcast L2seg: {}, Mac: {}. err: {}, msg: {}", l2seg->GetId(), mac,
                      status.error_code(), status.error_message());
    }
}

void
HalMulticast::AddEnic(Enic *enic)
{
    enic_refs[enic->GetId()] = enic;
    return TriggerHal();
}

void
HalMulticast::DelEnic(Enic *enic)
{
    enic_refs.erase(enic->GetId());
    return TriggerHal();
}

HalL2Segment *
HalMulticast::GetL2Seg()
{
    return l2seg;
}
mac_t
HalMulticast::GetMac()
{
    return mac;
}

uint32_t
HalMulticast::GetNumEnics()
{
    return enic_refs.size();
}
