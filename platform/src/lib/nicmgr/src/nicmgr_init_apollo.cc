//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifdef APOLLO
#include "platform/src/lib/rdmamgr_apollo/rdmamgr_apollo.hpp"
#endif
#include "platform/src/lib/nicmgr/include/nicmgr_init.hpp"
#include "nic/apollo/api/impl/devapi_impl.hpp"

rdmamgr *
rdma_manager_init (mpartition *mp, lif_mgr *lm)
{
#ifdef APOLLO
    return rdmamgr_apollo::factory(mp, lm);
#endif
    return NULL;
}

devapi *
devapi_init(void)
{
    return api::impl::devapi_impl::factory();
}
