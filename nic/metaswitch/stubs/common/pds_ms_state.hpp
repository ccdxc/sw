//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDS-MS global states
//--------------------------------------------------------------

#ifndef __PDS_MS_STATE_HPP__
#define __PDS_MS_STATE_HPP__

#include "nic/metaswitch/stubs/common/pds_ms_util.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_error.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_indirect_ps_store.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_tep_store.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_subnet_store.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_bd_store.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_if_store.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_vpc_store.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_route_store.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_pathset_store.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_ecmp_idx_guard.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_ifindex.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_ip_track_store.hpp"
#include "nic/sdk/lib/rte_indexer/rte_indexer.hpp"
#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/port.hpp"
#include "nic/sdk/lib/logger/logger.hpp"
#include "nic/apollo/learn/learn_api.hpp"
#include <mutex>
#include <memory>
#include <unordered_set>

#define PDS_BATCH_PARAMS_EPOCH  1
#define PDS_BATCH_PARAMS_ASYNC  true

namespace pds_ms {

enum slab_id_e {
    PDS_MS_TEP_SLAB_ID = 1,
    PDS_MS_IF_SLAB_ID,
    PDS_MS_SUBNET_SLAB_ID,
    PDS_MS_BD_SLAB_ID,
    PDS_MS_VPC_SLAB_ID,
    PDS_MS_MAC_SLAB_ID,
    PDS_MS_RTTABLE_SLAB_ID,
    PDS_MS_PATHSET_SLAB_ID,
    PDS_MS_ECMP_IDX_GUARD_SLAB_ID,
    PDS_MS_COOKIE_SLAB_ID,
    PDS_MS_INDIRECT_PS_SLAB_ID,
    PDS_MS_IP_TRACK_SLAB_ID,
    PDS_MS_MAX_SLAB_ID
};

// Singleton that holds all global state for the PDS-MS stubs
class state_t {
public:
    struct context_t {
    public:
        context_t(std::recursive_mutex& m, state_t* s) : l_(m), state_(s)  {
            if (unlikely(mgmt_state_locked(false))) {
                PDS_TRACE_ERR("Acquiring Stub State lock within Mgmt state lock is not allowed");
                SDK_ASSERT(0);
            }
        };
        state_t* state(void) {return state_;}
        void release(void) {l_.unlock(); state_ = nullptr;}
        context_t(context_t&& c) {
            state_ = c.state_;
            c.state_ = nullptr;
            l_ = std::move(c.l_);
        }
        context_t& operator=(context_t&& c) {
            state_ = c.state_;
            c.state_ = nullptr;
            l_ = std::move(c.l_);
            return *this;
        }

    private:
        std::unique_lock<std::recursive_mutex> l_;
        state_t* state_ = nullptr;
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
            learn::api_batch_commit(bg_.release());
        }
    }
public:
    tep_store_t& tep_store(void) {return tep_store_;}
    bd_store_t&  bd_store(void) {return bd_store_;}
    if_store_t&  if_store(void) {return if_store_;}
    vpc_store_t& vpc_store(void) {return vpc_store_;}
    subnet_store_t& subnet_store(void) {return subnet_store_;}
    route_table_store_t& route_table_store(void) {return route_table_store_;}
    pathset_store_t& pathset_store(void) {return pathset_store_;}
    indirect_ps_store_t& indirect_ps_store(void) {return indirect_ps_store_;}
    ip_track_store_t& ip_track_store(void) {return ip_track_store_;}
    ip_track_internalip_store_t& ip_track_internalip_store(void)
    {return ip_track_internalip_store_;}

    uint32_t get_slab_in_use(slab_id_e slab_id) {
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

    sdk_ret_t ecmp_idx_alloc(uint32_t* index) {
        return ecmp_idx_gen_->alloc(index);
    }
    sdk_ret_t ecmp_idx_free(uint32_t index) {
        return ecmp_idx_gen_->free(index);
    }

    void add_ignored_prefix(const ip_prefix_t& prefix) {
        ignored_prefixes_.insert(prefix);
    }
    bool reset_ignored_prefix(const ip_prefix_t& prefix) {
        auto it = ignored_prefixes_.find(prefix);
        if (it == ignored_prefixes_.end()) {
            return false;
        }
        ignored_prefixes_.erase(it);
        return true;
    }

    sdk_ret_t ip_track_internal_idx_alloc(uint32_t* index) {
        return ip_track_internal_idx_gen_->alloc(index);
    }
    sdk_ret_t ip_track_internal_idx_free(uint32_t index) {
        return ip_track_internal_idx_gen_->free(index);
    }

private:
    static constexpr uint32_t k_max_fp_ports = 2;
    // Unique ptr helps to uninitialize cleanly in case of initialization errors
    slab_uptr_t slabs_[PDS_MS_MAX_SLAB_ID];

    tep_store_t tep_store_;
    bd_store_t bd_store_;
    if_store_t if_store_;
    vpc_store_t vpc_store_;
    subnet_store_t subnet_store_;
    route_table_store_t route_table_store_;
    pathset_store_t pathset_store_;
    indirect_ps_store_t indirect_ps_store_;
    ip_track_store_t ip_track_store_;
    ip_track_internalip_store_t ip_track_internalip_store_;

    // Index generator for PDS HAL Overlay ECMP table
    sdk::lib::rte_indexer  *ecmp_idx_gen_;
    // Index generator for internal IP used to create static routes
    // for tracking destination IP
    sdk::lib::rte_indexer  *ip_track_internal_idx_gen_;

    static state_t* g_state_;
    static std::recursive_mutex g_mtx_;
    pds_batch_ctxt_guard_t bg_;
    uint32_t lnx_ifindex_table_[k_max_fp_ports] = {0};

    std::unordered_set<ip_prefix_t, ip_prefix_hash> ignored_prefixes_;

private:
    state_t(void);
    ~state_t(void) {
        sdk::lib::rte_indexer::destroy(ecmp_idx_gen_);
        sdk::lib::rte_indexer::destroy(ip_track_internal_idx_gen_);
    }
};

using tep_obj_uptr_t = std::unique_ptr<tep_obj_t>;
using subnet_obj_uptr_t = std::unique_ptr<subnet_obj_t>;
using bd_obj_uptr_t = std::unique_ptr<bd_obj_t>;
using if_obj_uptr_t = std::unique_ptr<if_obj_t>;
using rttbl_obj_uptr_t = std::unique_ptr<route_table_obj_t>;
using pathset_obj_uptr_t = std::unique_ptr<pathset_obj_t>;

void state_store_commit_objs (state_t::context_t& state_ctxt,
                              std::vector<base_obj_uptr_t>& objs);


ms_hw_tbl_id_t
state_lookup_indirect_ps_and_map_ip (state_t* state,
                                     ms_ps_id_t indirect_pathset,
                                     const ip_addr_t& ip,
                                     bool ms_evpn_tep_ip);
void
state_unmap_indirect_ps_from_ip (state_t* state, ms_ps_id_t indirect_pathset);

std::pair<ip_addr_t,bool>
state_indirect_ps_lookup_and_map_dpcorr (state_t* state,
                                         ms_ps_id_t indirect_pathset,
                                         ms_hw_tbl_id_t direct_ps_dpcorr);
} // End namespace

#endif
