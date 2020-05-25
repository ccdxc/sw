// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __SDK_LINKMGR_STATE_HPP__
#define __SDK_LINKMGR_STATE_HPP__

#include "lib/slab/slab.hpp"
#include "port.hpp"

using sdk::lib::slab;

namespace sdk {
namespace linkmgr {

class linkmgr_state {
public:
    static linkmgr_state *factory(void);
    // get APIs for port related state
    slab *port_slab(void) const { return port_slab_; }

    /// \brief  return the port bitmap of uplink link status
    /// \return port bitmap of uplink link status
    uint64_t port_bmap(void) { return port_bmap_; }

    /// \brief set the port bitmap of uplink link status
    void set_port_bmap(uint64_t port_bmap) {
        this->port_bmap_ = port_bmap;
    }

    /// \brief  return the port bitmap mask of uplink link status
    /// \return port bitmap mask of uplink link status
    uint64_t port_bmap_mask(void) { return port_bmap_mask_; }

    /// \brief set the port bitmap mask of uplink link status
    void set_port_bmap_mask(uint64_t port_bmap_mask) {
        this->port_bmap_mask_ = port_bmap_mask;
    }

private:
    sdk_ret_t init(void);

private:
    slab       *port_slab_;
    uint64_t   port_bmap_;              ///< bitmap of uplinks physical link status
    uint64_t   port_bmap_mask_;         ///< bitmap mask of uplinks physical link status
};

extern linkmgr_state *g_linkmgr_state;

}    // namespace linkmgr
}    // namespace sdk

#endif    // __SDK_LINKMGR_STATE_HPP__
