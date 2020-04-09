// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __ELBA_STATE_HPP__
#define __ELBA_STATE_HPP__

#include "asic/asic.hpp"
#include "nic/sdk/platform/utils/mpartition.hpp"

namespace sdk {
namespace platform {
namespace elba {

class elba_state_pd {
public:
    static elba_state_pd *factory(asic_cfg_t *cfg);
    ~elba_state_pd();

    // get APIs for TXS scheduler related state
    sdk::lib::BMAllocator *txs_scheduler_map_idxr(void)
    {
        return txs_scheduler_map_idxr_;
    }

    std::string cfg_path(void) const { return cfg_path_; }
    mpartition *mempartition(void) const { return mempartition_; }

private:
    // TXS scheduler related state
    struct {
        sdk::lib::BMAllocator    *txs_scheduler_map_idxr_;
    } __PACK__;
    std::string    cfg_path_;    // HAL config path
    mpartition     *mempartition_;

private:
    elba_state_pd();
    bool init(asic_cfg_t *cfg);
};
extern class elba_state_pd *g_elba_state_pd;

sdk_ret_t elba_state_pd_init(asic_cfg_t *cfg);

}    // namespace elba
}    // namespace platform
}    // namespace sdk

#endif    // __ELBA_STATE_HPP__

