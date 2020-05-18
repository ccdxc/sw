//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __DEBUG_HPP__
#define __DEBUG_HPP__

#include "nic/include/base.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "gen/proto/debug.pb.h"

using debug::MemTrackGetRequest;
using debug::MemTrackGetResponseMsg;
using debug::SlabGetRequest;
using debug::SlabGetResponseMsg;
using debug::TraceSpec;
using debug::TraceResponse;
using debug::TraceResponseMsg;
using debug::FlushLogsResponse;
using debug::ThreadResponseMsg;
using debug::ThreadResponse;
using debug::FteSpanRequestMsg;
using debug::FteSpanResponseMsg;
using debug::FteSpanRequest;
using debug::FteSpanResponse;
using debug::ClockResponse;
using debug::HbmBwGetRequest;
using debug::HbmBwGetRequestMsg;
using debug::HbmBwGetResponse;
using debug::HbmBwGetResponseMsg;
using debug::LlcSetupRequest;
using debug::LlcSetupRequestMsg;
using debug::LlcSetupResponse;
using debug::LlcSetupResponseMsg;
using debug::LlcGetResponse;
using debug::LlcGetResponseMsg;
using debug::HbmCacheRequest;
using debug::HbmCacheRequestMsg;
using debug::HbmCacheResponse;
using debug::HbmCacheResponseMsg;
using debug::SchedulerStatsResponse;
using debug::SessionCtrlRequestMsg;
using debug::SessionCtrlSpec;
using debug::AgingLogsRequest;

namespace hal {

typedef struct fte_span_s {
    sdk_spinlock_t              slock;
    // types::FTESpanMatchSelector sel;
    uint32_t                    sel;
    types::IPAddressFamily      ip_family;
    uint32_t                    src_lif;
    uint32_t                    src_lport;
    uint32_t                    dst_lport;
    uint32_t                    drop_reason;
    uint32_t                    flow_lkup_dir;
    uint32_t                    flow_lkup_type;
    uint32_t                    flow_lkup_vrf;
    ipvx_addr_t                 flow_lkup_src;
    ipvx_addr_t                 flow_lkup_dst;
    uint32_t                    flow_lkup_proto;
    uint32_t                    flow_lkup_sport;
    uint32_t                    flow_lkup_dport;
    uint64_t                    eth_dmac;
    bool                        from_cpu;
    bool                        is_egress;
    bool                        attach_stats;

    uint32_t                    span_lport;
    void                        *pd;

    hal_handle_t                hal_handle;

} __PACK__ fte_span_t;


hal_ret_t mtrack_get(debug::MemTrackGetRequest& req,
                     debug::MemTrackGetResponseMsg *rsp);

hal_ret_t slab_get_from_req(debug::SlabGetRequest& req,
                            debug::SlabGetResponseMsg *rsp);

hal_ret_t mpu_trace_enable(debug::MpuTraceRequest& req,
                           debug::MpuTraceResponseMsg *rsp);

hal_ret_t register_update(debug::RegisterRequest& req,
                           debug::RegisterResponse *rsp);

hal_ret_t trace_update(debug::TraceSpec& spec, debug::TraceResponse *rsp);
hal_ret_t trace_get(debug::TraceResponseMsg *rsp);
hal_ret_t flush_logs (FlushLogsResponse *rsp);
hal_ret_t thread_get (ThreadResponseMsg *rsp);
hal_ret_t session_ctrl_update(debug::SessionCtrlSpec& spec);

// HBM BW
hal_ret_t hbm_bw_get(const HbmBwGetRequest *req, HbmBwGetResponseMsg *rsp);

// LLC counters
hal_ret_t llc_clear(void);
hal_ret_t llc_setup(const LlcSetupRequest *req, LlcSetupResponse *rsp);
hal_ret_t llc_get(LlcGetResponse *rsp);

hal_ret_t hbm_cache_setup(HbmCacheRequest *req, HbmCacheResponse *rsp);

// FTE span APIs
hal_ret_t fte_span_create(FteSpanRequest& req,
                          FteSpanResponse *rsp);
hal_ret_t fte_span_update(FteSpanRequest& req,
                          FteSpanResponse *rsp);
hal_ret_t fte_span_get(FteSpanResponseMsg *rsp_msg);
hal_ret_t clock_get(ClockResponse *response);

hal_ret_t
scheduler_stats_get(SchedulerStatsResponse *rsp);

hal_ret_t
packet_buffer_update(debug::PacketBufferRequest *req, debug::PacketBufferResponse *rsp);

// enable/disable transceiver valid checks for links
hal_ret_t xcvr_valid_check_enable(bool enable);
// set span queue threshold
hal_ret_t span_threshold_update(uint32_t span_threshold);

hal_ret_t aging_logs_update(debug::AgingLogsRequest& req);

}    // namespace hal

#endif    // __DEBUG_HPP__

