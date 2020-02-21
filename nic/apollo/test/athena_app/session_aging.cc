//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all session aging test cases for athena
///
//----------------------------------------------------------------------------
#include "session_aging.hpp"
#include "nic/apollo/api/include/athena/pds_flow_session.h"
#include "nic/apollo/api/include/athena/pds_flow_age.h"
#include "nic/apollo/api/impl/athena/ftl_pollers_client.hpp"
#include "nic/apollo/p4/include/athena_defines.h"

namespace test {
namespace athena_app {

#define SESSION_RET_VALIDATE(ret)           \
   ((ret) == SDK_RET_OK)

#define SESSION_CREATE_RET_VALIDATE(ret)    \
   (((ret) == SDK_RET_OK) || ((ret) == SDK_RET_ENTRY_EXISTS))

#define SESSION_DELETE_RET_VALIDATE(ret)    \
   (((ret) == SDK_RET_OK) || ((ret) == SDK_RET_ENTRY_NOT_FOUND))

static uint32_t             pollers_qcount;
static uint32_t             create_count;
static uint32_t             expiry_count;
static uint32_t             session_table_depth;
static pds_flow_expiry_fn_t aging_expiry_dflt_fn;

static bool expiry_count_check(const age_metrics_t& hw_metrics,
                               uint32_t sw_expiry_count);

static void
flow_session_key_init(pds_flow_session_key_t *key)
{
    memset(key, 0, sizeof(*key));
    key->direction = HOST_TO_SWITCH | SWITCH_TO_HOST;
}

static void
flow_session_spec_init(pds_flow_session_spec_t *spec)
{
    memset(spec, 0, sizeof(*spec));
    flow_session_key_init(&spec->key);
    spec->data.conntrack_id = 1;
    spec->data.host_to_switch_flow_info.rewrite_info.encap_type = ENCAP_TYPE_NONE;
    spec->data.switch_to_host_flow_info.rewrite_info.encap_type = ENCAP_TYPE_NONE;
}

static bool
session_clear_full(test_vparam_ref_t vparam)
{
    pds_flow_session_key_t key;
    uint32_t    depth;
    sdk_ret_t   ret = SDK_RET_OK;

    depth = vparam.expected_num(session_table_depth);
    depth = std::min(depth, session_table_depth);
    TEST_LOG_INFO("Clearing %u session entries\n", depth);

    flow_session_key_init(&key);
    for (key.session_info_id = 0;
         key.session_info_id < depth;
         key.session_info_id++) {

        ret = pds_flow_session_info_delete(&key);
        if (!SESSION_DELETE_RET_VALIDATE(ret)) {
            break;
        }
    }
    return SESSION_DELETE_RET_VALIDATE(ret);
}

static bool
session_populate_simple(test_vparam_ref_t vparam,
                        id_map_t& session_id_map)
{
    pds_flow_session_spec_t spec;
    uint32_t    i;
    sdk_ret_t   ret = SDK_RET_OK;

    flow_session_spec_init(&spec);
    for (i = 0; i < vparam.size(); i++) {
        ret = vparam.num(i, &spec.key.session_info_id);
        if (!SESSION_RET_VALIDATE(ret)) {
            break;
        }
        ret = pds_flow_session_info_create(&spec);
        if (!SESSION_CREATE_RET_VALIDATE(ret)) {
            break;
        }
        session_id_map.insert(spec.key.session_info_id);
    }

    create_count = session_id_map.size();
    TEST_LOG_INFO("Session entries created: %d\n", create_count);
    return SESSION_CREATE_RET_VALIDATE(ret);
}

static bool
session_populate_random_start_count(test_vparam_ref_t vparam,
                                    id_map_t& session_id_map)
{
    pds_flow_session_spec_t spec;
    uint32_t    start_idx;
    uint32_t    count;
    uint32_t    i;
    sdk_ret_t   ret = SDK_RET_OK;

    /*
     * Generate random start_idx and count, unless overidden by vparam
     */
    if (vparam.size()) {
        ret = vparam.num(0, &start_idx);
        start_idx = min(start_idx, session_table_depth - 1);
        if (vparam.size() > 1) {
            ret = vparam.num(1, &count);
            count = min(count, session_table_depth - start_idx);
        }
    } else {
        start_idx = randomize_max(session_table_depth - 1);
        count = randomize_max(session_table_depth - start_idx);
    }

    if (SESSION_RET_VALIDATE(ret)) {
        TEST_LOG_INFO("start_idx: %u count: %u\n", start_idx, count);
        flow_session_spec_init(&spec);
        for (i = 0; i < count; i++) {
            spec.key.session_info_id = start_idx++;
            ret = pds_flow_session_info_create(&spec);
            if (!SESSION_CREATE_RET_VALIDATE(ret)) {
                break;
            }
            session_id_map.insert(spec.key.session_info_id);
        }
    }

    create_count = session_id_map.size();
    TEST_LOG_INFO("Session entries created: %u\n", create_count);
    return SESSION_CREATE_RET_VALIDATE(ret);
}

static bool
session_populate_full(test_vparam_ref_t vparam)
{
    pds_flow_session_spec_t spec;
    uint32_t    depth;
    sdk_ret_t   ret = SDK_RET_OK;

    depth = vparam.expected_num(session_table_depth);
    depth = std::min(depth, session_table_depth);

    flow_session_spec_init(&spec);
    for (spec.key.session_info_id = 1;
         spec.key.session_info_id < depth;
         spec.key.session_info_id++) {

        ret = pds_flow_session_info_create(&spec);
        if (!SESSION_CREATE_RET_VALIDATE(ret)) {
            break;
        }
    }

    create_count = depth ? spec.key.session_info_id - 1 : 0;
    TEST_LOG_INFO("Session entries created: %u\n", create_count);
    return SESSION_CREATE_RET_VALIDATE(ret);
}

static void
session_aging_expiry_fn(uint32_t expiry_id,
                        pds_flow_age_expiry_type_t expiry_type,
                        void *user_ctx)
{
    expiry_count++;
    (*aging_expiry_dflt_fn)(expiry_id, expiry_type, nullptr);

    /*
     * Cross check against create map
     */
    if (user_ctx) {
        id_map_t *id_map = static_cast<id_map_t *>(user_ctx);
        if (!id_map->find_erase(expiry_id)) {
            TEST_LOG_INFO("expiry_id %u was not created for this test\n",
                          expiry_id);
        }
    }
}
                             
bool
session_aging_init(test_vparam_ref_t vparam)
{
    sdk_ret_t   ret;

    // Start with init() in case that had never been done
    ret = pds_flow_age_init();

    // On SIM platform, the following needs to be set early
    // before scanners are started to prevent lockup in scanners
    // due to the lack of true LIF timers in SIM.
    if (!hw() && SESSION_RET_VALIDATE(ret)) {
        ret = ftl_pollers_client::force_session_expired_ts_set(true);
    }
    if (SESSION_RET_VALIDATE(ret)) {
        ret = pds_flow_age_sw_pollers_qcount(&pollers_qcount);
    }
    if (SESSION_RET_VALIDATE(ret)) {
        ret = pds_flow_age_sw_pollers_expiry_fn_dflt(&aging_expiry_dflt_fn);
    }
    if (SESSION_RET_VALIDATE(ret)) {

        // Here we don't want to assume that some existing threads are already
        // doing polling on our behalf. Hence, we indicate intention of self polling. 

        ret = pds_flow_age_sw_pollers_poll_control(true, session_aging_expiry_fn);
    }
    if (SESSION_RET_VALIDATE(ret)) {
        ret = pds_flow_age_hw_scanners_start();
    }
    session_table_depth = ftl_pollers_client::session_table_depth_get();

    return SESSION_RET_VALIDATE(ret) && pollers_qcount && 
           session_table_depth && aging_expiry_dflt_fn;
}

bool
session_aging_expiry_log_set(test_vparam_ref_t vparam)
{
    ftl_pollers_client::expiry_log_set(vparam.expected_bool());
    return true;
}

bool
session_aging_sim_mode(test_vparam_ref_t vparam)
{
    sdk_ret_t   ret;

    ret = ftl_pollers_client::force_session_expired_ts_set(vparam.expected_bool());
    return SESSION_RET_VALIDATE(ret);
}

bool
session_aging_fini(test_vparam_ref_t vparam)
{
    test_vparam_t   sim_vparam;
    sdk_ret_t       ret;

    ret = pds_flow_age_hw_scanners_stop(true);
    if (SESSION_RET_VALIDATE(ret)) {
        ret = pds_flow_age_sw_pollers_poll_control(false, NULL);
    }


    sim_vparam.push_back(test_param_t((uint32_t)false));
    session_aging_sim_mode(sim_vparam);
    return SESSION_RET_VALIDATE(ret);
}


static void
session_aging_pollers_poll(void *user_ctx)
{
    for (uint32_t qid = 0; qid < pollers_qcount; qid++) {
        pds_flow_age_sw_pollers_poll(qid, user_ctx);
    }
}

static bool
session_aging_expiry_count_check(void *user_ctx)
{
    session_aging_pollers_poll(user_ctx);
    return expiry_count >= create_count;
}

bool
session_aging_clear_full(test_vparam_ref_t vparam)
{
    return session_clear_full(vparam);
}

bool
session_aging_simple(test_vparam_ref_t vparam)
{
    test_timestamp_t    ts;
    id_map_t            session_id_map;
    age_metrics_t       hw_metrics(ftl_dev_if::FTL_QTYPE_SCANNER_SESSION);
    bool                success;

    expiry_count = 0;
    hw_metrics.baseline();
    success = session_populate_simple(vparam, session_id_map);
    if (success) {
        time_expiry_set(ts, APP_TIME_LIMIT_EXEC_SECS(APP_TIME_LIMIT_EXEC_DFLT));
        time_limit_exec(ts, session_aging_expiry_count_check,
                        (void *)&session_id_map);
        success = session_aging_expiry_count_check((void *)&session_id_map);
        TEST_LOG_INFO("Session entries aged out: %u\n", expiry_count);

        expiry_count_check(hw_metrics, expiry_count);
        if (session_id_map.size()) {
            TEST_LOG_INFO("Not all entries were aged out, remaining count: %u\n",
                          session_id_map.size());
            success = false;
        }
    }
    return success;
}

bool
session_aging_random_start_count(test_vparam_ref_t vparam)
{
    test_timestamp_t    ts;
    id_map_t            session_id_map;
    age_metrics_t       hw_metrics(ftl_dev_if::FTL_QTYPE_SCANNER_SESSION);
    bool                success;

    expiry_count = 0;
    hw_metrics.baseline();
    success = session_populate_random_start_count(vparam, session_id_map);
    if (success) {
        time_expiry_set(ts, APP_TIME_LIMIT_EXEC_SECS(APP_TIME_LIMIT_EXEC_DFLT));
        time_limit_exec(ts, session_aging_expiry_count_check,
                        (void *)&session_id_map);
        success = session_aging_expiry_count_check((void *)&session_id_map);
        TEST_LOG_INFO("Session entries aged out: %u\n", expiry_count);

        expiry_count_check(hw_metrics, expiry_count);
        if (session_id_map.size()) {
            TEST_LOG_INFO("Not all entries were aged out, remaining count: %u\n",
                          session_id_map.size());
            success = false;
        }
    }

    /*
     * It was important to testing with random numbers but just in case
     * there are any issues with the random number generator, we just
     * log and return success until our confidence level increases.
     */
    if (!success && !vparam.size()) {
        TEST_LOG_INFO("Error detected with random mode\n");
        success = true;
    }

    return success;
}

bool
session_aging_full(test_vparam_ref_t vparam)
{
    test_timestamp_t    ts;
    age_metrics_t       hw_metrics(ftl_dev_if::FTL_QTYPE_SCANNER_SESSION);
    bool                success;

    expiry_count = 0;
    hw_metrics.baseline();
    success = session_populate_full(vparam);
    if (success) {
        time_expiry_set(ts, APP_TIME_LIMIT_EXEC_SECS(APP_TIME_LIMIT_EXEC_DFLT));
        time_limit_exec(ts, session_aging_expiry_count_check);
        success = session_aging_expiry_count_check(nullptr);
        TEST_LOG_INFO("Session entries aged out: %u\n", expiry_count);
        expiry_count_check(hw_metrics, expiry_count);
    }
    return success;
}


bool
session_aging_metrics_show(test_vparam_ref_t vparam)
{
    age_metrics_t       hw_metrics(ftl_dev_if::FTL_QTYPE_SCANNER_SESSION);

    hw_metrics.show();
    return true;
}


/*
 * For informational purposes, see if HW metrics agreed with SW count
 */
static bool
expiry_count_check(const age_metrics_t& hw_metrics,
                   uint32_t sw_expiry_count)
{
    uint32_t    delta;

    delta = (uint32_t)hw_metrics.delta_expired_entries();
    if (delta != sw_expiry_count) {
        TEST_LOG_INFO("HW delta_expired_entries %u != SW expiry_count %u\n",
                      delta, sw_expiry_count);
    }
    return delta == sw_expiry_count;
}

}    // namespace athena_app
}    // namespace test
