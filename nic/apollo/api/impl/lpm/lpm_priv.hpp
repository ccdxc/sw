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
#include "nic/sdk/include/sdk/platform.hpp"

/**< invalid nexthop */
#define LPM_NEXTHOP_INVALID            -1
#define LPM_NEXTHOP_SIZE                2    /**< 2 byte nexthop */

#define LPM_MAX_STAGES    8    /**< 8 stages can serve more than 1M v6 routes */

/**< each LPM table is 64 bytes (512 bits/flit) */
#define LPM_TABLE_SIZE         (CACHE_LINE_SIZE)

/**< temporary stack elements maintained while building the interval table */
typedef struct itable_stack_elem_s {
    lpm_inode_t    interval;         /**< interval table node (to be emitted) */
    uint32_t       fallback_data;    /**< fallback data */
    uint32_t       fallback_prio;    /**< fallback priority */
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

/**
 * @brief    compare two given routes and return -1, 0 or 1 based on where they
 *           fit on the number line, breaking the tie by comparing prefix
 *           lengths
 * @param[in]    r1    pointer to 1st route
 * @param[in]    r2    pointer to 2nd route
 * @param[in]    ctxt  opaque context
 * @return  -1, 0, 1 based on the comparison result
 */
int route_compare_cb(const void *route1, const void *route2, void *ctxt);

/**
 * @brief    build an interval table corresponding to the give route table
 * @param[in]    route_table    route table
 * @param[in]   itable         interval node table that will be filled with
 *                             intervals and nexthop ids
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t lpm_build_interval_table(route_table_t *route_table,
                                   lpm_itable_t *itable);

/**
 * @brief    Finalizes the stages by programming the last table of each stage
 *           to the hardware to make sure that the key with all 1's is taken
 *           care of correctly.
 * @param[in] smeta                  meta information for the stages
 * @param[in] nstages                the number of stages
 * @param[in] itable                 the interval table
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t lpm_finalize_stages(lpm_stage_meta_t *smeta, uint32_t nstages,
                              lpm_itable_t *itable);

/**
 * @brief    Flushes the partially filled tabled to HW at the end of processing.
 * @param[in] smeta                  meta information for the stages
 * @param[in] nstages                the number of stages
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t lpm_flush_tables(lpm_stage_meta_t *smeta, uint32_t nstages,
                           lpm_itable_t *itable);

void lpm_promote_route(lpm_inode_t *inode, uint32_t stage,
                       lpm_stage_meta_t *smeta);

#endif    // __LPM_PRIV_HPP__
