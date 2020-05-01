//------------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include "nic/hal/src/stats/stats.hpp"
#include "lib/periodic/periodic.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/linkmgr/linkmgr.hpp"
#include "gen/proto/system.pb.h"
#include "gen/proto/dropstats/dropstats.delphi.hpp"
#include "gen/proto/dropstats/dropstats.pb.h"
#include "asic/rw/asicrw.hpp"
#include "nic/sdk/lib/catalog/catalog.hpp"

using sys::SystemResponse;
using sdk::lib::slab;

namespace hal {

static thread_local void *t_stats_timer;

static void
hal_update_drop_stats (SystemResponse *rsp) {
    delphi::objects::dropmetrics_t        dm;
    delphi::objects::egressdropmetrics_t  edm;
    sys::DropStats                        *stats = rsp->mutable_stats()->mutable_drop_stats();
    sys::EgressDropStats                  *egstats = rsp->mutable_stats()->mutable_egress_drop_stats();

    bzero(&dm, sizeof(delphi::objects::dropmetrics_t));
    bzero(&edm, sizeof(delphi::objects::egressdropmetrics_t));
    for (int idx = 0; idx < stats->drop_entries_size(); idx++) {
        sys::DropStatsEntry     entry = stats->drop_entries(idx);
        if (entry.reasons().drop_malformed_pkt() == true) {
            dm.drop_malformed_pkt = entry.drop_count();
        } else if (entry.reasons().drop_parser_icrc_error() == true) {
            dm.drop_parser_icrc_error = entry.drop_count();
        } else if (entry.reasons().drop_parse_len_error() == true) {
            dm.drop_parse_len_error = entry.drop_count();
        } else if (entry.reasons().drop_hardware_error() == true) {
            dm.drop_hardware_error = entry.drop_count();
        } else if (entry.reasons().drop_input_mapping() == true) {
            dm.drop_input_mapping = entry.drop_count();
        } else if (entry.reasons().drop_input_mapping_dejavu() == true) {
            dm.drop_input_mapping_dejavu = entry.drop_count();
        } else if (entry.reasons().drop_multi_dest_not_pinned_uplink() == true) {
            dm.drop_multi_dest_not_pinned_uplink = entry.drop_count();
        } else if (entry.reasons().drop_flow_hit() == true) {
            dm.drop_flow_hit = entry.drop_count();
        } else if (entry.reasons().drop_flow_miss() == true) {
            dm.drop_flow_miss = entry.drop_count();
        } else if (entry.reasons().drop_nacl() == true) {
            dm.drop_nacl = entry.drop_count();
        } else if (entry.reasons().drop_ipsg() == true) {
            dm.drop_ipsg = entry.drop_count();
        } else if (entry.reasons().drop_ip_normalization() == true) {
            dm.drop_ip_normalization = entry.drop_count();
        } else if (entry.reasons().drop_tcp_normalization() == true) {
            dm.drop_tcp_normalization = entry.drop_count();
        } else if (entry.reasons().drop_tcp_rst_with_invalid_ack_num() == true) {
            dm.drop_tcp_rst_with_invalid_ack_num = entry.drop_count();
        } else if (entry.reasons().drop_tcp_non_syn_first_pkt() == true) {
            dm.drop_tcp_non_syn_first_pkt = entry.drop_count();
        } else if (entry.reasons().drop_icmp_normalization() == true) {
            dm.drop_icmp_normalization = entry.drop_count();
        } else if (entry.reasons().drop_input_properties_miss() == true) {
            dm.drop_input_properties_miss = entry.drop_count();
        } else if (entry.reasons().drop_tcp_out_of_window() == true) {
            dm.drop_tcp_out_of_window = entry.drop_count();
        } else if (entry.reasons().drop_tcp_split_handshake() == true) {
            dm.drop_tcp_split_handshake = entry.drop_count();
        } else if (entry.reasons().drop_tcp_win_zero_drop() == true) {
            dm.drop_tcp_win_zero_drop = entry.drop_count();
        } else if (entry.reasons().drop_tcp_data_after_fin() == true) {
            dm.drop_tcp_data_after_fin = entry.drop_count();
        } else if (entry.reasons().drop_tcp_non_rst_pkt_after_rst() == true) {
            dm.drop_tcp_non_rst_pkt_after_rst = entry.drop_count();
        } else if (entry.reasons().drop_tcp_invalid_responder_first_pkt() == true) {
            dm.drop_tcp_invalid_responder_first_pkt = entry.drop_count();
        } else if (entry.reasons().drop_tcp_unexpected_pkt() == true) {
            dm.drop_tcp_unexpected_pkt = entry.drop_count();
        } else if (entry.reasons().drop_src_lif_mismatch() == true) {
            dm.drop_src_lif_mismatch = entry.drop_count();
        } else if (entry.reasons().drop_vf_ip_label_mismatch() == true) {
            dm.drop_vf_ip_label_mismatch = entry.drop_count();
        } else if (entry.reasons().drop_vf_bad_rr_dst_ip() == true) {
            dm.drop_vf_bad_rr_dst_ip = entry.drop_count();
        } else if (entry.reasons().drop_icmp_frag_pkt() == true) {
            dm.drop_icmp_frag_pkt = entry.drop_count();
        }
    }
    delphi::objects::DropMetrics::Publish(0, &dm);

    for (int idx = 0; idx < egstats->drop_entries_size(); idx++) {
        sys::EgressDropStatsEntry     entry = egstats->drop_entries(idx);
        if (entry.reasons().drop_output_mapping() == true) {
            edm.drop_output_mapping = entry.drop_count();
        } else if (entry.reasons().drop_prune_src_port() == true) {
            edm.drop_prune_src_port = entry.drop_count();
        } else if (entry.reasons().drop_mirror() == true) {
            edm.drop_mirror = entry.drop_count();
        } else if (entry.reasons().drop_policer() == true) {
            edm.drop_policer = entry.drop_count();
        } else if (entry.reasons().drop_copp() == true) {
            edm.drop_copp = entry.drop_count();
        } else if (entry.reasons().drop_checksum_err() == true) {
            edm.drop_checksum_err = entry.drop_count();
        }
    }
    delphi::objects::EgressDropMetrics::Publish(0, &edm);
}

// update hbm with uplink stats
static void
hal_update_pb_stats (SystemResponse *rsp)
{
    sdk_ret_t ret = SDK_RET_OK;
    int index = 0, drop_entries = 0, oflow_drop_entries = 0;
    uint32_t tm_port, offset_multiplier, len;
    ionic_pb_stats_t pb_stats;
    sdk::types::mem_addr_t uplink_stats_base = INVALID_MEM_ADDRESS;
    sdk::types::mem_addr_t uplink_port_base = INVALID_MEM_ADDRESS;
    sys::PacketBufferPortStats *port_stats = NULL;
    sys::PacketBufferStats *pakbuf_stats = rsp->mutable_stats()->mutable_packet_buffer_stats();

    // get uplink stats ptr in hbm
    uplink_stats_base = g_hal_cfg.mempartition->start_addr("uplink_stats");
    if ((uplink_stats_base == 0) || (uplink_stats_base == INVALID_MEM_ADDRESS)) {
        // nothing to do
        return;
    }

    // TODO: need to generalize with catalog object
    for (tm_port = 0; tm_port < TM_NUM_PORTS; tm_port++) {
        switch (tm_port) {
        case 0:
            offset_multiplier = 0; // logical (inb)  port 1
            break;
        case 1:
            offset_multiplier = 1; // logical (inb) port 5
            break;
        case 8:
            offset_multiplier = 2; // logical (oob) port 9
            break;
        default:
            continue;
        }

        memset(&pb_stats, 0, sizeof(pb_stats));
        port_stats = pakbuf_stats->mutable_port_stats(tm_port);
        pb_stats.sop_count_in = port_stats->mutable_buffer_stats()->sop_count_in();
        pb_stats.eop_count_in = port_stats->mutable_buffer_stats()->eop_count_in();
        pb_stats.sop_count_out = port_stats->mutable_buffer_stats()->sop_count_out();
        pb_stats.eop_count_out = port_stats->mutable_buffer_stats()->eop_count_out();

        auto buff_drop_stats = port_stats->mutable_buffer_stats()->mutable_drop_counts();
        drop_entries = buff_drop_stats->stats_entries_size();
        if (drop_entries > IONIC_BUFFER_DROP_MAX) {
            drop_entries = IONIC_BUFFER_DROP_MAX;
        }

        for (index = 0; index < drop_entries; index++) {
            auto entry = buff_drop_stats->mutable_stats_entries(index);
            pb_stats.drop_counts[index] = entry->drop_count();
        }

        auto oflow_drop_stats = port_stats->mutable_oflow_fifo_stats()->mutable_drop_counts();
        oflow_drop_entries = oflow_drop_stats->entry_size();
        if (drop_entries > IONIC_OFLOW_DROP_MAX) {
            drop_entries = IONIC_OFLOW_DROP_MAX;
        }

        for (index = 0; index < oflow_drop_entries; index++) {
            auto oflow_entry = oflow_drop_stats->mutable_entry(index);
            pb_stats.oflow_drop_counts[index] = oflow_entry->count();
        }

        auto qstats = port_stats->qos_queue_stats();

        // For any queue that is not assigned to a TC, the qos_group_idx will be 0.
        // If we update the stats in queue order 0...7, TC0's stats will be overwritten with the unmapped queue stats.
        // Fixing this by reversing the loop to update stats in the queue order 7...0 so that TC0 stats are updated at the end.
        for (index = IONIC_PORT_QOS_MAX_QUEUES-1; index >= 0; index--) {
            auto iqstats = qstats.input_queue_stats(index);
            auto oqstats = qstats.output_queue_stats(index);

            // iqstats fill
            uint64_t input_tc = iqstats.qos_group_idx(); // Get the Qos Group (TC) index for this iq
            pb_stats.input_queue_buffer_occupancy[input_tc] = iqstats.buffer_occupancy();
            pb_stats.input_queue_port_monitor[input_tc] = iqstats.port_monitor();
            pb_stats.input_queue_peak_occupancy[input_tc] = iqstats.peak_occupancy();
            pb_stats.input_queue_good_pkts_in[input_tc] = iqstats.mutable_oflow_fifo_stats()->good_pkts_in();
            pb_stats.input_queue_good_pkts_out[input_tc] = iqstats.mutable_oflow_fifo_stats()->good_pkts_out();
            pb_stats.input_queue_err_pkts_in[input_tc] = iqstats.mutable_oflow_fifo_stats()->errored_pkts_in();
            pb_stats.input_queue_fifo_depth[input_tc] = iqstats.mutable_oflow_fifo_stats()->fifo_depth();
            pb_stats.input_queue_max_fifo_depth[input_tc] = iqstats.mutable_oflow_fifo_stats()->max_fifo_depth();
            
            // oqstats fill
            uint64_t output_tc = oqstats.qos_group_idx(); // Get the Qos Group (TC) index for this oq
            pb_stats.output_queue_port_monitor[output_tc] = oqstats.port_monitor();
        }

        // ptr offset (not byte len)
        uplink_port_base = uplink_stats_base + (IONIC_PORT_PB_STATS_REPORT_SIZE * offset_multiplier);
        len = sizeof(pb_stats);

        // write to hbm mem
        ret = sdk::asic::asic_mem_write(uplink_port_base, (uint8_t*) &pb_stats, len);
        if (ret != SDK_RET_OK) {
            HAL_TRACE_ERR("update_pb_stats persist mem write fail for tm_port {} ret {}", tm_port, ret);
        }
    }
}

//------------------------------------------------------------------------------
// callback invoked by the HAL periodic thread for stats collection
//------------------------------------------------------------------------------
static void
stats_timer_cb (void *timer, uint32_t timer_id, void *ctxt)
{
    static uint8_t                      periodic_tmr_trig_cnt = 0;
    hal_ret_t ret;
    pd::pd_pb_stats_get_args_t          pb_args;
    pd::pd_system_drop_stats_get_args_t drop_args;
    pd::pd_system_args_t                pd_system_args;
    pd::pd_func_args_t                  pd_func_args;
    SystemResponse                      rsp;

    ret = pd::hal_pd_call(pd::PD_FUNC_ID_QOS_CLASS_PERIODIC_STATS_UPDATE, NULL);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error in updating qos periodic stats, ret {}", ret);
    }

    // linkmgr mac stats
    ret = linkmgr::port_metrics_update();
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error in updating port metrics, ret {}", ret);
    }

    periodic_tmr_trig_cnt++;
    if (periodic_tmr_trig_cnt == HAL_SYSTEM_STATS_COLL_TRIG_DELAY) {
        periodic_tmr_trig_cnt = 0;

        hal::hal_cfg_db_open(hal::CFG_OP_READ);

        bzero(&pd_func_args, sizeof(pd::pd_func_args_t));
        pd::pd_system_args_init(&pd_system_args);
        pd_system_args.rsp = &rsp;

        drop_args.pd_sys_args = &pd_system_args;
        pd_func_args.pd_system_drop_stats_get = &drop_args;
        ret = pd::hal_pd_call(pd::PD_FUNC_ID_SYSTEM_DROP_STATS_GET, &pd_func_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to get drop stats, err : {}", ret);
        }

        hal::hal_cfg_db_close();

        hal_update_drop_stats(&rsp);
    }

    // system PB stats
    if (hal::g_hal_cfg.device_cfg.forwarding_mode == sdk::lib::FORWARDING_MODE_CLASSIC) {
        bzero(&pd_func_args, sizeof(pd::pd_func_args_t));
        bzero(&pd_system_args, sizeof(pd::pd_system_args_t));
        bzero(&pb_args, sizeof(pd::pd_pb_stats_get_args_t));
        pd_system_args.rsp = &rsp;
        pb_args.pd_sys_args = &pd_system_args;
        pd_func_args.pd_pb_stats_get = &pb_args;

        ret = pd::hal_pd_call(pd::PD_FUNC_ID_PB_STATS_GET, &pd_func_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error in updating PB stats, ret {}", ret);
        }
        hal_update_pb_stats(&rsp);
    }
}

static void
hal_global_stats_init (void)
{
    // Register to publish Drop stats
    delphi::objects::DropMetrics::CreateTable();
    delphi::objects::EgressDropMetrics::CreateTable();
}

static void
stats_timer_start (void *timer, uint32_t timer_id, void *ctxt)
{
    t_stats_timer = sdk::lib::timer_schedule(HAL_TIMER_ID_STATS,
                                             HAL_STATS_COLLECTION_INTVL,
                                             (void *)0,    // ctxt
                                             stats_timer_cb, true);
    if (!t_stats_timer) {
        HAL_TRACE_ERR("Failed to start periodic stats timer");
        return;
    }
    HAL_TRACE_DEBUG("Started periodic stats timer with {} ms interval",
                    HAL_STATS_COLLECTION_INTVL);

    hal_global_stats_init ();
}

//------------------------------------------------------------------------------
// stats module initialization callback
//------------------------------------------------------------------------------
hal_ret_t
hal_stats_init (hal_cfg_t *hal_cfg)
{

    // no stats functionality in sim and rtl mode
    if (hal_cfg->platform == platform_type_t::PLATFORM_TYPE_SIM ||
        hal_cfg->platform == platform_type_t::PLATFORM_TYPE_RTL) {
        return HAL_RET_OK;
    }

    // wait until the periodic thread is ready
    while (!sdk::lib::periodic_thread_is_running()) {
        pthread_yield();
    }

    // start a delay timer to make sure ports are created by netagent
    // before collecting stats
    sdk::lib::timer_schedule(HAL_TIMER_ID_STATS,
                             HAL_STATS_START_INTVL,
                             (void *)0,    // ctxt
                             stats_timer_start, false);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// stats module cleanup callback
//------------------------------------------------------------------------------
hal_ret_t
hal_stats_cleanup (void)
{
    return HAL_RET_OK;
}

}    // namespace
