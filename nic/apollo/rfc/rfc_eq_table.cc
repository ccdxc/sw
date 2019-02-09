/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    rfc_eq_table.cc
 *
 * @brief   RFC library equivalence table handling
 */

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/rfc/rfc.hpp"
#include "nic/apollo/rfc/rfc_tree.hpp"
#include "nic/apollo/rfc/rfc_utils.hpp"
#include "nic/apollo/rfc/rte_bitmap_utils.hpp"
#include "nic/apollo/p4/include/slacl_defines.h"

namespace rfc {

/**
 * @brief    given the class bitmap tables of phase0, compute class
 *           bitmap tables of RFC phase 1
 * @param[in] policy      user specified policy
 * @param[in] rfc_ctxt    RFC context carrying all of the previous phases
 *                        information processed until now
 */
sdk_ret_t
rfc_compute_p1_eq_class_tables (policy_t *policy, rfc_ctxt_t *rfc_ctxt)
{
    //uint16_t      class_id;
    uint32_t      num_entries;
    rfc_tree_t    *rfc_tree1, *rfc_tree2;
    rte_bitmap    *cbm = rfc_ctxt->cbm;
    
    rfc_tree1 = &rfc_ctxt->pfx_tree;
    rfc_tree2 = &rfc_ctxt->port_tree;
    num_entries = rfc_tree1->rfc_table.num_classes *
                      rfc_tree2->rfc_table.num_classes;
    if (num_entries > SLACL_P1_TABLE_NUM_ENTRIES) {
        OCI_TRACE_ERR("RFC P1 table size %u > expected size %u",
                      num_entries, SLACL_P1_TABLE_NUM_ENTRIES);
        return SDK_RET_ERR;
    }

    /**< do cross product of bitmaps and pick unique bmaps */
    for (uint32_t i = 0; i < rfc_tree1->rfc_table.num_classes; i++) {
        for (uint32_t j = 0; j < rfc_tree2->rfc_table.num_classes; j++) {
            rte_bitmap_and(rfc_tree1->rfc_table.cbm_table[i],
                           rfc_tree2->rfc_table.cbm_table[j], cbm);
            (void)rfc_compute_class_id(policy, &rfc_ctxt->p1_table,
                                 cbm, rfc_ctxt->cbm_size);
        }
    }
    return SDK_RET_OK;
}

}    // namespace rfc
