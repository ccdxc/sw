//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/sdk/platform/rdmamgr/rdmamgr.hpp"
#include "platform/src/lib/rdmamgr_iris/rdmamgr_iris.hpp"
#include "platform/src/lib/nicmgr/include/nicmgr_init.hpp"
#include "platform/src/lib/devapi_iris/devapi_iris.hpp"

rdmamgr *
rdma_manager_init (mpartition *mp, lif_mgr *lm)
{
    return rdmamgr_iris::factory(mp, lm);

}

devapi *
devapi_init(void)
{
    return devapi_iris::factory();
}
