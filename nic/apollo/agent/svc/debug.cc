//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/table.hpp"
#include "nic/sdk/asic/pd/pd.hpp"
#include "nic/apollo/api/include/pds_debug.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/svc/util.hpp"
#include "nic/apollo/agent/svc/debug.hpp"
#include "nic/apollo/agent/trace.hpp"

Status
DebugSvcImpl::ClockFrequencyUpdate(ServerContext *context,
                                   const pds::ClockFrequencyRequest *proto_req,
                                   pds::ClockFrequencyResponse *proto_rsp) {
    sdk_ret_t ret;
    uint32_t freq;
    pds_clock_freq_t clock_freq;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }

    freq = proto_req->clockfrequency();
    switch (freq) {
    case 833:
        clock_freq = PDS_CLOCK_FREQUENCY_833;
        break;
    case 900:
        clock_freq = PDS_CLOCK_FREQUENCY_900;
        break;
    case 957:
        clock_freq = PDS_CLOCK_FREQUENCY_957;
        break;
    case 1033:
        clock_freq = PDS_CLOCK_FREQUENCY_1033;
        break;
    case 1100:
        clock_freq = PDS_CLOCK_FREQUENCY_1100;
        break;
    default:
        PDS_TRACE_ERR("Clock-frequency update failed, accepted frequencies are 833, 900, 957, 1033 and 1100");
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }

    ret = debug::pds_clock_frequency_update(clock_freq);
    proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    return Status::OK;
}

Status
DebugSvcImpl::SystemTemperatureGet(ServerContext *context, const Empty *proto_req,
                                   pds::SystemTemperatureGetResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_system_temperature_t temp = {0};

    ret = debug::pds_get_system_temperature(&temp);
    proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    proto_rsp->set_dietemp(temp.dietemp);
    proto_rsp->set_localtemp(temp.localtemp);
    proto_rsp->set_hbmtemp(temp.hbmtemp);
    return Status::OK;
}

Status
DebugSvcImpl::SystemPowerGet(ServerContext *context, const Empty *proto_req,
                             pds::SystemPowerGetResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_system_power_t pow = {0};

    ret = debug::pds_get_system_power(&pow);
    proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    proto_rsp->set_pin(pow.pin);
    proto_rsp->set_pout1(pow.pout1);
    proto_rsp->set_pout2(pow.pout2);
    return Status::OK;
}

Status
DebugSvcImpl::TraceUpdate(ServerContext *context, const pds::TraceRequest *proto_req,
                          pds::TraceResponse *proto_rsp) {
    utils::trace_level_e trace_level;

    switch (proto_req->trace_level()) {
    case pds::TRACE_LEVEL_NONE:
        trace_level = utils::trace_none;
        break;
    case pds::TRACE_LEVEL_DEBUG:
        trace_level = utils::trace_debug;
        break;
    case pds::TRACE_LEVEL_ERROR:
        trace_level = utils::trace_err;
        break;
    case pds::TRACE_LEVEL_WARN:
        trace_level = utils::trace_warn;
        break;
    case pds::TRACE_LEVEL_INFO:
        trace_level = utils::trace_info;
        break;
    case pds::TRACE_LEVEL_VERBOSE:
        trace_level = utils::trace_verbose;
        break;
    default:
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
        break;
    }

    core::trace_update(trace_level);
    proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OK);
    proto_rsp->set_tracelevel(proto_req->trace_level());
    return Status::OK;
}

void
pds_table_api_stats_fill (pds::TableApiStats *proto_stats,
                          sdk::table::sdk_table_api_stats_t *stats)
{
    auto entry = proto_stats->add_entry();
    entry->set_type(pds::TABLE_API_STATS_INSERT);
    entry->set_count(stats->insert);

    entry = proto_stats->add_entry();
    entry->set_type(pds::TABLE_API_STATS_INSERT_DUPLICATE);
    entry->set_count(stats->insert_duplicate);

    entry = proto_stats->add_entry();
    entry->set_type(pds::TABLE_API_STATS_INSERT_FAIL);
    entry->set_count(stats->insert_fail);

    entry = proto_stats->add_entry();
    entry->set_type(pds::TABLE_API_STATS_REMOVE);
    entry->set_count(stats->remove);

    entry = proto_stats->add_entry();
    entry->set_type(pds::TABLE_API_STATS_REMOVE_NOT_FOUND);
    entry->set_count(stats->remove_not_found);

    entry = proto_stats->add_entry();
    entry->set_type(pds::TABLE_API_STATS_REMOVE_FAIL);
    entry->set_count(stats->remove_fail);

    entry = proto_stats->add_entry();
    entry->set_type(pds::TABLE_API_STATS_UPDATE);
    entry->set_count(stats->update);

    entry = proto_stats->add_entry();
    entry->set_type(pds::TABLE_API_STATS_UPDATE_FAIL);
    entry->set_count(stats->update_fail);

    entry = proto_stats->add_entry();
    entry->set_type(pds::TABLE_API_STATS_GET);
    entry->set_count(stats->get);

    entry = proto_stats->add_entry();
    entry->set_type(pds::TABLE_API_STATS_GET_FAIL);
    entry->set_count(stats->get_fail);

    entry = proto_stats->add_entry();
    entry->set_type(pds::TABLE_API_STATS_RESERVE);
    entry->set_count(stats->reserve);

    entry = proto_stats->add_entry();
    entry->set_type(pds::TABLE_API_STATS_RESERVE_FAIL);
    entry->set_count(stats->reserve_fail);

    entry = proto_stats->add_entry();
    entry->set_type(pds::TABLE_API_STATS_RELEASE);
    entry->set_count(stats->release);

    entry = proto_stats->add_entry();
    entry->set_type(pds::TABLE_API_STATS_RELEASE_FAIL);
    entry->set_count(stats->release_fail);
}

void
pds_table_stats_fill (pds::TableStats *proto_stats,
                      sdk::table::sdk_table_stats_t *stats)
{
    auto entry = proto_stats->add_entry();
    entry->set_type(pds::TABLE_STATS_ENTRIES);
    entry->set_count(stats->entries);

    entry = proto_stats->add_entry();
    entry->set_type(pds::TABLE_STATS_COLLISIONS);
    entry->set_count(stats->collisions);
}

void
pds_table_stats_entry_fill (pds_table_stats_t *stats, void *ctxt)
{
    pds::TableStatsGetResponse *rsp = (pds::TableStatsGetResponse *)ctxt;
    auto table_api_stats = rsp->add_apistats();
    auto table_stats = rsp->add_stats();

    pds_table_api_stats_fill(table_api_stats, stats->api_stats);
    pds_table_stats_fill(table_stats, stats->stats);
    rsp->set_apistatus(types::ApiStatus::API_STATUS_OK);
}

Status
DebugSvcImpl::TableStatsGet(ServerContext *context, const Empty *proto_req,
                          pds::TableStatsGetResponse *proto_rsp) {
    sdk_ret_t ret;

    if ((ret = debug::pds_table_stats_get(pds_table_stats_entry_fill, proto_rsp)) != SDK_RET_OK) {
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    }

    return Status::OK;
}

Status
DebugSvcImpl::LlcSetup(ServerContext *context, const pds::LlcSetupRequest *proto_req,
                       pds::LlcSetupResponse *proto_rsp) {
    sdk_ret_t ret;
    sdk::asic::pd::llc_counters_t llc_args;

    memset (&llc_args, 0, sizeof(sdk::asic::pd::llc_counters_t));
    if (proto_req->type()) {
        llc_args.mask = (1 << (proto_req->type() - 1)); // Req Type starts at 1 so we need to subtract 1
    } else {
        llc_args.mask = 0xffffffff;                     // Clear LLC 
    }

    ret = debug::pds_llc_setup(&llc_args);
    proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));

    return Status::OK;
}

Status
DebugSvcImpl::LlcStatsGet(ServerContext *context, const Empty *proto_req,
                          pds::LlcStatsGetResponse *proto_rsp) {
    sdk_ret_t ret;
    sdk::asic::pd::llc_counters_t llc_args;
    auto stats = proto_rsp->mutable_stats();

    memset (&llc_args, 0, sizeof(llc_counters_t));
    ret = debug::pds_llc_get(&llc_args);
    if (ret == SDK_RET_OK) {
        if (llc_args.mask == 0xffffffff) {
            stats->set_type(pds::LLC_COUNTER_CACHE_NONE);
        } else {
            for (int i = int(pds::LlcCounterType_MIN) + 1; i <= int(pds::LlcCounterType_MAX); i ++) {
                if ((llc_args.mask & (1 << (i - 1))) == 1) {
                    stats->set_type(pds::LlcCounterType(i));
                    break;
                }
            }
        }
        for (int i = 0; i < 16; i ++) {
            stats->add_count(llc_args.data[i]);
        }
    }
    proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));

    return Status::OK;
}
