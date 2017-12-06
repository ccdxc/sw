/*
 * core.hpp
 */
#pragma once

#include "nic/include/fte.hpp"

namespace hal {
namespace plugins {
namespace alg_sip {

/*
 * Constants
 */
const std::string FTE_FEATURE_ALG_SIP("pensando.io/alg_sip:alg_sip");

/*
 * Function prototypes
 */
fte::pipeline_action_t alg_sip_exec(fte::ctx_t &ctx);

}  // namespace alg_sip 
}  // namespace plugins
}  // namespace hal
