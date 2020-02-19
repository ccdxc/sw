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
session_clear_full(void)
{
    pds_flow_session_key_t key;
    sdk_ret_t   ret = SDK_RET_OK;

    TEST_LOG_INFO("Clearing %u session entries\n", session_table_depth);
    flow_session_key_init(&key);
    for (key.session_info_id = 0;
         key.session_info_id < session_table_depth;
         key.session_info_id++) {

        ret = pds_flow_session_info_delete(&key);
        if (!SESSION_DELETE_RET_VALIDATE(ret)) {
            break;
        }
    }
    return SESSION_DELETE_RET_VALIDATE(ret);
}

static bool
session_populate_simple(void)
{
    pds_flow_session_spec_t spec;
    uint32_t    id_table[] = {2, 5, 11, 20, 113, 91};
    uint32_t    i;
    sdk_ret_t   ret = SDK_RET_OK;

    flow_session_spec_init(&spec);
    for (i = 0; i < ARRAY_SIZE(id_table); i++) {
        spec.key.session_info_id = id_table[i];
        ret = pds_flow_session_info_create(&spec);
        if (!SESSION_CREATE_RET_VALIDATE(ret)) {
            break;
        }
    }

    create_count = i;
    TEST_LOG_INFO("Session entries created: %d\n", create_count);
    return SESSION_CREATE_RET_VALIDATE(ret);
}

static bool
session_populate_random_start_count(void)
{
    pds_flow_session_spec_t spec;
    uint32_t    start_idx;
    uint32_t    count;
    uint32_t    i;
    sdk_ret_t   ret = SDK_RET_OK;

    start_idx = randomize_max(session_table_depth - 1);
    count = randomize_max(session_table_depth - start_idx);

    flow_session_spec_init(&spec);
    for (i = 0; i < count; i++) {
        spec.key.session_info_id = start_idx++;
        ret = pds_flow_session_info_create(&spec);
        if (!SESSION_CREATE_RET_VALIDATE(ret)) {
            break;
        }
    }

    create_count = i;
    TEST_LOG_INFO("Session entries created: %u\n", create_count);
    return SESSION_CREATE_RET_VALIDATE(ret);
}

static bool
session_populate_full(void)
{
    pds_flow_session_spec_t spec;
    sdk_ret_t   ret = SDK_RET_OK;

    flow_session_spec_init(&spec);
    for (spec.key.session_info_id = 1;
         spec.key.session_info_id < session_table_depth;
         spec.key.session_info_id++) {

        ret = pds_flow_session_info_create(&spec);
        if (!SESSION_CREATE_RET_VALIDATE(ret)) {
            break;
        }
    }

    create_count = spec.key.session_info_id - 1;
    TEST_LOG_INFO("Session entries created: %u\n", create_count);
    return SESSION_CREATE_RET_VALIDATE(ret);
}

static void
session_aging_expiry_fn(uint32_t expiry_id,
                        pds_flow_age_expiry_type_t expiry_type)
{
    expiry_count++;
    (*aging_expiry_dflt_fn)(expiry_id, expiry_type);
}
                             
bool
session_aging_init(void *arg)
{
    sdk_ret_t   ret;

    // Start with init() in case that had never been done
    ret = ftl_pollers_client::init();

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
session_aging_expiry_log_enable(void *arg)
{
    ftl_pollers_client::expiry_log_set(true);
    return true;
}

bool
session_aging_expiry_log_disable(void *arg)
{
    ftl_pollers_client::expiry_log_set(false);
    return true;
}

bool
session_aging_sim_mode(void *arg)
{
    sdk_ret_t   ret;

    ret = ftl_pollers_client::force_session_expired_ts_set(true);
    return SESSION_RET_VALIDATE(ret);
}

bool
session_aging_hw_mode(void *arg)
{
    sdk_ret_t   ret;

    ret = ftl_pollers_client::force_session_expired_ts_set(false);
    return SESSION_RET_VALIDATE(ret);
}

bool
session_aging_fini(void *arg)
{
    sdk_ret_t   ret;

    ret = pds_flow_age_hw_scanners_stop(true);
    if (SESSION_RET_VALIDATE(ret)) {
        ret = pds_flow_age_sw_pollers_poll_control(false, NULL);
    }
    session_aging_hw_mode(nullptr);
    return SESSION_RET_VALIDATE(ret);
}


static void
session_aging_pollers_poll(void)
{
    for (uint32_t qid = 0; qid < pollers_qcount; qid++) {
        ftl_pollers_client::poll(qid);
    }
}

static bool
session_aging_expiry_count_check(void)
{
    session_aging_pollers_poll();
    return expiry_count >= create_count;
}

bool
session_aging_clear_full(void *arg)
{
    return session_clear_full();
}

bool
session_aging_simple(void *arg)
{
    test_timestamp_t    ts;
    bool                success;

    expiry_count = 0;
    success = session_populate_simple();
    if (success) {
        time_expiry_set(ts, APP_TIME_LIMIT_EXEC_SECS(30));
        time_limit_exec(ts, session_aging_expiry_count_check);
        success = session_aging_expiry_count_check();
    }
    return success;
}

bool
session_aging_random_start_count(void *arg)
{
    test_timestamp_t    ts;
    bool                success;

    expiry_count = 0;
    success = session_populate_random_start_count();
    if (success) {
        time_expiry_set(ts, APP_TIME_LIMIT_EXEC_SECS(30));
        time_limit_exec(ts, session_aging_expiry_count_check);
        success = session_aging_expiry_count_check();
    }
    return success;
}

bool
session_aging_full(void *arg)
{
    test_timestamp_t    ts;
    bool                success;

    expiry_count = 0;
    success = session_populate_full();
    if (success) {
        time_expiry_set(ts, APP_TIME_LIMIT_EXEC_SECS(30));
        time_limit_exec(ts, session_aging_expiry_count_check);
        success = session_aging_expiry_count_check();
    }
    return success;
}

#if 0
static void
aging_metrics_show(pds_flow_age_expiry_type_t expiry_type)
{
    ftl_dev_if::lif_attr_metrics_t metrics;

    switch (expiry_type) {

    case EXPIRY_TYPE_SESSION:
        SDK_ASSERT(ftl_pollers_client::session_scanners_metrics_get(&metrics) == SDK_RET_OK);
        break;

    case EXPIRY_TYPE_CONNTRACK:
        SDK_ASSERT(ftl_pollers_client::conntrack_scanners_metrics_get(&metrics) == SDK_RET_OK);
        break;

    default:
        return;
    }

    TEST_LOG_INFO("total_cb_cfg_discards   : %" PRIu64 "\n", metrics.scanners.total_cb_cfg_discards);
    TEST_LOG_INFO("total_scan_invocations  : %" PRIu64 "\n", metrics.scanners.total_scan_invocations);
    TEST_LOG_INFO("total_expired_entries   : %" PRIu64 "\n", metrics.scanners.total_expired_entries);
    TEST_LOG_INFO("min_range_elapsed_ns    : %" PRIu64 "\n", metrics.scanners.min_range_elapsed_ns);
    TEST_LOG_INFO("avg_min_range_elapsed_ns: %" PRIu64 "\n", metrics.scanners.avg_min_range_elapsed_ns);
    TEST_LOG_INFO("max_range_elapsed_ns    : %" PRIu64 "\n", metrics.scanners.max_range_elapsed_ns);
    TEST_LOG_INFO("avg_max_range_elapsed_ns: %" PRIu64 "\n", metrics.scanners.avg_max_range_elapsed_ns);
}
#endif

}    // namespace athena_app
}    // namespace test
