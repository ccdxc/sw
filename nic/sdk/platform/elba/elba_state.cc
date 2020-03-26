// {C} Copyright 2020 Pensando Systems Inc. All rights reserved

#include "include/sdk/base.hpp"
#include "platform/elba/elba_state.hpp"
#include "platform/elba/elba_txs_scheduler.hpp"

namespace sdk {
namespace platform {
namespace elba {

//------------------------------------------------------------------------------
// (private) constructor method
//------------------------------------------------------------------------------
void
elba_state_pd::elba_state_pd ()
{
    txs_scheduler_map_idxr_ = NULL;
    mempartition_ = NULL;
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
void
elba_state_pd::~elba_state_pd ()
{
    txs_scheduler_map_idxr_ ? delete txs_scheduler_map_idxr_ : (void)0;
}

//------------------------------------------------------------------------------
// init() function to instantiate the state
//------------------------------------------------------------------------------
bool
elba_state_pd::init (asic_cfg_t *cfg)
{
    // BMAllocator based bmp range allocator to manage txs scheduler mapping
    txs_scheduler_map_idxr_ = 
                    new sdk::lib::BMAllocator(ELBA_TXS_SCHEDULER_MAP_MAX_ENTRIES);
    SDK_ASSERT_RETURN((txs_scheduler_map_idxr_ != NULL), false);
    mempartition_ = cfg->mempartition;
    cfg_path_ = cfg->cfg_path;

    return true;
}

//------------------------------------------------------------------------------
// factory method
//------------------------------------------------------------------------------
elba_state_pd *
elba_state_pd::factory (asic_cfg_t *cfg)
{
    elba_state_pd *state;

    state = new elba_state_pd();
    SDK_ASSERT_RETURN((state != NULL), NULL);
    if (state->init(cfg) == false) {
        delete state;
        return NULL;
    }
    return state;
}

}    // namespace elba
}    // namespace platform
}    // namespace sdk
