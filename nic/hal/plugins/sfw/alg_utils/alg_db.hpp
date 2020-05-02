//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#pragma once

#include "nic/include/base.hpp"
#include "lib/ht/ht.hpp"
#include "lib/twheel/twheel.hpp"
#include "nic/fte/fte_ctx.hpp"
#include "nic/fte/acl/ref.hpp"
#include "gen/proto/nwsec.pb.h"

using namespace acl;
using nwsec::SecurityFlowGateGetRequest;
using nwsec::SecurityFlowGateGetResponse;
using nwsec::SecurityFlowGateGetRequestMsg;
using nwsec::SecurityFlowGateGetResponseMsg;
using nwsec::FlowGateKey;
using types::FlowDirection;

namespace hal {
namespace plugins {
namespace alg_utils {

#define FTE_MAX_EXPECTED_FLOWS 524288
#define SET_EXP_FLOW_KEY(key, hal_key) {   \
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
    void                     *timer_ctxt;
    ref_t                     ref_count;
    bool                      deleting;
    sdk::lib::ht_ctxt_t       expected_flow_ht_ctxt;
};

hal_ret_t insert_expected_flow(expected_flow_t *entry);
expected_flow_t *remove_expected_flow(const exp_flow_key_t &key);
expected_flow_t *lookup_expected_flow(const exp_flow_key_t &key);
expected_flow_t *lookup_expected_flow(const hal::flow_key_t &key,
                                      bool exact_match = false);
void start_expected_flow_timer(expected_flow_t *entry, uint32_t timer_id,
                               uint64_t time_intvl, sdk::lib::twheel_cb_t cb,
                               void *timer_ctxt);
void* delete_expected_flow_timer(expected_flow_t *entry);
void dec_ref_count(expected_flow_t *entry);

hal_ret_t walk_expected_flow(nwsec::SecurityFlowGateGetRequest&      req,
                             nwsec::SecurityFlowGateGetResponseMsg   *res);

void* update_expected_flow_timer(expected_flow_t *entry, 
                                 uint64_t time_intvl, void *timer_ctxt);
void* update_expected_flow_timer_ctxt(expected_flow_t *entry, void *timer_ctxt);
void  flow_gate_key_to_proto(expected_flow_t *flow, FlowGateKey *key);
void  flow_gate_key_from_proto(expected_flow_t *flow, const FlowGateKey &key);

} // namespace alg_utils
} // namespace plugins
} // namespace hal
