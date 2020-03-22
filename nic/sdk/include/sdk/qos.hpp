//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#ifndef __SDK_QOS_HPP__
#define __SDK_QOS_HPP__

#include "include/sdk/base.hpp"
#include "include/sdk/platform.hpp"

namespace sdk {

#define SDK_DEFAULT_POLICER_REFRESH_INTERVAL    250 // in usecs
#define SDK_MAX_POLICER_TOKENS_PER_INTERVAL     ((1ull<<39)-1)

typedef enum policer_type_e {
    POLICER_TYPE_NONE = 0,
    POLICER_TYPE_PPS  = 1,    ///< packets-per-second policer
    POLICER_TYPE_BPS  = 2,    ///< bytes-per-second policer
} policer_type_t;

typedef struct policer_s {
    policer_type_t    type;    ///< type of the policer
    uint64_t          rate;    ///< rate in bytes-per-sec or packets-per-sec
    uint64_t          burst;   ///< burst size in bytes or packets
} policer_t;

typedef enum {
    BUFFER_INTRINSIC_DROP = 0,        // Pkts dropped due to intrinsic drop bit being set
    BUFFER_DISCARDED,                 // Pkts dropped due to error
    BUFFER_ADMITTED,                  // Errored pkts admitted to buffering
    BUFFER_OUT_OF_CELLS_DROP,         // Pkts dropped due to cell exhaustion (first cell)
    BUFFER_OUT_OF_CELLS_DROP_2,       // Pkts dropped due to cell exhaustion (subsequent cell)
    BUFFER_OUT_OF_CREDIT_DROP,        // Pkts dropped due to cell-credits exhaustion
    BUFFER_TRUNCATION_DROP,           // Pkts dropped due to size bigger than the configured MTU
    BUFFER_PORT_DISABLED_DROP,        // Pkts dropped due to port disable
    BUFFER_COPY_TO_CPU_TAIL_DROP,     // Copy-to-cpu pkts tail dropped
    BUFFER_SPAN_TAIL_DROP,            // Span pkts tail dropped
    BUFFER_MIN_SIZE_VIOLATION_DROP,   // Pkts dropped due to lesser than min size
    BUFFER_ENQUEUE_ERROR_DROP,        // Pkts dropped due to enqueue to reserved queues
    BUFFER_INVALID_PORT_DROP,         // Pkts dropped due to destined to invalid ports
    BUFFER_INVALID_OUTPUT_QUEUE_DROP, // Pkts dropped due to destined to invalid output queues
    BUFFER_DROP_MAX,
} tm_debug_buffer_drop_stats_t;

typedef struct tm_debug_buffer_stats_s {
    uint32_t sop_count_in;                 // Count of start-of-packets in
    uint32_t eop_count_in;                 // Count of end-of-packets in
    uint32_t sop_count_out;                // Count of start-of-packets out
    uint32_t eop_count_out;                // Count of end-of-packets out
    uint32_t drop_counts[BUFFER_DROP_MAX]; // Drop counts
} tm_debug_buffer_stats_t;

typedef struct oflow_fifo_drop_stats_s {
    uint32_t occupancy_drop_count;                // Pkts dropped due to fifo full
    uint32_t emergency_stop_drop_count;           // Pkts dropped due to emergency condition hit due to slow oflow memory to write-buffer communication
    uint32_t write_buffer_ack_fill_up_drop_count; // Pkts dropped due to write buffer's ack fifo filling up
    uint32_t write_buffer_ack_full_drop_count;    // Pkts dropped due to write buffer's ack fifo full
    uint32_t write_buffer_full_drop_count;        // Pkts dropped due to write buffer filling up
    uint32_t control_fifo_full_drop_count;        // Pkts dropped due to control fifo full
} tm_debug_oflow_fifo_drop_stats_t;

typedef struct oflow_fifo_stats_s {
    uint32_t sop_count_in;  // Count of start-of-packets in
    uint32_t eop_count_in;  // Count of end-of-packets in
    uint32_t sop_count_out; // Count of start-of-packets out
    uint32_t eop_count_out; // Count of end-of-packets out
    tm_debug_oflow_fifo_drop_stats_t drop_counts;   // Drop counts
} tm_debug_oflow_fifo_stats_t;

typedef struct tm_debug_stats_s {
    tm_debug_buffer_stats_t      buffer_stats;
    tm_debug_oflow_fifo_stats_t  oflow_fifo_stats;
} tm_debug_stats_t;

typedef struct p4plus_table_params_s {
    platform_type_t platform_type;
    int stage_apphdr;
    int stage_tableid_apphdr;
    int stage_apphdr_ext;
    int stage_tableid_apphdr_ext;
    int stage_apphdr_off;
    int stage_tableid_apphdr_off;
    int stage_apphdr_ext_off;
    int stage_tableid_apphdr_ext_off;
    int stage_txdma_act;
    int stage_tableid_txdma_act;
    int stage_txdma_act_ext;
    int stage_tableid_txdma_act_ext;
    int stage_sxdma_act;
    int stage_tableid_sxdma_act;
} p4plus_table_params_t;

typedef uint32_t tm_port_t;

static inline sdk_ret_t
policer_to_token_rate (policer_t *policer, uint64_t refresh_interval_us,
                       uint64_t max_policer_tokens_per_interval,
                       uint64_t *token_rate, uint64_t *token_burst)
{
    uint64_t    rate_per_sec = policer->rate;
    uint64_t    burst = policer->burst;
    uint64_t    rate_tokens;

    if (rate_per_sec > UINT64_MAX/refresh_interval_us) {
        SDK_TRACE_ERR("Policer rate %u is too high", rate_per_sec);
        return SDK_RET_INVALID_ARG;
    }
    rate_tokens = (refresh_interval_us * rate_per_sec)/1000000;

    if (rate_tokens == 0) {
        SDK_TRACE_ERR("Policer rate %u too low for refresh interval %uus",
                      rate_per_sec, refresh_interval_us);
        return SDK_RET_INVALID_ARG;
    }

    if ((burst + rate_tokens) > max_policer_tokens_per_interval) {
        SDK_TRACE_ERR("Policer rate %u is too high for "
                      "refresh interval %uus",
                      rate_per_sec, refresh_interval_us);
        return SDK_RET_INVALID_ARG;
    }
    *token_rate = rate_tokens;
    *token_burst = rate_tokens + burst;
    return SDK_RET_OK;
}

static inline sdk_ret_t
policer_token_to_rate (uint64_t token_rate, uint64_t token_burst,
                       uint64_t refresh_interval_us,
                       uint64_t *rate, uint64_t *burst)
{
    *rate = (token_rate * 1000000)/refresh_interval_us;
    *burst = (token_burst - token_rate);
    return SDK_RET_OK;
}

}    // namespace sdk

#endif    // __SDK_QOS_HPP__
