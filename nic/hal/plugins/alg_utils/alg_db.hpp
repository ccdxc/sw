#pragma once

#include "nic/include/base.h"
#include "sdk/ht.hpp"
#include "nic/fte/fte_ctx.hpp"

namespace hal {
namespace plugins {
namespace alg_utils {

#define FTE_MAX_EXPECTED_FLOWS 524288

//------------------------------------------------------------------------------
// ALG Expected flow entry db
//------------------------------------------------------------------------------
typedef struct expected_flow_s expected_flow_t;

typedef hal_ret_t (*expected_flow_handler_t)(fte::ctx_t& ctx, expected_flow_t* entry);

struct expected_flow_s {
    hal::flow_key_t           key;
    expected_flow_handler_t   handler;
    sdk::lib::ht_ctxt_t     expected_flow_ht_ctxt;  
};

hal_ret_t insert_expected_flow(expected_flow_t *entry);
expected_flow_t *remove_expected_flow(const hal::flow_key_t &key);
expected_flow_t *lookup_expected_flow(const hal::flow_key_t &key, bool exact_match = false);

} // namespace alg_utils
} // namespace plugins
} // namespace hal
