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
#include "nic/include/base.h"
#include "sdk/catalog.hpp"

// 320 bytes in one TM cell
#define HAL_TM_CELL_SIZE            320
#define HAL_TM_HBM_FIFO_ALLOC_SIZE  64 // Min size of the HBM fifo in bytes
#define HAL_TM_MAX_HBM_ETH_CONTEXTS 32
#define HAL_TM_MAX_HBM_DMA_CONTEXTS 16
#define HAL_TM_MAX_HBM_CONTEXTS     (HAL_TM_MAX_HBM_ETH_CONTEXTS > HAL_TM_MAX_HBM_DMA_CONTEXTS ? \
                                    HAL_TM_MAX_HBM_ETH_CONTEXTS : HAL_TM_MAX_HBM_DMA_CONTEXTS)
#define HAL_TM_COUNT_L0_NODES       32
#define HAL_TM_COUNT_L1_NODES       16
#define HAL_TM_COUNT_L2_NODES       4
#define HAL_TM_MAX_DSCP_VALS        64
#define HAL_TM_NUM_BUFFER_ISLANDS   2

#define HAL_TM_BUFFER_ISLAND_0_CELL_COUNT 8192
#define HAL_TM_BUFFER_ISLAND_1_CELL_COUNT 5120 

#define HAL_TM_INVALID_Q            -1

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
//      - Traffic from Uplink will enter P4-ig with OQs 24-31 (HAL_TM_P4_UPLINK_IQ_OFFSET)
//        and get remapped to OQs 24-29 and 16-17 by the qos-table
//        (HAL_TM_P4_UPLINK_EGRESS_OQ_OFFSET)
//
// For recirc, oq 31 (TM_P4_IG_RECIRC_QUEUE) is used in P4-ig and oq 29 is used in P4-eg
//
// Offset at which the uplink queues start at P4-ig
#define HAL_TM_P4_UPLINK_IQ_OFFSET          24 
// Offset at which the uplink queues start at P4-eg
#define HAL_TM_P4_UPLINK_EGRESS_OQ_OFFSET   16
// Offset at which the oqs for rxdma only classes start
#define HAL_TM_RXDMA_OQ_OFFSET              16

#define HAL_TM_P4_SPAN_QUEUE                            30
#define HAL_TM_P4_CPU_COPY_QUEUE                        31
/* On P4 EG, due to the resercation of queues 30 and 31 for span and cpu-copy,
 * the uplink queues which are using these values in p4-ig need to use the 
 * below two values
 */
#define HAL_TM_P4_EG_UPLINK_SPAN_QUEUE_REPLACEMENT      (HAL_TM_P4_UPLINK_EGRESS_OQ_OFFSET + 0)
#define HAL_TM_P4_EG_UPLINK_CPU_COPY_QUEUE_REPLACEMENT  (HAL_TM_P4_UPLINK_EGRESS_OQ_OFFSET + 1) 

typedef uint32_t tm_port_t;
typedef int32_t tm_q_t;

#define TM_PORT_TYPES(ENTRY)                                \
    ENTRY(TM_PORT_TYPE_UPLINK,      0, "uplink")            \
    ENTRY(TM_PORT_TYPE_P4IG,        1, "p4ig")              \
    ENTRY(TM_PORT_TYPE_P4EG,        2, "p4eg")              \
    ENTRY(TM_PORT_TYPE_DMA,         3, "dma")               \
    ENTRY(NUM_TM_PORT_TYPES,        4, "num-tm-port-types")

DEFINE_ENUM(tm_port_type_e, TM_PORT_TYPES)
#undef TM_PORT_TYPES

#define TM_HBM_FIFO_TYPES(ENTRY) \
    ENTRY(TM_HBM_FIFO_TYPE_UPLINK,      0, "uplink-hbm-fifo") \
    ENTRY(TM_HBM_FIFO_TYPE_TXDMA,       1, "txdma-hbm-fifo") \
    ENTRY(NUM_TM_HBM_FIFO_TYPES,        2, "num-tm-hbm-fifo-types")

DEFINE_ENUM(tm_hbm_fifo_type_e, TM_HBM_FIFO_TYPES)
#undef TM_HBM_FIFO_TYPES

static inline bool
capri_tm_q_valid (tm_q_t tm_q)
{
    if (tm_q < 0) {
        return false;
    }
    return true;
}

bool 
capri_tm_port_is_uplink_port(uint32_t port);
bool 
capri_tm_port_is_dma_port(uint32_t port);

// APIs to update the hardware
typedef struct tm_uplink_iq_params_s {
    uint32_t mtu;
    uint32_t xoff_threshold;
    uint32_t xon_threshold;
    tm_q_t   p4_q;
} __PACK__ tm_uplink_iq_params_t;

hal_ret_t
capri_tm_uplink_iq_params_update(tm_port_t port,
                                 tm_q_t iq,
                                 tm_uplink_iq_params_t *iq_params);

hal_ret_t
capri_tm_uplink_input_map_update(tm_port_t port,
                                 uint32_t dot1q_pcp,
                                 tm_q_t iq);

typedef struct tm_uplink_input_dscp_map_s {
    bool        ip_dscp[HAL_TM_MAX_DSCP_VALS];
    uint32_t    dot1q_pcp;
} tm_uplink_input_dscp_map_t;

hal_ret_t
capri_tm_uplink_input_dscp_map_update(tm_port_t port,
                                      tm_uplink_input_dscp_map_t *dscp_map);

hal_ret_t
capri_tm_uplink_oq_update(tm_port_t port,
                          tm_q_t oq,
                          bool xoff_enable,
                          uint32_t xoff_cos);

#define TM_SCHED_TYPES(ENTRY)                    \
    ENTRY(TM_SCHED_TYPE_DWRR,       0, "dwrr")   \
    ENTRY(TM_SCHED_TYPE_STRICT,     1, "strict")

DEFINE_ENUM(tm_sched_type_e, TM_SCHED_TYPES);
#undef TM_SCHED_TYPES

typedef struct tm_queue_node_params_s {
    uint32_t            parent_node;
    tm_sched_type_e     sched_type;
    union {
        struct {
            uint32_t  weight;
        } dwrr;
        struct {
            uint32_t  rate;
        } strict;
    };
} tm_queue_node_params_t;

typedef enum {
    TM_QUEUE_NODE_TYPE_LEVEL_0,
    TM_QUEUE_NODE_TYPE_LEVEL_1,
    TM_QUEUE_NODE_TYPE_LEVEL_2,
} tm_queue_node_type_e;

/** capri_tm_scheduler_map_update
 * API to update the output queue scheduler
 *
 * @param port[in]: TM port number
 * @param scheduler_map[in]: The mapping between the nodes of the Hierarchical
 *                           queue scheduler
 * @return hal_ret_t: Status of the operation
 */
hal_ret_t 
capri_tm_scheduler_map_update(tm_port_t port,
                              tm_queue_node_type_e node_type,
                              uint32_t node,
                              tm_queue_node_params_t *node_params);

/** capri_tm_uplink_lif_set
 * API to program the lif value on an uplink port
 *
 * @param port[in]: TM port number
 * @param lif[in]: The lif value to set
 * @return hal_ret_t: Status of the operation
 */
hal_ret_t 
capri_tm_uplink_lif_set(tm_port_t port,
                        uint32_t lif);

/** capri_tm_hw_config_load_poll
 * API to poll the config load completion
 *
 */
hal_ret_t 
capri_tm_hw_config_load_poll(int phase);

/** capri_tm_asic_init
 * API to call asic initialization routines for the PBC block 
 *
 * @return hal_ret_t: Status of the operation
 */
hal_ret_t
capri_tm_asic_init(void);

/** capri_tm_init
 * API to initialize the PBC block in hardware 
 *
 * @return hal_ret_t: Status of the operation
 */
hal_ret_t
capri_tm_init(sdk::lib::catalog* catalog);

/** capri_tm_repl_table_base_addr_set
 * API to program the base address in HBM for the replication table
 *
 * @param addr[in]: HBM Base address for the replication table
 * @return hal_ret_t: Status of the operation
 */
hal_ret_t
capri_tm_repl_table_base_addr_set(uint64_t addr);

/** capri_tm_repl_table_token_size_set
 * API to program the size of the replication token
 *
 * @param num_tokens[in]: Size of the token in bits
 * @return hal_ret_t: Status of the operation
 */
hal_ret_t
capri_tm_repl_table_token_size_set(uint32_t size_in_bits);

/** capri_tm_get_clock_tick
 * API to get the hardware clock tick.
 *
 * @param  tick[in]: Pointer to store the returned hardware clock.
 * @return hal_ret_t: Status of the operation
 */
hal_ret_t
capri_tm_get_clock_tick(uint64_t *tick);

/** capri_tm_enable_disable_uplink_port
 * API to enable/disable an uplink port. Need to be called for link up/down 
 * events etc
 */
hal_ret_t
capri_tm_enable_disable_uplink_port (tm_port_t port, bool enable);

void
capri_tm_dump_debug_regs(void);
void
capri_tm_dump_config_regs(void);
void
capri_tm_dump_all_regs(void);

typedef struct tm_iq_stats_s {
    uint64_t good_pkts_in;
    uint64_t good_pkts_out;
    uint64_t errored_pkts_in;
    uint32_t oflow_fifo_depth;
    uint32_t max_oflow_fifo_depth;
    uint32_t buffer_occupancy;
} __PACK__ tm_iq_stats_t;

hal_ret_t
capri_tm_get_iq_stats(tm_port_t port, tm_q_t iq, tm_iq_stats_t *iq_stats);

hal_ret_t
capri_tm_reset_iq_stats(tm_port_t port, tm_q_t iq);

typedef struct tm_oq_stats_s {
    uint32_t queue_depth;
} __PACK__ tm_oq_stats_t;

hal_ret_t
capri_tm_get_oq_stats(tm_port_t port, tm_q_t oq, tm_oq_stats_t *oq_stats);

hal_ret_t
capri_tm_periodic_stats_update(void);

#endif
