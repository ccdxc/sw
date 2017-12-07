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

// plugin.cc
fte::pipeline_action_t alg_sip_exec(fte::ctx_t &ctx);

// utils.cc
int callid_len(const char *buf, const char *end, int *shift);
int media_len(const char *buf, const char *end, int *shift);
const char * sip_skip_whitespace(const char *buf, const char *limit);
int sip_skp_epaddr_len(uint16_t af, const char *buf, const char *limit,
                       int *shift);

}  // namespace alg_sip 
}  // namespace plugins
}  // namespace hal
