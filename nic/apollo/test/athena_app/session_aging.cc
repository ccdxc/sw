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
#include "athena_test.hpp"
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/api/include/athena/pds_flow_session_info.h"
#include "nic/apollo/api/include/athena/pds_flow_age.h"
#include "nic/apollo/api/impl/athena/ftl_pollers_client.hpp"
#include "nic/apollo/p4/include/athena_defines.h"

namespace test {
namespace athena_app {

#define SESSION_RET_VALIDATE(ret)           \
   ((ret) == PDS_RET_OK)

#define SESSION_CREATE_RET_VALIDATE(ret)    \
   (((ret) == PDS_RET_OK) || ((ret) == PDS_RET_ENTRY_EXISTS))

#define SESSION_DELETE_RET_VALIDATE(ret)    \
   (((ret) == PDS_RET_OK) || ((ret) == PDS_RET_ENTRY_NOT_FOUND))

static uint32_t             pollers_qcount;
static pds_flow_expiry_fn_t aging_expiry_dflt_fn;

/*
 * Global tolerance for use across multiple linked tests
 */
static aging_tolerance_t    session_tolerance;
static aging_metrics_t      session_metrics(ftl_dev_if::FTL_QTYPE_SCANNER_SESSION);

const aging_tolerance_t&
session_tolerance_get(void)
{
    return session_tolerance;
}

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
    pds_ret_t   ret = PDS_RET_OK;

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
    session_tolerance.tolerance_secs_set(secs);
    return true;
}

bool
session_populate_simple(test_vparam_ref_t vparam)
{
    pds_flow_session_spec_t spec;
    pds_ret_t   ret = PDS_RET_OK;

    session_metrics.baseline();
    flow_session_spec_init(&spec);

    session_tolerance.reset(vparam.size());
    for (uint32_t i = 0; i < vparam.size(); i++) {
        ret = vparam.num(i, &spec.key.session_info_id);
        if (!SESSION_RET_VALIDATE(ret)) {
            break;
        }
        ret = pds_flow_session_info_create(&spec);
        if (!SESSION_CREATE_RET_VALIDATE(ret)) {
            break;
        }
        session_tolerance.create_id_map_insert(spec.key.session_info_id);
    }

    TEST_LOG_INFO("Session entries created: %d\n",
                  session_tolerance.create_id_map_size());
    return SESSION_CREATE_RET_VALIDATE(ret);
}

bool
session_populate_random(test_vparam_ref_t vparam)
{
    pds_flow_session_spec_t spec;
    uint32_t    start_idx;
    uint32_t    count  = 0;
    pds_ret_t   ret = PDS_RET_OK;

    session_metrics.baseline();

    /*
     * Generate random start_idx and count, unless overidden by vparam
     */
    if (vparam.size()) {
        ret = vparam.num(0, &start_idx);
        if (!SESSION_RET_VALIDATE(ret)) {
            return FALSE;
        }
        start_idx = min(start_idx, session_table_depth() - 1);
        if (vparam.size() > 1) {
            ret = vparam.num(1, &count);
            if (!SESSION_RET_VALIDATE(ret)) {
                return FALSE;
            }
            count = min(count, session_table_depth() - start_idx);
        }
    } else {
        start_idx = randomize_max(session_table_depth() - 1);
        count = randomize_max(session_table_depth() - start_idx);
    }

    session_tolerance.reset(count);
    if (SESSION_RET_VALIDATE(ret)) {
        TEST_LOG_INFO("start_idx: %u count: %u\n", start_idx, count);
        flow_session_spec_init(&spec);
        for (uint32_t i = 0; i < count; i++) {
            spec.key.session_info_id = start_idx++;
            ret = pds_flow_session_info_create(&spec);
            if (!SESSION_CREATE_RET_VALIDATE(ret)) {
                break;
            }
            session_tolerance.create_id_map_insert(spec.key.session_info_id);
        }
    }

    TEST_LOG_INFO("Session entries created: %u\n",
                  session_tolerance.create_id_map_size());
    return SESSION_CREATE_RET_VALIDATE(ret);
}

bool
session_populate_full(test_vparam_ref_t vparam)
{
    pds_flow_session_spec_t spec;
    uint32_t    depth;
    pds_ret_t   ret = PDS_RET_OK;

    session_metrics.baseline();
    depth = vparam.expected_num(session_table_depth());
    depth = std::min(depth, session_table_depth());

    session_tolerance.reset(depth);
    flow_session_spec_init(&spec);
    for (spec.key.session_info_id = 1;
         spec.key.session_info_id < depth;
         spec.key.session_info_id++) {

        ret = pds_flow_session_info_create(&spec);
        if (!SESSION_CREATE_RET_VALIDATE(ret)) {
            break;
        }
        session_tolerance.create_id_map_insert(spec.key.session_info_id);
    }

    TEST_LOG_INFO("Session entries created: %u\n",
                  session_tolerance.create_id_map_size());
    return SESSION_CREATE_RET_VALIDATE(ret);
}

bool
session_and_cache_populate(test_vparam_ref_t vparam)
{
    pds_flow_session_spec_t session_spec;
    tuple_eval_t            tuple_eval;
    std::string             field_type;
    uint64_t                ids_max;
    flow_key_field_t        vnic;
    flow_key_field_t        sip;
    flow_key_field_t        dip;
    flow_key_field_t        dport;
    flow_key_field_t        sport;
    uint32_t                value;
    uint32_t                count;
    uint32_t                proto = IPPROTO_NONE;
    pds_ret_t               ret = PDS_RET_OK;
    pds_ret_t               cache_ret = PDS_RET_OK;

    session_metrics.baseline();
    flow_session_spec_init(&session_spec);

    if (vparam.size() == 0) {
        TEST_LOG_ERR("A protocol type (UDP/TCP/ICMP) is required\n");
        return false;
    }
    for (uint32_t i = 0; i < vparam.size(); i++) {

        if (i == 0) {
            ret = vparam.proto(0, &proto);
            if (ret != PDS_RET_OK) {
                return false;
            }
            continue;
        }

        /*
         * Here we expect tuples of the form {type value [count]},
         * e.g., {sip 192.168.1.1 1000}. Note: count is optional (default 1)
         */
        tuple_eval.reset(vparam, i);
        field_type = tuple_eval.str(0);
        value = tuple_eval.num(1);
        count = tuple_eval.size() > 2 ? tuple_eval.num(2) : 0;
        if (field_type == "vnic") {
            vnic.reset(value, count);
        }else if (field_type == "sip") {
            sip.reset(value, count);
        } else if (field_type == "dip") {
            dip.reset(value, count);
        } else if (field_type == "sport") {
            sport.reset(value, count);
        } else if (field_type == "dport") {
            dport.reset(value, count);
        } else {
            TEST_LOG_ERR("Unknown tuple type %s\n", field_type.c_str());
            return false;
        }
    }

    ids_max = (uint64_t)vnic.count() * (uint64_t)sip.count()   *
              (uint64_t)dip.count()  * (uint64_t)sport.count() *
              (uint64_t)dport.count();
    session_tolerance.reset(ids_max > UINT32_MAX ? UINT32_MAX : ids_max);
    session_tolerance.using_fte_indices(true);

    for (vnic.restart(); vnic.count(); vnic.next_value()) {
        for (sip.restart(); sip.count(); sip.next_value()) {
            for (dip.restart(); dip.count(); dip.next_value()) {
                for (sport.restart(); sport.count(); sport.next_value()) {
                    for (dport.restart(); dport.count(); dport.next_value()) {

                        /*
                         * Create a session for 1-to-1 mapping to cache entry
                         * but only do so if this iteration is not a recirc
                         * due to an earlier "cache entry already exists".
                         */
                        if (cache_ret != PDS_RET_ENTRY_EXISTS) {
                            ret = (pds_ret_t)fte_ath::fte_session_index_alloc(
                                             &session_spec.key.session_info_id);
                            if (ret != PDS_RET_OK) {
                                TEST_LOG_INFO("session table full at %u entries\n",
                                         session_tolerance.create_id_map_size());
                                ret = PDS_RET_OK;
                                goto done;
                            }
                            ret = pds_flow_session_info_create(&session_spec);
                            if (!SESSION_CREATE_RET_VALIDATE(ret)) {
                                goto done;
                            }
                            session_tolerance.create_id_map_insert(
                                              session_spec.key.session_info_id);
                        }

                        if (proto == IPPROTO_ICMP) {
                            cache_ret = (pds_ret_t)
                                      fte_ath::fte_flow_create_icmp(vnic.value(),
                                               sip.value(), dip.value(),
                                               proto, sport.value(), dport.value(),
                                               session_spec.key.session_info_id,
                                               PDS_FLOW_SPEC_INDEX_SESSION,
                                               session_spec.key.session_info_id);
                        } else {
                            cache_ret = (pds_ret_t)
                                      fte_ath::fte_flow_create(vnic.value(),
                                               sip.value(), dip.value(),
                                               proto, sport.value(), dport.value(),
                                               PDS_FLOW_SPEC_INDEX_SESSION,
                                               session_spec.key.session_info_id);
                        }

                        switch (cache_ret) {

                        case PDS_RET_OK:
                            break;

                        case PDS_RET_NO_RESOURCE:
                            TEST_LOG_INFO("flow cache table full at %u entries\n",
                                     session_tolerance.create_id_map_size());
                            cache_ret = PDS_RET_OK;
                            goto done;

                        case PDS_RET_ENTRY_EXISTS:

                            /*
                             * Continue cache entry creation even on key hash
                             * collision but use the last session ID created.
                             */
                            break;

                        default:
                            TEST_LOG_ERR("failed cache create - vnic:%u "
                                 "sip:0x%x dip:0x%x sport:%u dport:%u proto:%u "
                                 "at session:%u error:%d\n", vnic.value(),
                                 sip.value(), dip.value(), sport.value(),
                                 dport.value(), proto,
                                 session_spec.key.session_info_id, cache_ret);
                            goto done;
                        }
                    }
                }
            }
        }
    }

done:
    TEST_LOG_INFO("Session entries created: %d\n",
                  session_tolerance.create_id_map_size());
    /*
     * Flow cache entry creations were best effort due to hash outcomes
     * so any non-zero count would be considered a success.
     */
    return session_tolerance.create_id_map_size() &&
           SESSION_CREATE_RET_VALIDATE(ret) &&
           SESSION_CREATE_RET_VALIDATE(cache_ret);
}

pds_ret_t
session_aging_expiry_fn(uint32_t expiry_id,
                        pds_flow_age_expiry_type_t expiry_type,
                        void *user_ctx)
{
    pds_ret_t   ret = PDS_RET_OK;
    sdk_ret_t   fte_ret = SDK_RET_OK;

    switch (expiry_type) {

    case EXPIRY_TYPE_SESSION:
        if (aging_expiry_dflt_fn) {

            session_tolerance.expiry_count_inc();
            session_tolerance.session_tmo_tolerance_check(expiry_id);
            session_tolerance.create_id_map_find_erase(expiry_id);
            ret = (*aging_expiry_dflt_fn)(expiry_id, expiry_type, user_ctx);
            if (session_tolerance.using_fte_indices()) {
                fte_ret = fte_ath::fte_session_index_free(expiry_id);
            }
        }
        break;

    case EXPIRY_TYPE_CONNTRACK:
        ret = conntrack_aging_expiry_fn(expiry_id, expiry_type, user_ctx);
        break;

    default:
        ret = PDS_RET_INVALID_ARG;
        break;
    }

    if ((ret != PDS_RET_OK) || (fte_ret != SDK_RET_OK)) {
        if (session_tolerance.delete_errors() == 0) {
            TEST_LOG_ERR("failed flow deletion on session_id %u: "
                         "ret %d fte_ret %d\n", expiry_id, ret, fte_ret);
        }
        session_tolerance.delete_errors_inc();
    }
    return ret;
}
                             
bool
session_aging_init(test_vparam_ref_t vparam)
{
    pds_ret_t   ret;

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
    pds_ret_t   ret;

    ret = ftl_pollers_client::force_session_expired_ts_set(
                                            vparam.expected_bool());
    return SESSION_RET_VALIDATE(ret);
}

bool
session_aging_fini(test_vparam_ref_t vparam)
{
    test_vparam_t   sim_vparam;
    pds_ret_t       ret;

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
    return session_tolerance.expiry_count() >= session_tolerance.create_count();
}

bool
session_4combined_expiry_count_check(bool poll_needed)
{
    if (poll_needed) {
        session_aging_pollers_poll((void *)&session_tolerance);
    }
    return session_tolerance.expiry_count() >= session_tolerance.create_count();
}

bool
session_4combined_result_check(void)
{
    TEST_LOG_INFO("Session entries aged out: %u, over_age_min: %u, "
                  "over_age_max: %u\n", session_tolerance.expiry_count(),
                  session_tolerance.over_age_min(),
                  session_tolerance.over_age_max());
    session_tolerance.create_id_map_empty_check();
    session_metrics.expiry_count_check(session_tolerance.expiry_count());
    return session_4combined_expiry_count_check() &&
           session_tolerance.zero_failures();
}

bool
session_aging_test(test_vparam_ref_t vparam)
{
    test_timestamp_t    ts;

    ts.time_expiry_set(APP_TIME_LIMIT_EXEC_SECS(session_tolerance.curr_max_tmo() +
                                                APP_TIME_LIMIT_EXEC_GRACE));
    ts.time_limit_exec(session_aging_expiry_count_check, nullptr, 0);
    return session_4combined_result_check();
}

bool
session_aging_normal_tmo_set(test_vparam_ref_t vparam)
{
    session_tolerance.reset();
    session_tolerance.normal_tmo.session_tmo_set(vparam.expected_num());
    return session_tolerance.zero_failures();
}

bool
session_aging_accel_tmo_set(test_vparam_ref_t vparam)
{
    session_tolerance.reset();
    session_tolerance.accel_tmo.session_tmo_set(vparam.expected_num());
    return session_tolerance.zero_failures();
}

bool
session_aging_accel_control(test_vparam_ref_t vparam)
{
    session_tolerance.reset();
    session_tolerance.age_accel_control(vparam.expected_bool());
    return session_tolerance.zero_failures();
}

bool
session_aging_metrics_show(test_vparam_ref_t vparam)
{
    aging_metrics_t scanner_metrics(ftl_dev_if::FTL_QTYPE_SCANNER_SESSION);
    aging_metrics_t poller_metrics(ftl_dev_if::FTL_QTYPE_POLLER);
    aging_metrics_t timestamp_metrics(ftl_dev_if::FTL_QTYPE_MPU_TIMESTAMP);

    scanner_metrics.show();
    poller_metrics.show();
    timestamp_metrics.show();
    return true;
}

}    // namespace athena_app
}    // namespace test
