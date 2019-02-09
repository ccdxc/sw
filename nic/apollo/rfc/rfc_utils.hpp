/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    rfc_utils.hpp
 *
 * @brief   RFC library internal helper functions
 */

#if !defined (__RFC_UTILS_HPP__)
#define __RFC_UTILS_HPP__

#include "nic/apollo/rfc/rfc.hpp"
#include "nic/apollo/rfc/rfc_tree.hpp"

namespace rfc {

sdk_ret_t rfc_build_lpm_trees(policy_t *policy, rfc_ctxt_t *rfc_ctxt,
                              mem_addr_t rfc_tree_root_addr, uint32_t mem_size);

/**
 * @brief    given a class bitmap (cbm), check if that exists in the RFC table
 *           already and if not assign new class-id, if it exists already,
 *           use the current class id for that class bitmap
 * @param[in]    policy    user specified policy
 * @param[in]    rfc_table RFC table to add the class id to
 * @param[in]    cbm       class bitmap that needs class id to be computed for
 * @param[in]    cbm_size  class bitmap size
 * @return    SDK_RET_OK on success, failure status code on error
 */
uint16_t rfc_compute_class_id(policy_t *policy, rfc_table_t *rfc_table,
                              rte_bitmap *cbm, uint32_t cbm_size);

/**
 * @brief    sort interval table entries in each of the phase 0
 *           LPM trees
 * @param[in]    rfc_ctxt    RFC context carrying all the intermediate state for
 *                           this policy
 * @param[in]    policy      user specified policy
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t rfc_sort_itables(rfc_ctxt_t *rfc_ctxt, policy_t *policy);

}    // namespace rfc

#endif    /** __RFC_UTILS_HPP__ */
