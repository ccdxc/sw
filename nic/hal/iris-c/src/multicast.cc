

#include <iostream>
#include <iomanip>
#include <grpc++/grpc++.h>

#include "multicast.hpp"

using namespace std;


map<mcast_key_t, weak_ptr<Multicast>> Multicast::registry;


Multicast::Multicast(shared_ptr<Vrf> vrf,
                     mac_t mac, vlan_t vlan)
{
    grpc::ClientContext context;
    grpc::Status status;

    multicast::MulticastEntrySpec *spec;
    multicast::MulticastEntryResponse rsp;
    multicast::MulticastEntryRequestMsg req_msg;
    multicast::MulticastEntryResponseMsg rsp_msg;

    _mac = mac;
    _vlan = vlan;

    if (hal->GetMode() == FWD_MODE_HOSTPIN) {
        return;
    }

    shared_ptr<L2Segment> l2seg = L2Segment::GetInstance(vrf, vlan);

    spec = req_msg.add_request();
    spec->mutable_meta()->set_vrf_id(vrf->GetId());
    spec->mutable_key_or_handle()->mutable_key()->mutable_l2segment_key_handle()->set_l2segment_handle(l2seg->GetHandle());
    spec->mutable_key_or_handle()->mutable_key()->mutable_mac()->set_group(mac);

    status = hal->multicast_stub_->MulticastEntryCreate(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            cerr << "[ERROR] " << __FUNCTION__
                 << ": mac " << hex << mac << resetiosflags(ios::hex)
                 << " vrf id " << vrf->GetId() << " handle " << vrf->GetHandle()
                 << " l2seg id " << l2seg->GetId() << " handle " << l2seg->GetHandle()
                 << ", API Status " << rsp.api_status()
                 << endl;
            throw ("Failed to create multicast entry");

        } else {
            cout << "[INFO] " << __FUNCTION__
                 << ": mac " << hex << mac << resetiosflags(ios::hex)
                 << " vrf id " << vrf->GetId() << " handle " << vrf->GetHandle()
                 << " l2seg id " << l2seg->GetId() << " handle " << l2seg->GetHandle()
                 << " succeeded!"
                 << endl;
        }
    } else {
        cerr << "[ERROR] " << __FUNCTION__
             << ": mac " << hex << mac << resetiosflags(ios::hex)
             << " vrf id " << vrf->GetId() << " handle " << vrf->GetHandle()
             << " l2seg id " << l2seg->GetId() << " handle " << l2seg->GetHandle()
             << ", Status " << status.error_code() << ":" << status.error_message()
             << endl;
        throw ("Failed to create multicast entry");
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

    if (hal->GetMode() == FWD_MODE_HOSTPIN) {
        return;
    }

    shared_ptr<L2Segment> l2seg = L2Segment::GetInstance(vrf_ref, _vlan);

    req = req_msg.add_request();
    req->mutable_meta()->set_vrf_id(vrf_ref->GetId());
    req->mutable_key_or_handle()->mutable_key()->mutable_l2segment_key_handle()->set_l2segment_handle(l2seg_ref->GetHandle());
    req->mutable_key_or_handle()->mutable_key()->mutable_mac()->set_group(_mac);

    status = hal->multicast_stub_->MulticastEntryDelete(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            cout << "[ERROR] " << __FUNCTION__
                 << ": mac " << hex << _mac << resetiosflags(ios::hex)
                 << " vrf id " << vrf_ref->GetId() << " handle " << vrf_ref->GetHandle()
                 << " l2seg id " << l2seg_ref->GetId() << " handle " << l2seg_ref->GetHandle()
                 << ", API Status " << rsp.api_status()
                 << endl;
        } else {
            cout << "[INFO] " << __FUNCTION__
                 << ": mac " << hex << _mac << resetiosflags(ios::hex)
                 << " vrf id " << vrf_ref->GetId() << " handle " << vrf_ref->GetHandle()
                 << " l2seg id " << l2seg_ref->GetId() << " handle " << l2seg_ref->GetHandle()
                 << " succeeded!"
                 << endl;
        }
    } else {
        cout << "[ERROR] " << __FUNCTION__
             << ": mac " << hex << _mac << resetiosflags(ios::hex)
             << " vrf id " << vrf_ref->GetId() << " handle " << vrf_ref->GetHandle()
             << " l2seg id " << l2seg_ref->GetId() << " handle " << l2seg_ref->GetHandle()
             << ", Status " << status.error_code() << ":" << status.error_message()
             << endl;
    }
}

shared_ptr<Multicast>
Multicast::GetInstance(shared_ptr<Vrf> vrf,
                       mac_t mac, vlan_t vlan)
{
    shared_ptr<L2Segment> l2seg = L2Segment::GetInstance(vrf, vlan);
    mcast_key_t key(vrf->GetId(), l2seg->GetId(), mac);
    shared_ptr<Multicast> mcast;

    if (registry.find(key) == registry.end()) {
        // Create the mac
        mcast = make_shared<Multicast>(vrf, mac, vlan);
        registry[key] = weak_ptr<Multicast>(mcast);
    } else {
        // Return existing entry
        mcast = shared_ptr<Multicast>(registry[key]);
        cout << "[INFO] " << __FUNCTION__
             << ": mac " << hex << mac << resetiosflags(ios::hex)
             << " vrf id " << vrf->GetId() << " handle " << vrf->GetHandle()
             << " l2seg id " << l2seg->GetId() << " handle " << l2seg->GetHandle()
             << " already exists!"
             << endl;
    }

    return mcast;
}

int
Multicast::Update()
{
    grpc::ClientContext context;
    grpc::Status status;

    multicast::MulticastEntrySpec *spec;
    multicast::MulticastEntryResponse rsp;
    multicast::MulticastEntryRequestMsg req_msg;
    multicast::MulticastEntryResponseMsg rsp_msg;

    if (hal->GetMode() == FWD_MODE_HOSTPIN) {
        return 0;
    }

    shared_ptr<L2Segment> l2seg = L2Segment::GetInstance(vrf_ref, _vlan);

    spec = req_msg.add_request();
    spec->mutable_meta()->set_vrf_id(vrf_ref->GetId());
    spec->mutable_key_or_handle()->mutable_key()->mutable_l2segment_key_handle()->set_l2segment_handle(l2seg->GetHandle());
    spec->mutable_key_or_handle()->mutable_key()->mutable_mac()->set_group(_mac);
    for (auto it = enic_refs.cbegin(); it != enic_refs.cend(); it++) {
        spec->add_oif_key_handles()->set_interface_id(it->second->GetId());
    }

    status = hal->multicast_stub_->MulticastEntryUpdate(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            cerr << "[ERROR] " << __FUNCTION__
                 << ": mac " << hex << _mac << resetiosflags(ios::hex)
                 << " vrf id " << vrf_ref->GetId() << " handle " << vrf_ref->GetHandle()
                 << " l2seg id " << l2seg_ref->GetId() << " handle " << l2seg_ref->GetHandle()
                 << " oifs_list ";
            for (auto it = enic_refs.cbegin(); it != enic_refs.cend(); it++)
                cout << it->second->GetId() << ' ';
            cerr << ", API Status " << rsp.api_status() << endl;
        } else {
            cerr << "[INFO] " << __FUNCTION__
                 << " mac " << hex << _mac << resetiosflags(ios::hex)
                 << " vrf id " << vrf_ref->GetId() << " handle " << vrf_ref->GetHandle()
                 << " l2seg id " << l2seg_ref->GetId() << " handle " << l2seg_ref->GetHandle()
                 << " oifs_list ";
            for (auto it = enic_refs.cbegin(); it != enic_refs.cend(); it++)
                cout << it->second->GetId() << ' ';
            cerr << " succeeded," << endl;
            return 0;
        }
    } else {
        cerr << "[ERROR] " << __FUNCTION__
             << ": group " << hex << _mac << resetiosflags(ios::hex)
             << " vrf id " << vrf_ref->GetId() << " handle " << vrf_ref->GetHandle()
             << " l2seg id " << l2seg_ref->GetId() << " handle " << l2seg_ref->GetHandle()
             << " oifs_list ";
        for (auto it = enic_refs.cbegin(); it != enic_refs.cend(); it++)
            cerr << it->second->GetId() << ' ';
        cerr << ", Status " << status.error_code() << ":" << status.error_message() << endl;
    }

    return -1;
}

int
Multicast::AddEnic(shared_ptr<Enic> enic)
{
    enic_refs[enic->GetId()] = enic;
    return Update();
}

int
Multicast::DelEnic(shared_ptr<Enic> enic)
{
    enic_refs.erase(enic->GetId());
    return Update();
}
