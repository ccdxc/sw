//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// APIs declared in this file need to be implemented by the pipeline in a
/// pipeline specific manner
///
//----------------------------------------------------------------------------

#ifndef __RFC_IMPL_HPP__
#define __RFC_IMPL_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/api/impl/rfc/rfc_tree.hpp"

namespace rfc {

uint16_t sacl_sip_v4_tree_max_classes(void);
uint16_t sacl_dip_v4_tree_max_classes(void);
uint16_t sacl_sip_v6_tree_max_classes(void);
uint16_t sacl_dip_v6_tree_max_classes(void);
uint16_t sacl_sport_tree_max_classes(void);
uint16_t sacl_proto_dport_tree_max_classes(void);
uint16_t sacl_stag_tree_max_classes(void);
uint16_t sacl_dtag_tree_max_classes(void);
uint16_t sacl_p1_max_classes(void);
uint16_t sacl_p2_max_classes(void);

/**
 * @brief    walk the policy rules and build all phase 0 RFC interval tables
 * @param[in]    rfc_ctxt    pointer to RFC trees
 *               intervals and class ids
 * @param[in]    num_nodes number of nodes in the itables
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t rfc_build_itables(rfc_ctxt_t *rfc_ctxt);

/**
 * @brief    sort interval table entries in each of the phase 0
 *           LPM trees
 * @param[in]    rfc_ctxt    RFC context carrying all the intermediate state for
 *                           this policy
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t rfc_sort_itables(rfc_ctxt_t *rfc_ctxt);

/**
 * @brief    compute phase 0 equivalence classes
 * @param[in]    rfc_ctxt    RFC context carrying all the intermediate state for
 *                           this policy
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t rfc_compute_p0_classes(rfc_ctxt_t *rfc_ctxt);

/**
 * @brief    build LPM trees for phase 0 of RFC
 * @param[in]    rfc_ctxt    RFC context carrying all the intermediate state for
 *                           this policy
 * @param[in]    rfc_tree_root_addr memory address where the tree will be placed
 * @param[in]    mem_size size of the memory block reserved for RFC tree
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t rfc_build_lpm_trees(rfc_ctxt_t *rfc_ctxt,
                              mem_addr_t rfc_tree_root_addr, uint32_t mem_size);

/**
 * @brief    given the class bitmap tables of phase0 & phase1, compute class
 *           bitmap table(s) of RFC phase 2, and set the results bits
 * @param[in] rfc_ctxt    RFC context carrying all of the previous phases
 *                        information processed until now
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t rfc_build_eqtables(rfc_ctxt_t *rfc_ctxt);

}    // namespace rfc

#endif    // __RFC_IMPL_HPP__
