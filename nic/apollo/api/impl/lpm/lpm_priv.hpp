//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// LPM library internal/private datastructures and APIs shared across pipelines
///
//----------------------------------------------------------------------------

#ifndef __LPM_PRIV_HPP__
#define __LPM_PRIV_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/types.hpp"

/**< invalid nexthop */
#define LPM_NEXTHOP_INVALID            -1
#define LPM_NEXTHOP_SIZE                2    /**< 2 byte nexthop */

#define LPM_MAX_STAGES    8    /**< 8 stages can serve more than 1M v6 routes */

/**< each LPM table is 64 bytes (512 bits/flit) */
#define LPM_TABLE_SIZE         (CACHE_LINE_SIZE)

/**< temporary stack elements maintained while building the interval table */
typedef struct itable_stack_elem_s {
    lpm_inode_t    interval;         /**< interval table node (to be emitted) */
    uint32_t       fallback_nhid;    /**< fallback nexthop */
    uint32_t       priority;         /**< Priority */
} itable_stack_elem_t;

/**
 * each LPM stage consists of a set of LPM index tables and this stage meta
 * contains temporary state we maintain while building the table
 */
typedef struct lpm_stage_info_s {
    mem_addr_t    curr_table_addr;            /**< current table address */
    mem_addr_t    end_marker_addr;            /**< end of current stage address */
    uint32_t      curr_index;                 /**< current entry (key/key+nh) being populated */
    uint8_t       curr_table[LPM_TABLE_SIZE]; /**< current table being populated */
} lpm_stage_info_t;

typedef struct lpm_stage_meta_s {
    itree_type_t        tree_type;       /**< type of the tree being built */
    uint32_t            key_sz;          /**< key size for the tree */
    uint32_t            keys_per_table;  /**< no. of keys per table is same in
                                              all stages except last stage */
    /**< per stage info */
    lpm_stage_info_t    stage_info[LPM_MAX_STAGES];
} lpm_stage_meta_t;

#endif    // __LPM_PRIV_HPP__
