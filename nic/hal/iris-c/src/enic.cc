
#include <iostream>
#include <grpc++/grpc++.h>

// #include "kh.grpc.pb.h"

#include "enic.hpp"

using namespace std;


std::map<enic_classic_key_t, std::weak_ptr<Enic>> Enic::classic_registry;
std::map<enic_hostpin_key_t, std::weak_ptr<Enic>> Enic::hostpin_registry;
sdk::lib::indexer *Enic::allocator = sdk::lib::indexer::factory(Enic::max_enics, false, true);


void
Enic::Probe()
{
    grpc::ClientContext context;
    grpc::Status status;

    intf::InterfaceGetRequest         *req __attribute__((unused));
    intf::InterfaceGetResponse        rsp;
    intf::InterfaceGetRequestMsg      req_msg;
    intf::InterfaceGetResponseMsg     rsp_msg;

    uint64_t id, handle, lif_id;

    req = req_msg.add_request();
    status = hal->intf_stub_->InterfaceGet(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        for (int i = 0; i < rsp_msg.response().size(); i++) {
            rsp = rsp_msg.response(i);
            if (rsp.api_status() != types::API_STATUS_OK) {
                cerr << "[ERROR] " << __FUNCTION__ << ": Status = " << rsp.api_status() << endl;
                throw ("Failed to discover ENICs");
            } else {
                id = rsp.spec().key_or_handle().interface_id();
                if (rsp.spec().type() == intf::IF_TYPE_ENIC) {
                    handle = rsp.status().if_handle();
                    lif_id = rsp.spec().if_enic_info().lif_key_or_handle().lif_id();
                    cout << "[INFO] Discovered Enic "
                         << " id = " << id << " handle = " << handle
                         << " lif = " << lif_id << endl;
                }
                allocator->alloc_withid(id);
            }
        }
        return;
    } else {
        cerr << "[ERROR] " << __FUNCTION__ << ": Status = " << status.error_code() << ":" << status.error_message() << endl;
        throw ("Failed to discover ENICs");
    }
}

/**
 * Classic ENIC
 */

// Classic ENIC constructor
Enic::Enic(std::shared_ptr<Lif> lif, std::shared_ptr<Vrf> vrf)
{
    grpc::ClientContext             context;
    grpc::Status                    status;

    intf::InterfaceSpec             *spec;
    intf::InterfaceResponse         rsp;
    intf::InterfaceRequestMsg       req_msg;
    intf::InterfaceResponseMsg      rsp_msg;

    if (hal->GetMode() != FWD_MODE_CLASSIC) {
        throw ("Cannot create classic ENIC in hostpin mode!");
    }

    if (allocator->alloc(&id) != sdk::lib::indexer::SUCCESS) {
        throw ("Failed to allocate ENIC");
    }

    lif_ref = lif;
    vrf_ref = vrf;

    spec = req_msg.add_request();
    spec->mutable_key_or_handle()->set_interface_id(id);
    spec->set_type(::intf::IfType::IF_TYPE_ENIC);
    spec->set_admin_status(::intf::IfStatus::IF_STATUS_UP);
    spec->mutable_if_enic_info()->set_enic_type(::intf::IF_ENIC_TYPE_CLASSIC);
    spec->mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(lif->GetId());
    // spec->mutable_if_enic_info()->mutable_classic_enic_info()->set_native_l2segment_handle(native_l2seg_handle);
    // spec->mutable_if_enic_info()->mutable_classic_enic_info()->add_l2segment_key_handle()->set_l2segment_handle(l2seg->GetHandle());

    status = hal->intf_stub_->InterfaceCreate(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            cout << "[ERROR] " << __FUNCTION__ << ": Status = " << rsp.api_status() << endl;
        } else {
            handle = rsp.status().if_handle();
            cout << "[INFO] ENIC create succeeded,"
                 << " id = " << id << " handle = " << handle
                 << endl;
            return;
        }
    } else {
        cout << "[ERROR] " << __FUNCTION__ << ": Status = " << status.error_code() << ":" << status.error_message() << endl;
    }

    throw ("Failed to create ENIC");
}

// Classic NIC instance getter
shared_ptr<Enic>
Enic::GetInstance(std::shared_ptr<Lif> lif, std::shared_ptr<Vrf> vrf)
{
    if (hal->GetMode() != FWD_MODE_CLASSIC) {
        throw ("Cannot get ENIC instance!");
    }

    enic_classic_key_t key(lif->GetId());
    shared_ptr<Enic> enic;

    if (classic_registry.find(key) == classic_registry.cend()) {
        // create ENIC and endpoint
        enic = make_shared<Enic>(lif, vrf);
        classic_registry[key] = weak_ptr<Enic>(enic);
    } else {
        enic = shared_ptr<Enic>(classic_registry[key]);
    }

    return enic;
}

int
Enic::Update()
{
    grpc::ClientContext             context;
    grpc::Status                    status;

    intf::InterfaceSpec             *spec;
    intf::InterfaceResponse         rsp;
    intf::InterfaceRequestMsg       req_msg;
    intf::InterfaceResponseMsg      rsp_msg;

    shared_ptr<L2Segment> l2seg;

    if (hal->GetMode() != FWD_MODE_CLASSIC) {
        throw ("Cannot update ENIC!");
    }

    spec = req_msg.add_request();
    spec->mutable_key_or_handle()->set_interface_id(id);
    spec->set_type(::intf::IfType::IF_TYPE_ENIC);
    spec->set_admin_status(::intf::IfStatus::IF_STATUS_UP);
    spec->mutable_if_enic_info()->set_enic_type(::intf::IF_ENIC_TYPE_CLASSIC);
    spec->mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(lif_ref->GetId());
    // spec->mutable_if_enic_info()->mutable_classic_enic_info()->set_native_l2segment_handle(native_l2seg_handle);
    for (auto l2seg_it = l2seg_refs.cbegin(); l2seg_it != l2seg_refs.cend(); l2seg_it++) {
        l2seg = l2seg_it->second;
        spec->mutable_if_enic_info()->mutable_classic_enic_info()->add_l2segment_key_handle()->set_l2segment_handle(l2seg->GetHandle());
    }

    status = hal->intf_stub_->InterfaceUpdate(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            cerr << "[ERROR] " << __FUNCTION__ << ": Status = " << rsp.api_status() << endl;
            return -1;
        } else {
            handle = rsp.status().if_handle();
            cout << "[INFO] ENIC update succeeded,"
                 << " id = " << id << " handle = " << handle
                 << endl;
        }
    } else {
        cerr << "[ERROR] " << __FUNCTION__ << ": Status = " << status.error_code() << ":" << status.error_message() << endl;
        return -1;
    }

    return 0;
}

int
Enic::AddVlan(vlan_t vlan)
{
    int ret = 0;
    shared_ptr<L2Segment> l2seg = L2Segment::GetInstance(vrf_ref, vlan);

    if (hal->GetMode() != FWD_MODE_CLASSIC) {
        throw ("Cannot add VLAN to ENIC!");
    }

    if (l2seg_refs.find(vlan) == l2seg_refs.cend()) {

        cout << "[INFO] Adding VLAN " << vlan << " to ENIC"
             << " id = " << id << " handle = " << handle
             << endl;

        l2seg_refs[vlan] = l2seg;
        ret = Update();

    } else {
        cout << "[INFO] VLAN " << vlan << " is already added to ENIC "
             << " id = " << id << " handle = " << handle
             << endl;
    }

    return ret;
}

int
Enic::DelVlan(vlan_t vlan)
{
    int ret = 0;
    shared_ptr<L2Segment> l2seg = L2Segment::GetInstance(vrf_ref, vlan);

    if (hal->GetMode() != FWD_MODE_CLASSIC) {
        throw ("Cannot delete VLAN from ENIC!");
    }

    if (l2seg_refs.find(vlan) != l2seg_refs.cend()) {

        cout << "[INFO] Deleting VLAN " << vlan << " from ENIC"
             << " id = " << id << " handle = " << handle
             << endl;

        l2seg_refs.erase(vlan);
        ret = Update();

    } else {
        cout << "[INFO] VLAN " << vlan << " is already removed from ENIC "
             << " id = " << id << " handle = " << handle
             << endl;
    }

    return ret;
}

/**
 * Hostpin ENIC
 */

// Hostpin ENIC constructor
Enic::Enic(std::shared_ptr<Lif> lif, std::shared_ptr<Vrf> vrf, mac_t mac, vlan_t vlan)
{
    grpc::ClientContext             context;
    grpc::Status                    status;

    intf::InterfaceSpec             *spec;
    intf::InterfaceResponse         rsp;
    intf::InterfaceRequestMsg       req_msg;
    intf::InterfaceResponseMsg      rsp_msg;

    if (hal->GetMode() != FWD_MODE_HOSTPIN) {
        throw ("Cannot create ENIC!");
    }

    if (allocator->alloc(&id) != sdk::lib::indexer::SUCCESS) {
        throw ("Failed to allocate VRF");
    }

    _mac = mac;
    _vlan = vlan;
    lif_ref = lif;
    vrf_ref = vrf;

    shared_ptr<L2Segment> l2seg = L2Segment::GetInstance(vrf, vlan);
    l2seg_refs[vlan] = l2seg;

    spec = req_msg.add_request();
    spec->mutable_key_or_handle()->set_interface_id(id);
    spec->set_type(::intf::IfType::IF_TYPE_ENIC);
    spec->set_admin_status(::intf::IfStatus::IF_STATUS_UP);
    spec->mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(lif->GetId());
    spec->mutable_if_enic_info()->set_enic_type(::intf::IF_ENIC_TYPE_USEG);
    spec->mutable_if_enic_info()->mutable_enic_info()->set_mac_address(mac);
    spec->mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(vlan);
    spec->mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_l2segment_handle(l2seg->GetHandle());

    status = hal->intf_stub_->InterfaceCreate(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            cout << "[ERROR] " << __FUNCTION__ << ": Status = " << rsp.api_status() << endl;
        } else {
            handle = rsp.status().if_handle();
            cout << "[INFO] ENIC create succeeded,"
                 << " id = " << id << " handle = " << handle
                 << endl;
            return;
        }
    } else {
        cout << "[ERROR] " << __FUNCTION__ << ": Status = " << status.error_code() << ":" << status.error_message() << endl;
    }

    throw ("Failed to create ENIC");
}

// Hostpin ENIC instance getter
shared_ptr<Enic>
Enic::GetInstance(std::shared_ptr<Lif> lif, std::shared_ptr<Vrf> vrf, mac_t mac, vlan_t vlan)
{
    if (hal->GetMode() != FWD_MODE_HOSTPIN) {
        throw ("Cannot get ENIC instance!");
    }

    enic_hostpin_key_t key(lif->GetId(), mac, vlan);
    shared_ptr<Enic> enic;

    if (hostpin_registry.find(key) == hostpin_registry.cend()) {
        // create ENIC and Endpoint
        enic = make_shared<Enic>(lif, vrf, mac, vlan);
        hostpin_registry[key] = weak_ptr<Enic>(enic);
    } else {
        // if ENIC exists then retrieve it
        enic = shared_ptr<Enic>(hostpin_registry[key]);
    }

    return enic;
}

/**
 * Common
 */

Enic::~Enic()
{
    grpc::ClientContext                   context;
    grpc::Status                          status;

    intf::InterfaceDeleteRequest          *req;
    intf::InterfaceDeleteResponse         rsp;
    intf::InterfaceDeleteRequestMsg       req_msg;
    intf::InterfaceDeleteResponseMsg      rsp_msg;

    assert(l2seg_refs.empty());
    l2seg_refs.clear();

    req = req_msg.add_request();
    req->mutable_key_or_handle()->set_interface_id(id);
    status = hal->intf_stub_->InterfaceDelete(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            cerr << "[ERROR] " << __FUNCTION__
                 << ": Id = " << id
                 << ", Status = " << rsp.api_status()
                 << endl;
        } else {
            cout << "[INFO] ENIC delete succeeded,"
                 << " id = " << id << " handle = " << handle
                 << endl;
        }
    } else {
        cout << "[ERROR] " << __FUNCTION__
             << ": Id = " << id
             << ", Status = " << status.error_code() << ":" << status.error_message()
             << endl;
    }
}

uint64_t
Enic::GetId()
{
    return id;
}

uint64_t
Enic::GetHandle()
{
    return handle;
}
