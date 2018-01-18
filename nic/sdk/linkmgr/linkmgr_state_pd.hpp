#ifndef __LINKMGR_STATE_PD_HPP__
#define __LINKMGR_STATE_PD_HPP__

#include "sdk/slab.hpp"
#include "port.hpp"

using sdk::lib::slab;

namespace sdk {
namespace linkmgr {
namespace pd {

class linkmgr_state_pd {
public:
    static linkmgr_state_pd* factory(void);
    // get APIs for port related state
    slab *port_slab(void) const { return port_slab_; }

private:
    sdk_ret_t init(void);

private:
    slab       *port_slab_;
};

extern linkmgr_state_pd *g_linkmgr_state_pd;

}    // namespace pd
}    // namespace linkmgr
}    // namespace sdk

#endif    // __LINKMGR_STATE_PD_HPP__

