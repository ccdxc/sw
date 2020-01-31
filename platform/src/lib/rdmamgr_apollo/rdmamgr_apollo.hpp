//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __RDMAMGR_APOLLO_HPP__
#define __RDMAMGR_APOLLO_HPP__

#include "nic/sdk/platform/rdmamgr/rdmamgr.hpp"
#include "nic/sdk/lib/pal/pal.hpp"

#define READ_MEM        sdk::lib::pal_mem_read
#define WRITE_MEM       sdk::lib::pal_mem_write

namespace apollo {

class rdmamgr_apollo : public rdmamgr {
private:
    mpartition *mp_;
    lif_mgr *lm_;

private:
    sdk_ret_t init_(mpartition *mp, lif_mgr *lm);
public:
    static rdmamgr_apollo *factory(mpartition *mp, lif_mgr *lm_);
    static void destroy(rdmamgr *rmgr);

    sdk_ret_t lif_init(uint32_t lif, uint32_t max_keys,
                       uint32_t max_ahs, uint32_t max_ptes,
                       uint64_t mem_bar_addr, uint32_t mem_bar_size,
                       uint32_t max_prefetch_wqes);

    uint64_t rdma_get_pt_base_addr (uint32_t lif);
    uint64_t rdma_get_kt_base_addr (uint32_t lif);

protected:
    rdmamgr_apollo() {}
    ~rdmamgr_apollo() {}

};

}    // namespace apollo

using apollo::rdmamgr_apollo;

#endif    // __RDMAMGR_APOLLO_HPP__
