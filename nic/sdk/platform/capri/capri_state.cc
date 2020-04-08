// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#include "include/sdk/base.hpp"
#include "platform/capri/csrint/csr_init.hpp"
#include "asic/asic.hpp"
#include "platform/capri/capri_state.hpp"
#include "platform/capri/capri_txs_scheduler.hpp"
#include "third-party/asic/capri/model/cap_top/cap_top_csr.h"

namespace sdk {
namespace platform {
namespace capri {

//------------------------------------------------------------------------------
// (private) constructor method
//------------------------------------------------------------------------------
capri_state_pd::capri_state_pd (void)
{
    txs_scheduler_map_idxr_ = NULL;
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
capri_state_pd::~capri_state_pd (void)
{
    txs_scheduler_map_idxr_ ? delete txs_scheduler_map_idxr_ : (void)0;
}

//------------------------------------------------------------------------------
// init() function to instantiate the state
//------------------------------------------------------------------------------
bool
capri_state_pd::init (asic_cfg_t *cfg)
{
    if (cfg) {
        // BMAllocator based bmp range allocator to manage txs scheduler mapping
        txs_scheduler_map_idxr_ =
                        new sdk::lib::BMAllocator(CAPRI_TXS_SCHEDULER_MAP_MAX_ENTRIES);
        SDK_ASSERT_RETURN((txs_scheduler_map_idxr_ != NULL), false);
        cfg_ = *cfg;
    }

    cap_top_ = &CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);

    return true;
}

//------------------------------------------------------------------------------
// factory method
//------------------------------------------------------------------------------
capri_state_pd *
capri_state_pd::factory (asic_cfg_t *cfg)
{
    capri_state_pd *state;

    state = new capri_state_pd();
    SDK_ASSERT_RETURN((state != NULL), NULL);
    if (state->init(cfg) == false) {
        delete state;
        return NULL;
    }
    return state;
}

sdk_ret_t
capri_state_pd_init (asic_cfg_t *cfg)
{
    if (g_capri_state_pd) {
        return SDK_RET_OK;
    }
    sdk::platform::capri::csr_init();

    g_capri_state_pd = sdk::platform::capri::capri_state_pd::factory(cfg);
    SDK_ASSERT_TRACE_RETURN((g_capri_state_pd != NULL), SDK_RET_INVALID_ARG,
                            "Failed to instantiate Capri PD");
    return SDK_RET_OK;
}

}    // namespace capri
}    // namespace platform
}    // namespace sdk
