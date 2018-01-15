/*
 * core.hpp
 */
#pragma once

#include "nic/include/fte.hpp"
#include "sdk/list.hpp"
#include "nic/hal/plugins/alg_utils/core.hpp"

namespace hal {
namespace plugins {
namespace alg_dns {

using namespace hal::plugins::alg_utils;

/*
 * Externs
 */
extern alg_state_t *g_dns_state;

/*
 * Constants
 */
const std::string FTE_FEATURE_ALG_DNS("pensando.io/alg_dns:alg_dns");

/*
 * Forward declarations
 */
typedef union dns_info_ dns_info_t; 

/*
 * Function prototypes
 */

// plugin.cc
fte::pipeline_action_t alg_dns_exec (fte::ctx_t &ctx);
void dnsinfo_cleanup_hdlr (l4_alg_status_t *l4_sess);
typedef hal_ret_t (*dns_cb_t) (fte::ctx_t& ctx, l4_alg_status_t *exp_flow);

/*
 * Data Structures
 */
typedef union dns_info_ {
    uint16_t        dnsid;
    dllist_ctxt_t   lentry;
    uint32_t parse_errors;
} __PACK__ dns_info_t;

}  // namespace alg_dns
}  // namespace plugins
}  // namespace hal
