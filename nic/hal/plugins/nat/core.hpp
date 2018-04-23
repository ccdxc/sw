/*
 * core.hpp
 */
#pragma once

#include "nic/include/fte.hpp"
#include "sdk/list.hpp"

namespace hal {
namespace plugins {
namespace nat {

/*
 * Constants
 */
const std::string FTE_FEATURE_NAT("pensando.io/nat:nat");

/*
 * Function prototypes
 */

// exec.cc
fte::pipeline_action_t nat_exec (fte::ctx_t &ctx);

}  // namespace nat
}  // namespace plugins
}  // namespace hal
