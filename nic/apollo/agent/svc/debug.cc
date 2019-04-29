//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/table.hpp"
#include "nic/sdk/platform/capri/capri_tm_rw.hpp"
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
    auto response = rsp->add_response();
    auto api_stats = response->mutable_apistats();
    auto table_stats = response->mutable_tablestats();

    response->set_tablename(stats->table_name);
    pds_table_api_stats_fill(api_stats, &stats->api_stats);
    pds_table_stats_fill(table_stats, &stats->table_stats);
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

void
pds_pb_stats_port_fill (pds::PacketBufferPort *buf_port, uint32_t port)
{
    if ((port >= TM_UPLINK_PORT_BEGIN) && (port <= TM_UPLINK_PORT_END)) {
        buf_port->set_porttype(pds::PACKET_BUFFER_PORT_TYPE_UPLINK);
        buf_port->set_portnum(port-TM_UPLINK_PORT_BEGIN);
    } else if ((port >= TM_DMA_PORT_BEGIN) && (port <= TM_DMA_PORT_END)) {
        buf_port->set_porttype(pds::PACKET_BUFFER_PORT_TYPE_DMA);
        buf_port->set_portnum(TM_PORT_DMA);
    } else if (port == TM_PORT_INGRESS) {
        buf_port->set_porttype(pds::PACKET_BUFFER_PORT_TYPE_P4IG);
        buf_port->set_portnum(TM_PORT_INGRESS);
    } else if (port == TM_PORT_EGRESS) {
        buf_port->set_porttype(pds::PACKET_BUFFER_PORT_TYPE_P4EG);
        buf_port->set_portnum(TM_PORT_EGRESS);
    }
}

void
pds_pb_stats_entry_fill (pds_pb_debug_stats_t *pds_stats, void *ctxt)
{
    sdk::platform::capri::tm_pb_debug_stats_t *stats = &pds_stats->stats;
    pds::PbStatsGetResponse *rsp = (pds::PbStatsGetResponse *)ctxt;
    auto pb_stats = rsp->mutable_pbstats()->add_portstats();
    auto port = pb_stats->mutable_packetbufferport();
    auto buffer_stats = pb_stats->mutable_bufferstats();
    auto oflow_fifo_stats = pb_stats->mutable_oflowfifostats();

    pds_pb_stats_port_fill(port, pds_stats->port);

    buffer_stats->set_sopcountin(stats->buffer_stats.sop_count_in);
    buffer_stats->set_eopcountin(stats->buffer_stats.eop_count_in);
    buffer_stats->set_sopcountout(stats->buffer_stats.sop_count_out);
    buffer_stats->set_eopcountout(stats->buffer_stats.eop_count_out);

    auto drop_stats = buffer_stats->mutable_dropcounts();
    for (int i = sdk::platform::capri::BUFFER_INTRINSIC_DROP; i < sdk::platform::capri::BUFFER_DROP_MAX; i ++) {
        auto drop_stats_entry = drop_stats->add_statsentries();
        drop_stats_entry->set_reasons(pds::BufferDropReasons(i));
        drop_stats_entry->set_dropcount(stats->buffer_stats.drop_counts[i]);
    }

    oflow_fifo_stats->set_sopcountin(stats->oflow_fifo_stats.sop_count_in);
    oflow_fifo_stats->set_eopcountin(stats->oflow_fifo_stats.eop_count_in);
    oflow_fifo_stats->set_sopcountout(stats->oflow_fifo_stats.sop_count_out);
    oflow_fifo_stats->set_eopcountout(stats->oflow_fifo_stats.eop_count_out);

    auto drop_counts = oflow_fifo_stats->mutable_dropcounts();
    drop_counts->add_entry()->set_type(pds::OflowFifoDropType::OCCUPANCY_DROP);
    drop_counts->mutable_entry(0)->set_count(stats->oflow_fifo_stats.drop_counts.occupancy_drop_count);
    drop_counts->add_entry()->set_type(pds::OflowFifoDropType::EMERGENCY_STOP_DROP);
    drop_counts->mutable_entry(1)->set_count(stats->oflow_fifo_stats.drop_counts.emergency_stop_drop_count);
    drop_counts->add_entry()->set_type(pds::OflowFifoDropType::WRITE_BUFFER_ACK_FILL_UP_DROP);
    drop_counts->mutable_entry(2)->set_count(stats->oflow_fifo_stats.drop_counts.write_buffer_ack_fill_up_drop_count);
    drop_counts->add_entry()->set_type(pds::OflowFifoDropType::WRITE_BUFFER_ACK_FULL_DROP);
    drop_counts->mutable_entry(3)->set_count(stats->oflow_fifo_stats.drop_counts.write_buffer_ack_full_drop_count);
    drop_counts->add_entry()->set_type(pds::OflowFifoDropType::WRITE_BUFFER_FULL_DROP);
    drop_counts->mutable_entry(4)->set_count(stats->oflow_fifo_stats.drop_counts.write_buffer_full_drop_count);
    drop_counts->add_entry()->set_type(pds::OflowFifoDropType::CONTROL_FIFO_FULL_DROP);
    drop_counts->mutable_entry(5)->set_count(stats->oflow_fifo_stats.drop_counts.control_fifo_full_drop_count);

    rsp->set_apistatus(types::ApiStatus::API_STATUS_OK);
}

Status
DebugSvcImpl::PbStatsGet(ServerContext *context, const Empty *proto_req,
                         pds::PbStatsGetResponse *proto_rsp) {
    sdk_ret_t ret;

    if ((ret = debug::pds_pb_stats_get(pds_pb_stats_entry_fill, proto_rsp)) != SDK_RET_OK) {
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    }

    return Status::OK;
}

Status
DebugSvcImpl::FlowClear(ServerContext *context, const Empty *req,
                        Empty *rsp) {
    PDS_TRACE_DEBUG("Flow clear received");

    return Status::OK;
}

Status
DebugSvcImpl::TraceFlush(ServerContext *context, const Empty *req,
                         Empty *rsp) {
    core::flush_logs();
    return Status::OK;
}
