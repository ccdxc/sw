//---------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
// State to make Mgmt gRPC wait for HAL commit to finish
//--------------------------------------------------------------

#ifndef __PDS_MS_HAL_WAIT_STATE_HPP__
#define __PDS_MS_HAL_WAIT_STATE_HPP__

#include "nic/metaswitch/stubs/common/pds_ms_error.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_defs.hpp"
#include "nic/sdk/lib/logger/logger.hpp"
#include <mutex>
#include <memory>
#include <unordered_set>
#include <condition_variable>

namespace pds_ms {

class hal_wait_state_t {
public:
    static void create(void) {
        SDK_ASSERT (hal_wait_state_t::g_state_ == nullptr);
        hal_wait_state_t::g_state_ = new hal_wait_state_t;
    }
    static void destroy(void) {
        delete(hal_wait_state_t::g_state_); hal_wait_state_t::g_state_ = nullptr;
    }

public:
    static void add_vrf_id(uint32_t vrf_id);
    static void del_vrf_id(uint32_t vrf_id);

    static void add_bd_id(ms_bd_id_t bd_id);
    static void del_bd_id(ms_bd_id_t bd_id);

    // These should be called only after taking the hal_wait mutex
    static bool has_vrf_id_non_reentrant_(uint32_t vrf_id);
    static bool has_bd_id_non_reentrant_(ms_bd_id_t bd_id);

    static void wait(std::function<bool()>);
    static void notify(void);

private:
    static hal_wait_state_t* g_state_;
    static std::mutex g_mtx_;
    static std::condition_variable  g_cv_;

    std::unordered_set<uint32_t> vrf_id_store_;
    std::unordered_set<ms_bd_id_t> bd_id_store_;
};

} // End namespace

#endif
