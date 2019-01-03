// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#include "include/sdk/base.hpp"
#include "include/sdk/platform/capri/capri_cfg.hpp"
#include "include/sdk/platform/capri/capri_state.hpp"
#include "include/sdk/platform/capri/capri_txs_scheduler.hpp"

namespace sdk {
namespace platform {
namespace capri {

//------------------------------------------------------------------------------
// (private) constructor method
//------------------------------------------------------------------------------
capri_state_pd::capri_state_pd()
{
    txs_scheduler_map_idxr_ = NULL;
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
capri_state_pd::~capri_state_pd()
{
    txs_scheduler_map_idxr_ ? delete txs_scheduler_map_idxr_ : (void)0;
}

//------------------------------------------------------------------------------
// factory method
//------------------------------------------------------------------------------
capri_state_pd *
capri_state_pd::factory(void)
{
    capri_state_pd *state;

    state = new capri_state_pd();
    SDK_ASSERT_RETURN((state != NULL), NULL);
    if (state->init() == false) {
        delete state;
        return NULL;
    }
    return state;
}

//------------------------------------------------------------------------------
// init() function to instantiate the state
//------------------------------------------------------------------------------
bool
capri_state_pd::init(void)
{
    // BMAllocator based bmp range allocator to manage txs scheduler mapping
    txs_scheduler_map_idxr_ = 
                    new sdk::lib::BMAllocator(CAPRI_TXS_SCHEDULER_MAP_MAX_ENTRIES);
    SDK_ASSERT_RETURN((txs_scheduler_map_idxr_ != NULL), false);

    return true;
}

} // namespace capri
} // namespace platform
} // namespace sdk
