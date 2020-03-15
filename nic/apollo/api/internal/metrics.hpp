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

extern void *g_port_metrics_hndl;
extern void *g_mgmt_port_metrics_hndl;
extern void *g_host_if_metrics_hndl;

extern sdk_ret_t register_metrics(void);

}    // namespace api

#endif    // __INTERNAL_METERICS_HPP__
