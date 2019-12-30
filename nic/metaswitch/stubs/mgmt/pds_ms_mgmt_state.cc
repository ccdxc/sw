//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Mgmt stub state implementation
//---------------------------------------------------------------

#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_state.hpp"
#include "nic/sdk/lib/logger/logger.hpp"

namespace pds_ms {

mgmt_state_t* mgmt_state_t::g_state_ = nullptr;
bool mgmt_state_t::g_response_ready_ = false;
std::mutex mgmt_state_t::g_cv_mtx_;
std::mutex mgmt_state_t::g_state_mtx_;
std::condition_variable mgmt_state_t::g_cv_resp_;
types::ApiStatus mgmt_state_t::g_ms_response_;

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

}
