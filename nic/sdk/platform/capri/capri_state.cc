// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#include "include/sdk/base.hpp"
#include "platform/capri/capri_cfg.hpp"
#include "platform/capri/capri_state.hpp"
#include "platform/capri/capri_txs_scheduler.hpp"

namespace sdk {
namespace platform {
namespace capri {

//------------------------------------------------------------------------------
// (private) constructor method
//------------------------------------------------------------------------------
capri_state_pd::capri_state_pd()
{
    txs_scheduler_map_idxr_ = NULL;
    mempartition_ = NULL;
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
capri_state_pd::~capri_state_pd()
{
    txs_scheduler_map_idxr_ ? delete txs_scheduler_map_idxr_ : (void)0;
}

//------------------------------------------------------------------------------
// init() function to instantiate the state
//------------------------------------------------------------------------------
bool
capri_state_pd::init(capri_cfg_t *cfg)
{
    // BMAllocator based bmp range allocator to manage txs scheduler mapping
    txs_scheduler_map_idxr_ = 
                    new sdk::lib::BMAllocator(CAPRI_TXS_SCHEDULER_MAP_MAX_ENTRIES);
    SDK_ASSERT_RETURN((txs_scheduler_map_idxr_ != NULL), false);
    mempartition_ = cfg->mempartition;
    cfg_path_ = cfg->cfg_path;

    return true;
}

//------------------------------------------------------------------------------
// factory method
//------------------------------------------------------------------------------
capri_state_pd *
capri_state_pd::factory(capri_cfg_t *cfg)
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

}    // namespace capri
}    // namespace platform
}    // namespace sdk
