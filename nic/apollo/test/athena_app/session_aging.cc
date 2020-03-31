//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains all session aging test cases for athena
///
//----------------------------------------------------------------------------
#include "session_aging.hpp"
#include "conntrack_aging.hpp"
#include "nic/apollo/api/include/athena/pds_flow_session_info.h"
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
static pds_flow_expiry_fn_t aging_expiry_dflt_fn;

/*
 * Global tolerance for use across multiple linked tests
 */
static aging_tolerance_t    glb_tolerance;
static aging_metrics_t      session_metrics(ftl_dev_if::FTL_QTYPE_SCANNER_SESSION);

static uint32_t
session_table_depth(void)
{
    static uint32_t table_depth;

    if (!table_depth) {
        table_depth = std::min(ftl_pollers_client::session_table_depth_get(),
                                   (uint32_t)PDS_FLOW_SESSION_INFO_ID_MAX);
    }
    return table_depth;
}

static void
flow_session_spec_init(pds_flow_session_spec_t *spec)
{
    memset(spec, 0, sizeof(*spec));
    flow_session_key_init(&spec->key);
    spec->data.conntrack_id = 1;
}

bool
session_table_clear_full(test_vparam_ref_t vparam)
{
    pds_flow_session_key_t key;
    uint32_t    depth;
    sdk_ret_t   ret = SDK_RET_OK;

    depth = vparam.expected_num(session_table_depth());
    depth = std::min(depth, session_table_depth());

    flow_session_key_init(&key);
    for (key.session_info_id = 1;
         key.session_info_id < depth;
         key.session_info_id++) {

        ret = pds_flow_session_info_delete(&key);
        if (!SESSION_DELETE_RET_VALIDATE(ret)) {
            break;
        }
    }
    TEST_LOG_INFO("Cleared %u session entries\n", key.session_info_id);
    return SESSION_DELETE_RET_VALIDATE(ret);
}

bool
session_aging_tolerance_secs_set(test_vparam_ref_t vparam)
{
    uint32_t secs = vparam.expected_num();
    glb_tolerance.tolerance_secs_set(secs);
    return true;
}

bool
session_populate_simple(test_vparam_ref_t vparam)
{
    pds_flow_session_spec_t spec;
    sdk_ret_t   ret = SDK_RET_OK;

    session_metrics.baseline();
    flow_session_spec_init(&spec);

    glb_tolerance.reset(vparam.size());
    for (uint32_t i = 0; i < vparam.size(); i++) {
        ret = vparam.num(i, &spec.key.session_info_id);
        if (!SESSION_RET_VALIDATE(ret)) {
            break;
        }
        ret = pds_flow_session_info_create(&spec);
        if (!SESSION_CREATE_RET_VALIDATE(ret)) {
            break;
        }
        glb_tolerance.create_id_map_insert(spec.key.session_info_id);
    }

    TEST_LOG_INFO("Session entries created: %d\n",
                  glb_tolerance.create_id_map_size());
    return SESSION_CREATE_RET_VALIDATE(ret);
}

bool
session_populate_random(test_vparam_ref_t vparam)
{
    pds_flow_session_spec_t spec;
    uint32_t    start_idx;
    uint32_t    count  = 0;
    sdk_ret_t   ret = SDK_RET_OK;

    session_metrics.baseline();

    /*
     * Generate random start_idx and count, unless overidden by vparam
     */
    if (vparam.size()) {
        ret = vparam.num(0, &start_idx);
        start_idx = min(start_idx, session_table_depth() - 1);
        if (vparam.size() > 1) {
            ret = vparam.num(1, &count);
            count = min(count, session_table_depth() - start_idx);
        }
    } else {
        start_idx = randomize_max(session_table_depth() - 1);
        count = randomize_max(session_table_depth() - start_idx);
    }

    glb_tolerance.reset(count);
    if (SESSION_RET_VALIDATE(ret)) {
        TEST_LOG_INFO("start_idx: %u count: %u\n", start_idx, count);
        flow_session_spec_init(&spec);
        for (uint32_t i = 0; i < count; i++) {
            spec.key.session_info_id = start_idx++;
            ret = pds_flow_session_info_create(&spec);
            if (!SESSION_CREATE_RET_VALIDATE(ret)) {
                break;
            }
            glb_tolerance.create_id_map_insert(spec.key.session_info_id);
        }
    }

    TEST_LOG_INFO("Session entries created: %u\n",
                  glb_tolerance.create_id_map_size());
    return SESSION_CREATE_RET_VALIDATE(ret);
}

bool
session_populate_full(test_vparam_ref_t vparam)
{
    pds_flow_session_spec_t spec;
    uint32_t    depth;
    sdk_ret_t   ret = SDK_RET_OK;

    session_metrics.baseline();
    depth = vparam.expected_num(session_table_depth());
    depth = std::min(depth, session_table_depth());

    glb_tolerance.reset(depth);
    flow_session_spec_init(&spec);
    for (spec.key.session_info_id = 1;
         spec.key.session_info_id < depth;
         spec.key.session_info_id++) {

        ret = pds_flow_session_info_create(&spec);
        if (!SESSION_CREATE_RET_VALIDATE(ret)) {
            break;
        }
        glb_tolerance.create_id_map_insert(spec.key.session_info_id);
    }

    TEST_LOG_INFO("Session entries created: %u\n",
                  glb_tolerance.create_id_map_size());
    return SESSION_CREATE_RET_VALIDATE(ret);
}

sdk_ret_t
session_aging_expiry_fn(uint32_t expiry_id,
                        pds_flow_age_expiry_type_t expiry_type,
                        void *user_ctx)
{
    sdk_ret_t ret = SDK_RET_OK;

    switch (expiry_type) {

    case EXPIRY_TYPE_SESSION:
        if (aging_expiry_dflt_fn) {
            glb_tolerance.expiry_count_inc();
            glb_tolerance.session_tmo_tolerance_check(expiry_id);
            glb_tolerance.create_id_map_find_erase(expiry_id);
            ret = (*aging_expiry_dflt_fn)(expiry_id, expiry_type, user_ctx);
        }
        break;

    case EXPIRY_TYPE_CONNTRACK:
        ret = conntrack_aging_expiry_fn(expiry_id, expiry_type, user_ctx);
        break;

    default:
        ret = SDK_RET_INVALID_ARG;
        break;
    }
    return ret;
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
    return SESSION_RET_VALIDATE(ret) && pollers_qcount && 
           session_table_depth() && aging_expiry_dflt_fn;
}

bool
session_aging_expiry_log_set(test_vparam_ref_t vparam)
{
    ftl_pollers_client::expiry_log_set(vparam.expected_bool());
    return true;
}

bool
session_aging_force_expired_ts(test_vparam_ref_t vparam)
{
    sdk_ret_t   ret;

    ret = ftl_pollers_client::force_session_expired_ts_set(
                                            vparam.expected_bool());
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
    session_aging_force_expired_ts(sim_vparam);
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
    return glb_tolerance.expiry_count() >= glb_tolerance.create_count();
}

bool
session_4combined_expiry_count_check(bool poll_needed)
{
    if (poll_needed) {
        session_aging_pollers_poll((void *)&glb_tolerance);
    }
    return glb_tolerance.expiry_count() >= glb_tolerance.create_count();
}

bool
session_4combined_result_check(void)
{
    TEST_LOG_INFO("Session entries aged out: %u\n",
                  glb_tolerance.expiry_count());
    glb_tolerance.create_id_map_empty_check();
    session_metrics.expiry_count_check(glb_tolerance.expiry_count());
    return session_4combined_expiry_count_check() &&
           glb_tolerance.zero_failures();
}

bool
session_aging_test(test_vparam_ref_t vparam)
{
    test_timestamp_t    ts;

    ts.time_expiry_set(APP_TIME_LIMIT_EXEC_SECS(APP_TIME_LIMIT_EXEC_DFLT));
    ts.time_limit_exec(session_aging_expiry_count_check, nullptr, 0);
    return session_4combined_result_check();
}

bool
session_aging_normal_tmo_set(test_vparam_ref_t vparam)
{
    glb_tolerance.reset();
    glb_tolerance.normal_tmo.session_tmo_set(vparam.expected_num());
    return glb_tolerance.zero_failures();
}

bool
session_aging_accel_tmo_set(test_vparam_ref_t vparam)
{
    glb_tolerance.reset();
    glb_tolerance.accel_tmo.session_tmo_set(vparam.expected_num());
    return glb_tolerance.zero_failures();
}

bool
session_aging_accel_control(test_vparam_ref_t vparam)
{
    glb_tolerance.reset();
    glb_tolerance.age_accel_control(vparam.expected_bool());
    return glb_tolerance.zero_failures();
}

bool
session_aging_metrics_show(test_vparam_ref_t vparam)
{
    aging_metrics_t scanner_metrics(ftl_dev_if::FTL_QTYPE_SCANNER_SESSION);
    aging_metrics_t poller_metrics(ftl_dev_if::FTL_QTYPE_POLLER);

    scanner_metrics.show();
    poller_metrics.show();
    return true;
}

}    // namespace athena_app
}    // namespace test
