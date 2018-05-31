//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#pragma once

#include "nic/include/base.h"
#include "sdk/ht.hpp"
#include "sdk/twheel.hpp"
#include "nic/fte/fte_ctx.hpp"
#include "nic/fte/acl/ref.hpp"

using namespace acl;

namespace hal {
namespace plugins {
namespace alg_utils {

#define FTE_MAX_EXPECTED_FLOWS 524288
#define SET_EXP_FLOW_KEY(key, hal_key) {   \
     key.dir = hal_key.dir;              \
     key.svrf_id = hal_key.svrf_id;      \
     key.dvrf_id = hal_key.dvrf_id;      \
     key.sip = hal_key.sip.v4_addr;      \
     key.dip = hal_key.dip.v4_addr;      \
     key.proto = hal_key.proto;          \
     key.sport = hal_key.sport;          \
     key.dport = hal_key.dport;          \
}                             

//------------------------------------------------------------------------------
// ALG Expected flow entry db
//------------------------------------------------------------------------------
typedef struct expected_flow_s expected_flow_t;

typedef hal_ret_t (*expected_flow_handler_t)(fte::ctx_t& ctx, expected_flow_t* entry);

typedef struct exp_flow_key_ {
    uint8_t      dir;
    vrf_id_t     svrf_id;
    vrf_id_t     dvrf_id;
    ipv4_addr_t  sip;
    ipv4_addr_t  dip;
    uint8_t      proto;
    uint16_t     sport;
    uint16_t     dport;
} __PACK__ exp_flow_key_t;
std::ostream& operator<<(std::ostream& os, const exp_flow_key_t val);

struct expected_flow_s {
    exp_flow_key_t            key;
    expected_flow_handler_t   handler;
    void                     *timer;
    ref_t                     ref_count;
    bool                      deleting;
    sdk::lib::ht_ctxt_t       expected_flow_ht_ctxt;
};

hal_ret_t insert_expected_flow(expected_flow_t *entry);
expected_flow_t *remove_expected_flow(const exp_flow_key_t &key);
expected_flow_t *lookup_expected_flow(const hal::flow_key_t &key,
                                      bool exact_match = false);
void start_expected_flow_timer(expected_flow_t *entry, uint32_t timer_id,
                               uint32_t time_intvl, sdk::lib::twheel_cb_t cb,
                               void *timer_ctxt);
void*delete_expected_flow_timer(expected_flow_t *entry);
void dec_ref_count(expected_flow_t *entry);

} // namespace alg_utils
} // namespace plugins
} // namespace hal
