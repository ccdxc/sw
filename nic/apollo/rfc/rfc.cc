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
#include "nic/apollo/rfc/rfc_lpm.hpp"
#include "gen/p4gen/apollo_rxdma/include/apollo_rxdma_p4pd.h"
#include "gen/p4gen/apollo_txdma/include/apollo_txdma_p4pd.h"

namespace rfc {

/**
 * @brief    walk the policy rules and build all phase 0 RFC interval tables
 * @param[in]    policy      policy table
 * @param[in]    rfc_trees  pointer to RFC trees
 *               intervals and class ids
 * @param[in]    num_nodes number of nodes in the itables
 * @return    SDK_RET_OK on success, failure status code on error
 */
static inline sdk_ret_t
rfc_build_itables (policy_t *policy, rfc_trees_t *rfc_trees)
{
    rule_t      *rule;
    uint32_t    rule_num = 0;
    itable_t    *addr_itable = &rfc_trees->pfx_tree.itable;
    itable_t    *port_itable = &rfc_trees->port_tree.itable;
    itable_t    *proto_port_itable = &rfc_trees->proto_port_tree.itable;
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

/**
 * @brief    compare two proto-port tree keys and return -1 or 1 based on where
 *           they fit on the number line
 * @param[in]    n1    pointer to 1st node
 * @param[in]    n2    pointer to 2nd node
 * @param[in]    ctxt  context carrying tree_type information
 * @return  -1 or 1 based on the comparison result
 * NOTE: when keys are same, return -1
 */
static int
inode_compare_cb (const void *n1, const void *n2, void *ctxt)
{
    inode_t         *inode1 = (inode_t *)n1;
    inode_t         *inode2 = (inode_t *)n2;
    itree_type_t    tree_type = reinterpret_cast<itree_type_t&>(ctxt);

    if ((tree_type == ITREE_TYPE_IPV4) ||
        (tree_type == ITREE_TYPE_IPV6)) {
        if (IPADDR_GT(&inode1->ipaddr, &inode2->ipaddr)) {
            return 1;
        }
        return -1;
    } else if (tree_type == ITREE_TYPE_PORT) {
        if (inode1->port > inode2->port) {
            return 1;
        }
        return  -1;
    } else {
        if (inode1->key32 > inode2->key32) {
            return 1;
        }
        return -1;
    }
}

static inline sdk_ret_t
rfc_sort_itables (rfc_trees_t *rfc_trees, policy_t *policy)
{
    if (policy->af == IP_AF_IPV4) {
        qsort_r(rfc_trees->pfx_tree.itable.nodes,
                rfc_trees->pfx_tree.itable.num_nodes,
                sizeof(inode_t), inode_compare_cb,
                (void *)ITREE_TYPE_IPV4);
    } else if (policy->af == IP_AF_IPV6) {
        qsort_r(rfc_trees->pfx_tree.itable.nodes,
                rfc_trees->pfx_tree.itable.num_nodes,
                sizeof(inode_t), inode_compare_cb,
                (void *)ITREE_TYPE_IPV6);
    }
    qsort_r(rfc_trees->port_tree.itable.nodes,
            rfc_trees->port_tree.itable.num_nodes,
            sizeof(inode_t), inode_compare_cb,
            (void *)ITREE_TYPE_PORT);
    qsort_r(rfc_trees->proto_port_tree.itable.nodes,
            rfc_trees->proto_port_tree.itable.num_nodes,
            sizeof(inode_t), inode_compare_cb,
            (void *)ITREE_TYPE_PROTO_PORT);
    return SDK_RET_OK;
}

static inline uint16_t
rfc_compute_class_id (policy_t *policy, rfc_tree_t *rfc_tree,
                      rte_bitmap *cbm, uint32_t cbm_size)
{
    uint8_t       *bits;
    uint16_t      class_id;
    rte_bitmap    *cbm_new;

    auto it = rfc_tree->cbm_map.find(cbm);
    if (it != rfc_tree->cbm_map.end()) {
        /**< found the bit combination, return corresponding class id */
        return it->second;
    }
    class_id = rfc_tree->num_classes++;
    SDK_ASSERT(class_id < RFC_MAX_EQ_CLASSES);
    bits = (uint8_t *)calloc(1, RTE_CACHE_LINE_ROUNDUP(cbm_size));
    cbm_new = rte_bitmap_init(policy->max_rules, bits, cbm_size);
    rte_bitmap_or(cbm, cbm_new, cbm_new);
    rfc_tree->cbm_table[class_id] = cbm_new;
    rfc_tree->cbm_map[cbm_new] = class_id;

    return class_id;
}

static inline sdk_ret_t
rfc_compute_classes (policy_t *policy, rfc_trees_t *rfc_trees)
{
    uint32_t    num_intervals;
    itable_t    *addr_itable = &rfc_trees->pfx_tree.itable;
    itable_t    *port_itable = &rfc_trees->port_tree.itable;
    itable_t    *proto_port_itable = &rfc_trees->proto_port_tree.itable;
    inode_t     *inode;

    /**
     * walk over the address interval table, compute class id and class bitmap
     * for each unique interval
     */
    num_intervals = 0;
    rte_bitmap_reset(rfc_trees->cbm);
    for (uint32_t i = 0; i < addr_itable->num_nodes; i++) {
        inode = &addr_itable->nodes[i];
        if (inode->rfc.start) {
            rte_bitmap_set(rfc_trees->cbm, inode->rfc.rule_no);
        } else {
            rte_bitmap_clear(rfc_trees->cbm, inode->rfc.rule_no);
        }

        /**< skip next interval if key is same, note that we updated the bmap */
        if (((i + 1) < addr_itable->num_nodes) &&
            IPADDR_EQ(&inode->ipaddr,
                      &addr_itable->nodes[i+1].ipaddr)) {
            continue;
        }
        inode->rfc.class_id =
            rfc_compute_class_id(policy, &rfc_trees->pfx_tree,
                                 rfc_trees->cbm, rfc_trees->cbm_size);
        addr_itable->nodes[num_intervals++] = *inode;
    }
    rfc_trees->pfx_tree.num_intervals = num_intervals;

    num_intervals = 0;
    rte_bitmap_reset(rfc_trees->cbm);
    for (uint32_t i = 0; i < port_itable->num_nodes; i++) {
        inode = &port_itable->nodes[i];
        if (inode->rfc.start) {
            rte_bitmap_set(rfc_trees->cbm, inode->rfc.rule_no);
        } else {
            rte_bitmap_clear(rfc_trees->cbm, inode->rfc.rule_no);
        }

        /**< skip next interval if key is same, note that we updated the bmap */
        if (((i + 1) < port_itable->num_nodes) &&
            (inode->port == port_itable->nodes[i+1].port)) {
            continue;
        }
        inode->rfc.class_id =
            rfc_compute_class_id(policy, &rfc_trees->port_tree,
                                 rfc_trees->cbm, rfc_trees->cbm_size);
        port_itable->nodes[num_intervals++] = *inode;
    }
    rfc_trees->port_tree.num_intervals = num_intervals;

    num_intervals = 0;
    rte_bitmap_reset(rfc_trees->cbm);
    for (uint32_t i = 0; i < proto_port_itable->num_nodes; i++) {
        inode = &proto_port_itable->nodes[i];
        if (inode->rfc.start) {
            rte_bitmap_set(rfc_trees->cbm, inode->rfc.rule_no);
        } else {
            rte_bitmap_clear(rfc_trees->cbm, inode->rfc.rule_no);
        }

        /**< skip next interval if key is same, note that we updated the bmap */
        if (((i + 1) < proto_port_itable->num_nodes) &&
            (inode->key32 == proto_port_itable->nodes[i+1].key32)) {
            continue;
        }
        inode->rfc.class_id =
            rfc_compute_class_id(policy, &rfc_trees->proto_port_tree,
                                 rfc_trees->cbm, rfc_trees->cbm_size);
        proto_port_itable->nodes[num_intervals++] = *inode;
    }
    rfc_trees->proto_port_tree.num_intervals = num_intervals;

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
    sdk_ret_t      ret;
    rfc_trees_t    rfc_trees = { 0 };

    if (unlikely(policy->num_rules == 0)) {
        return sdk::SDK_RET_INVALID_ARG;
    }

    /**< allocate memory for all the RFC itree tables */
    ret = rfc_trees_init(&rfc_trees, policy);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    /**< build all the interval trees with the given policy */
    rfc_build_itables(policy, &rfc_trees);

    /**< sort intervals in all the trees */
    rfc_sort_itables(&rfc_trees, policy);

    /**< compute equivalence classes for the intervals in the interval trees */
    rfc_compute_classes(policy, &rfc_trees);

    /**< build LPM trees for phase 0 of RFC */
    rfc_build_lpm_trees(policy, &rfc_trees, rfc_tree_root_addr, mem_size);

    /**< build equivalence class index tables for subsequent phases of RFC */
    //rfc_build_eqtables(policy, &rfc_trees);

    rfc_trees_destroy(&rfc_trees);

    return ret;
}

}    // namespace rfc
