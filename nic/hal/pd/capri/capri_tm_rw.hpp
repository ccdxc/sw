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
#define HAL_TM_CELL_SIZE        320
#define HAL_TM_COUNT_L0_NODES   32
#define HAL_TM_COUNT_L1_NODES   16
#define HAL_TM_COUNT_L2_NODES   4
#define HAL_TM_MAX_DSCP_VALS    64

#define HAL_TM_INVALID_Q        -1
// Offset at which the uplink queues start at P4-ig
#define HAL_TM_P4_UPLINK_IQ_OFFSET  16
// Offset at which the oqs for rxdma only classes start
#define HAL_TM_RXDMA_OQ_OFFSET      16

#define HAL_TM_P4_SPAN_QUEUE        30
#define HAL_TM_P4_CPU_COPY_QUEUE    31

typedef uint32_t tm_port_t;
typedef int32_t tm_q_t;

#define TM_PORT_TYPES(ENTRY)                                \
    ENTRY(TM_PORT_TYPE_UPLINK,      0, "uplink")            \
    ENTRY(TM_PORT_TYPE_P4,          1, "p4")                \
    ENTRY(TM_PORT_TYPE_DMA,         2, "dma")               \
    ENTRY(NUM_TM_PORT_TYPES,        3, "num-tm-port-types")

DEFINE_ENUM(tm_port_type_e, TM_PORT_TYPES)
#undef TM_PORT_TYPES

static inline uint32_t
tm_get_num_iqs_for_port_type (tm_port_type_e port_type)
{
    switch(port_type) {
        case TM_PORT_TYPE_UPLINK: 
            return 8;
        case TM_PORT_TYPE_P4:
            return 32;
        case TM_PORT_TYPE_DMA:
            return 16;
        default:
            return 0;
    }
    return 0;
}

static inline uint32_t
tm_get_num_oqs_for_port_type (tm_port_type_e port_type)
{
    return tm_get_num_iqs_for_port_type(port_type);
}

static inline bool
tm_q_valid (tm_q_t tm_q)
{
    if (tm_q < 0) {
        return false;
    }
    return true;
}

tm_port_type_e
tm_port_type_get(tm_port_t port);

bool 
tm_port_is_uplink_port(uint32_t port);
bool 
tm_port_is_dma_port(uint32_t port);

static inline uint32_t
capri_tm_buffer_bytes_to_cells (uint32_t bytes)
{
    return (bytes + HAL_TM_CELL_SIZE - 1)/HAL_TM_CELL_SIZE;
}

/* Pool group parameters */
typedef struct tm_pg_params_s {
    uint32_t reserved_min; // Minimum no of bytes reserved for this group
    uint32_t xon_threshold;
    uint32_t headroom;
    uint32_t low_limit;
    uint32_t alpha;
    uint32_t mtu;
} tm_pg_params_t;

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
                          tm_pg_params_t *pg_params);

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

#endif
