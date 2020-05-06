//------------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#ifndef __STATS_HPP__
#define __STATS_HPP__

#include "nic/include/base.hpp"
#include "nic/include/hal_cfg.hpp"

namespace hal {

#define IONIC_PORT_QOS_MAX_QUEUES  (8)
#define IONIC_PORT_PB_STATS_REPORT_SIZE   (1024)

#define HAL_STATS_COLLECTION_INTVL            ((1 * TIME_MSECS_PER_SEC) / 2)  // 500 msec
#define HAL_STATS_DELPHI_PUBLISH_INTVL        ((1 * TIME_MSECS_PER_SEC))      // 1 sec
#define HAL_STATS_START_INTVL                 (120 * TIME_MSECS_PER_SEC)      // 2 minutes
//system stats collection interval is 20 secs;
//the stats is collected in same timer_cb as period stats, hence
//setting the delay trigger count for system stats collection in
//proportion to periodic stats collection interval.
//Adjust accordingly if the stats interval changes.
#define HAL_SYSTEM_STATS_COLL_TRIG_DELAY      (40)  // 20 secs


// until we figure out how to include ionic_if.h
// need to create replica of interested pb_stats from hal-pd
// ref: $srcroot/platform/drivers/common/ionic_if.h
typedef enum {
    IONIC_BUFFER_INTRINSIC_DROP = 0,        // Pkts dropped due to intrinsic drop bit being set
    IONIC_BUFFER_DISCARDED,                 // Pkts dropped due to error
    IONIC_BUFFER_ADMITTED,                  // Errored pkts admitted to buffering
    IONIC_BUFFER_OUT_OF_CELLS_DROP,         // Pkts dropped due to cell exhaustion (first cell)
    IONIC_BUFFER_OUT_OF_CELLS_DROP_2,       // Pkts dropped due to cell exhaustion (subsequent cell)
    IONIC_BUFFER_OUT_OF_CREDIT_DROP,        // Pkts dropped due to cell-credits exhaustion
    IONIC_BUFFER_TRUNCATION_DROP,           // Pkts dropped due to size bigger than the configured MTU
    IONIC_BUFFER_PORT_DISABLED_DROP,        // Pkts dropped due to port disable
    IONIC_BUFFER_COPY_TO_CPU_TAIL_DROP,     // Copy-to-cpu pkts tail dropped
    IONIC_BUFFER_SPAN_TAIL_DROP,            // Span pkts tail dropped
    IONIC_BUFFER_MIN_SIZE_VIOLATION_DROP,   // Pkts dropped due to lesser than min size
    IONIC_BUFFER_ENQUEUE_ERROR_DROP,        // Pkts dropped due to enqueue to reserved queues
    IONIC_BUFFER_INVALID_PORT_DROP,         // Pkts dropped due to destined to invalid ports
    IONIC_BUFFER_INVALID_OUTPUT_QUEUE_DROP, // Pkts dropped due to destined to invalid output queues
    IONIC_BUFFER_DROP_MAX,
} ionic_pb_buffer_drop_stats_t;

// until we figure out how to include ionic_if.h
// need to create replica of interested oflow_drop_stats from hal-pd
// ref: $srcroot/platform/drivers/common/ionic_if.h
typedef enum {
    IONIC_OFLOW_OCCUPANCY_DROP,                     // Pkts dropped due to fifo full
    IONIC_OFLOW_EMERGENCY_STOP_DROP,                // Pkts dropped due to emergency condition hit due to slow oflow memory to write-buffer communication
    IONIC_OFLOW_WRITE_BUFFER_ACK_FILL_UP_DROP,      // Pkts dropped due to write buffer's ack fifo filling up
    IONIC_OFLOW_WRITE_BUFFER_ACK_FULL_DROP,         // Pkts dropped due to write buffer's ack fifo full
    IONIC_OFLOW_WRITE_BUFFER_FULL_DROP,             // Pkts dropped due to write buffer filling up
    IONIC_OFLOW_CONTROL_FIFO_FULL_DROP,             // Pkts dropped due to control fifo full
    IONIC_OFLOW_DROP_MAX,
} ionic_oflow_drop_stats_t;

/*
 * Packet Buffer statistics
 * Note: This content is frozen from top down
 * Any new entries should be added at end of struct, including new drop_counts
 * This is to maintain host backward compatibility
 * Also must match format of ionic_if.h struct port_pb_stats
 */
typedef struct ionic_pb_stats_s {
    uint64_t   sop_count_in;                       // Count of start-of-packets in
    uint64_t   eop_count_in;                       // Count of end-of-packets in
    uint64_t   sop_count_out;                      // Count of start-of-packets out
    uint64_t   eop_count_out;                      // Count of end-of-packets out
    uint64_t   drop_counts[IONIC_BUFFER_DROP_MAX]; // Drop counts; drop reason is index
    uint64_t   input_queue_buffer_occupancy[IONIC_PORT_QOS_MAX_QUEUES];
    uint64_t   input_queue_port_monitor[IONIC_PORT_QOS_MAX_QUEUES];
    uint64_t   output_queue_port_monitor[IONIC_PORT_QOS_MAX_QUEUES];
    uint64_t   oflow_drop_counts[IONIC_OFLOW_DROP_MAX]; // Oflow Drop counts; drop reason is index
    uint64_t   input_queue_good_pkts_in[IONIC_PORT_QOS_MAX_QUEUES];           // Count of good packets in
    uint64_t   input_queue_good_pkts_out[IONIC_PORT_QOS_MAX_QUEUES];          // Count of good packets out
    uint64_t   input_queue_err_pkts_in[IONIC_PORT_QOS_MAX_QUEUES];            // Count of errored packes in
    uint64_t   input_queue_fifo_depth[IONIC_PORT_QOS_MAX_QUEUES];             // Current FIFO depth
    uint64_t   input_queue_max_fifo_depth[IONIC_PORT_QOS_MAX_QUEUES];         // Max FIFO depth
    uint64_t   input_queue_peak_occupancy[IONIC_PORT_QOS_MAX_QUEUES];	      // Peak buffer occupancy
    uint64_t   output_queue_buffer_occupancy[IONIC_PORT_QOS_MAX_QUEUES];
} ionic_pb_stats_t;


hal_ret_t hal_stats_init(hal_cfg_t *hal_cfg);
hal_ret_t hal_stats_cleanup(void);

}    // namespace

#endif    // __STATS_HPP__
