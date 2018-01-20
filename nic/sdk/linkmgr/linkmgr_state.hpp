// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __SDK_LINKMGR_STATE_HPP__
#define __SDK_LINKMGR_STATE_HPP__

#include "sdk/slab.hpp"
#include "port.hpp"

using sdk::lib::slab;

namespace sdk {
namespace linkmgr {

class linkmgr_state {
public:
    static linkmgr_state *factory(void);
    // get APIs for port related state
    slab *port_slab(void) const { return port_slab_; }

private:
    sdk_ret_t init(void);

private:
    slab       *port_slab_;
};

extern linkmgr_state *g_linkmgr_state;

}    // namespace linkmgr
}    // namespace sdk

#endif    // __SDK_LINKMGR_STATE_HPP__

