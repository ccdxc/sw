/*
 * core.hpp
 */
#pragma once

#include "nic/include/fte.hpp"
#include "nic/hal/plugins/alg_utils/core.hpp"

namespace hal {
namespace plugins {
namespace alg_ftp {

/*
 * Externs
 */
extern hal::plugins::alg_utils::alg_state_t *g_ftp_state;

/*
 * Constants
 */
const std::string FTE_FEATURE_ALG_FTP("pensando.io/alg_ftp:alg_ftp");

/*
 * Function prototypes
 */

// plugin.cc
fte::pipeline_action_t alg_ftp_exec(fte::ctx_t &ctx);

}  // namespace alg_ftp
}  // namespace plugins
}  // namespace hal
