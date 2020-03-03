//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// metrics related internal APIs
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/metrics/metrics.hpp"

#ifndef __INTERNAL_METERICS_HPP__
#define __INTERNAL_METRICS_HPP__

namespace api {

extern sdk::metrics::metrics_schema_t port_metrics_schema;
extern sdk::metrics::metrics_schema_t mgmt_port_metrics_schema;
extern sdk::metrics::metrics_schema_t host_if_metrics_schema;

}    // namespace api

#endif    // __INTERNAL_METERICS_HPP__
