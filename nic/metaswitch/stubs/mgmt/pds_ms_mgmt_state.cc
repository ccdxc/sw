//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Mgmt stub state implementation
//---------------------------------------------------------------

#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_state.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_uuid_obj.hpp"
#include "nic/sdk/lib/logger/logger.hpp"

namespace pds_ms {

mgmt_state_t* mgmt_state_t::g_state_ = nullptr;
bool mgmt_state_t::g_response_ready_ = false;
std::mutex mgmt_state_t::g_cv_mtx_;
std::recursive_mutex mgmt_state_t::g_state_mtx_;
std::condition_variable mgmt_state_t::g_cv_resp_;
types::ApiStatus mgmt_state_t::g_ms_response_;

mgmt_state_t::mgmt_state_t(void) {
    bgp_peer_uuid_obj_slab_init(slabs_, PDS_MS_MGMT_BGP_PEER_SLAB_ID);
    bgp_peer_af_uuid_obj_slab_init(slabs_, PDS_MS_MGMT_BGP_PEER_AF_SLAB_ID);
    vpc_uuid_obj_slab_init (slabs_, PDS_MS_MGMT_VPC_SLAB_ID);
    subnet_uuid_obj_slab_init(slabs_, PDS_MS_MGMT_SUBNET_SLAB_ID);
}

void mgmt_state_t::commit_pending_uuid() {
    for (auto& uuid_pair: uuid_pending_create_) {
        SDK_TRACE_VERBOSE("Commit create UUID %s", uuid_pair.first.str());
        uuid_store_[uuid_pair.first] = std::move(uuid_pair.second);
    }
    uuid_pending_create_.clear();

    for (auto& uuid: uuid_pending_delete_) {
        auto uuid_it = uuid_store_.find(uuid);
        if (uuid_it == uuid_store_.end()) return;
        // Delay release until MS HAL stub delete is invoked
        // NOTE - Assumption that the deleted UUID will not be
        //        used again for Create immediately
        if (uuid_it->second->delay_release()) continue;
        SDK_TRACE_VERBOSE("Commit delete UUID %s", uuid.str());
        uuid_store_.erase(uuid_it);
    }
    uuid_pending_delete_.clear();
}

void mgmt_state_t::ms_response_ready(types::ApiStatus resp) {
    std::unique_lock<std::mutex> lock(g_cv_mtx_);
    g_response_ready_ = true;
    g_ms_response_ = resp;
    if (g_ms_response_ != types::API_STATUS_OK) {
        // Delete any UUIDs stashed in temporary pending store
        // Any internal MIB IDs allocated will be released here
        auto mgmt_ctxt = mgmt_state_t::thread_context();
        mgmt_ctxt.state()->release_pending_uuid();
    } else {
        // Commit pending UUID operations to permanent store
        auto mgmt_ctxt = mgmt_state_t::thread_context();
        mgmt_ctxt.state()->commit_pending_uuid();
    }
    g_cv_resp_.notify_all();
}

void mgmt_state_t::set_pending_uuid_create(const pds_obj_key_t& uuid, 
                             uuid_obj_uptr_t&& obj) {
    if (lookup_uuid(uuid) != nullptr) {
        SDK_TRACE_VERBOSE("Cannot create existing UUID %s", uuid.str());
        return;
    }
    SDK_TRACE_VERBOSE("UUID %s in pending Create list", uuid.str());
    uuid_pending_create_[uuid] = std::move(obj);
}

uuid_obj_t* mgmt_state_t::lookup_uuid(const pds_obj_key_t& uuid) {
    auto obj = uuid_store_.find(uuid);
    if (obj != uuid_store_.end()) {return obj->second.get();}
    // Some UUID objects are held in pending cache until
    // MS HAL stub completes asynchronously
    auto obj_pend = uuid_pending_create_.find(uuid);
    if (obj_pend != uuid_store_.end()) {
        return obj_pend->second.get();
    }
    return nullptr;
}

bool 
mgmt_state_init (void)
{
    try { 
        mgmt_state_t::create();

    } catch (pds_ms::Error& e) {
        SDK_TRACE_ERR("Mgmt state Initialization failed - %s", e.what());
        return false;
    }
    if (std::getenv("PDS_MOCK_MODE")) {
        mgmt_state_t::thread_context().state()->set_pds_mock_mode(true);
    }

    SDK_TRACE_INFO ("Mgmt State Initialization successful");
    return true;
}

void 
mgmt_state_destroy (void)
{
    mgmt_state_t::destroy();
}

} // End namespace
