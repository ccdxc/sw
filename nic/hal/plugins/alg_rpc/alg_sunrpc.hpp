//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#pragma once

#include "core.hpp"
#include "nic/include/fte.hpp"

namespace hal {
namespace plugins {
namespace alg_rpc {

using namespace hal::plugins::alg_utils;

/*
 * Function prototypes
 */
hal_ret_t parse_sunrpc_control_flow(fte::ctx_t& ctx, l4_alg_status_t *l4_sess);

}  // namespace alg_rpc
}  // namespace plugins
}  // namespace hal
