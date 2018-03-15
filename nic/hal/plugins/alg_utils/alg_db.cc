#include "nic/include/periodic.hpp"
#include "alg_db.hpp"

namespace hal {
namespace plugins {
namespace alg_utils {

static void * expected_flow_get_key_func(void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((expected_flow_t *)entry)->key);
}

static uint32_t expected_flow_compute_hash_func(void *key, uint32_t ht_size)
{
    return (sdk::lib::hash_algo::fnv_hash(key, 
                                            sizeof(hal::flow_key_t)) % ht_size);
}

static bool expected_flow_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    return (memcmp(key1, key2, sizeof(hal::flow_key_t)) == 0);
}

//------------------------------------------------------------------------------
// Expected flow entries hash table
//------------------------------------------------------------------------------
static sdk::lib::ht *expected_flow_ht()
{
    static sdk::lib::ht* ht_ =
        sdk::lib::ht::factory(FTE_MAX_EXPECTED_FLOWS,
                                expected_flow_get_key_func,
                                expected_flow_compute_hash_func,
                                expected_flow_compare_key_func);
    
    return ht_;
}

//------------------------------------------------------------------------------
// Insert an expected flow
//------------------------------------------------------------------------------
hal_ret_t
insert_expected_flow(expected_flow_t *entry)
{
    HAL_TRACE_DEBUG("ALG::insert_expected_flow key={}", entry->key);
    ref_init(&entry->ref_count, NULL);
    entry->deleting = false;
    entry->expected_flow_ht_ctxt.reset();
    return hal_sdk_ret_to_hal_ret(expected_flow_ht()->
                                  insert(entry, &entry->expected_flow_ht_ctxt));
}

//------------------------------------------------------------------------------
// Remove an expected flow entry
//------------------------------------------------------------------------------
expected_flow_t *
remove_expected_flow(const hal::flow_key_t &key)
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
                          uint32_t time_intvl, sdk::lib::twheel_cb_t cb,
                          void *timer_ctxt)
{
    entry->timer = hal::periodic::timer_schedule(timer_id, time_intvl,
                                            (void *)timer_ctxt, cb, false);
    if (!entry->timer) {
        HAL_TRACE_ERR("Failed to start timer for expected flow with key: {}",
                      entry->key);
    }
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

    hal::flow_key_t key = ikey;

    // Exact match
    if ((entry = (expected_flow_t *)expected_flow_ht()->lookup((void *)&key))) {
        HAL_TRACE_DEBUG("ALG::lookup_expected_flow exact match key={}", key);
        goto end;
    }

    if (exact_match) {
        return NULL;
    }

    // wildcard matches are supported for tcp/udp only
    if ((key.flow_type != hal::FLOW_TYPE_V4 && key.flow_type != hal::FLOW_TYPE_V6) ||
        (key.proto != IPPROTO_TCP && key.proto != IPPROTO_UDP)) {
        return NULL;
    }

    // Reverse lookup for Exact match
    key.sport = ikey.dport;
    key.dport = ikey.sport;
    key.sip = ikey.dip;
    key.dip = ikey.sip;
    if ((entry = (expected_flow_t *)expected_flow_ht()->lookup((void *)&key))) {
        HAL_TRACE_DEBUG("ALG::lookup_expected_flow reverse exact match key={}", key);
        goto end;
    }

    key = ikey;

    // Mask SPORT and do lookup (tftp)
    key.sport = 0;
    if ((entry = (expected_flow_t *)expected_flow_ht()->lookup((void *)&key))) {
        HAL_TRACE_DEBUG("ALG::lookup_expected_flow widcard sport key={}", key);
        goto end;
    }

    //Mask DIR only and do lookup (Active FTP)
    key.dir = 0;
    key.sport = ikey.sport;
    if ((entry = (expected_flow_t *)expected_flow_ht()->lookup((void *)&key))) {
        HAL_TRACE_DEBUG("ALG::lookup_expected_flow wildcard dir key={}", key);
        goto end;
    }

    // Mask SIP, DIR, SPORT and do lookup (RPC/Passive FTP)
    key.sip = {};
    key.sport = 0;
    if ((entry = (expected_flow_t *)expected_flow_ht()->lookup((void *)&key))) {
        HAL_TRACE_DEBUG("ALG::lookup_expected_flow wildcard sip/sport/dir key={}", key);
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

} // namespace alg_utils
} // namespace plugins
} // namespace hal
