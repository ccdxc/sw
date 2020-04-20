//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#ifndef __SDK_QOS_HPP__
#define __SDK_QOS_HPP__

#include "include/sdk/base.hpp"
#include "include/sdk/platform.hpp"

namespace sdk {
namespace qos {

#define SDK_DEFAULT_POLICER_REFRESH_INTERVAL    250 // in usecs
#define SDK_MAX_POLICER_TOKENS_PER_INTERVAL     ((1ull<<39)-1)
#define TM_MAX_DSCP_VALS                        64

#define QOS_CMAP_TYPES(ENTRY)                     \
    ENTRY(QOS_CMAP_TYPE_NONE,         0, "none")  \
    ENTRY(QOS_CMAP_TYPE_PCP,          1, "pcp")   \
    ENTRY(QOS_CMAP_TYPE_DSCP,         2, "dscp")

SDK_DEFINE_ENUM(qos_cmap_type_e, QOS_CMAP_TYPES);
#undef QOS_CMAP_TYPES

#define QOS_IQS(ENTRY)                                        \
    ENTRY(QOS_IQ_COMMON,                 0, "common")         \
    ENTRY(QOS_IQ_TX_UPLINK_GROUP_0,      1, "uplink-group-0") \
    ENTRY(QOS_IQ_TX_UPLINK_GROUP_1,      2, "uplink-group-1") \
    ENTRY(QOS_IQ_RX,                     3, "rx")             \
    ENTRY(QOS_NUM_IQ_TYPES,              4, "num-iq-types")

SDK_DEFINE_ENUM(qos_iq_type_e, QOS_IQS)
#undef QOS_IQS

#define QOS_OQS(ENTRY)                                \
    ENTRY(QOS_OQ_COMMON,                 0, "common") \
    ENTRY(QOS_OQ_RXDMA,                  1, "rxdma")
SDK_DEFINE_ENUM(qos_oq_type_e, QOS_OQS)
#undef QOS_OQS

typedef uint32_t tm_port_t;
typedef int32_t tm_q_t;

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

typedef struct tm_uplink_q_params_s {
    tm_q_t   iq;                           ///< uplink iq
    uint32_t mtu;                          ///< max size for incoming packet
    uint32_t xoff_threshold;               ///< threshold for transmitter off
    uint32_t xon_threshold;                ///< threshold for transmitter on
    tm_q_t   p4_q;                         ///< p4 ingress oq
    uint32_t dot1q_pcp;                    ///< incoming pcp cos value for classification
    bool     no_drop;                      ///< queue is drop or no-drop
    bool     use_ip;                       ///< use ip_dscp for classification
    bool     ip_dscp[TM_MAX_DSCP_VALS];    ///< valid dscp values
} tm_uplink_q_params_t;

typedef struct qos_q_alloc_params_s {
    uint32_t         cnt_uplink_iq;        ///< number of uplink IQs to be allocated
    uint32_t         cnt_txdma_iq;         ///< number of TxDMA IQs to be allocated
    uint32_t         cnt_oq;               ///< number of OQs to be allocated
    qos_oq_type_e    dest_oq_type;         ///< destination OQ type - COMMON/RxDMA
    bool             pcie_oq;              ///< indicates if rxdma oq is towards pcie/hbm
} qos_q_alloc_params_t;

typedef struct qos_cmap_s {
    qos_cmap_type_e type;                       ///< classfication type - PCP/DSCP
    uint32_t        dot1q_pcp;                  ///< pcp cos value
    bool            ip_dscp[TM_MAX_DSCP_VALS];  ///< valid dscp values (0-63)
} __PACK__ qos_cmap_t;

static inline bool
cmap_type_pcp (qos_cmap_type_e type) {
    return type == QOS_CMAP_TYPE_PCP;
}

static inline bool
cmap_type_dscp (qos_cmap_type_e type) {
    return type == QOS_CMAP_TYPE_DSCP;
}

static inline bool
tm_q_valid (tm_q_t tm_q)
{
    return tm_q < 0? false : true;
}

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

}    // namespace qos
}    // namespace sdk

using sdk::qos::tm_port_t;
using sdk::qos::tm_q_t;
using sdk::qos::tm_uplink_q_params_t;
using sdk::qos::qos_q_alloc_params_t;
using sdk::qos::qos_cmap_t;
using sdk::qos::policer_type_t;
using sdk::qos::policer_t;
using sdk::qos::tm_debug_buffer_drop_stats_t;
using sdk::qos::tm_debug_buffer_stats_t;
using sdk::qos::tm_debug_oflow_fifo_drop_stats_t;
using sdk::qos::tm_debug_oflow_fifo_stats_t;
using sdk::qos::tm_debug_stats_t;
using sdk::qos::p4plus_table_params_t;

#endif    // __SDK_QOS_HPP__
