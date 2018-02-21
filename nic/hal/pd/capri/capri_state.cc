// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#include "nic/include/base.h"
#include "nic/include/hal_mem.hpp"
#include "nic/hal/pd/capri/capri.hpp"
#include "nic/hal/pd/capri/capri_state.hpp"
#include "nic/hal/pd/capri/capri_txs_scheduler.hpp"
#include "nic/include/asic_pd.hpp"

//------------------------------------------------------------------------------
// (private) constructor method
//------------------------------------------------------------------------------
capri_state_pd::capri_state_pd ()
{
    txs_scheduler_map_idxr_ = NULL;
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
capri_state_pd::~capri_state_pd ()
{
    txs_scheduler_map_idxr_ ? delete txs_scheduler_map_idxr_ : HAL_NOP;
}
//------------------------------------------------------------------------------
// factory method
//------------------------------------------------------------------------------
capri_state_pd *
capri_state_pd::factory (void)
{
    capri_state_pd *state;

    state = new capri_state_pd();
    HAL_ASSERT_RETURN((state != NULL), NULL);
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
capri_state_pd::init (void)
{
    // BMAllocator based bmp range allocator to manage txs scheduler mapping
    txs_scheduler_map_idxr_ = 
                    new hal::BMAllocator(CAPRI_TXS_SCHEDULER_MAP_MAX_ENTRIES);
    HAL_ASSERT_RETURN((txs_scheduler_map_idxr_ != NULL), false);

    return true;
}
