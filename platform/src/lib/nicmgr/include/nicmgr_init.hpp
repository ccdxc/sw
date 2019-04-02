//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __NICMGR_INIT_HPP__
#define __NICMGR_INIT_HPP__

#include "nic/sdk/platform/rdmamgr/rdmamgr.hpp"
#include "nic/sdk/platform/devapi/devapi.hpp"

rdmamgr *
rdma_manager_init(mpartition *mp, lif_mgr *lm);

devapi *devapi_init(void);

#endif /* __NICMGR_INIT_HPP__ */
