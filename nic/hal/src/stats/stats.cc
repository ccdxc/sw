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

using sys::SystemResponse;

namespace hal {

static thread_local void *t_stats_timer;

#define HAL_STATS_COLLECTION_INTVL            ((1 * TIME_MSECS_PER_SEC) / 2)  // 500 msec
#define HAL_STATS_START_INTVL                 (120 * TIME_MSECS_PER_SEC)    // 2 minutes

#if 0
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
#endif

//------------------------------------------------------------------------------
// callback invoked by the HAL periodic thread for stats collection
//------------------------------------------------------------------------------
static void
stats_timer_cb (void *timer, uint32_t timer_id, void *ctxt)
{
    hal_ret_t ret;
    SystemResponse                      rsp;
#if 0
    pd::pd_system_args_t                pd_system_args;
    pd::pd_system_drop_stats_get_args_t drop_args;
    pd::pd_func_args_t                  pd_func_args;
#endif

    ret = pd::hal_pd_call(pd::PD_FUNC_ID_QOS_CLASS_PERIODIC_STATS_UPDATE, NULL);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error in updating qos periodic stats, ret {}", ret);
    }

    ret = linkmgr::port_metrics_update();
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error in updating port metrics, ret {}", ret);
    }

}

#if 0
static void
hal_global_stats_init (void)
{
    // Register to publish Drop stats
    delphi::objects::DropMetrics::CreateTable();
    delphi::objects::EgressDropMetrics::CreateTable();
}
#endif

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
