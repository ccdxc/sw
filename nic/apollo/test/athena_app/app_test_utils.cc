//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains athena_app test utility API
///
//----------------------------------------------------------------------------

#include "app_test_utils.hpp"
#include "nic/apollo/api/impl/athena/ftl_pollers_client.hpp"

namespace test {
namespace athena_app {

const static std::map<string,bool> truefalse2hool_map =
{
    {"true",            true},
    {"false",           false},
    {"TRUE",            true},
    {"FALSE",           false},
    {"True",            true},
    {"False",           false},
};

const static std::map<string,uint32_t> flowtype2num_map =
{
    {"tcp",             CONNTRACK_FLOW_TYPE_TCP},
    {"TCP",             CONNTRACK_FLOW_TYPE_TCP},
    {"udp",             CONNTRACK_FLOW_TYPE_UDP},
    {"UDP",             CONNTRACK_FLOW_TYPE_UDP},
    {"icmp",            CONNTRACK_FLOW_TYPE_ICMP},
    {"ICMP",            CONNTRACK_FLOW_TYPE_ICMP},
    {"others",          CONNTRACK_FLOW_TYPE_OTHERS},
    {"OTHERS",          CONNTRACK_FLOW_TYPE_OTHERS},
};

const static std::map<string,uint32_t> flowstate2num_map =
{
    {"unestablished",   CONNTRACK_FLOW_STATE_UNESTABLISHED},
    {"UNESTABLISHED",   CONNTRACK_FLOW_STATE_UNESTABLISHED},
    {"syn_sent",        CONNTRACK_FLOW_STATE_SYN_SENT},
    {"SYN_SENT",        CONNTRACK_FLOW_STATE_SYN_SENT},
    {"syn_recv",        CONNTRACK_FLOW_STATE_SYN_RECV},
    {"SYN_RECV",        CONNTRACK_FLOW_STATE_SYN_RECV},
    {"synack_sent",     CONNTRACK_FLOW_STATE_SYNACK_SENT},
    {"SYNACK_SENT",     CONNTRACK_FLOW_STATE_SYNACK_SENT},
    {"synack_recv",     CONNTRACK_FLOW_STATE_SYNACK_RECV},
    {"SYNACK_RECV",     CONNTRACK_FLOW_STATE_SYNACK_RECV},
    {"established",     CONNTRACK_FLOW_STATE_ESTABLISHED},
    {"ESTABLISHED",     CONNTRACK_FLOW_STATE_ESTABLISHED},
    {"fin_sent",        CONNTRACK_FLOW_STATE_FIN_SENT},
    {"FIN_SENT",        CONNTRACK_FLOW_STATE_FIN_SENT},
    {"fin_recv",        CONNTRACK_FLOW_STATE_FIN_RECV},
    {"FIN_RECV",        CONNTRACK_FLOW_STATE_FIN_RECV},
    {"time_wait",       CONNTRACK_FLOW_STATE_TIME_WAIT},
    {"TIME_WAIT",       CONNTRACK_FLOW_STATE_TIME_WAIT},
    {"rst_close",       CONNTRACK_FLOW_STATE_RST_CLOSE},
    {"RST_CLOSE",       CONNTRACK_FLOW_STATE_RST_CLOSE},
    {"removed",         CONNTRACK_FLOW_STATE_REMOVED},
    {"REMOVED",         CONNTRACK_FLOW_STATE_REMOVED},
};

static const bool *
truefalse2hool_find(const std::string& token)
{
    auto iter = truefalse2hool_map.find(token);
    if (iter != truefalse2hool_map.end()) {
        return &iter->second;
    }
    return nullptr;
}

static const uint32_t * __attribute__((used))
flowtype2num_find(const std::string& token)
{
    auto iter = flowtype2num_map.find(token);
    if (iter != flowtype2num_map.end()) {
        return &iter->second;
    }
    return nullptr;
}

static const uint32_t *  __attribute__((used))
flowstate2num_find(const std::string& token)
{
    auto iter = flowstate2num_map.find(token);
    if (iter != flowstate2num_map.end()) {
        return &iter->second;
    }
    return nullptr;
}

/**
 * Vector parameters
 */
sdk_ret_t 
test_vparam_t::num(uint32_t idx,
                   uint32_t *ret_num,
                   bool suppress_err_log) const
{
    *ret_num = 0;
    if (idx < vparam.size()) {
        const test_param_t& param = vparam.at(idx);

        if (param.type == TOKEN_TYPE_NUM) {
            *ret_num = param.num;
            return SDK_RET_OK;
        }
    }

    if (!suppress_err_log) {
        TEST_LOG_INFO("Numeric token not found at index %u\n", idx);
    }
    return SDK_RET_ERR;
}

sdk_ret_t 
test_vparam_t::str(uint32_t idx,
                   std::string *ret_str,
                   bool suppress_err_log) const
{
    ret_str->clear();
    if (idx < vparam.size()) {
        const test_param_t& param = vparam.at(idx);

        if (param.type == TOKEN_TYPE_STR) {
            ret_str->assign(param.str);
            return SDK_RET_OK;
        }
    }

    if (!suppress_err_log) {
        TEST_LOG_INFO("String token not found at index %u\n", idx);
    }
    return SDK_RET_ERR;
}

bool
test_vparam_t::expected_bool(bool dflt) const
{
    if (vparam.size()) {
        const test_param_t& param = vparam.at(0);
        if (param.type == TOKEN_TYPE_STR) {
            const bool *ret_bool = truefalse2hool_find(param.str);
            if (ret_bool) {
                return *ret_bool;
            }
            TEST_LOG_INFO("Unrecognized token %s\n", param.str.c_str());
        } else {
            return !!param.num;
        }
    }
    return dflt;
}

uint32_t
test_vparam_t::expected_num(uint32_t dflt) const
{
    if (vparam.size()) {
        const test_param_t& param = vparam.at(0);
        if (param.type == TOKEN_TYPE_NUM) {
            return param.num;
        }
    }
    return dflt;
}

const std::string&
test_vparam_t::expected_str(void) const
{
    static std::string empty_str;

    if (vparam.size()) {
        const test_param_t& param = vparam.at(0);
        if (param.type == TOKEN_TYPE_STR) {
            return param.str;
        }
    }
    return empty_str;
}

/**
 * Metrics
 */
sdk_ret_t
age_metrics_t::baseline(void)
{
    return refresh(base);
}

uint64_t
age_metrics_t::delta_expired_entries(void) const
{
    ftl_dev_if::lif_attr_metrics_t  curr;
    uint64_t                        delta = 0;

    switch (qtype) {

    case ftl_dev_if::FTL_QTYPE_SCANNER_SESSION:
    case ftl_dev_if::FTL_QTYPE_SCANNER_CONNTRACK:

        if (refresh(curr) == SDK_RET_OK) {
            delta = curr.scanners.total_expired_entries -
                    base.scanners.total_expired_entries;
        }
        break;

    default:
        break;
    }

    return delta;
}

uint64_t
age_metrics_t::delta_num_qfulls(void) const
{
    ftl_dev_if::lif_attr_metrics_t  curr;
    uint64_t                        delta = 0;

    switch (qtype) {

    case ftl_dev_if::FTL_QTYPE_POLLER:

        if (refresh(curr) == SDK_RET_OK) {
            delta = curr.pollers.total_num_qfulls -
                    base.pollers.total_num_qfulls;
        }
        break;

    default:
        break;
    }

    return delta;
}

void
age_metrics_t::show(bool latest) const
{
    ftl_dev_if::lif_attr_metrics_t  curr;

    if (latest) {
        refresh(curr);
    } else {
        curr = base;
    }

    switch (qtype) {

    case ftl_dev_if::FTL_QTYPE_SCANNER_SESSION:
    case ftl_dev_if::FTL_QTYPE_SCANNER_CONNTRACK:

        TEST_LOG_INFO("total_cb_cfg_discards   : %" PRIu64 "\n", 
                      curr.scanners.total_cb_cfg_discards);
        TEST_LOG_INFO("total_scan_invocations  : %" PRIu64 "\n",
                      curr.scanners.total_scan_invocations);
        TEST_LOG_INFO("total_expired_entries   : %" PRIu64 "\n",
                      curr.scanners.total_expired_entries);
        TEST_LOG_INFO("min_range_elapsed_ns    : %" PRIu64 "\n",
                      curr.scanners.min_range_elapsed_ns);
        TEST_LOG_INFO("avg_min_range_elapsed_ns: %" PRIu64 "\n",
                      curr.scanners.avg_min_range_elapsed_ns);
        TEST_LOG_INFO("max_range_elapsed_ns    : %" PRIu64 "\n",
                      curr.scanners.max_range_elapsed_ns);
        TEST_LOG_INFO("avg_max_range_elapsed_ns: %" PRIu64 "\n",
                      curr.scanners.avg_max_range_elapsed_ns);
        break;

    case ftl_dev_if::FTL_QTYPE_POLLER:
        TEST_LOG_INFO("total_num_qfulls        : %" PRIu64 "\n", 
                      curr.pollers.total_num_qfulls);
        break;

    default:
        break;
    }
}

sdk_ret_t
age_metrics_t::refresh(ftl_dev_if::lif_attr_metrics_t& m) const
{
    sdk_ret_t   ret = SDK_RET_OK;

    switch (qtype) {

    case ftl_dev_if::FTL_QTYPE_SCANNER_SESSION:
        ret = ftl_pollers_client::session_scanners_metrics_get(&m);
        break;

    case ftl_dev_if::FTL_QTYPE_SCANNER_CONNTRACK:
        ret = ftl_pollers_client::conntrack_scanners_metrics_get(&m);
        break;

    case ftl_dev_if::FTL_QTYPE_POLLER:
        ret = ftl_pollers_client::pollers_metrics_get(&m);
        break;

    default:
        memset(&m, 0, sizeof(m));
        break;
    }

    return ret;
}

}    // namespace athena_app
}    // namespace test

