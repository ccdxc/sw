//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/table.hpp"
#include "nic/sdk/platform/capri/capri_tm_rw.hpp"
#include "nic/sdk/platform/capri/capri_tm_utils.hpp"
#include "nic/sdk/asic/pd/pd.hpp"
#include "nic/apollo/api/include/pds_debug.hpp"
#include "nic/apollo/api/debug.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/svc/debug.hpp"
#include "nic/apollo/agent/trace.hpp"
#include <malloc.h>

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
    if (freq) {
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
    }

    freq = proto_req->armclockfrequency();
    if (freq) {
        switch (freq) {
        case 1667:
            clock_freq = PDS_CLOCK_FREQUENCY_1666;
            break;
        case 2200:
            clock_freq = PDS_CLOCK_FREQUENCY_2200;
            break;
        default:
            PDS_TRACE_ERR("Clock-frequency update failed, accepted frequencies are 1667 and 2200");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
            return Status::OK;
        }
        ret = debug::pds_arm_clock_frequency_update(clock_freq);
    }

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

Status
DebugSvcImpl::TableStatsGet(ServerContext *context, const Empty *proto_req,
                            pds::TableStatsGetResponse *proto_rsp) {
    sdk_ret_t ret;

    if ((ret = debug::pds_table_stats_get(pds_table_stats_entry_to_proto, proto_rsp)) != SDK_RET_OK) {
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

Status
DebugSvcImpl::PbStatsGet(ServerContext *context, const Empty *proto_req,
                         pds::PbStatsGetResponse *proto_rsp) {
    sdk_ret_t ret;

    if ((ret = debug::pds_pb_stats_get(pds_pb_stats_entry_to_proto, proto_rsp)) != SDK_RET_OK) {
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    }

    return Status::OK;
}

Status
DebugSvcImpl::FteStatsGet(ServerContext *context, const Empty *req,
                          pds::FteStatsGetResponse *rsp) {
    sdk_ret_t ret;

    if ((ret = debug::pds_fte_api_stats_get()) != SDK_RET_OK) {
        rsp->set_apistatus(sdk_ret_to_api_status(ret));
    }
    if ((ret = debug::pds_fte_table_stats_get()) != SDK_RET_OK) {
        rsp->set_apistatus(sdk_ret_to_api_status(ret));
    }

    return Status::OK;
}

Status
DebugSvcImpl::FteStatsClear(ServerContext *context, const pds::FteStatsClearRequest *req,
                          pds::FteStatsClearResponse *rsp) {
    sdk_ret_t ret;

    if (req->apistats()) {
        if ((ret = debug::pds_fte_api_stats_clear()) != SDK_RET_OK) {
            rsp->set_apistatus(sdk_ret_to_api_status(ret));
        }
    }

    if (req->tablestats()) {
        if ((ret = debug::pds_fte_table_stats_clear()) != SDK_RET_OK) {
            rsp->set_apistatus(sdk_ret_to_api_status(ret));
        }
    }

    return Status::OK;
}

Status
DebugSvcImpl::TraceFlush(ServerContext *context, const Empty *req,
                         Empty *rsp) {
    core::flush_logs();
    return Status::OK;
}

Status
DebugSvcImpl::MemoryTrim(ServerContext *context, const Empty *req,
                         Empty *rsp) {
    malloc_trim(0);
    return Status::OK;
}

Status
DebugSvcImpl::HeapGet(ServerContext *context, const Empty *req,
                      pds::HeapGetResponse *rsp) {
    auto stats = rsp->mutable_stats();

    struct mallinfo minfo = {0};
    minfo = mallinfo();

    stats->set_numarenabytesalloc(minfo.arena);
    stats->set_numfreeblocks(minfo.ordblks);
    stats->set_numfastbinfreeblocks(minfo.smblks);
    stats->set_nummmapblocksalloc(minfo.hblks);
    stats->set_nummmapbytesalloc(minfo.hblkhd);
    stats->set_maxblocksalloc(minfo.usmblks);
    stats->set_numfastbinfreebytes(minfo.fsmblks);
    stats->set_numbytesalloc(minfo.uordblks);
    stats->set_numfreebytes(minfo.fordblks);
    stats->set_releasablefreebytes(minfo.keepcost);
    rsp->set_apistatus(sdk_ret_to_api_status(SDK_RET_OK));

    return Status::OK;
}

Status
DebugSvcImpl::MeterStatsGet(ServerContext *context, const pds::MeterStatsGetRequest *req,
                            pds::MeterStatsGetResponse *rsp) {
    sdk_ret_t ret;

    if ((ret = debug::pds_meter_stats_get(pds_meter_debug_stats_to_proto,
                                          req->statsindexlow(),
                                          req->statsindexhigh(),
                                          rsp)) != SDK_RET_OK) {
        rsp->set_apistatus(sdk_ret_to_api_status(ret));
    }

    return Status::OK;
}

Status
DebugSvcImpl::SessionStatsGet(ServerContext *context, const pds::SessionStatsGetRequest *req,
                              pds::SessionStatsGetResponse *rsp) {
    sdk_ret_t ret;

    if ((ret = debug::pds_session_stats_get(pds_session_debug_stats_to_proto,
                                            req->statsindexlow(),
                                            req->statsindexhigh(),
                                            rsp)) != SDK_RET_OK) {
        rsp->set_apistatus(sdk_ret_to_api_status(ret));
    }

    return Status::OK;
}

Status
DebugSvcImpl::QueueCreditsGet(ServerContext *context, const Empty *req,
                              pds::QueueCreditsGetResponse *rsp) {
    sdk_ret_t ret;
    ret = sdk::asic::pd::queue_credits_get(pds_queue_credits_to_proto, rsp);
    rsp->set_apistatus(sdk_ret_to_api_status(ret));
    return Status::OK;
}

Status
DebugSvcImpl::StartAacsServer(ServerContext *context,
                              const pds::AacsRequest *proto_req,
                              Empty *proto_rsp) {
    PDS_TRACE_VERBOSE("Received AACS Server Start");
    if (proto_req) {
        debug::start_aacs_server(proto_req->aacsserverport());
    }

    return Status::OK;
}

Status
DebugSvcImpl::StopAacsServer(ServerContext *context,
                             const Empty *proto_req,
                             Empty *proto_rsp) {
    PDS_TRACE_VERBOSE("Received AACS Server Stop");
    debug::stop_aacs_server();
    return Status::OK;
}

Status
DebugSvcImpl::SlabGet(ServerContext *context,
                      const Empty *proto_req,
                      pds::SlabGetResponse *proto_rsp) {
    sdk_ret_t ret;
    ret = core::agent_state::state()->slab_walk(pds_slab_to_proto, proto_rsp);
    if (ret == SDK_RET_OK) {
        ret = debug::pds_slab_get(pds_slab_to_proto, proto_rsp);
    }
    proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    return Status::OK;
}
