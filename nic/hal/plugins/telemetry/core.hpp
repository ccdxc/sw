//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#pragma once

#include "nic/include/fte.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/hal/plugins/cfg/telemetry/telemetry.hpp"

namespace hal {
namespace plugins {
namespace telemetry {

/*
 * NAT feature fte state (stored in the fte ctx for programming rflow)
 */
typedef struct telemetry_info_s {
} telemetry_info_t;

/*
 * Constants
 */
const std::string FTE_FEATURE_TELEMETRY("pensando.io/telemetry:telemetry");

/*
 * Function prototypes
 */

// exec.cc
fte::pipeline_action_t telemetry_exec(fte::ctx_t &ctx);

}  // namespace telemetry
}  // namespace plugins
}  // namespace hal
