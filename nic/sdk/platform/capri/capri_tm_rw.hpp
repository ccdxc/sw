// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
/*
 * capri_tm_rw.hpp
 * Vasanth Kumar (Pensando Systems)
 */

#ifndef __CAPRI_TM_RW_HPP__
#define __CAPRI_TM_RW_HPP__

#include <stdio.h>
#include <string>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include "include/sdk/base.hpp"
#include "include/sdk/qos.hpp"
#include "lib/indexer/indexer.hpp"
#include "lib/catalog/catalog.hpp"
#include "asic/asic.hpp"
#include "platform/capri/capri_p4.hpp"

namespace sdk {
namespace platform {
namespace capri {

using sdk::lib::indexer;

// 320 bytes in one TM cell
#define CAPRI_TM_CELL_SIZE            320
#define CAPRI_TM_HBM_FIFO_ALLOC_SIZE  64 // Min size of the HBM fifo in bytes
#define CAPRI_TM_MAX_HBM_ETH_CONTEXTS 32
#define CAPRI_TM_MAX_HBM_DMA_CONTEXTS 16
#define CAPRI_TM_MAX_HBM_CONTEXTS     (CAPRI_TM_MAX_HBM_ETH_CONTEXTS > CAPRI_TM_MAX_HBM_DMA_CONTEXTS ? \
                                    CAPRI_TM_MAX_HBM_ETH_CONTEXTS : CAPRI_TM_MAX_HBM_DMA_CONTEXTS)
#define CAPRI_TM_COUNT_L0_NODES       32
#define CAPRI_TM_COUNT_L1_NODES       16
#define CAPRI_TM_COUNT_L2_NODES       4
#define CAPRI_TM_MAX_DSCP_VALS        64
#define CAPRI_TM_NUM_BUFFER_ISLANDS   2
#define CAPRI_TM_MAX_SCHED_NODES      16
#define CAPRI_TM_MAX_SCHED_NODE_INPUTS 32
#define CAPRI_TM_MAX_PORTS             12
#define CAPRI_TM_DWRR_UNIT             10000
#define CAPRI_TM_SCHED_TIMER           5000
#define CAPRI_TM_CLK_PERIOD            833
#define CAPRI_TM_BUS_WIDTH             512
#define CAPRI_TM_MAX_QUEUES            32

#define CAPRI_TM_MAX_IQS               32
#define CAPRI_TM_MAX_OQS               32

#define CAPRI_TM_BUFFER_ISLAND_0_CELL_COUNT 8192
#define CAPRI_TM_BUFFER_ISLAND_1_CELL_COUNT 5120

#define CAPRI_TM_INVALID_Q            -1

#define CAPRI_TM_SCHEDULER_RATE_REFRESH_INTERVAL_US   4000 // 4000us
#define CAPRI_TM_CLOCK_SPEED_MHZ                      833 // 833MHz

#define CAPRI_TM_MAX_INTERPIPE_HDR_SZ CAPRI_TM_CELL_SIZE

// thresholds relative to payload size
#define CAPRI_TM_DEFAULT_RELATIVE_XON_THRESHOLD_BYTES  (64*2000)
#define CAPRI_TM_DEFAULT_RELATIVE_XOFF_THRESHOLD_BYTES (64*1000)
#define CAPRI_TM_DEFAULT_XON_THRESHOLD  0x4B8800
#define CAPRI_TM_DEFAULT_XOFF_THRESHOLD 0x4C8200

// There are 32 queues at both P4-ig and P4-eg. The idea is to
// maintain the same queue when pkt goes through the pipeline in P4-ig and
// P4-eg. However, hardware imposes few restrictions on the queue usage:
//      - In the P4-ig port, the uplink traffic can use only 24-31 oqs
//      - In the P4-eg port, 30 and 31 oqs are reserved for SPAN and CPU-COPY
//      - In both the ports one oq is needed for recirc cases.
//
// To accomodate these, the queue allocation is done as follows:
//      - Traffic from TxDMA will enter P4-ig with OQs 0-15, and carry over the
//        same while going to P4-eg
//      - Traffic from Uplink will enter P4-ig with OQs 24-31 (CAPRI_TM_P4_UPLINK_IQ_OFFSET)
//        and get remapped to OQs 24-29 and 16-17 by the qos-table
//        (CAPRI_TM_P4_UPLINK_EGRESS_OQ_OFFSET)
//
// For recirc, oq 31 (TM_P4_IG_RECIRC_QUEUE) is used in P4-ig and oq 29 is used in P4-eg
//
// Offset at which the uplink queues start at P4-ig
#define CAPRI_TM_P4_UPLINK_IQ_OFFSET          24
// Offset at which the uplink queues start at P4-eg
#define CAPRI_TM_P4_UPLINK_EGRESS_OQ_OFFSET   16
// Offset at which the oqs for rxdma only classes start
#define CAPRI_TM_RXDMA_OQ_OFFSET              16

#define CAPRI_TM_P4_CPU_COPY_QUEUE                        30
#define CAPRI_TM_P4_SPAN_QUEUE                            31
/* On P4 EG, due to the resercation of queues 30 and 31 for span and cpu-copy,
 * the uplink queues which are using these values in p4-ig need to use the
 * below two values
 */
#define CAPRI_TM_P4_EG_UPLINK_CPU_COPY_QUEUE_REPLACEMENT  (CAPRI_TM_P4_UPLINK_EGRESS_OQ_OFFSET + 0)
#define CAPRI_TM_P4_EG_UPLINK_SPAN_QUEUE_REPLACEMENT      (CAPRI_TM_P4_UPLINK_EGRESS_OQ_OFFSET + 1)

// SWM related defines
#define CAPRI_TM_NUM_BMC_QUEUES                 2   // NCSI and Pass-through Qs
#define CAPRI_TM_P4_SWM_NCSI_QUEUE              29
#define CAPRI_TM_P4_SWM_UC_QUEUE                CAPRI_TM_P4_CPU_COPY_QUEUE  // 30
#define CAPRI_TM_P4_SWM_UC_QUEUE_REPLACEMENT    CAPRI_TM_P4_EG_UPLINK_CPU_COPY_QUEUE_REPLACEMENT  // 16
#define CAPRI_TM_P4_SWM_FLOOD_QUEUE             CAPRI_TM_P4_SPAN_QUEUE      // 31
#define CAPRI_TM_P4_SWM_FLOOD_QUEUE_REPLACEMENT CAPRI_TM_P4_EG_UPLINK_SPAN_QUEUE_REPLACEMENT      // 17

#define QOS_SWM_CAM_ENTRY       0
#define QOS_SWM_CAM_COS         (CAPRI_TM_P4_SWM_UC_QUEUE - CAPRI_TM_P4_UPLINK_IQ_OFFSET)   // TC 6
#define QOS_SWM_CAM_NCSI_COS    (CAPRI_TM_P4_SWM_NCSI_QUEUE - CAPRI_TM_P4_UPLINK_IQ_OFFSET) // TC 5
#define QOS_SWM_NCSI_ETHERTYPE  0x88F8

#define QOS_QUEUE_DEFAULT 0
#define QOS_COS_DEFAULT   0
#define QOS_UPLINK_IQ_START_INDEX   0
#define QOS_UPLINK_IQ_END_INDEX     7

typedef uint32_t tm_port_t;
typedef int32_t tm_q_t;

#define TM_PORT_TYPES(ENTRY)                                \
    ENTRY(TM_PORT_TYPE_UPLINK,      0, "uplink")            \
    ENTRY(TM_PORT_TYPE_P4IG,        1, "p4ig")              \
    ENTRY(TM_PORT_TYPE_P4EG,        2, "p4eg")              \
    ENTRY(TM_PORT_TYPE_DMA,         3, "dma")               \
    ENTRY(NUM_TM_PORT_TYPES,        4, "num-tm-port-types")

SDK_DEFINE_ENUM(tm_port_type_e, TM_PORT_TYPES)
#undef TM_PORT_TYPES

#define TM_HBM_FIFO_TYPES(ENTRY) \
    ENTRY(TM_HBM_FIFO_TYPE_UPLINK,      0, "uplink-hbm-fifo") \
    ENTRY(TM_HBM_FIFO_TYPE_TXDMA,       1, "txdma-hbm-fifo") \
    ENTRY(NUM_TM_HBM_FIFO_TYPES,        2, "num-tm-hbm-fifo-types")

SDK_DEFINE_ENUM(tm_hbm_fifo_type_e, TM_HBM_FIFO_TYPES)
#undef TM_HBM_FIFO_TYPES

static inline bool
capri_tm_q_valid (tm_q_t tm_q)
{
    if (tm_q < 0) {
        return false;
    }
    return true;
}

bool capri_tm_port_is_uplink_port(uint32_t port);
bool capri_tm_port_is_dma_port(uint32_t port);

// APIs to update the hardware
typedef struct tm_uplink_iq_params_s {
    uint32_t mtu;
    uint32_t xoff_threshold;
    uint32_t xon_threshold;
    tm_q_t   p4_q;
} __PACK__ tm_uplink_iq_params_t;

sdk_ret_t capri_tm_uplink_iq_no_drop_update (tm_port_t port, tm_q_t iq,
                                             bool no_drop);
sdk_ret_t capri_tm_uplink_iq_params_update(tm_port_t port, tm_q_t iq,
                                           tm_uplink_iq_params_t *iq_params);

sdk_ret_t capri_tm_uplink_input_map_update(tm_port_t port, uint32_t dot1q_pcp,
                                           tm_q_t iq);

typedef struct tm_uplink_input_dscp_map_s {
    bool        ip_dscp[CAPRI_TM_MAX_DSCP_VALS];
    uint32_t    dot1q_pcp;
} tm_uplink_input_dscp_map_t;

sdk_ret_t capri_tm_uplink_input_dscp_map_update(tm_port_t port,
                                                tm_uplink_input_dscp_map_t *dscp_map);
sdk_ret_t capri_tm_uplink_oq_update(tm_port_t port, tm_q_t oq,
                                    uint32_t xoff_cos);
sdk_ret_t capri_tm_set_uplink_mac_xoff(tm_port_t port, bool reset_all_xoff,
                                       bool set_all_xoff, bool reset_pfc_xoff,
                                       bool set_pfc_xoff,
                                       uint32_t xoff_cos_bitmap);
sdk_ret_t capri_tm_get_uplink_mac_xoff (tm_port_t port,
                                        uint32_t *xoff_cos_bitmap);
sdk_ret_t capri_tm_get_uplink_oq_xoff_map(tm_port_t port, tm_q_t oq,
                                          uint32_t *xoff_cos);
sdk_ret_t capri_tm_uplink_set_cam_type (tm_port_t port,
                                        uint32_t  entry,
                                        uint32_t  etype);
sdk_ret_t capri_tm_uplink_set_cam_da (tm_port_t port, uint32_t entry,
                                      uint64_t dmac);
sdk_ret_t capri_tm_uplink_set_cam_cos (tm_port_t port, uint32_t entry,
                                       uint32_t cos);
sdk_ret_t capri_tm_set_uplink_iq_to_p4_oq_map (tm_port_t port, tm_q_t iq,
                                               tm_q_t p4_q);

#define TM_SCHED_TYPES(ENTRY)                    \
    ENTRY(TM_SCHED_TYPE_DWRR,       0, "dwrr")   \
    ENTRY(TM_SCHED_TYPE_STRICT,     1, "strict")

SDK_DEFINE_ENUM(tm_sched_type_e, TM_SCHED_TYPES);
#undef TM_SCHED_TYPES

typedef struct tm_queue_node_params_s {
    uint32_t            parent_node;
    tm_sched_type_e     sched_type;
    union {
        struct {
            uint32_t  weight;
        } dwrr;
        struct {
            uint64_t  rate;
        } strict;
    };
} tm_queue_node_params_t;

typedef enum tm_queue_node_type_e {
    TM_QUEUE_NODE_TYPE_LEVEL_1,
    TM_QUEUE_NODE_TYPE_LEVEL_2,
    TM_QUEUE_NODE_TYPE_LEVEL_3,
    TM_QUEUE_NODE_TYPE_LEVEL_MAX,
} tm_queue_node_type_t;

/** capri_tm_scheduler_map_update
 * API to update the output queue scheduler
 *
 * @param port[in]: TM port number
 * @param scheduler_map[in]: The mapping between the nodes of the Hierarchical
 *                           queue scheduler
 * @return sdk_ret_t: Status of the operation
 */
sdk_ret_t capri_tm_scheduler_map_update(tm_port_t port,
                                        tm_queue_node_type_e node_type,
                                        uint32_t node,
                                        tm_queue_node_params_t *node_params);

/** capri_tm_uplink_lif_set
 * API to program the lif value on an uplink port
 *
 * @param port[in]: TM port number
 * @param lif[in]: The lif value to set
 * @return sdk_ret_t: Status of the operation
 */
sdk_ret_t capri_tm_uplink_lif_set(tm_port_t port, uint32_t lif);

/** capri_tm_hw_config_load_poll
 * API to poll the config load completion
 *
 */
sdk_ret_t capri_tm_hw_config_load_poll(int phase);

/** capri_tm_asic_init
 * API to call asic initialization routines for the PBC block
 *
 * @return sdk_ret_t: Status of the operation
 */
sdk_ret_t capri_tm_asic_init(void);

/** capri_tm_init
 * API to initialize the PBC block in hardware
 *
 * @return sdk_ret_t: Status of the operation
 */
sdk_ret_t capri_tm_init(sdk::lib::catalog* catalog,
                        sdk::lib::qos_profile_t *qos_profile);

/** capri_tm_repl_table_base_addr_set
 * API to program the base address in HBM for the replication table
 *
 * @param addr[in]: HBM Base address for the replication table
 * @return sdk_ret_t: Status of the operation
 */
sdk_ret_t capri_tm_repl_table_base_addr_set(uint64_t addr);

/** capri_tm_repl_table_token_size_set
 * API to program the size of the replication token
 *
 * @param num_tokens[in]: Size of the token in bits
 * @return sdk_ret_t: Status of the operation
 */
sdk_ret_t capri_tm_repl_table_token_size_set(uint32_t size_in_bits);

/** capri_tm_get_clock_tick
 * API to get the hardware clock tick.
 *
 * @param  tick[in]: Pointer to store the returned hardware clock.
 * @return sdk_ret_t: Status of the operation
 */
sdk_ret_t capri_tm_get_clock_tick(uint64_t *tick);

/** capri_tm_enable_disable_uplink_port
 * API to enable/disable an uplink port. Need to be called for link up/down
 * events etc
 */
sdk_ret_t capri_tm_enable_disable_uplink_port(tm_port_t port, bool enable);
sdk_ret_t capri_tm_flush_uplink_port(tm_port_t port, bool enable);
sdk_ret_t capri_tm_write_control_uplink_port(tm_port_t port, bool enable);
sdk_ret_t capri_tm_drain_uplink_port(tm_port_t port);

/**
 * return front panel port number give TM/PB port number
 */
static inline int
capri_tm_port_to_fp_port (uint32_t tm_port)
{
    SDK_ASSERT((tm_port >= CAPRI_TM_UPLINK_PORT_BEGIN) &&
               (tm_port <= CAPRI_TM_UPLINK_PORT_END));
    return tm_port - CAPRI_TM_UPLINK_PORT_BEGIN + 1;
}

void capri_tm_dump_debug_regs(void);
void capri_tm_dump_config_regs(void);

typedef struct tm_iq_oflow_fifo_stats_s {
    uint64_t good_pkts_in;
    uint64_t good_pkts_out;
    uint64_t errored_pkts_in;
    uint32_t fifo_depth;
    uint32_t max_fifo_depth;
} tm_iq_oflow_fifo_stats_t;

typedef struct tm_iq_stats_s {
    tm_iq_oflow_fifo_stats_t oflow;
    uint32_t                 buffer_occupancy;
    uint32_t                 peak_occupancy;
    uint64_t                 port_monitor;
} __PACK__ tm_iq_stats_t;

sdk_ret_t capri_tm_get_iq_stats(tm_port_t port, tm_q_t iq,
                                tm_iq_stats_t *iq_stats);

sdk_ret_t capri_tm_reset_iq_stats(tm_port_t port, tm_q_t iq);

typedef struct tm_oq_stats_s {
    uint32_t queue_depth;
    uint64_t port_monitor;
} __PACK__ tm_oq_stats_t;

sdk_ret_t capri_tm_get_oq_stats(tm_port_t port, tm_q_t oq,
                                tm_oq_stats_t *oq_stats);

sdk_ret_t capri_tm_periodic_stats_update(void);

typedef struct pb_sched_node_input_info_s {
    uint32_t weight;
    uint64_t cfg_quota;
    float    dwrr_ratio;

    // strict priotiry Q params
    bool     is_strict;
    uint64_t priority_bypass_timer;
    uint64_t sp_rate_mbps;
} pb_sched_node_input_info_t;

typedef struct pb_sched_node_input_cfg_s {
    uint32_t  num_inputs;
    uint64_t  total_weights;
    indexer   *inputs_bitmap;
    pb_sched_node_input_info_t input_info[
                                CAPRI_TM_MAX_SCHED_NODE_INPUTS];
} pb_sched_node_input_cfg_t;

typedef struct pb_sched_node_cfg_s {
    tm_queue_node_type_t      node_type;
    uint32_t                  num_nodes;
    indexer                   *node_bitmap;
    pb_sched_node_input_cfg_t input_cfg[CAPRI_TM_MAX_SCHED_NODES];
} pb_sched_node_cfg_t;

typedef struct pb_sched_port_cfg_s {
    pb_sched_node_cfg_t node_cfg[TM_QUEUE_NODE_TYPE_LEVEL_MAX];
} pb_sched_port_cfg_t;

sdk_ret_t capri_tm_debug_stats_get (tm_port_t port,
                                    tm_debug_stats_t *debug_stats, bool reset);

uint32_t capri_tm_get_num_iqs_for_port(tm_port_t port);
uint32_t capri_tm_get_num_oqs_for_port(tm_port_t port);

sdk_ret_t cap_pb_sched_spq_pgm(uint32_t chip_id, uint32_t inst_id,
                               tm_port_t port, tm_q_t oq,
                               pb_sched_node_input_info_t *input_info);
uint32_t capri_tm_get_port_occupancy(tm_port_t port, uint32_t iq);
uint32_t capri_tm_get_xon_threshold(uint32_t ctx);
uint32_t capri_tm_get_xoff_threshold(uint32_t ctx);
uint64_t capri_tm_get_port_mon_out(int chip_id, int inst_id, tm_port_t tm_port,
                                   tm_q_t tm_oq);
uint64_t capri_tm_get_port_mon_in(int chip_id, int inst_id, tm_port_t tm_port,
                                  tm_q_t tm_iq);
// set the span queue threshold
sdk_ret_t capri_tm_set_span_threshold(uint32_t span_threshold);
// set the reserved min for uplink ports
sdk_ret_t capri_tm_set_reserved_min(uint32_t reserved_min);

sdk_ret_t
capri_tm_get_current_credits(tm_port_t tm_port,
                             tm_q_t oq,
                             uint32_t *val);

}    // namespace capri
}    // namespace platform
}    // namespace sdk

#endif    // __CAPRI_TM_RW_HPP__
