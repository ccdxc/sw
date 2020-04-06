//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains all conntrack aging test cases for athena
///
//----------------------------------------------------------------------------
#include "conntrack_aging.hpp"
#include "session_aging.hpp"
#include "nic/apollo/api/include/athena/pds_conntrack.h"
#include "nic/apollo/api/include/athena/pds_flow_age.h"
#include "nic/apollo/api/impl/athena/ftl_pollers_client.hpp"
#include "nic/apollo/p4/include/athena_defines.h"

namespace test {
namespace athena_app {

#define CONNTRACK_RET_VALIDATE(ret)         \
   ((ret) == PDS_RET_OK)

#define CONNTRACK_CREATE_RET_VALIDATE(ret)  \
   (((ret) == PDS_RET_OK) || ((ret) == PDS_RET_ENTRY_EXISTS))

#define CONNTRACK_DELETE_RET_VALIDATE(ret)  \
   (((ret) == PDS_RET_OK) || ((ret) == PDS_RET_ENTRY_NOT_FOUND))

static uint32_t             pollers_qcount;
static pds_flow_expiry_fn_t aging_expiry_dflt_fn;

/*
 * Global tolerance for use across multiple linked tests
 */
static aging_tolerance_t    glb_tolerance;
static aging_metrics_t      conntrack_metrics(ftl_dev_if::FTL_QTYPE_SCANNER_CONNTRACK);

static uint32_t
conntrack_table_depth(void)
{
    static uint32_t table_depth;

    if (!table_depth) {
        table_depth = std::min(ftl_pollers_client::conntrack_table_depth_get(),
                                           (uint32_t)PDS_CONNTRACK_ID_MAX);
    }
    return table_depth;
}

static inline void
conntrack_spec_init(pds_conntrack_spec_t *spec)
{
    memset(spec, 0, sizeof(*spec));
}

bool
conntrack_table_clear_full(test_vparam_ref_t vparam)
{
    pds_conntrack_key_t key;
    uint32_t    depth;
    pds_ret_t   ret = PDS_RET_OK;

    depth = vparam.expected_num(conntrack_table_depth());
    depth = std::min(depth, conntrack_table_depth());

    flow_conntrack_key_init(&key);
    for (key.conntrack_id = 1;
         key.conntrack_id < depth;
         key.conntrack_id++) {

        ret = pds_conntrack_state_delete(&key);
        if (!CONNTRACK_DELETE_RET_VALIDATE(ret)) {
            break;
        }
    }
    TEST_LOG_INFO("Cleared %u conntrack entries\n", key.conntrack_id);
    return CONNTRACK_DELETE_RET_VALIDATE(ret);
}

bool
conntrack_aging_tolerance_secs_set(test_vparam_ref_t vparam)
{
    uint32_t secs = vparam.expected_num();
    glb_tolerance.tolerance_secs_set(secs);
    return true;
}

bool
conntrack_populate_simple(test_vparam_ref_t vparam)
{
    pds_conntrack_spec_t    spec;
    tuple_eval_t            tuple_eval;
    pds_ret_t               ret = PDS_RET_OK;

    conntrack_metrics.baseline();
    conntrack_spec_init(&spec);

    glb_tolerance.reset(vparam.size());
    for (uint32_t i = 0; i < vparam.size(); i++) {

        /*
         * Here we expect tuple of the form {id flowtype flowstate}
         */
        tuple_eval.reset(vparam, i);
        spec.key.conntrack_id = tuple_eval.num(0);
        spec.data.flow_type = tuple_eval.flowtype(1);
        spec.data.flow_state = tuple_eval.flowstate(2);
        if (!tuple_eval.zero_failures()) {
            break;
        }

        ret = pds_conntrack_state_create(&spec);
        if (!CONNTRACK_CREATE_RET_VALIDATE(ret)) {
            break;
        }
        glb_tolerance.create_id_map_insert(spec.key.conntrack_id);
    }

    TEST_LOG_INFO("Conntrack entries created: %d\n",
                  glb_tolerance.create_id_map_size());
    return CONNTRACK_CREATE_RET_VALIDATE(ret) && tuple_eval.zero_failures();
}

bool
conntrack_populate_random(test_vparam_ref_t vparam)
{
    pds_conntrack_spec_t    spec;
    tuple_eval_t            tuple_eval;
    uint32_t                start_idx;
    uint32_t                count = 0;
    bool                    randomize_typestate;
    pds_ret_t               ret = PDS_RET_OK;

    conntrack_metrics.baseline();

    /*
     * 3 possible different sets of parameters:
     *
     * - no params: generate random {start count} and random {flowtype flowstate}
     * - 1 tuple  : use specified {start count} and generate random {flowtype flowstate}
     * - 2 tuples : use specified {start count} and specified {flowtype flowstate}
     */
    conntrack_spec_init(&spec);
    randomize_typestate = true;
    switch (vparam.size()) {

    case 0:
        start_idx = randomize_max(conntrack_table_depth() - 1);
        count = randomize_max(conntrack_table_depth() - start_idx);
        break;

    case 1:
    case 2:
        tuple_eval.reset(vparam, 0);
        start_idx = min(tuple_eval.num(0), conntrack_table_depth() - 1);
        count = min(tuple_eval.num(1), conntrack_table_depth() - start_idx);

        if (!tuple_eval.zero_failures() || (vparam.size() < 2)) {
            break;
        }

        randomize_typestate = false;
        tuple_eval.reset(vparam, 1);
        spec.data.flow_type = tuple_eval.flowtype(0);
        spec.data.flow_state = tuple_eval.flowstate(1);
        break;

    default:
        TEST_LOG_ERR("Too many tuples specified, only a max of 2 needed\n");
        ret = PDS_RET_INVALID_ARG;
        break;
    }

    glb_tolerance.reset(count);
    if (tuple_eval.zero_failures() && CONNTRACK_RET_VALIDATE(ret)) {
        TEST_LOG_INFO("start_idx: %u count: %u\n", start_idx, count);
        for (uint32_t i = 0; i < count; i++) {
            spec.key.conntrack_id = start_idx++;

            /*
             * Randomize flowtype and flowstate if needed
             */
            if (randomize_typestate) {
                spec.data.flow_type = (pds_flow_type_t)
                     randomize_max((uint32_t)PDS_FLOW_TYPE_OTHERS, true);
                /*
                 * Exclude REMOVED state as it would not be aged
                 */
                spec.data.flow_state = (pds_flow_state_t)
                     randomize_max((uint32_t)RST_CLOSE, true);
            }

            ret = pds_conntrack_state_create(&spec);
            if (!CONNTRACK_CREATE_RET_VALIDATE(ret)) {
                break;
            }
            if (spec.data.flow_state != REMOVED) {
                glb_tolerance.create_id_map_insert(spec.key.conntrack_id);
            }
        }
    }

    TEST_LOG_INFO("Conntrack entries created: %u\n",
                  glb_tolerance.create_id_map_size());
    return CONNTRACK_CREATE_RET_VALIDATE(ret) && tuple_eval.zero_failures();
}

bool
conntrack_populate_full(test_vparam_ref_t vparam)
{
    pds_conntrack_spec_t    spec;
    tuple_eval_t            tuple_eval;
    uint32_t                depth;
    pds_ret_t               ret = PDS_RET_OK;

    conntrack_metrics.baseline();

    /*
     * Here we expect tuple of the form {flowtype flowstate depth}
     */
    tuple_eval.reset(vparam, 0);
    spec.data.flow_type = tuple_eval.flowtype(0);
    spec.data.flow_state = tuple_eval.flowstate(1);
    depth = tuple_eval.num(2);

    glb_tolerance.reset(depth);
    if (tuple_eval.zero_failures()) {
        conntrack_spec_init(&spec);
        depth = std::min(depth, conntrack_table_depth());

        for (spec.key.conntrack_id = 1;
             spec.key.conntrack_id < depth;
             spec.key.conntrack_id++) {

            ret = pds_conntrack_state_create(&spec);
            if (!CONNTRACK_CREATE_RET_VALIDATE(ret)) {
                break;
            }
            glb_tolerance.create_id_map_insert(spec.key.conntrack_id);
        }
    }

    TEST_LOG_INFO("Conntrack entries created: %u\n",
                  glb_tolerance.create_id_map_size());
    return CONNTRACK_CREATE_RET_VALIDATE(ret) && tuple_eval.zero_failures();
}

pds_ret_t
conntrack_aging_expiry_fn(uint32_t expiry_id,
                        pds_flow_age_expiry_type_t expiry_type,
                        void *user_ctx)
{
    pds_ret_t   ret = PDS_RET_OK;;

    switch (expiry_type) {

    case EXPIRY_TYPE_CONNTRACK:
        if (aging_expiry_dflt_fn) {
            glb_tolerance.expiry_count_inc();
            glb_tolerance.conntrack_tmo_tolerance_check(expiry_id);
            glb_tolerance.create_id_map_find_erase(expiry_id);
            ret = (*aging_expiry_dflt_fn)(expiry_id, expiry_type, user_ctx);
        }
        break;

    case EXPIRY_TYPE_SESSION:
        ret = (pds_ret_t)session_aging_expiry_fn(expiry_id, expiry_type, user_ctx);
        break;

    default:
        ret = PDS_RET_INVALID_ARG;
        break;
    }
    return ret;
}
                             
bool
conntrack_aging_init(test_vparam_ref_t vparam)
{
    pds_ret_t   ret;

    // Start with init() in case that had never been done
    ret = pds_flow_age_init();

    // On SIM platform, the following needs to be set early
    // before scanners are started to prevent lockup in scanners
    // due to the lack of true LIF timers in SIM.
    if (!hw() && CONNTRACK_RET_VALIDATE(ret)) {
        ret = (pds_ret_t)ftl_pollers_client::force_conntrack_expired_ts_set(true);
    }
    if (CONNTRACK_RET_VALIDATE(ret)) {
        ret = pds_flow_age_sw_pollers_qcount(&pollers_qcount);
    }
    if (CONNTRACK_RET_VALIDATE(ret)) {
        ret = pds_flow_age_sw_pollers_expiry_fn_dflt(&aging_expiry_dflt_fn);
    }
    if (CONNTRACK_RET_VALIDATE(ret)) {

        // Here we don't want to assume that some existing threads are already
        // doing polling on our behalf. Hence, we indicate intention of self polling. 

        ret = pds_flow_age_sw_pollers_poll_control(true, conntrack_aging_expiry_fn);
    }
    if (CONNTRACK_RET_VALIDATE(ret)) {
        ret = pds_flow_age_hw_scanners_start();
    }
    return CONNTRACK_RET_VALIDATE(ret) && pollers_qcount && 
           conntrack_table_depth() && aging_expiry_dflt_fn;
}

bool
conntrack_aging_expiry_log_set(test_vparam_ref_t vparam)
{
    ftl_pollers_client::expiry_log_set(vparam.expected_bool());
    return true;
}

bool
conntrack_aging_force_expired_ts(test_vparam_ref_t vparam)
{
    sdk_ret_t   ret;

    ret = ftl_pollers_client::force_conntrack_expired_ts_set(vparam.expected_bool());
    return CONNTRACK_RET_VALIDATE((pds_ret_t)ret);
}

bool
conntrack_aging_fini(test_vparam_ref_t vparam)
{
    test_vparam_t   sim_vparam;
    pds_ret_t       ret;

    ret = pds_flow_age_hw_scanners_stop(true);
    if (CONNTRACK_RET_VALIDATE(ret)) {
        ret = pds_flow_age_sw_pollers_poll_control(false, NULL);
    }


    sim_vparam.push_back(test_param_t((uint32_t)false));
    conntrack_aging_force_expired_ts(sim_vparam);
    return CONNTRACK_RET_VALIDATE(ret);
}


static void
conntrack_aging_pollers_poll(void *user_ctx)
{
    for (uint32_t qid = 0; qid < pollers_qcount; qid++) {
        pds_flow_age_sw_pollers_poll(qid, user_ctx);
    }
}

static bool
conntrack_aging_expiry_count_check(void *user_ctx)
{
    conntrack_aging_pollers_poll(user_ctx);
    return glb_tolerance.expiry_count() >= glb_tolerance.create_count();
}

bool
conntrack_4combined_expiry_count_check(bool poll_needed)
{
    if (poll_needed) {
        conntrack_aging_pollers_poll((void *)&glb_tolerance);
    }
    return glb_tolerance.expiry_count() >= glb_tolerance.create_count();
}

bool
conntrack_4combined_result_check(void)
{
    TEST_LOG_INFO("Conntrack entries aged out: %u\n",
                  glb_tolerance.expiry_count());
    glb_tolerance.create_id_map_empty_check();
    conntrack_metrics.expiry_count_check(glb_tolerance.expiry_count());
    return conntrack_4combined_expiry_count_check() &&
           glb_tolerance.zero_failures();
}

bool
conntrack_aging_test(test_vparam_ref_t vparam)
{
    test_timestamp_t    ts;

    ts.time_expiry_set(APP_TIME_LIMIT_EXEC_SECS(APP_TIME_LIMIT_EXEC_DFLT));
    ts.time_limit_exec(conntrack_aging_expiry_count_check, nullptr, 0);
    return conntrack_4combined_result_check();
}

bool
conntrack_aging_normal_tmo_set(test_vparam_ref_t vparam)
{
    tuple_eval_t        tuple_eval;
    pds_flow_type_t     flowtype;
    pds_flow_state_t    flowstate;
    uint32_t            tmo_val;
    pds_ret_t           ret = PDS_RET_OK;

    glb_tolerance.reset();

    /*
     * Here we expect tuples of the form {flowtype flowstate tmo_val}
     */
    for (uint32_t i = 0; i < vparam.size(); i++) {

        /*
         * Here we expect tuple of the form {flowtype flowstate tmo_val}
         */
        tuple_eval.reset(vparam, i);
        flowtype = tuple_eval.flowtype(0);
        flowstate = tuple_eval.flowstate(1);
        tmo_val = tuple_eval.num(2);
        if (!tuple_eval.zero_failures()) {
            break;
        }
        glb_tolerance.normal_tmo.conntrack_tmo_set(flowtype, flowstate, tmo_val);
    }
    return CONNTRACK_RET_VALIDATE(ret) && tuple_eval.zero_failures() &&
           glb_tolerance.zero_failures();
}

bool
conntrack_aging_accel_tmo_set(test_vparam_ref_t vparam)
{
    tuple_eval_t        tuple_eval;
    pds_flow_type_t     flowtype;
    pds_flow_state_t    flowstate;
    uint32_t            tmo_val;
    pds_ret_t           ret = PDS_RET_OK;

    glb_tolerance.reset();

    /*
     * Here we expect tuples of the form {flowtype flowstate tmo_val}
     */
    for (uint32_t i = 0; i < vparam.size(); i++) {

        /*
         * Here we expect tuple of the form {flowtype flowstate tmo_val}
         */
        tuple_eval.reset(vparam, i);
        flowtype = tuple_eval.flowtype(0);
        flowstate = tuple_eval.flowstate(1);
        tmo_val = tuple_eval.num(2);
        if (!tuple_eval.zero_failures()) {
            break;
        }
        glb_tolerance.accel_tmo.conntrack_tmo_set(flowtype, flowstate, tmo_val);
    }
    return CONNTRACK_RET_VALIDATE(ret) && tuple_eval.zero_failures() &&
           glb_tolerance.zero_failures();
}

bool
conntrack_aging_accel_control(test_vparam_ref_t vparam)
{
    glb_tolerance.reset();
    glb_tolerance.age_accel_control(vparam.expected_bool());
    return glb_tolerance.zero_failures();
}

bool
conntrack_aging_metrics_show(test_vparam_ref_t vparam)
{
    aging_metrics_t scanner_metrics(ftl_dev_if::FTL_QTYPE_SCANNER_CONNTRACK);
    aging_metrics_t poller_metrics(ftl_dev_if::FTL_QTYPE_POLLER);

    scanner_metrics.show();
    poller_metrics.show();
    return true;
}


}    // namespace athena_app
}    // namespace test
