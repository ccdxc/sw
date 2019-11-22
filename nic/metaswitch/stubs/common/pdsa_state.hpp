//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDSA global states
//--------------------------------------------------------------

#ifndef __PDSA_STATE_HPP__
#define __PDSA_STATE_HPP__

#include "nic/metaswitch/stubs/common/pdsa_util.hpp"
#include "nic/metaswitch/stubs/common/pdsa_error.hpp"
#include "nic/metaswitch/stubs/common/pdsa_tep_store.hpp"
#include "nic/metaswitch/stubs/common/pdsa_bd_store.hpp"
#include "nic/metaswitch/stubs/common/pdsa_if_store.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_ifindex.hpp"
#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/port.hpp"
#include "nic/sdk/lib/logger/logger.hpp"
#include <mutex>
#include <memory>

namespace pdsa_stub {

enum pdsa_tep_slab_id_e {
    PDSA_TEP_SLAB_ID = 1,
    PDSA_IF_SLAB_ID,
    PDSA_HOST_LIF_SLAB_ID,
    PDSA_BD_SLAB_ID,
    PDSA_COOKIE_SLAB_ID,
    PDSA_MAX_SLAB_ID
};

// Singleton that holds all global state for the PDSA stubs
class state_t {
public:
    struct context_t {
    public:    
        context_t(std::mutex& m, state_t* s) : l_(m), state_(s)  {};
        state_t* state(void) {return state_;}
        void release(void) {l_.unlock(); state_ = nullptr;}
    private:    
        std::unique_lock<std::mutex> l_;
        state_t* state_;
    };
    static void create(void) { 
        SDK_ASSERT (g_state_ == nullptr);
        g_state_ = new state_t;
    }
    static void destroy(void) {
        delete(g_state_); g_state_ = nullptr;
    }

    // Obtain a mutual exclusion context for thread safe access to the 
    // global stub state. Automatic release when the context goes 
    // out of scope. Direct external access to Stub state without
    // a context is prohibited.
    // Calling this more than once from the same thread will deadlock.
    static context_t thread_context(void) {
        SDK_ASSERT(g_state_ != nullptr);
        return context_t(g_mtx_, g_state_);
    }
    void set_pds_batch(pds_batch_ctxt_t bctxt) {
        bg_.set (bctxt);
    }
    void flush_outstanding_pds_batch() {
        if (bg_.get() != 0) {
            pds_batch_commit(bg_.release());
        }
    }
public:
    tep_store_t& tep_store(void) {return tep_store_;}
    bd_store_t&  bd_store(void) {return bd_store_;}
    if_store_t&  if_store(void) {return if_store_;}
    host_lif_store_t&  host_lif_store(void) {return host_lif_store_;}

    uint32_t get_slab_in_use(pdsa_tep_slab_id_e slab_id) {
        return slabs_[slab_id]->num_in_use();
    } 
    uint32_t lnx_ifindex(uint32_t pds_ifindex) {
        // Check that we are not passed a LIF by mistake
        SDK_ASSERT (IFINDEX_TO_IFTYPE(pds_ifindex) == IF_TYPE_L3);
        uint32_t port = ETH_IFINDEX_TO_PARENT_PORT(pds_ifindex);
        SDK_ASSERT(port <= k_max_fp_ports);
        return lnx_ifindex_table_[port-1];
    }
    void set_lnx_ifindex(uint32_t pds_ifindex, uint32_t lnx_ifindex) {
        // Check that we are not passed a LIF by mistake
        SDK_ASSERT (IFINDEX_TO_IFTYPE(pds_ifindex) == IF_TYPE_L3);
        uint32_t port = ETH_IFINDEX_TO_PARENT_PORT(pds_ifindex);
        SDK_ASSERT(port <= k_max_fp_ports);
        lnx_ifindex_table_[port-1] = lnx_ifindex;
    }

private:
    static constexpr uint32_t k_max_fp_ports = 2;
    // Unique ptr helps to uninitialize cleanly in case of initialization errors
    slab_uptr_t slabs_[PDSA_MAX_SLAB_ID];

    tep_store_t tep_store_; 
    bd_store_t bd_store_; 
    if_store_t if_store_; 
    host_lif_store_t host_lif_store_;

    static state_t* g_state_;
    static std::mutex g_mtx_;
    pds_batch_ctxt_guard_t bg_;
    uint32_t lnx_ifindex_table_[k_max_fp_ports] = {0};

private:
    state_t(void);
};

using tep_obj_uptr_t = std::unique_ptr<tep_obj_t>;
using bd_obj_uptr_t = std::unique_ptr<bd_obj_t>;
using if_obj_uptr_t = std::unique_ptr<if_obj_t>;

}

#endif
