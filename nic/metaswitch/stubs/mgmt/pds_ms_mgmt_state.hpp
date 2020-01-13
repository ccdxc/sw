//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Mgmt stub global states
//--------------------------------------------------------------

#ifndef __PDS_MS_MGMT_STATE_HPP__
#define __PDS_MS_MGMT_STATE_HPP__

#include "nic/metaswitch/stubs/mgmt/pds_ms_mib_idx_gen.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_uuid_obj.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_util.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_error.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_object_store.hpp"
#include "nic/sdk/lib/logger/logger.hpp"
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/thread/thread.hpp"
#include "gen/proto/types.pb.h"
#include <mutex>
#include <condition_variable>

#define PDS_MOCK_MODE() \
            (mgmt_state_t::thread_context().state()->pds_mock_mode())

namespace pds_ms {

enum mgmt_slab_id_e {
    PDS_MS_MGMT_BGP_PEER_SLAB_ID = 1,
    PDS_MS_MGMT_BGP_PEER_AF_SLAB_ID,
    PDS_MS_MGMT_VPC_SLAB_ID,
    PDS_MS_MGMT_SUBNET_SLAB_ID,
    PDS_MS_MGMT_MAX_SLAB_ID
};

// Singleton that holds all global state for the mgmt stub
// Placeholder class currently as we do not have any global
// mgmt stub state yet
class mgmt_state_t {
public:
    struct context_t {
        public:    
            context_t(std::recursive_mutex& m, mgmt_state_t* s) : l_(m), state_(s)  {};
            mgmt_state_t* state(void) {return state_;}
            void release(void) {l_.unlock(); state_ = nullptr;}
        private:    
            std::unique_lock<std::recursive_mutex> l_;
            mgmt_state_t* state_;
    };
    static void create(void) { 
        SDK_ASSERT (g_state_ == nullptr);
        g_state_ = new mgmt_state_t;
        g_response_ready_ = false;
    }
    static void destroy(void) {
        delete(g_state_); g_state_ = nullptr;
    }

    // Obtain a mutual exclusion context for thread safe access to the 
    // stub state. Automatic release when the context goes 
    // out of scope. Direct external access to Stub state without
    // a context is prohibited.
    // Calling this more than once from the same thread will deadlock.
    static context_t thread_context(void) {
        SDK_ASSERT(g_state_ != nullptr);
        return context_t(g_state_mtx_, g_state_);
    }
    
    // Blocking function call. To be used only from the grpc thread
    // context while waiting for the async response back from nbase
    static types::ApiStatus ms_response_wait(void) {
        std::unique_lock<std::mutex> lock(g_cv_mtx_);
        g_cv_resp_.wait(lock, response_ready);
        g_response_ready_ = false;
        return g_ms_response_;
    }
    
    // Called from the response method in the nbase thread context to
    // unblock the grpc thread
    static void ms_response_ready(types::ApiStatus resp);

    void set_nbase_thread(sdk::lib::thread *tptr) {
        nbase_thread_ = tptr;
    }

    sdk::lib::thread *nbase_thread(void) {
        return nbase_thread_;
    }
    
    bool pds_mock_mode(void) const { return pds_mock_mode_;  }
    
    void set_pds_mock_mode(bool val) { pds_mock_mode_ = val; }

    void set_pending_uuid_create(const uuid_t& uuid, 
                                 uuid_obj_uptr_t&& obj) {
        if (lookup_uuid(uuid) != nullptr) {
            SDK_TRACE_VERBOSE("Cannot create existing UUID %s",
                              uuid.str());
            return;
        }
        uuid_pending_create_[uuid] = std::move(obj);
    }
    void release_pending_uuid() {
        uuid_pending_create_.clear();
        uuid_pending_delete_.clear();
    }
    void set_pending_uuid_delete(const uuid_t& uuid) {
        uuid_pending_delete_.push_back(uuid);
    }
    // Commit all pending UUID operations to permanent store
    void commit_pending_uuid();
    void remove_uuid(const uuid_t& uuid) {
        uuid_store_.erase(uuid);
    }

    uuid_obj_t* lookup_uuid(const uuid_t& uuid) {
        auto obj = uuid_store_.find(uuid);
        if (obj != uuid_store_.end()) {return obj->second.get();}
        // Some UUID objects are held in pending cache until
        // MS HAL stub completes asynchronously
        auto obj_pend = uuid_pending_create_.find(uuid);
        if (obj_pend != uuid_store_.end()) {return obj_pend->second.get();}
        return nullptr;
    }
    mib_idx_gen_indexer_t&  mib_indexer() {return mib_indexer_;}

private:
    static mgmt_state_t* g_state_;
    // Predicate to avoid spurious wake-up calls
    static bool g_response_ready_;
    static std::mutex g_cv_mtx_;
    static std::recursive_mutex g_state_mtx_;
    static std::condition_variable g_cv_resp_;
    static types::ApiStatus g_ms_response_;
    bool pds_mock_mode_ = false;
    std::unordered_map<uuid_t, uuid_obj_uptr_t, uuid_hash> uuid_store_;
    std::unordered_map<uuid_t, uuid_obj_uptr_t, uuid_hash> uuid_pending_create_;
    std::vector<uuid_t> uuid_pending_delete_;
    mib_idx_gen_indexer_t mib_indexer_;
    slab_uptr_t slabs_[PDS_MS_MGMT_MAX_SLAB_ID];

private:
    mgmt_state_t(void);
    static bool response_ready() {
        return g_response_ready_;
    }
    // Store the nbase thread ptr. Used to set thread ready upon nbase
    // init completion
    sdk::lib::thread *nbase_thread_;
};

// Function prototypes
bool  mgmt_state_init(void);
void  mgmt_state_destroy(void);

}

#endif    // __PDS_MS_MGMT_STATE_HPP__
