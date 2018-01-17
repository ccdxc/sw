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
    expected_flow_t *entry;

    hal::flow_key_t key = ikey;

    // Exact match
    if ((entry = (expected_flow_t *)expected_flow_ht()->lookup((void *)&key))) {
        HAL_TRACE_DEBUG("ALG::lookup_expected_flow exact match key={}", key);
        return entry;
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
        return entry;
    }

    key = ikey;

    // Mask SPORT and do lookup (tftp)
    key.sport = 0;
    if ((entry = (expected_flow_t *)expected_flow_ht()->lookup((void *)&key))) {
        HAL_TRACE_DEBUG("ALG::lookup_expected_flow widcard sport key={}", key);
        return entry;
    }

    // Mask SIP, DIR and do lookup (RPC)
    key.sip = {};
    key.dir = 0;
    if ((entry = (expected_flow_t *)expected_flow_ht()->lookup((void *)&key))) {
        HAL_TRACE_DEBUG("ALG::lookup_expected_flow wildcard sip/sport/dir key={}", key);
        return entry;
    }

    return NULL;
}

} // namespace alg_utils
} // namespace plugins
} // namespace hal
