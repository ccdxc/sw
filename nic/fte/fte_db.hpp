#pragma once


#include "nic/include/base.h"
#include "nic/sdk/include/ht.hpp"

namespace fte {

class ctx_t;

#define FTE_MAX_EXPECTED_FLOWS 524288

//------------------------------------------------------------------------------
// FTE Expected flow entry db
//------------------------------------------------------------------------------
typedef struct expected_flow_s expected_flow_t;

typedef hal_ret_t (*expected_flow_handler_t)(ctx_t& ctx, expected_flow_t* entry);

struct expected_flow_s {
    hal::flow_key_t           key;
    expected_flow_handler_t   handler;
    sdk::lib::ht_ctxt_t     expected_flow_ht_ctxt;  
};

hal_ret_t insert_expected_flow(expected_flow_t *entry);
expected_flow_t *remove_expected_flow(const hal::flow_key_t &key);
expected_flow_t *lookup_expected_flow(const hal::flow_key_t &key, bool exact_match = false);

}

