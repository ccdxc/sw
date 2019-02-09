/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    rfc.cc
 *
 * @brief   RFC library implementation
 */

#include "nic/apollo/rfc/rte_bitmap_utils.hpp"
#include "nic/apollo/lpm/lpm.hpp"
#include "nic/apollo/rfc/rfc.hpp"
#include "nic/apollo/rfc/rfc_tree.hpp"
#include "nic/apollo/rfc/rfc_utils.hpp"
#include "gen/p4gen/apollo_rxdma/include/apollo_rxdma_p4pd.h"
#include "gen/p4gen/apollo_txdma/include/apollo_txdma_p4pd.h"

namespace rfc {

/**
 * @brief    walk the policy rules and build all phase 0 RFC interval tables
 * @param[in]    policy      policy table
 * @param[in]    rfc_ctxt    pointer to RFC trees
 *               intervals and class ids
 * @param[in]    num_nodes number of nodes in the itables
 * @return    SDK_RET_OK on success, failure status code on error
 */
static inline sdk_ret_t
rfc_build_itables (policy_t *policy, rfc_ctxt_t *rfc_ctxt)
{
    rule_t      *rule;
    uint32_t    rule_num = 0;
    itable_t    *addr_itable = &rfc_ctxt->pfx_tree.itable;
    itable_t    *port_itable = &rfc_ctxt->port_tree.itable;
    itable_t    *proto_port_itable = &rfc_ctxt->proto_port_tree.itable;
    inode_t     *addr_inode, *port_inode, *proto_port_inode;

    /** walk the policy and start building tables */
    SDK_ASSERT(policy->af == IP_AF_IPV4);
    addr_inode = &addr_itable->nodes[0];
    port_inode = &port_itable->nodes[0];
    proto_port_inode = &proto_port_itable->nodes[0];
    for (rule_num = 0; rule_num < policy->num_rules; rule_num++) {
        rule = &policy->rules[rule_num];
        itable_add_address_inodes(rule_num, addr_inode,
                                  &rule->match.l3_match.ip_pfx);
        itable_add_port_inodes(rule_num, port_inode,
                               &rule->match.l4_match.sport_range);
        itable_add_proto_port_inodes(rule_num, proto_port_inode,
                                     rule->match.l3_match.ip_proto,
                                     &rule->match.l4_match.dport_range);
        addr_inode++;
        port_inode++;
        proto_port_inode++;
    }
    addr_itable->num_nodes = port_itable->num_nodes =
        proto_port_itable->num_nodes = rule_num << 1;
    return SDK_RET_OK;
}

static inline sdk_ret_t
rfc_compute_classes (policy_t *policy, rfc_ctxt_t *rfc_ctxt)
{
    uint32_t    num_intervals;
    itable_t    *addr_itable = &rfc_ctxt->pfx_tree.itable;
    itable_t    *port_itable = &rfc_ctxt->port_tree.itable;
    itable_t    *proto_port_itable = &rfc_ctxt->proto_port_tree.itable;
    inode_t     *inode;

    /**
     * walk over the address interval table, compute class id and class bitmap
     * for each unique interval
     */
    num_intervals = 0;
    rte_bitmap_reset(rfc_ctxt->cbm);
    for (uint32_t i = 0; i < addr_itable->num_nodes; i++) {
        inode = &addr_itable->nodes[i];
        if (inode->rfc.start) {
            rte_bitmap_set(rfc_ctxt->cbm, inode->rfc.rule_no);
        } else {
            rte_bitmap_clear(rfc_ctxt->cbm, inode->rfc.rule_no);
        }

        /**< skip next interval if key is same, note that we updated the bmap */
        if (((i + 1) < addr_itable->num_nodes) &&
            IPADDR_EQ(&inode->ipaddr,
                      &addr_itable->nodes[i+1].ipaddr)) {
            continue;
        }
        inode->rfc.class_id =
            rfc_compute_class_id(policy, &rfc_ctxt->pfx_tree.rfc_table,
                                 rfc_ctxt->cbm, rfc_ctxt->cbm_size);
        addr_itable->nodes[num_intervals++] = *inode;
    }
    rfc_ctxt->pfx_tree.num_intervals = num_intervals;

    num_intervals = 0;
    rte_bitmap_reset(rfc_ctxt->cbm);
    for (uint32_t i = 0; i < port_itable->num_nodes; i++) {
        inode = &port_itable->nodes[i];
        if (inode->rfc.start) {
            rte_bitmap_set(rfc_ctxt->cbm, inode->rfc.rule_no);
        } else {
            rte_bitmap_clear(rfc_ctxt->cbm, inode->rfc.rule_no);
        }

        /**< skip next interval if key is same, note that we updated the bmap */
        if (((i + 1) < port_itable->num_nodes) &&
            (inode->port == port_itable->nodes[i+1].port)) {
            continue;
        }
        inode->rfc.class_id =
            rfc_compute_class_id(policy, &rfc_ctxt->port_tree.rfc_table,
                                 rfc_ctxt->cbm, rfc_ctxt->cbm_size);
        port_itable->nodes[num_intervals++] = *inode;
    }
    rfc_ctxt->port_tree.num_intervals = num_intervals;

    num_intervals = 0;
    rte_bitmap_reset(rfc_ctxt->cbm);
    for (uint32_t i = 0; i < proto_port_itable->num_nodes; i++) {
        inode = &proto_port_itable->nodes[i];
        if (inode->rfc.start) {
            rte_bitmap_set(rfc_ctxt->cbm, inode->rfc.rule_no);
        } else {
            rte_bitmap_clear(rfc_ctxt->cbm, inode->rfc.rule_no);
        }

        /**< skip next interval if key is same, note that we updated the bmap */
        if (((i + 1) < proto_port_itable->num_nodes) &&
            (inode->key32 == proto_port_itable->nodes[i+1].key32)) {
            continue;
        }
        inode->rfc.class_id =
            rfc_compute_class_id(policy, &rfc_ctxt->proto_port_tree.rfc_table,
                                 rfc_ctxt->cbm, rfc_ctxt->cbm_size);
        proto_port_itable->nodes[num_intervals++] = *inode;
    }
    rfc_ctxt->proto_port_tree.num_intervals = num_intervals;

    return SDK_RET_OK;
}

/**
 * @brief    build interval tree based RFC LPM trees and index tables for
 *           subsequent RFC phases, starting at the given memory address
 * @param[in] policy           pointer to the policy
 * @param[in] rfc_tree_root    pointer to the memory address at which tree
 *                             should be built
 * @param[in] mem_size         memory block size provided (for error
 *                             detection)
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
rfc_policy_create (policy_t *policy, mem_addr_t rfc_tree_root_addr,
                   uint32_t mem_size)
{
    sdk_ret_t     ret;
    rfc_ctxt_t    rfc_ctxt;

    if (unlikely(policy->num_rules == 0)) {
        return sdk::SDK_RET_INVALID_ARG;
    }

    /**< allocate memory for all the RFC itree tables */
    ret = rfc_ctxt_init(&rfc_ctxt, policy);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    /**< build all the interval trees with the given policy */
    rfc_build_itables(policy, &rfc_ctxt);

    /**< sort intervals in all the trees */
    rfc_sort_itables(&rfc_ctxt, policy);

    /**< compute equivalence classes for the intervals in the interval trees */
    rfc_compute_classes(policy, &rfc_ctxt);

    /**< build LPM trees for phase 0 of RFC */
    rfc_build_lpm_trees(policy, &rfc_ctxt, rfc_tree_root_addr, mem_size);

    /**< build equivalence class index tables for subsequent phases of RFC */
    //rfc_build_eqtables(policy, &rfc_ctxt);

    rfc_ctxt_destroy(&rfc_ctxt);

    return ret;
}

}    // namespace rfc
