//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "lib/periodic/periodic.hpp"
#include "alg_db.hpp"

namespace hal {
namespace plugins {
namespace alg_utils {

static void * expected_flow_get_key_func(void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((expected_flow_t *)entry)->key);
}

static uint32_t expected_flow_key_size(void) {
    return sizeof(exp_flow_key_t);
}

//------------------------------------------------------------------------------
// Expected flow entries hash table
//------------------------------------------------------------------------------
static sdk::lib::ht *expected_flow_ht()
{
    static sdk::lib::ht* ht_ =
        sdk::lib::ht::factory(FTE_MAX_EXPECTED_FLOWS,
                                expected_flow_get_key_func,
                                expected_flow_key_size());

    return ht_;
}

//------------------------------------------------------------------------------
// Insert an expected flow
//------------------------------------------------------------------------------
hal_ret_t
insert_expected_flow(expected_flow_t *entry)
{
    HAL_TRACE_DEBUG("ALG::insert_expected_flow key={}", entry->key);
    ref_init(&entry->ref_count, [] (const acl::ref_t * ref_count) {});
    entry->deleting = false;
    entry->expected_flow_ht_ctxt.reset();
    return hal_sdk_ret_to_hal_ret(expected_flow_ht()->
                                  insert(entry, &entry->expected_flow_ht_ctxt));
}

//------------------------------------------------------------------------------
// Remove an expected flow entry
//------------------------------------------------------------------------------
expected_flow_t *
remove_expected_flow(const exp_flow_key_t &key)
{
    HAL_TRACE_DEBUG("ALG::remove_expected_flow  key={}", key);
    return (expected_flow_t *)expected_flow_ht()->remove((void *)&key);
}

//------------------------------------------------------------------------------
// Decrement the reference count once done using the entry -- needed for exp flow
// timer expiry delete handling
//------------------------------------------------------------------------------
void
dec_ref_count(expected_flow_t *entry)
{
    ref_dec(&entry->ref_count);
}

//------------------------------------------------------------------------------
// Start a timer on the expected flow - Some ALGs such as SUNRPC, MSRPC need it
//------------------------------------------------------------------------------
void
start_expected_flow_timer(expected_flow_t *entry, uint32_t timer_id,
                          uint64_t time_intvl, sdk::lib::twheel_cb_t cb,
                          void *timer_ctxt)
{
    entry->timer = sdk::lib::timer_schedule(timer_id, time_intvl,
                                            (void *)timer_ctxt, cb, false);
    if (!entry->timer) {
        HAL_TRACE_ERR("Failed to start timer for expected flow with key: {}",
                      entry->key);
    }
}

//------------------------------------------------------------------------------
// delete expected flow timer - Some ALGs such as SUNRPC, MSRPC need it
//------------------------------------------------------------------------------
void*
delete_expected_flow_timer(expected_flow_t *entry)
{
    return (sdk::lib::timer_delete(entry->timer));
}

//------------------------------------------------------------------------------
// update expected flow timer - Some ALGs such as SUNRPC, MSRPC need it
//------------------------------------------------------------------------------
void*
update_expected_flow_timer(expected_flow_t *entry, uint64_t time_intvl, void *timer_ctxt)
{
    return (sdk::lib::timer_update(entry->timer, time_intvl, false, timer_ctxt));
}

//------------------------------------------------------------------------------
// update expected flow timer context - Some ALGs such as SUNRPC, MSRPC need it
//------------------------------------------------------------------------------
void*
update_expected_flow_timer_ctxt(expected_flow_t *entry, void *timer_ctxt)
{
    return (sdk::lib::timer_update_ctxt(entry->timer, timer_ctxt));
}

//------------------------------------------------------------------------------
// lookup expected flow
//------------------------------------------------------------------------------
expected_flow_t *
lookup_expected_flow(const exp_flow_key_t &key) 
{
    return((expected_flow_t *)expected_flow_ht()->lookup((void *)&key));
}

//------------------------------------------------------------------------------
// Lookup a expected_flow entry
// This will do the following lookupos
//  1. Exact match
//  2. Reverse key lookup - exact match
//  3. Wildcard SPORT
//  4. Wildcard SPORT, SIP and DIR
//------------------------------------------------------------------------------
expected_flow_t *
lookup_expected_flow(const hal::flow_key_t &ikey, bool exact_match)
{
    expected_flow_t *entry = NULL;
    exp_flow_key_t key, lookup_key;

    if (!expected_flow_ht()->num_entries())
        return NULL;

    bzero((void *)&key, sizeof(exp_flow_key_t));
    SET_EXP_FLOW_KEY(key, ikey);

    // Exact match
    if (exact_match) {
       if ((entry = (expected_flow_t *)expected_flow_ht()->lookup((void *)&key))) {
           HAL_TRACE_DEBUG("ALG::lookup_expected_flow exact match key={}", key);
           goto end;
       }
       return NULL;
    }

    // wildcard matches are supported for tcp/udp only
    if ((ikey.flow_type != hal::FLOW_TYPE_V4 && ikey.flow_type != hal::FLOW_TYPE_V6) ||
        (ikey.proto != IPPROTO_TCP && ikey.proto != IPPROTO_UDP)) {
        return NULL;
    }

    // Mask SPORT and do lookup (tftp)
    lookup_key = key;
    lookup_key.sport = 0;
    if ((entry = (expected_flow_t *)expected_flow_ht()->lookup((void *)&lookup_key))) {
        HAL_TRACE_DEBUG("ALG::lookup_expected_flow widcard sport key={}", lookup_key);
        goto end;
    }

    //Mask DIR, SPORT and do lookup (Active FTP)
    lookup_key.dir = 0;
    if ((entry = (expected_flow_t *)expected_flow_ht()->lookup((void *)&lookup_key))) {
        HAL_TRACE_DEBUG("ALG::lookup_expected_flow wildcard dir key={}", lookup_key);
        goto end;
    }

    // Mask SIP, DIR, SPORT and do lookup (RPC/Passive FTP)
    lookup_key.sip = 0;
    if ((entry = (expected_flow_t *)expected_flow_ht()->lookup((void *)&lookup_key))) {
        HAL_TRACE_DEBUG("ALG::lookup_expected_flow wildcard sip/sport/dir key={}", lookup_key);
        goto end;
    }

    // Mask DIR only and do lookup (RTSP)
    lookup_key = key;
    lookup_key.dir = 0;
    if ((entry = (expected_flow_t *)expected_flow_ht()->lookup((void *)&lookup_key))) {
        HAL_TRACE_DEBUG("ALG::lookup_expected_flow wildcard dir key={}", lookup_key);
        goto end;
    }

end:
    if (entry) {
        if (!entry->deleting) {
            ref_inc(&entry->ref_count);
        } else {
            entry = NULL;
        }
    }

    return entry;
}

//Temporary data structure to
//get the ALG name from ALG utils
//container
struct alg_utils {
   expected_flow_t   flow;
   nwsec::ALGName    alg;
};

void
flow_gate_key_to_proto(expected_flow_t *flow, FlowGateKey *key)
{
    key->mutable_dst_ip()->set_v4_addr(flow->key.dip);
    key->mutable_src_ip()->set_v4_addr(flow->key.sip);
    key->set_src_vrf_id(flow->key.svrf_id);
    key->set_dst_vrf_id(flow->key.dvrf_id);
    key->set_ip_proto((types::IPProtocol)flow->key.proto);
    key->set_src_port(flow->key.sport);
    key->set_dst_port(flow->key.dport);
    key->set_direction((flow->key.dir == FLOW_DIR_FROM_UPLINK) ?
                        types::FLOW_DIRECTION_FROM_UPLINK :\
                        types::FLOW_DIRECTION_FROM_HOST);
}

void
flow_gate_key_from_proto(expected_flow_t *flow, const FlowGateKey &key)
{
    flow->key.dip     = key.dst_ip().v4_addr();
    flow->key.sip     = key.src_ip().v4_addr();
    flow->key.svrf_id = key.src_vrf_id();
    flow->key.dvrf_id = key.dst_vrf_id();
    flow->key.proto   = key.ip_proto();
    flow->key.sport   = key.src_port();
    flow->key.dport   = key.dst_port();
    flow->key.dir     = (key.direction() == types::FLOW_DIRECTION_FROM_UPLINK
                                                    ? FLOW_DIR_FROM_UPLINK : FLOW_DIR_FROM_DMA);
}

void expected_flow_get_fill_rsp(expected_flow_t *flow,
                                SecurityFlowGateGetResponse *resp) {
    FlowGateKey *key = resp->mutable_flow_gate_key();

    flow_gate_key_to_proto(flow, key);

    resp->set_alg(((struct alg_utils *)flow)->alg);
    resp->set_delete_marked(flow->deleting);
    resp->set_ref_count(flow->ref_count.count);
    if (flow->timer != NULL) 
        resp->set_time_to_age(sdk::lib::get_timeout_remaining(flow->timer)/TIME_MSECS_PER_SEC);
    else
        resp->set_time_to_age(0xFFFFFFFF);
}

hal_ret_t walk_expected_flow(SecurityFlowGateGetRequest&      req,
                             SecurityFlowGateGetResponseMsg   *res) {

    auto walk_func = [](void *entry, void *ctxt) {
        expected_flow_t *flow = (expected_flow_t *)entry;
        SecurityFlowGateGetResponseMsg *rsp = ((SecurityFlowGateGetResponseMsg *)\
                           ctxt);

        expected_flow_get_fill_rsp(flow, rsp->add_response());
        return false;
    };

    expected_flow_ht()->walk_safe(walk_func, (void *)res);     
    return HAL_RET_OK;
}

std::ostream& operator<<(std::ostream& os, const exp_flow_key_t val) {
    os << "{dir=" << val.dir;
    os << " ,svrf_id=" << val.svrf_id;
    os << " ,dvrf_id=" << val.dvrf_id;
    os << " ,sip=" << ipv4addr2str(val.sip);
    os << " ,dip=" << ipv4addr2str(val.dip);
    os << " ,proto=" << val.proto;
    os << " ,sport=" << val.sport;
    os << " ,dport=" << val.dport;
    return os << " }";
}

} // namespace alg_utils
} // namespace plugins
} // namespace hal
