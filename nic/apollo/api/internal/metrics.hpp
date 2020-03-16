//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// metrics related internal APIs
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/metrics/metrics.hpp"

#ifndef __INTERNAL_METERICS_HPP__
#define __INTERNAL_METRICS_HPP__

namespace api {

extern sdk::metrics::schema_t port_schema;
extern sdk::metrics::schema_t mgmt_port_schema;
extern sdk::metrics::schema_t hostif_schema;
extern sdk_ret_t register_metrics(void);

}    // namespace api

#endif    // __INTERNAL_METERICS_HPP__
