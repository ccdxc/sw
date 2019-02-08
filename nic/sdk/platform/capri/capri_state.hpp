// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __CAPRI_STATE_HPP__
#define __CAPRI_STATE_HPP__

#include "nic/sdk/platform/utils/mpartition.hpp"
#include "nic/sdk/platform/capri/capri_cfg.hpp"
#include "third-party/asic/capri/model/cap_top/cap_top_csr.h"

namespace sdk {
namespace platform {
namespace capri {

class capri_state_pd {
public:
    static capri_state_pd *factory(capri_cfg_t *cfg);
    ~capri_state_pd();

    // get APIs for TXS scheduler related state
    sdk::lib::BMAllocator *txs_scheduler_map_idxr(void) { return txs_scheduler_map_idxr_; }
    std::string cfg_path(void) const { return cfg_path_; }
    mpartition *mempartition(void) const { return mempartition_; }
    cap_top_csr_t& cap_top() { return *cap_top_; }

private:
    // TXS scheduler related state
    struct {
        sdk::lib::BMAllocator    *txs_scheduler_map_idxr_;
    } __PACK__;
    std::string    cfg_path_;    // HAL config path
    mpartition     *mempartition_;
    cap_top_csr_t *cap_top_;

private:
    capri_state_pd();
    bool init(capri_cfg_t *cfg);
};
extern class capri_state_pd *g_capri_state_pd;

extern sdk_ret_t capri_state_pd_init(capri_cfg_t *cfg);

}    // namespace capri
}    // namespace platform
}    // namespace sdk

#endif    // __CAPRI_STATE_HPP__

