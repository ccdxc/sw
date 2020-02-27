//-----------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/sdk/platform/evutils/include/evutils.h"
#include "eth_lif.hpp"
#include "logger.hpp"
#include "gen/proto/nicmgr/metrics.delphi.hpp"


void EthLif::AddLifMetrics (void) 
{
    auto lif_stats =
        delphi::objects::LifMetrics::NewLifMetrics(hal_lif_info_.lif_id, lif_stats_addr);
    if (lif_stats == NULL) {
        NIC_LOG_ERR("{}: Failed lif metrics registration with delphi",
            hal_lif_info_.name);
        throw;
    }
}
