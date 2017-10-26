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

/* Pool group parameters */
typedef struct tm_pg_params_s {
    uint32_t reserved_min; // Minimum no of bytes reserved for this group
    uint32_t xon_threshold;
    uint32_t headroom;
    uint32_t low_limit;
    uint32_t alpha;
    uint32_t mtu;
    uint32_t ncos;
    uint32_t cos_map[32];
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
capri_tm_pg_params_update(uint32_t port,
                          uint32_t pg,
                          tm_pg_params_t *pg_params);

typedef enum {
    TM_SCHED_TYPE_NONE = 0,
    TM_SCHED_TYPE_STRICT = 1,
    TM_SCHED_TYPE_DWRR = 2
} tm_sched_type_e;

typedef union tm_sched_config_s {
    struct {
        uint32_t  weight;
    } dwrr;
    struct {
        uint32_t  rate;
    } strict;
} tm_sched_config_t;

typedef struct tm_queue_node_s {
    bool                in_use;
    uint32_t            parent_node;
    tm_sched_type_e     sched_type;
    tm_sched_config_t   u;
} tm_queue_node_t;

typedef struct tm_scheduler_map_s {
    tm_queue_node_t l0_nodes[HAL_TM_COUNT_L0_NODES];
    tm_queue_node_t l1_nodes[HAL_TM_COUNT_L1_NODES];
} tm_scheduler_map_t;

/** capri_tm_scheduler_map_update
 * API to update the output queue scheduler
 *
 * @param port[in]: TM port number
 * @param scheduler_map[in]: The mapping between the nodes of the Hierarchical
 *                           queue scheduler
 * @return hal_ret_t: Status of the operation
 */
hal_ret_t 
capri_tm_scheduler_map_update(uint32_t port,
                              tm_scheduler_map_t *scheduler_map);

/** capri_tm_uplink_lif_set
 * API to program the lif value on an uplink port
 *
 * @param port[in]: TM port number
 * @param lif[in]: The lif value to set
 * @return hal_ret_t: Status of the operation
 */
hal_ret_t 
capri_tm_uplink_lif_set(uint32_t port,
                        uint32_t lif);

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
capri_tm_repl_table_base_addr_set(uint32_t addr);

/** capri_tm_repl_table_num_tokens_set
 * API to program the number of tokens per replication table entry
 *
 * @param num_tokens[in]: Number of tokens per replication table entry
 * @return hal_ret_t: Status of the operation
 */
hal_ret_t
capri_tm_repl_table_num_tokens_set(uint32_t num_tokens);

#endif
