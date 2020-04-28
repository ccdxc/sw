//
//  {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
//
// C++ routines for interacting with the metrics library

#include <nic/sdk/lib/metrics/metrics.hpp>
#include <nic/apollo/api/utils.hpp>
#include "pdsa_hdlr.h"


static sdk::metrics::schema_t vpp_schema = {
    FLOW_STATS_SCHEMA_NAME,
    sdk::metrics::SW,
    {
#define _(n,s) s,
        foreach_flow_type_counter
#undef _
        NULL,
    }
};

void *
pdsa_flow_stats_init (void)
{
    return sdk::metrics::create(&vpp_schema);
}

void
pdsa_flow_stats_publish (void *metrics_hdl, uint64_t *counter)
{
    sdk::metrics::metrics_update(metrics_hdl,
                                 *(sdk::metrics::key_t *)
                                 api::uuid_from_objid(0).id, counter);
}
