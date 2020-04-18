//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines protobuf API for debug object
///
//----------------------------------------------------------------------------

#ifndef __AGENT_SVC_DEBUG_SVC_HPP__
#define __AGENT_SVC_DEBUG_SVC_HPP__

#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/svc/debug.hpp"
#include "nic/sdk/asic/port.hpp"

using sdk::asic::pd::port_queue_credit_t;
using sdk::asic::pd::queue_credit_t;

static inline void
pds_queue_credits_to_proto (uint32_t port_num,
                            port_queue_credit_t *credit,
                            void *ctxt)
{
    QueueCreditsGetResponse *proto = (QueueCreditsGetResponse *)ctxt;
    auto port_credit = proto->add_portqueuecredit();
    port_credit->set_port(port_num);

    for (uint32_t j = 0; j < credit->num_queues; j++) {
        auto queue_credit = port_credit->add_queuecredit();
        queue_credit->set_queue(credit->queues[j].oq);
        queue_credit->set_credit(credit->queues[j].credit);
    }
}

static inline bool
pds_slab_to_proto (void *entry, void *ctxt)
{
    pds::SlabGetResponse *rsp = (pds::SlabGetResponse *)ctxt;
    slab *s = (slab *)entry;

    auto rsp_entry = rsp->add_slab();
    auto spec = rsp_entry->mutable_spec();
    auto stats = rsp_entry->mutable_stats();

    spec->set_name(s->name());
    spec->set_id(s->slab_id());
    spec->set_elementsize(s->elem_sz());
    spec->set_elementsperblock(s->elems_per_block());
    spec->set_threadsafe(s->thread_safe());
    spec->set_growondemand(s->grow_on_demand());
    //spec->set_delaydelete(s->delay_delete());
    spec->set_zeroonallocation(s->zero_on_alloc());
    spec->set_rawblocksize(s->raw_block_sz());
    stats->set_numelementsinuse(s->num_in_use());
    stats->set_numallocs(s->num_allocs());
    stats->set_numfrees(s->num_frees());
    stats->set_numallocerrors(s->num_alloc_fails());
    stats->set_numblocks(s->num_blocks());

    return false;
}

static inline void
pds_session_debug_stats_to_proto (uint32_t idx,
                                  pds_session_debug_stats_t *stats, void *ctxt)
{
    pds::SessionStatsGetResponse *rsp = (pds::SessionStatsGetResponse *)ctxt;
    auto proto_stats = rsp->add_stats();

    proto_stats->set_statsindex(idx);
    proto_stats->set_initiatorflowpkts(stats->iflow_packet_count);
    proto_stats->set_initiatorflowbytes(stats->iflow_bytes_count);
    proto_stats->set_responderflowpkts(stats->rflow_packet_count);
    proto_stats->set_responderflowbytes(stats->rflow_bytes_count);
}

static inline void
pds_pb_stats_port_to_proto (pds::PacketBufferPort *buf_port, uint32_t port)
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

static inline void
pds_pb_stats_entry_to_proto (pds_pb_debug_stats_t *pds_stats, void *ctxt)
{
    sdk::tm_debug_stats_t *stats = &pds_stats->stats;
    sdk::platform::capri::capri_queue_stats_t *qos_queue_stats =
        &pds_stats->qos_queue_stats;
    pds::PbStatsGetResponse *rsp = (pds::PbStatsGetResponse *)ctxt;
    auto pb_stats = rsp->mutable_pbstats()->add_portstats();
    auto port = pb_stats->mutable_packetbufferport();
    auto buffer_stats = pb_stats->mutable_bufferstats();
    auto oflow_fifo_stats = pb_stats->mutable_oflowfifostats();
    auto q_stats = pb_stats->mutable_qosqueuestats();

    pds_pb_stats_port_to_proto(port, pds_stats->port);

    buffer_stats->set_sopcountin(stats->buffer_stats.sop_count_in);
    buffer_stats->set_eopcountin(stats->buffer_stats.eop_count_in);
    buffer_stats->set_sopcountout(stats->buffer_stats.sop_count_out);
    buffer_stats->set_eopcountout(stats->buffer_stats.eop_count_out);

    auto drop_stats = buffer_stats->mutable_dropcounts();
    for (int i = sdk::BUFFER_INTRINSIC_DROP; i < sdk::BUFFER_DROP_MAX; i ++) {
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

    for (int i = 0; i < CAPRI_TM_MAX_QUEUES; i++) {
        if (qos_queue_stats->iq_stats[i].iq.valid) {
            auto input_stats = q_stats->add_inputqueuestats();
            auto iq_stats = &qos_queue_stats->iq_stats[i].stats;
            input_stats->set_inputqueueidx(qos_queue_stats->iq_stats[i].iq.queue);
            input_stats->mutable_oflowfifostats()->set_goodpktsin(iq_stats->oflow.good_pkts_in);
            input_stats->mutable_oflowfifostats()->set_goodpktsout(iq_stats->oflow.good_pkts_out);
            input_stats->mutable_oflowfifostats()->set_erroredpktsin(iq_stats->oflow.errored_pkts_in);
            input_stats->mutable_oflowfifostats()->set_fifodepth(iq_stats->oflow.fifo_depth);
            input_stats->mutable_oflowfifostats()->set_maxfifodepth(iq_stats->oflow.max_fifo_depth);
            input_stats->set_bufferoccupancy(iq_stats->buffer_occupancy);
            input_stats->set_peakoccupancy(iq_stats->peak_occupancy);
            input_stats->set_portmonitor(iq_stats->port_monitor);
        }
        if (qos_queue_stats->oq_stats[i].oq.valid) {
            auto output_stats = q_stats->add_outputqueuestats();
            auto oq_stats = &qos_queue_stats->oq_stats[i].stats;
            output_stats->set_outputqueueidx(qos_queue_stats->oq_stats[i].oq.queue);
            output_stats->set_queuedepth(oq_stats->queue_depth);
            output_stats->set_portmonitor(oq_stats->port_monitor);
        }
    }

    rsp->set_apistatus(types::ApiStatus::API_STATUS_OK);
}

static inline void
pds_table_stats_to_proto (pds::TableStats *proto_stats,
                          sdk::table::sdk_table_stats_t *stats)
{
    auto entry = proto_stats->add_entry();
    entry->set_type(pds::TABLE_STATS_ENTRIES);
    entry->set_count(stats->entries);

    entry = proto_stats->add_entry();
    entry->set_type(pds::TABLE_STATS_COLLISIONS);
    entry->set_count(stats->collisions);
}

static inline void
pds_table_api_stats_to_proto (pds::TableApiStats *proto_stats,
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

static inline void
pds_table_stats_entry_to_proto (pds_table_stats_t *stats, void *ctxt)
{
    pds::TableStatsGetResponse *rsp = (pds::TableStatsGetResponse *)ctxt;
    auto response = rsp->add_response();
    auto api_stats = response->mutable_apistats();
    auto table_stats = response->mutable_tablestats();

    response->set_tablename(stats->table_name);
    pds_table_api_stats_to_proto(api_stats, &stats->api_stats);
    pds_table_stats_to_proto(table_stats, &stats->table_stats);
    rsp->set_apistatus(types::ApiStatus::API_STATUS_OK);
}

static inline void
pds_flow_stats_summary_to_proto (pds_flow_stats_summary_t *flow_stats,
                                 void *ctxt)
{
    pds::FlowStatsSummaryResponse *proto_rsp =
        (pds::FlowStatsSummaryResponse *)ctxt;

    proto_rsp->set_numtcpv4sessions(flow_stats->value[
                                    FLOW_STATS_TCPV4_SESSION_COUNT]);
    proto_rsp->set_numudpv4sessions(flow_stats->value[
                                    FLOW_STATS_UDPV4_SESSION_COUNT]);
    proto_rsp->set_numicmpv4sessions(flow_stats->value[
                                     FLOW_STATS_ICMPV4_SESSION_COUNT]);
    proto_rsp->set_numotheripv4sessions(flow_stats->value[
                                        FLOW_STATS_OTHERV4_SESSION_COUNT]);
    proto_rsp->set_numtcpv6sessions(flow_stats->value[
                                    FLOW_STATS_TCPV6_SESSION_COUNT]);
    proto_rsp->set_numudpv6sessions(flow_stats->value[
                                    FLOW_STATS_UDPV6_SESSION_COUNT]);
    proto_rsp->set_numicmpv6sessions(flow_stats->value[
                                     FLOW_STATS_ICMPV6_SESSION_COUNT]);
    proto_rsp->set_numotheripv6sessions(flow_stats->value[
                                        FLOW_STATS_OTHERV6_SESSION_COUNT]);
    proto_rsp->set_numl2sessions(flow_stats->value[
                                 FLOW_STATS_L2_SESSION_COUNT]);
    proto_rsp->set_numsessionerrors(flow_stats->value[
                                    FLOW_STATS_ERROR_SESSION_COUNT]);
}

#endif    //__AGENT_SVC_DEBUG_SVC_HPP__
