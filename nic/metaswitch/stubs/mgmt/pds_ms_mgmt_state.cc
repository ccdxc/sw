//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Mgmt stub state implementation
//---------------------------------------------------------------

#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_state.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_state.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_uuid_obj.hpp"
#include "nic/sdk/lib/logger/logger.hpp"
#include <utility>

namespace pds_ms {

mgmt_state_t* mgmt_state_t::g_state_ = nullptr;
bool mgmt_state_t::g_response_ready_ = false;
std::mutex mgmt_state_t::g_cv_mtx_;
std::mutex mgmt_state_t::g_grpc_lock_;
std::recursive_mutex mgmt_state_t::g_state_mtx_;
std::condition_variable mgmt_state_t::g_cv_resp_;
types::ApiStatus mgmt_state_t::g_ms_response_;

mgmt_state_t::mgmt_state_t(void) {
    bgp_peer_uuid_obj_slab_init(slabs_, PDS_MS_MGMT_BGP_PEER_SLAB_ID);
    bgp_peer_af_uuid_obj_slab_init(slabs_, PDS_MS_MGMT_BGP_PEER_AF_SLAB_ID);
    vpc_uuid_obj_slab_init (slabs_, PDS_MS_MGMT_VPC_SLAB_ID);
    subnet_uuid_obj_slab_init(slabs_, PDS_MS_MGMT_SUBNET_SLAB_ID);
    interface_uuid_obj_slab_init(slabs_, PDS_MS_MGMT_INTERFACE_SLAB_ID);
}

void mgmt_state_t::commit_pending_uuid() {
    for (auto& uuid_pair: uuid_pending_create_) {
        PDS_TRACE_DEBUG("Commit create UUID %s", uuid_pair.first.str());
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
        PDS_TRACE_DEBUG("Commit delete UUID %s", uuid.str());
        uuid_store_.erase(uuid_it);
    }
    uuid_pending_delete_.clear();
}

void mgmt_state_t::ms_response_ready(types::ApiStatus resp) {
    std::unique_lock<std::mutex> lock(g_cv_mtx_);
    g_response_ready_ = true;
    g_ms_response_ = resp;
    vector<pend_rt_t> rt_add_list;
    vector<pend_rt_t> rt_del_list;
    vector<bgp_peer_pend_obj_t> bgp_peer_list;
    if (g_ms_response_ != types::API_STATUS_OK) {
        {
            // Delete any UUIDs stashed in temporary pending store
            // Any internal MIB IDs allocated will be released here
            auto mgmt_ctxt = mgmt_state_t::thread_context();
            mgmt_ctxt.state()->release_pending_uuid();
            rt_add_list = mgmt_ctxt.state()->get_rt_pending_add();
            rt_del_list = mgmt_ctxt.state()->get_rt_pending_delete();
            bgp_peer_list = mgmt_ctxt.state()->get_bgp_peer_pend();
            mgmt_ctxt.state()->clear_rt_pending();
            mgmt_ctxt.state()->clear_bgp_peer_pend();
        }
        if (!rt_add_list.empty()) {
            // delete pending rt from the store it is added
            redo_rt_pending (rt_add_list, false);
        }
        if (!rt_del_list.empty()) {
            // add pending rt back to the store from where it is deleted
            redo_rt_pending (rt_del_list, true);
        }
        if (!bgp_peer_list.empty()) {
            // redo bgp peer store update on failure
            redo_bgp_peer_pend(bgp_peer_list);
        }
    } else {
        // Commit pending UUID operations to permanent store
        auto mgmt_ctxt = mgmt_state_t::thread_context();
        mgmt_ctxt.state()->commit_pending_uuid();
        mgmt_ctxt.state()->clear_rt_pending();
        mgmt_ctxt.state()->clear_bgp_peer_pend();
    }
    g_cv_resp_.notify_all();
}

void mgmt_state_t::set_pending_uuid_create(const pds_obj_key_t& uuid, 
                             uuid_obj_uptr_t&& obj) {
    if (lookup_uuid(uuid) != nullptr) {
        PDS_TRACE_DEBUG("Cannot create existing UUID %s", uuid.str());
        return;
    }
    PDS_TRACE_DEBUG("UUID %s in pending Create list", uuid.str());
    uuid_pending_create_[uuid] = std::move(obj);
}

uuid_obj_t* mgmt_state_t::lookup_uuid(const pds_obj_key_t& uuid) {
    if (is_pds_obj_key_invalid (uuid)) {
        throw Error (std::string("Invalid key ").append(uuid.str()), 
                     SDK_RET_INVALID_ARG);
    }
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

// failed to add/delete RT in metaswitch. these RTs should be added to or
// removed from the respected stores
void mgmt_state_t::redo_rt_pending (vector<pend_rt_t>& rt_list, bool add) {
    auto state_ctxt = state_t::thread_context();
    subnet_obj_t* subnet_obj = nullptr;
    vpc_obj_t* vpc_obj = nullptr;

    for (auto& obj: rt_list) {
        if (obj.type == rt_type_e::EVI) {
            subnet_obj = state_ctxt.state()->subnet_store().get(obj.src_id);
            if (subnet_obj) {
                PDS_TRACE_VERBOSE ("EVI RT %s is %s subnet-id %d "
                                   "due to failure",
                                   rt2str(obj.rt_str),
                                   add ? "added back to" : "removed from",
                                   obj.src_id);
                if (add) {
                    subnet_obj->rt_store.add(obj.rt_str);
                } else {
                    subnet_obj->rt_store.del(obj.rt_str);
                }
            }
        } else if (obj.type == rt_type_e::VRF) {
            vpc_obj = state_ctxt.state()->vpc_store().get(obj.src_id);
            if (vpc_obj) {
                PDS_TRACE_VERBOSE ("VRF RT %s is %s vpc-id %d "
                                   "due to failure",
                                   rt2str(obj.rt_str),
                                   add ? "added back to" : "removed from",
                                   obj.src_id);
                if (add) {
                    vpc_obj->rt_store.add(obj.rt_str);
                } else {
                    vpc_obj->rt_store.del(obj.rt_str);
                }
            }
        }
    }
}

// bgp peer config failure: walk through the pending bgp peer list and redo
void mgmt_state_t::redo_bgp_peer_pend (vector<bgp_peer_pend_obj_t>& list) {
    auto mgmt_ctxt = mgmt_state_t::thread_context();
    auto bgp_peer_store = mgmt_ctxt.state()->bgp_peer_store();
    for (auto& obj: list) {
        if (obj.add_oper) {
            //bgp peer is added to list, delete it now
            bgp_peer_store.del(obj.local_addr, obj.peer_addr);
        } else {
            //bgp peer is deleted from list, add it now
            bgp_peer_store.add(obj.local_addr, obj.peer_addr);
        }
    }
}

mgmt_obj_t* mgmt_state_t::check_vni_match(pds_vnid_id_t vni,
                                          mgmt_obj_type_e obj_type,
                                          ms_idx_t ms_id,
                                          const pds_obj_key_t& uuid) {
     auto it = vni_store_.find(vni);
     if (it == vni_store_.end()) {
         return nullptr;
     }
     if ((obj_type == it->second.obj_type) &&
         (ms_id == it->second.ms_id) && (uuid == it->second.uuid)) {
         return nullptr;
     }
     return &(it->second);
}

void mgmt_state_t::set_vni(pds_vnid_id_t vni, mgmt_obj_type_e obj_type, 
                           ms_idx_t id, const pds_obj_key_t& uuid) {
     auto ret_pair = vni_store_.emplace(std::piecewise_construct,
                                        std::forward_as_tuple(vni),
                                        std::forward_as_tuple(obj_type, id, uuid));
     // Should be a new VNI and so should get emplaced successfully
     SDK_ASSERT(ret_pair.second);
}

void mgmt_state_t::reset_vni(pds_vnid_id_t vni) {
    vni_store_.erase(vni);
}

static std::string mgmt_obj_str(mgmt_obj_type_e obj_type) {
    switch (obj_type) {
    case mgmt_obj_type_e::VPC:
        return "VPC";
    case mgmt_obj_type_e::SUBNET:
        return "Subnet";
    }
    return "";
}

bool mgmt_check_vni(pds_vnid_id_t vni, mgmt_obj_type_e obj_type,
                         ms_idx_t ms_id, const pds_obj_key_t& uuid) {
    if (vni == 0) {
        PDS_TRACE_ERR("Invalid VNI 0");
        return false;
    }
    // Check unique VNI
    auto mgmt_ctxt = mgmt_state_t::thread_context();
    auto obj_info = mgmt_ctxt.state()->
        check_vni_match(vni, obj_type, ms_id, uuid);
    if (obj_info != nullptr) {
        PDS_TRACE_ERR("Wrong VNI - already associated to %s %s MS Idx %d",
                      mgmt_obj_str(obj_info->obj_type).c_str(),
                      obj_info->uuid.str(), obj_info->ms_id);
        return false;
    }
    return true;
}

bool
mgmt_state_init (void)
{
    try { 
        mgmt_state_t::create();

    } catch (pds_ms::Error& e) {
        PDS_TRACE_ERR("Mgmt state Initialization failed - %s", e.what());
        return false;
    }
    if (std::getenv("PDS_MOCK_MODE")) {
        mgmt_state_t::thread_context().state()->set_pds_mock_mode(true);
        PDS_TRACE_INFO("Running in PDS MOCK MODE");
    }

    PDS_TRACE_INFO ("Mgmt State Initialization successful");
    return true;
}

void 
mgmt_state_destroy (void)
{
    mgmt_state_t::destroy();
}

} // End namespace
