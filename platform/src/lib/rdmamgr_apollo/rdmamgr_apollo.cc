//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include "rdmamgr_apollo.hpp"
#include "rdmamgr_apollo_mem.hpp"

namespace apollo {

rdmamgr_apollo *
rdmamgr_apollo::factory(mpartition *mp, lif_mgr *lm)
{
    sdk_ret_t ret      = SDK_RET_OK;
    void *mem          = NULL;
    rdmamgr_apollo      *rapollo = NULL;

    mem = (rdmamgr_apollo *)RDMAMGR_CALLOC(RDMAMGR_APOLLO_MEM_ALLOC_RDMA_MGR_APOLLO,
                                           sizeof(rdmamgr_apollo));
    if (mem) {
        rapollo = new (mem) rdmamgr_apollo();
        ret = rapollo->init_(mp, lm);
        if (ret != SDK_RET_OK) {
            rapollo->~rdmamgr_apollo();
            RDMAMGR_FREE(RDMAMGR_MEM_ALLOC_RDMA_MGR_apollo, mem);
            rapollo = NULL;
        }
    }
    return rapollo;
}

void
rdmamgr_apollo::destroy(rdmamgr *rmgr)
{
    NIC_LOG_DEBUG("Destroy rdmamgr_apollo");
    rdmamgr_apollo *rmgr_apollo = dynamic_cast<rdmamgr_apollo*>(rmgr);
    rmgr_apollo->~rdmamgr_apollo();
    RDMAMGR_FREE(RDMAMGR_MEM_ALLOC_RDMA_MGR_apollo, rmgr_apollo);
}


sdk_ret_t
rdmamgr_apollo::init_(mpartition *mp, lif_mgr *lm)
{
    mp_       = mp;
    lm_       = lm;
    return SDK_RET_OK;
}

sdk_ret_t
rdmamgr_apollo::lif_init(uint32_t lif, uint32_t max_keys,
                       uint32_t max_ahs, uint32_t max_ptes,
                       uint64_t mem_bar_addr, uint32_t mem_bar_size,
                       uint32_t max_prefetch_wqes)
{
    return SDK_RET_OK;
}

uint64_t
rdmamgr_apollo::rdma_get_pt_base_addr(uint32_t lif)
{
    return 0;
}

uint64_t
rdmamgr_apollo::rdma_get_kt_base_addr(uint32_t lif)
{
    return 0;
}

}    // namespace apollo
