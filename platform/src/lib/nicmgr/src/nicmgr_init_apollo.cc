//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "platform/src/lib/rdmamgr_apollo/rdmamgr_apollo.hpp"
#include "platform/src/lib/nicmgr/include/nicmgr_init.hpp"
#include "nic/apollo/api/impl/devapi_impl.hpp"

rdmamgr *
rdma_manager_init (mpartition *mp, lif_mgr *lm)
{
    return rdmamgr_apollo::factory(mp, lm);
}

devapi *
devapi_init(void)
{
    return api::impl::devapi_impl::factory();
}
