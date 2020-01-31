//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __RDMA_LIF_HPP__
#define __RDMA_LIF_HPP__

#include "include/sdk/base.hpp"
#include "nic/sdk/platform/utils/mpartition.hpp"
#include "nic/sdk/platform/utils/lif_mgr/lif_mgr.hpp"

namespace sdk {
namespace platform {

class rdmamgr {
public:
    static rdmamgr *factory(mpartition mp, lif_mgr *lm_);
    static void destroy(rdmamgr *rmgr);

    virtual sdk_ret_t lif_init(uint32_t lif, uint32_t max_keys,
                               uint32_t max_ahs, uint32_t max_ptes,
                               uint64_t mem_bar_addr, uint32_t mem_bar_size,
                               uint32_t max_prefetch) = 0;
;
    virtual uint64_t rdma_get_pt_base_addr (uint32_t lif) = 0;
    virtual uint64_t rdma_get_kt_base_addr (uint32_t lif) = 0;

protected:
    rdmamgr() {}
    virtual ~rdmamgr() {}
};

}    // namespace nicmgr
}    // namespace platform

using sdk::platform::rdmamgr;

#endif    // __RDMA_LIF_HPP__
