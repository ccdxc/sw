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

// 320 bytes in one TM cell
#define HAL_TM_CELL_SIZE            320
#define HAL_TM_JUMBO_SIZE           9216 // Jumbo pkt size
#define HAL_TM_HBM_FIFO_ALLOC_SIZE  64 // Min size of the HBM fifo in bytes
#define HAL_TM_MAX_HBM_ETH_QS       32
#define HAL_TM_MAX_HBM_DMA_QS       16
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
    ENTRY(TM_PORT_TYPE_BMC,         1, "bmc")               \
    ENTRY(TM_PORT_TYPE_P4_IG,       2, "p4ig")              \
    ENTRY(TM_PORT_TYPE_P4_EG,       3, "p4eg")              \
    ENTRY(TM_PORT_TYPE_DMA,         4, "dma")               \
    ENTRY(NUM_TM_PORT_TYPES,        5, "num-tm-port-types")

DEFINE_ENUM(tm_port_type_e, TM_PORT_TYPES)
#undef TM_PORT_TYPES

static inline uint32_t
capri_tm_get_num_iqs_for_port_type (tm_port_type_e port_type)
{
    switch(port_type) {
        case TM_PORT_TYPE_UPLINK: 
            return 8;
        case TM_PORT_TYPE_BMC:
            return 1;
        case TM_PORT_TYPE_P4_IG:
        case TM_PORT_TYPE_P4_EG:
            return 32;
        case TM_PORT_TYPE_DMA:
            return 16;
        case NUM_TM_PORT_TYPES:
            return 0;
    }
    return 0;
}

static inline uint32_t
capri_tm_get_num_oqs_for_port_type (tm_port_type_e port_type)
{
    return capri_tm_get_num_iqs_for_port_type(port_type);
}


static inline uint32_t
capri_tm_get_island_for_port_type (tm_port_type_e port_type)
{
    switch(port_type) {
        case TM_PORT_TYPE_UPLINK: 
        case TM_PORT_TYPE_BMC:
        case TM_PORT_TYPE_P4_EG:
            return 1;
        case TM_PORT_TYPE_P4_IG:
        case TM_PORT_TYPE_DMA:
            return 0;
        case NUM_TM_PORT_TYPES:
            return 0;
    }

    return 0;
}

static inline uint32_t
capri_tm_get_max_cells_for_island (uint32_t island)
{
    uint32_t cells = 0;
    if (island == 0) {
        cells = HAL_TM_BUFFER_ISLAND_0_CELL_COUNT; 
    } else if (island == 1) {
        cells = HAL_TM_BUFFER_ISLAND_1_CELL_COUNT; 
    }
    return cells;
}

static inline bool
capri_tm_q_valid (tm_q_t tm_q)
{
    if (tm_q < 0) {
        return false;
    }
    return true;
}

tm_port_type_e
capri_tm_get_port_type(tm_port_t port);

uint32_t
capri_tm_num_active_uplink_ports (void);

bool 
capri_tm_port_is_uplink_port(uint32_t port);
bool 
capri_tm_port_is_dma_port(uint32_t port);

static inline uint32_t
capri_tm_buffer_bytes_to_cells (uint32_t bytes)
{
    return (bytes + HAL_TM_CELL_SIZE - 1)/HAL_TM_CELL_SIZE;
}

static inline uint32_t
capri_tm_get_pbc_cells_needed_for_port_type (tm_port_type_e port_type)
{
    // - Allocate 3 Jumbo cells per port in uplink 
    // - Allocate 4 jumbo cells for p4 ig and p4 eg
    // - Allocate 4 Jumbo cells per port in txdma 
    switch(port_type) {
        case TM_PORT_TYPE_UPLINK: 
        case TM_PORT_TYPE_BMC:
            return 3 * capri_tm_buffer_bytes_to_cells(HAL_TM_JUMBO_SIZE);
        case TM_PORT_TYPE_P4_EG:
        case TM_PORT_TYPE_P4_IG:
            return 4 * capri_tm_buffer_bytes_to_cells(HAL_TM_JUMBO_SIZE);
        case TM_PORT_TYPE_DMA:
            return 4 * capri_tm_buffer_bytes_to_cells(HAL_TM_JUMBO_SIZE);
        case NUM_TM_PORT_TYPES:
            return 0;
    }

    return 0;
}

// APIs to update the hardware
/* Pool group parameters */
typedef struct tm_pg_params_s {
    uint32_t reserved_min; // Number of cells for this PG
    uint32_t xon_threshold;
    uint32_t headroom;
    uint32_t low_limit;
    uint32_t alpha;
    uint32_t mtu;
} tm_pg_params_t;

typedef struct tm_hbm_fifo_params_s {
    uint64_t payload_offset; // 64B aligned offset from the HBM base
    uint64_t payload_size; // Size in 64B increments
    uint64_t control_offset;
    uint64_t control_size;
    uint32_t xoff_threshold; // in 64B increments
    uint32_t xon_threshold; // in 64B increments
} tm_hbm_fifo_params_t;

/** capri_tm_pg_params_update
 * API to update the pool group parameters on a Capri TM port
 *
 * @param port[in]: TM port number
 * @param pg[in]: Pool group number
 * @param pg_params[in]: Pool group parameters
 * @return hal_ret_t: Status of the operation
 */
hal_ret_t 
capri_tm_pg_params_update(tm_port_t port,
                          uint32_t pg,
                          tm_pg_params_t *pg_params,
                          tm_hbm_fifo_params_t *hbm_params);

typedef struct tm_tc_to_pg_map_s {
    uint32_t tc;
    uint32_t pg;
} tm_tc_to_pg_map_t;

/** capri_tm_tc_map_update
 * API to update the cos-map parameters on the port
 */
hal_ret_t
capri_tm_tc_map_update(tm_port_t port,
                       uint32_t count,
                       tm_tc_to_pg_map_t *tc_map);

typedef struct tm_uplink_input_map_s {
    bool        ip_dscp[HAL_TM_MAX_DSCP_VALS];
    uint32_t    tc;
    tm_q_t      p4_oq;
} tm_uplink_input_map_t;

hal_ret_t
capri_tm_uplink_input_map_update(tm_port_t port,
                                 tm_uplink_input_map_t *imap);

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
capri_tm_init(void);

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

void
capri_tm_dump_debug_regs(void);
void
capri_tm_dump_config_regs(void);
void
capri_tm_dump_all_regs(void);

#endif
