//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/sdk/platform/rdmamgr/rdmamgr.hpp"
#include "platform/src/lib/rdmamgr_iris/rdmamgr_iris.hpp"
#include "platform/src/lib/nicmgr/include/nicmgr_init.hpp"

rdmamgr *
rdma_manager_init (mpartition *mp, lif_mgr *lm)
{
    return rdmamgr_iris::factory(mp, lm);

}
