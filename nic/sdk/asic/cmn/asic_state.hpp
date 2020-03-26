// {C} Copyright 2020 Pensando Systems Inc. All rights reserved

#ifndef __ASIC_STATE_HPP__
#define __ASIC_STATE_HPP__

#include "platform/utils/mpartition.hpp"
#include "asic/cmn/asic_cfg.hpp"

namespace sdk {
namespace asic {

class asic_state {
public:
    static asic_state  *factory(asic_cfg_t *cfg);
    ~asic_state();
    mpartition *mempartition(void) const { return cfg_.mempartition; }
    asic_cfg_t *asic_cfg(void) const { return (asic_cfg_t *) &cfg_; }
private:
    asic_cfg_t     cfg_; // config
    //std::string    cfg_path_;    // HAL config path
    asic_state();
    bool init(asic_cfg_t *cfg);
};

extern class asic_state *g_asic_state;
sdk_ret_t asic_state_init(asic_cfg_t *cfg);

}    // namespace asic 
}    // namespace sdk 

#endif    // __ASIC_STATE_HPP__
